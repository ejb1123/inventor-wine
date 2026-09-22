{ pkgs ? import <nixpkgs> { } }:
let
  wine = import ./custom-wine.nix { inherit pkgs; };
in wine.overrideAttrs (old: {
  pname = "winex11-inventor-shape-reset";
  patches = old.patches ++ [ ./patches/0005-sync-x11-shape-on-first-flush.patch ];
  buildPhase = ''
    runHook preBuild
    make -j"$NIX_BUILD_CORES" dlls/winex11.drv/winex11.so
    runHook postBuild
  '';
  installPhase = ''
    mkdir -p "$out/lib/wine/x86_64-unix"
    install -m755 dlls/winex11.drv/winex11.so "$out/lib/wine/x86_64-unix/winex11.so"
  '';
  postInstall = "";
})
