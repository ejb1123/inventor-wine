#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
[[ -d "$WINEPREFIX/drive_c/windows/system32" ]] || {
  echo "Initialize the Wine prefix first." >&2
  exit 1
}

# Same Microsoft redistributable and SHA-256 as Winetricks 20260125's msxml6.
# Keep Microsoft's binaries in ignored local storage, never in the repository.
nix --extra-experimental-features 'nix-command flakes' develop --command bash <<'SCRIPT'
set -euo pipefail
cache="$PROJECT_DIR/fixtures/msxml6"
mkdir -p "$cache"
archive="$cache/msxml6-KB2957482-enu-amd64.exe"
digest=260cd870851ffc3c6d10b71691f134e20d8d03ac26073bb36951eacb7aa85897
if [[ ! -f "$archive" ]]; then
  curl --fail --location --output "$archive.tmp" \
    https://download.microsoft.com/download/2/7/7/277681BE-4048-4A58-ABBA-259C465B1699/msxml6-KB2957482-enu-amd64.exe
  mv "$archive.tmp" "$archive"
fi
printf '%s  %s\n' "$digest" "$archive" | sha256sum --check
cabextract -q -d "$cache" "$archive"
cabextract -q -d "$cache" "$cache/msxml6.msi"
backup="$PROJECT_DIR/logs/msxml-before-compat-$(date +%Y%m%d-%H%M%S)"
for arch in system32 syswow64; do
  suffix=1ECC0691_D2EB_4A33_9CBF_5487E5CB17DB
  [[ "$arch" != syswow64 ]] || suffix=86F857F6_A743_463D_B2FE_98CB5F727E09
  for dll in msxml6 msxml6r; do
    source_file="$cache/$dll.dll.$suffix"
    destination="$WINEPREFIX/drive_c/windows/$arch/$dll.dll"
    if [[ -e "$destination" ]] && ! cmp -s "$source_file" "$destination"; then
      mkdir -p "$backup/$arch"
      cp -p "$destination" "$backup/$arch/$dll.dll"
    fi
    install -m644 "$source_file" "$destination"
  done
done
echo "Installed Microsoft XML 6 runtime; setup launchers select it with msxml6=n,b."
SCRIPT
