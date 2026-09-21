#!/usr/bin/env bash
set -euo pipefail

export WINEPREFIX="/home/ej/.local/share/wineprefixes/inventor-2027"
export WINEARCH="win64"
export WINEDEBUG="${WINEDEBUG:--all}"

export INVENTOR_MEDIA_DIR="/home/ej/Downloads"
export INVENTOR_BASE_INSTALLER="$INVENTOR_MEDIA_DIR/Inventor_Professional_2027_English_Win_64bit_db_001_002.exe"
export INVENTOR_BASE_ARCHIVE="$INVENTOR_MEDIA_DIR/Inventor_Professional_2027_English_Win_64bit_db_002_002.7z"
export INVENTOR_UPDATE_INSTALLER="$INVENTOR_MEDIA_DIR/Inventor_2027.1_Update.exe"

mkdir -p "$WINEPREFIX" "/home/ej/Projects/inventor-wine/logs"
