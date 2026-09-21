#!/usr/bin/env bash

# Shared observability helpers for installer launchers. Call obs_init after
# sourcing env.sh, then arrange for obs_finish to run on exit.

obs_json_escape() {
  local value=${1-}
  value=${value//\\/\\\\}
  value=${value//\"/\\\"}
  value=${value//$'\n'/\\n}
  value=${value//$'\r'/\\r}
  value=${value//$'\t'/\\t}
  printf '%s' "$value"
}

obs_sha256() {
  if [[ -f "$1" ]]; then sha256sum -- "$1" | cut -d' ' -f1; else printf 'missing'; fi
}

obs_git() {
  nix --extra-experimental-features 'nix-command flakes' shell nixpkgs#git -c git "$@"
}

obs_wine() {
  local wine_bin="$PROJECT_DIR/result/bin/wine"
  if [[ ! -x "$wine_bin" ]]; then wine_bin=$(command -v wine 2>/dev/null || true); fi
  [[ -n "$wine_bin" ]] || return 127
  "$wine_bin" "$@"
}

obs_wineserver() {
  local server_bin="$PROJECT_DIR/result/bin/wineserver"
  if [[ ! -x "$server_bin" ]]; then server_bin=$(command -v wineserver 2>/dev/null || true); fi
  [[ -n "$server_bin" ]] || return 127
  "$server_bin" "$@"
}

obs_set_trace_mode() {
  case "${INVENTOR_TRACE_MODE:-normal}" in
    normal)  export WINEDEBUG='+timestamp,+pid,+tid,err+all' ;;
    service) export WINEDEBUG='+timestamp,+pid,+tid,+service,+process,+loaddll,+seh,+registry,err+all,warn+all' ;;
    full)    export WINEDEBUG='+timestamp,+pid,+tid,trace+all' ;;
    custom)  : ;; # Preserve caller-supplied WINEDEBUG.
    *) printf 'Unknown INVENTOR_TRACE_MODE: %s\n' "$INVENTOR_TRACE_MODE" >&2; return 2 ;;
  esac
}

obs_event() {
  local level=$1 message=$2 now
  now=$(date --iso-8601=seconds)
  printf '{"time":"%s","level":"%s","message":"%s"}\n' \
    "$(obs_json_escape "$now")" "$(obs_json_escape "$level")" \
    "$(obs_json_escape "$message")" >>"$OBS_RUN_DIR/events.jsonl"
}

obs_exit_trap() {
  local exit_code=$?
  obs_finish "$exit_code"
  exit "$exit_code"
}

obs_write_status() {
  local phase=$1 exit_code=${2:-null} tmp now
  now=$(date --iso-8601=seconds)
  tmp="$OBS_RUN_DIR/status.json.tmp"
  printf '{\n  "schema": 1,\n  "run_id": "%s",\n  "operation": "%s",\n  "phase": "%s",\n  "updated_at": "%s",\n  "exit_code": %s\n}\n' \
    "$(obs_json_escape "$OBS_RUN_ID")" "$(obs_json_escape "$OBS_OPERATION")" \
    "$(obs_json_escape "$phase")" "$(obs_json_escape "$now")" "$exit_code" >"$tmp"
  mv -f -- "$tmp" "$OBS_RUN_DIR/status.json"
}

obs_prefix_snapshot() {
  local label=$1
  local out="$OBS_RUN_DIR/prefix-$label"
  mkdir -p "$out"
  {
    printf 'captured_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'prefix=%s\n' "$WINEPREFIX"
    du -sh -- "$WINEPREFIX" 2>/dev/null || true
    find "$WINEPREFIX/drive_c/Program Files/Autodesk" \
      "$WINEPREFIX/drive_c/ProgramData/Autodesk" -maxdepth 3 -type d -print 2>/dev/null | sort || true
  } >"$out/filesystem.txt"
  find "$WINEPREFIX" -type f -printf '%T@\t%s\t%p\n' 2>/dev/null | sort -n >"$out/file-index.tsv" || true

  # Registry queries are intentionally limited to software/package/service
  # metadata. Full registry exports can contain unrelated private user data.
  # shellcheck disable=SC2016 # Expansion is intentionally performed by the nested shell.
  timeout 30s bash -c 'source "$1"; obs_wine reg query "HKLM\\Software\\Autodesk" /s' _ \
    "$PROJECT_DIR/lib/observability.sh" >"$out/autodesk-registry.txt" 2>&1 || true
  # shellcheck disable=SC2016
  timeout 30s bash -c 'source "$1"; obs_wine reg query "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall" /s' _ \
    "$PROJECT_DIR/lib/observability.sh" \
    >"$out/uninstall-registry.txt" 2>&1 || true
  # shellcheck disable=SC2016
  timeout 30s bash -c 'source "$1"; obs_wine reg query "HKLM\\System\\CurrentControlSet\\Services" /s' _ \
    "$PROJECT_DIR/lib/observability.sh" \
    >"$out/services-registry.txt" 2>&1 || true
  # shellcheck disable=SC2016
  timeout 30s bash -c 'source "$1"; obs_wine sc query type= service state= all' _ \
    "$PROJECT_DIR/lib/observability.sh" >"$out/services.txt" 2>&1 || true
}

obs_extract_failures() {
  local output="$OBS_RUN_DIR/failures.txt"
  {
    printf 'Failure summary for %s\nGenerated %s\n\n' "$OBS_RUN_ID" "$(date --iso-8601=seconds)"
    find "$OBS_RUN_DIR" "$WINEPREFIX/drive_c/users/$USER/AppData/Local/Autodesk" \
      "$WINEPREFIX/drive_c/users/$USER/AppData/Local/Temp" \
      -type f \( -iname '*.log' -o -iname '*.txt' \) -newer "$OBS_RUN_DIR/start.marker" \
      -print0 2>/dev/null |
      xargs -0 -r grep -HainE -C 3 \
        'fatal|error|failed|failure|exception|rollback|timed? ?out|stub|unimplemented|cannot|can.t create|HRESULT|return(ed)? (code )?[1-9][0-9]+' \
        2>/dev/null | tail -n 5000 || true
  } >"$output"
}

obs_collect_logs() {
  local source_root="$WINEPREFIX/drive_c" destination="$OBS_RUN_DIR/autodesk-logs" file rel
  mkdir -p "$destination"
  while IFS= read -r -d '' file; do
    rel=${file#"$source_root"/}
    mkdir -p "$destination/$(dirname -- "$rel")"
    cp -p -- "$file" "$destination/$rel"
  done < <(find "$source_root/users/$USER/AppData/Local/Autodesk" \
    "$source_root/users/$USER/AppData/Local/Temp" -type f \
    \( -iname '*.log' -o -iname '*.txt' \) -newer "$OBS_RUN_DIR/start.marker" \
    -size -50M -print0 2>/dev/null)
}

obs_sample() {
  local sample_dir="$OBS_RUN_DIR/samples" stamp tmp
  stamp=$(date +%Y%m%d-%H%M%S)
  mkdir -p "$sample_dir"
  ps -eo pid,ppid,stat,etime,%cpu,%mem,rss,cmd --sort=-%cpu >"$sample_dir/processes-$stamp.txt"
  if [[ ! -s "$OBS_RUN_DIR/progress.tsv" ]]; then
    printf 'time\tload1\tmem_available_kib\tdisk_available_kib\tnewest_log_epoch\tnewest_log\n' \
      >"$OBS_RUN_DIR/progress.tsv"
  fi
  {
    printf '%s\t' "$(date --iso-8601=seconds)"
    awk '{printf "%s\t", $1}' /proc/loadavg
    awk '/MemAvailable:/ {printf "%s\t", $2}' /proc/meminfo
    df -Pk "$PROJECT_DIR" | awk 'NR == 2 {printf "%s\t", $4}'
    find "$OBS_RUN_DIR" "$WINEPREFIX/drive_c/users/$USER/AppData/Local/Autodesk" \
      -type f -printf '%T@\t%p\n' 2>/dev/null | sort -n | tail -1
  } >>"$OBS_RUN_DIR/progress.tsv"
  tmp="$OBS_RUN_DIR/changed-files.txt.tmp"
  find "$WINEPREFIX" -type f -newer "$OBS_RUN_DIR/start.marker" -printf '%TY-%Tm-%TdT%TH:%TM:%TS\t%s\t%p\n' \
    2>/dev/null | sort >"$tmp" || true
  mv -f -- "$tmp" "$OBS_RUN_DIR/changed-files.txt"
  {
    for log in \
      "$WINEPREFIX/drive_c/users/$USER/AppData/Local/Autodesk/ODIS/Install.log" \
      "$WINEPREFIX/drive_c/users/$USER/AppData/Local/Autodesk/ODIS/DDA.log"; do
      [[ -f "$log" ]] || continue
      printf '\n===== %s =====\n' "$log"
      tail -n 120 "$log"
    done
  } >"$OBS_RUN_DIR/latest-installer-progress.txt"
  obs_write_status running
}

obs_monitor_loop() {
  while :; do
    obs_sample
    sleep "${OBS_SAMPLE_SECONDS:-15}"
  done
}

obs_init() {
  OBS_OPERATION=$1
  shift || true
  OBS_RUN_ID="$(date +%Y%m%d-%H%M%S)-$OBS_OPERATION"
  OBS_RUN_DIR="$PROJECT_DIR/logs/runs/$OBS_RUN_ID"
  export OBS_OPERATION OBS_RUN_ID OBS_RUN_DIR
  mkdir -p "$OBS_RUN_DIR/samples"
  touch "$OBS_RUN_DIR/start.marker" "$OBS_RUN_DIR/events.jsonl"
  ln -sfn -- "$OBS_RUN_DIR" "$PROJECT_DIR/logs/latest"

  local git_commit git_dirty wine_path wine_version nix_system
  git_commit=$(obs_git rev-parse HEAD 2>/dev/null || printf unknown)
  git_dirty=$(obs_git status --porcelain 2>/dev/null || true)
  wine_path="$PROJECT_DIR/result/bin/wine"
  [[ -x "$wine_path" ]] || wine_path=$(command -v wine 2>/dev/null || printf unavailable)
  wine_version=$(obs_wine --version 2>/dev/null || printf unavailable)
  nix_system=$(nix eval --raw --impure --expr builtins.currentSystem 2>/dev/null || printf unknown)
  {
    printf 'run_id=%s\noperation=%s\nstarted_at=%s\n' "$OBS_RUN_ID" "$OBS_OPERATION" "$(date --iso-8601=seconds)"
    printf 'project_dir=%s\nprefix=%s\nhost=%s\nuser=%s\n' "$PROJECT_DIR" "$WINEPREFIX" "$(hostname)" "$USER"
    printf 'nix_system=%s\ngit_commit=%s\ngit_dirty=%s\n' "$nix_system" "$git_commit" "${git_dirty//$'\n'/;}"
    printf 'wine_path=%s\nwine_version=%s\n' "$wine_path" "$wine_version"
    printf 'wine_debug=%s\nsetup_date=%s\ncommand=' "${WINEDEBUG-}" "${INVENTOR_SETUP_DATE-}"
    printf '%q ' "$@"
    printf '\nbase_installer_sha256=%s\nbase_archive_sha256=%s\nupdate_installer_sha256=%s\n' \
      "$(obs_sha256 "$INVENTOR_BASE_INSTALLER")" "$(obs_sha256 "$INVENTOR_BASE_ARCHIVE")" \
      "$(obs_sha256 "$INVENTOR_UPDATE_INSTALLER")"
  } >"$OBS_RUN_DIR/manifest.env"
  uname -a >"$OBS_RUN_DIR/uname.txt"
  df -h >"$OBS_RUN_DIR/disk-before.txt"
  obs_write_status initializing
  obs_event info "run initialized"
  obs_prefix_snapshot before
  obs_monitor_loop &
  OBS_MONITOR_PID=$!
  export OBS_MONITOR_PID
  obs_event info "progress monitor started as PID $OBS_MONITOR_PID"
}

obs_finish() {
  local exit_code=${1:-0}
  trap - EXIT INT TERM
  if [[ -n "${OBS_MONITOR_PID:-}" ]]; then
    kill "$OBS_MONITOR_PID" 2>/dev/null || true
    wait "$OBS_MONITOR_PID" 2>/dev/null || true
  fi
  obs_sample || true
  obs_prefix_snapshot after || true
  obs_collect_logs || true
  obs_extract_failures || true
  diff -u "$OBS_RUN_DIR/prefix-before/file-index.tsv" "$OBS_RUN_DIR/prefix-after/file-index.tsv" \
    >"$OBS_RUN_DIR/prefix-file-changes.diff" || true
  df -h >"$OBS_RUN_DIR/disk-after.txt"
  obs_event info "run finished with exit code $exit_code"
  obs_write_status finished "$exit_code"
  printf 'Investigation record: %s\n' "$OBS_RUN_DIR"
  return "$exit_code"
}
