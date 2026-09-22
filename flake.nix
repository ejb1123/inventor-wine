{
  description = "Custom Wine-staging build for Autodesk Inventor 2027";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/a9e6d84f9c2f";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      customWine = import ./custom-wine.nix { inherit pkgs; };
      customWineServer = import ./custom-wineserver.nix { inherit pkgs; };
    in {
      packages.${system} = {
        default = customWine;
        wine = customWine;
        wineserver = customWineServer;
        winex11 = import ./custom-winex11.nix { inherit pkgs; };
        compat-tools = import ./compat-tools.nix { inherit pkgs; };
      };

      devShells.${system}.default = pkgs.mkShell {
        shellHook = ''
          export WINESERVER=${customWineServer}/bin/wineserver
          export INVENTOR_FAKETIME_LIBRARY=${pkgs.libfaketime}/lib/libfaketime.so.1
        '';
        packages = with pkgs; [
          customWine
          gcc
          python3
          curl
          winetricks
          p7zip
          cabextract
          file
          binutils
          libfaketime
          strace
          shellcheck
          jq
          pciutils
          vulkan-tools
        ];
      };
    };
}
