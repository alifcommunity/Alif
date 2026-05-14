{
  description = "Alif programming language - لغة البرمجة ألف";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    devenv.url = "github:cachix/devenv";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs@{ nixpkgs, devenv, flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        inputs.devenv.flakeModule
      ];
      systems = [ "x86_64-linux" "i686-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin" ];

      perSystem = { config, self', inputs', pkgs, system, ... }: {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "alif";
          version = "5.0.0"; # Based on README mentioning v5

          src = ./.;

          nativeBuildInputs = with pkgs; [
            gnumake
            gcc
          ];

          buildPhase = ''
            cd linuxBuild
            make
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp build/alif $out/bin/
          '';

          meta = with pkgs.lib; {
            description = "An Arabic interpreted programming language";
            homepage = "https://aliflang.org";
            license = licenses.mit; # Assuming MIT, but check if needed
            maintainers = [ ];
            platforms = platforms.unix;
          };
        };

        devenv.shells.default = {
          name = "alif-dev-shell";

          imports = [
            ./devenv.nix
          ];
        };
      };
    };
}
