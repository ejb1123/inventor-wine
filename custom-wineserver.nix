{ pkgs ? import <nixpkgs> { } }:

# Reuse precisely the source, staging patches, and registry protocol of our
# existing Wine build, but rebuild only its server for the clock and service
# reporting workarounds. Neither workaround changes the server protocol.
let
  wine = import ./custom-wine.nix { inherit pkgs; };
in wine.overrideAttrs (old: {
  pname = "wineserver-inventor";
  patches = old.patches ++ [
    ./patches/0002-allow-disabling-ntsync-for-faketime.patch
    ./patches/0003-service-session-reporting.patch
  ];
  buildPhase = ''
    runHook preBuild
    make -j"$NIX_BUILD_CORES" server/wineserver
    runHook postBuild
  '';
  installPhase = ''
    mkdir -p "$out/bin" "$out/share"
    install -m755 server/wineserver "$out/bin/wineserver"
    ln -s ${wine}/share/wine "$out/share/wine"
  '';
  postInstall = "";
})
