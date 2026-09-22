#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh
inventor="$WINEPREFIX/drive_c/Program Files/Autodesk/Inventor 2027/Bin/Inventor.exe"
[[ -f "$inventor" ]] || { echo "Inventor is not installed in $WINEPREFIX" >&2; exit 1; }
export inventor
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:+$WINEDLLOVERRIDES;}winewayland.drv=d;msxml6=n,b"
obs_set_trace_mode
# Application launches use the real date. Finish setup before launching.
# Start the companion server explicitly; Wine can otherwise select its own.
obs_wineserver -p3
obs_init inventor "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
echo "Inventor launch log: $OBS_RUN_DIR/wine.log"
# shellcheck disable=SC2016
nix --extra-experimental-features 'nix-command flakes' develop --command bash -c '
  "$WINESERVER" -p3
  exec wine "$inventor" /language=ENU "$@"
' _ "$@" > "$OBS_RUN_DIR/wine.log" 2>&1
