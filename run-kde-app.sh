#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
mkdir -p logs/kde
launch_log="$PWD/logs/kde/$(date +%Y%m%d-%H%M%S-%N).log"
status=0
./run-research-contained.sh "$@" > "$launch_log" 2>&1 || status=$?
if (( status != 0 )); then
  if (( status == 73 )); then
    message='A research app or appearance preview is already open. Close it before starting another Autodesk app.'
  else
    message="Launch ended with status $status. Details: $launch_log"
  fi
  printf '%s\n' "$message" >> "$launch_log"
  if [[ -x fixtures/desktop-tools/notify-send ]]; then
    fixtures/desktop-tools/notify-send -a 'Inventor Wine' -i dialog-error \
      'Autodesk launcher' "$message" || true
  else
    qdbus org.kde.plasmashell /org/kde/osdService org.kde.osdService.infoMessage \
      dialog-error "$message" >/dev/null 2>&1 || true
  fi
fi
exit "$status"
