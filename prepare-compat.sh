#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "$0")/env.sh"
cd "$PROJECT_DIR"

[[ -d "$WINEPREFIX/drive_c/windows/system32" ]] || {
  echo "Initialize the Wine prefix before preparing compatibility tools." >&2
  exit 1
}
nix --extra-experimental-features 'nix-command flakes' build .#compat-tools -o result-compat
destination="$WINEPREFIX/drive_c/windows/system32/tar.exe"
if [[ -e "$destination" ]] && ! cmp -s result-compat/libexec/tar.exe "$destination"; then
  backup="$PROJECT_DIR/logs/tar-before-compat-$(date +%Y%m%d-%H%M%S).exe"
  cp -p "$destination" "$backup"
  echo "Preserved existing tar.exe at $backup"
fi
install -m755 result-compat/libexec/tar.exe "$destination"
echo "Installed Wine archive bridge at $destination"
./prepare-msxml.sh
