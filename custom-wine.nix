{ pkgs ? import <nixpkgs> { } }:

pkgs.wineWow64Packages.stagingFull.overrideAttrs (old: {
  pname = "wine-wow64-staging-inventor";
  patches = (old.patches or [ ]) ++ [
    ./patches/0001-implement-RegLoadAppKey-and-binary-hives.patch
  ];
})
