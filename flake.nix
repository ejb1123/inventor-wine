{
  description = "Custom Wine-staging build for Autodesk Inventor 2027";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/a9e6d84f9c2f";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      customWine = import ./custom-wine.nix { inherit pkgs; };
    in {
      packages.${system} = {
        default = customWine;
        wine = customWine;
      };

      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          customWine
          gcc
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
