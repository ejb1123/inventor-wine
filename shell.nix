{ pkgs ? import <nixpkgs> { } }:
let
  customWine = import ./custom-wine.nix { inherit pkgs; };
  customWineServer = import ./custom-wineserver.nix { inherit pkgs; };
in
pkgs.mkShell {
  shellHook = ''
    export WINESERVER=${customWineServer}/bin/wineserver
    export INVENTOR_FAKETIME_LIBRARY=${pkgs.libfaketime}/lib/libfaketime.so.1
  '';
  packages = with pkgs; [
    customWine
    gcc
    winetricks
    p7zip
    cabextract
    file
    binutils
    libfaketime
    python3
    curl
    vulkan-tools
  ];
}
