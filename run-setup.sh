#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

# Extraction names vary between machines and repeated bootstrap runs.
shopt -s nullglob
setup_candidates=("$WINEPREFIX"/drive_c/Autodesk/WI/Inventor_Professional_2027_English_Win_64bit_db_002_002*/image/Setup.exe)
shopt -u nullglob
if (( ${#setup_candidates[@]} != 1 )); then
  printf 'Expected one extracted Inventor Setup.exe; found %s.\n' "${#setup_candidates[@]}" >&2
  if (( ${#setup_candidates[@]} )); then
    printf '  %s\n' "${setup_candidates[@]}" >&2
  else
    printf 'Run ./run-installer.sh to extract the installation media first.\n' >&2
  fi
  exit 1
fi
export setup="${setup_candidates[0]}"
setup_relative="${setup#"$WINEPREFIX/drive_c/"}"
setup_windows="C:\\${setup_relative//\//\\}"
setup_windows_dir="${setup_windows%\\*}"
export WINEPATH="$setup_windows_dir\\ODIS\\odis.bs.win;$setup_windows_dir\\ODIS\\odis.bs.wx"
# Wine's Wayland driver fails to initialize the installer desktop on the current
# NVIDIA host. Use X11/XWayland for setup; retain caller-supplied DLL overrides.
# Native MSXML6 avoids minutes of XML parsing for each large ADIX package.
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:+$WINEDLLOVERRIDES;}winewayland.drv=d;msxml6=n,b"
# The Autodesk installer writes detailed ODIS logs of its own. Wine output is
# restricted to errors so those logs remain the primary diagnostic source.
obs_set_trace_mode

# Install material libraries separately with prepare-materials.sh first.
# July 2026 covers Core and RSA/REX while Wine's timestamp support is incomplete.
# The host clock and signed packages are unchanged.
export INVENTOR_SETUP_DATE="${INVENTOR_SETUP_DATE:-2026-07-01 12:00:00}"
# Only certificate validation needs an adjusted wall clock; preserve monotonic
# timers used by Wine and the host graphics libraries.
export FAKETIME_DONT_FAKE_MONOTONIC=1
# Kernel ntsync absolute deadlines use the host clock, not libfaketime's clock.
# The companion Wine server supports falling back to server-managed waits.
export WINE_DISABLE_NTSYNC=1
obs_init setup "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
log="$OBS_RUN_DIR/wine.log"
echo "Setup log: $log"
# The before-state registry capture may start a real-time wineserver. Stop it
# before launching both the server and installer beneath libfaketime.
obs_wineserver -k 2>/dev/null || true
obs_wineserver -w
# Start a fresh wineserver inside the same faketime environment. Reusing a
# real-time wineserver makes Electron's UI process exit before creating a window.
# Apply the library directly: the faketime CLI waits for persistent descendants
# even after setup exits. Compute one wall-clock offset before loading it, and
# let the server exit three seconds after its last client finishes.
# shellcheck disable=SC2016 # Variables are expanded by the Nix-shell command.
nix --extra-experimental-features 'nix-command flakes' develop --command \
  bash -c '
    target_epoch=$(date -d "$INVENTOR_SETUP_DATE" +%s) || exit
    now_epoch=$(date +%s) || exit
    printf -v FAKETIME "%+d" "$((target_epoch - now_epoch))"
    export FAKETIME
    export LD_PRELOAD="${INVENTOR_FAKETIME_LIBRARY:?missing libfaketime path}${LD_PRELOAD:+:$LD_PRELOAD}"
    "${WINESERVER:-wineserver}" -p3 >> "$OBS_RUN_DIR/wineserver.log" 2>&1 || exit
    exec wine "$setup"
  ' \
  > "$log" 2>&1
