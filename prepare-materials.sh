#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

shopt -s nullglob
images=("$WINEPREFIX"/drive_c/Autodesk/WI/Inventor_Professional_2027_English_Win_64bit_db_002_002*/image)
shopt -u nullglob
if (( ${#images[@]} != 1 )); then
  echo "Expected exactly one extracted Inventor image directory." >&2
  exit 1
fi
installer="$WINEPREFIX/drive_c/Program Files/Autodesk/AdODIS/V1/Installer.exe"
[[ -f "$installer" ]] || { echo "Autodesk ODIS must be installed first." >&2; exit 1; }
export installer
export materials_image="${images[0]}"
relative="${materials_image#"$WINEPREFIX/drive_c/"}"
export materials_image_windows="C:\\${relative//\//\\}"
export materials_destination="$WINEPREFIX/drive_c/inventor-wine-material-preparation"
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:+$WINEDLLOVERRIDES;}winewayland.drv=d;msxml6=n,b"
export INVENTOR_SETUP_DATE='2025-09-01 12:00:00'
export WINE_DISABLE_NTSYNC=1 FAKETIME_DONT_FAKE_MONOTONIC=1
obs_set_trace_mode
obs_init prepare-materials "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
obs_wineserver -k 2>/dev/null || true
obs_wineserver -w

# shellcheck disable=SC2016 # Expansion belongs to the Nix-shell subprocess.
nix --extra-experimental-features 'nix-command flakes' develop --command bash -c '
  set -e
  python3 lib/make-materials-manifest.py "$materials_image" "$materials_destination"
  target_epoch=$(date -d "$INVENTOR_SETUP_DATE" +%s)
  now_epoch=$(date +%s)
  printf -v FAKETIME "%+d" "$((target_epoch - now_epoch))"
  export FAKETIME
  export LD_PRELOAD="${INVENTOR_FAKETIME_LIBRARY:?}${LD_PRELOAD:+:$LD_PRELOAD}"
  "$WINESERVER" -p3 >> "$OBS_RUN_DIR/wineserver.log" 2>&1
  exec wine "$installer" --install_mode install \
    --manifest "C:\inventor-wine-material-preparation\setup.xml" \
    --mount_point "$materials_image_windows" --install_source "$materials_image_windows" \
    --silent --offline_mode
' > "$OBS_RUN_DIR/wine.log" 2>&1
