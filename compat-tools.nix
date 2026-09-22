{ pkgs ? import <nixpkgs> { } }:
let
  wine = import ./custom-wine.nix { inherit pkgs; };
in pkgs.runCommand "inventor-wine-compat-tools" {
  nativeBuildInputs = [ wine pkgs.stdenv.cc ];
} ''
  mkdir -p "$out/libexec"
  winegcc -m64 -Wall -Wextra -Werror \
    '-DBSDTAR_PATH="${pkgs.libarchive}/bin/bsdtar"' \
    -o "$out/libexec/tar.exe" ${./compat/tar-bridge.c}
  # Wine's CreateProcess needs the Winelib ELF directly, not winegcc's shell
  # launcher. The latter can appear to finish before extraction has occurred.
  mv -f "$out/libexec/tar.exe.so" "$out/libexec/tar.exe"
''
