#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
export PROJECT_DIR
export WINEPREFIX="${XDG_DATA_HOME:-$HOME/.local/share}/wineprefixes/inventor-2027"
export WINEARCH="win64"
export WINEDEBUG="${WINEDEBUG:--all}"
# Go-based Autodesk services use the service manager's reported session ID
# to decide whether to enter their Windows service dispatcher.
export WINE_SERVICE_SESSION_ZERO="${WINE_SERVICE_SESSION_ZERO:-1}"
if [[ -z "${WINESERVER:-}" && -x "$PROJECT_DIR/result-server/bin/wineserver" ]]; then
  export WINESERVER="$PROJECT_DIR/result-server/bin/wineserver"
fi

export INVENTOR_MEDIA_DIR="${INVENTOR_MEDIA_DIR:-$PROJECT_DIR/installers}"
export INVENTOR_BASE_INSTALLER="$INVENTOR_MEDIA_DIR/Inventor_Professional_2027_English_Win_64bit_db_001_002.exe"
export INVENTOR_BASE_ARCHIVE="$INVENTOR_MEDIA_DIR/Inventor_Professional_2027_English_Win_64bit_db_002_002.7z"
export INVENTOR_UPDATE_INSTALLER="$INVENTOR_MEDIA_DIR/Inventor_2027.1_Update.exe"

mkdir -p "$WINEPREFIX" "$PROJECT_DIR/logs"
