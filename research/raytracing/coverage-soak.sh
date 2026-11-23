#!/usr/bin/env bash
# Run only against an isolated instance with coverage.exe and layout helpers installed.
set -euo pipefail
research_pid=${1:?host Inventor PID required}
research_cycles=${2:-18}
research_output=${3:?output directory required}
research_wine=${WINE_BINARY:?absolute Wine binary path inside the target namespace required}
[[ $research_pid =~ ^[0-9]+$ && $research_cycles =~ ^[0-9]+$ ]] || exit 2
(( research_cycles > 0 && research_cycles <= 60 )) || exit 2
mkdir -p "$research_output"
research_uid=$(id -u)
research_gid=$(id -g)
research_call() {
  sudo -n nsenter -t "$research_pid" -a -S "$research_uid" -G "$research_gid" -e -w \
    "$research_wine" "$@"
}
for ((research_cycle=1; research_cycle<=research_cycles; research_cycle++)); do
  research_log="$research_output/cycle-$research_cycle.log"
  date -Iseconds > "$research_log"
  research_call 'C:\ResearchUI\coverage.exe' camera "$((10759 + research_cycle % 4))" >> "$research_log" 2>&1
  if ((research_cycle % 3 == 0)); then
    for research_action in layout-restore layout-maximize; do
      research_call 'C:\ResearchUI\'"$research_action"'.exe' >> "$research_log" 2>&1
      sleep 1
    done
  fi
  sleep 8
  ps -p "$research_pid" -o pid=,rss=,etime= >> "$research_log"
  if command -v nvidia-smi >/dev/null; then
    nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader >> "$research_log"
  fi
done
