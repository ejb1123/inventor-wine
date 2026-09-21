#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

export setup="$WINEPREFIX/drive_c/Autodesk/WI/Inventor_Professional_2027_English_Win_64bit_db_002_002 (1)/image/Setup.exe"
export WINEPATH='C:\Autodesk\WI\Inventor_Professional_2027_English_Win_64bit_db_002_002 (1)\image\ODIS\odis.bs.win;C:\Autodesk\WI\Inventor_Professional_2027_English_Win_64bit_db_002_002 (1)\image\ODIS\odis.bs.wx'
# The Autodesk installer writes detailed ODIS logs of its own. Wine output is
# restricted to errors so those logs remain the primary diagnostic source.
obs_set_trace_mode

# Autodesk's 2027 ADIX packages were signed with a certificate that expired on
# 2026-08-14. Windows honors their Authenticode timestamp, but Wine currently
# fails to decode that timestamp and MSIX Core returns 0x8BAD0042. Keeping the
# process-local clock before expiry lets MSIX Core validate the same unmodified
# packages. This does not alter the host clock.
export INVENTOR_SETUP_DATE="${INVENTOR_SETUP_DATE:-2026-07-01 12:00:00}"
obs_init setup "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
log="$OBS_RUN_DIR/wine.log"
# The before-state registry capture may start a real-time wineserver. Stop it
# before launching both the server and installer beneath libfaketime.
obs_wineserver -k 2>/dev/null || true
# Start a fresh wineserver inside the same faketime environment. Reusing a
# real-time wineserver makes Electron's UI process exit before creating a window.
# shellcheck disable=SC2016 # setup is exported for the nested fake-time shell.
nix --extra-experimental-features 'nix-command flakes' develop --command \
  faketime "$INVENTOR_SETUP_DATE" bash -c 'wineserver -p; exec wine "$setup"' \
  2>&1 | tee "$log"
