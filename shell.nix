{ pkgs ? import <nixpkgs> { } }:
let
  customWine = import ./custom-wine.nix { inherit pkgs; };
in
pkgs.mkShell {
  packages = with pkgs; [
    customWine
    gcc
    winetricks
    p7zip
    cabextract
    file
    binutils
    libfaketime
    vulkan-tools
  ];
}
