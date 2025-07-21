{
  description = "libtabcrypt";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      packages = with pkgs; [
        cmake
        gcc
        gnumake
      ];
    in
    {

      packages.${system}.libtabcrypt = pkgs.stdenv.mkDerivation {
        pname = "libtabcrypt";
        version = "1.0";
        src = ./.;

        buildInputs = packages;
        buildPhase = ''
          cmake -B build -S .
        '';

        installPhase = ''
          mkdir -p $out/lib
          mkdir -p $out/include
          cp -r $src/include $out/include
          # TODO copy cmake outputs
        '';
      };

      devShells.${system}.default = pkgs.mkShell {
        packages = [ pkgs.gnumake ];

        inputsFrom = packages;

        shellHook = ''
          git status
        '';
      };

    };
}
