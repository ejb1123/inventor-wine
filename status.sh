#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh

run_dir=${1:-}
if [[ -z "$run_dir" && -L "$PROJECT_DIR/logs/latest" ]]; then
  run_dir=$(readlink -f -- "$PROJECT_DIR/logs/latest")
fi
if [[ -z "$run_dir" || ! -d "$run_dir" ]]; then
  printf 'No recorded run found.\n' >&2
  exit 1
fi

printf 'Run: %s\n\n' "$run_dir"
if [[ -f "$run_dir/status.json" ]]; then cat "$run_dir/status.json"; fi
printf '\nRecent progress samples:\n'
tail -n 8 "$run_dir/progress.tsv" 2>/dev/null || true
printf '\nActive Autodesk/Wine processes:\n'
# shellcheck disable=SC2009 # We need a human-readable process snapshot, not only PIDs.
ps -eo pid,ppid,stat,etime,%cpu,%mem,cmd | \
  grep -Ei 'wine|wineserver|autodesk|install_manager|setup\.exe|msiexec|cer_service' | \
  grep -vE 'grep -E|status\.sh' || true
printf '\nLatest likely failures:\n'
tail -n 40 "$run_dir/failures.txt" 2>/dev/null || \
  grep -Eai 'fatal|error|failed|exception|rollback|timeout|cannot' \
    "$run_dir/wine.log" 2>/dev/null | tail -n 40 || true
