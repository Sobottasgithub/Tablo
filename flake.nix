{
  description = "Tablo";

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

      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "tablo-full";
        version = "1.0";
        src = ./.;

        buildInputs = packages;

        configurePhase = ''
          cmake -B build -S $src -DCMAKE_BUILD_TYPE=Release
        '';

        buildPhase = ''
          cmake --build build
        '';

        installPhase = ''
          cmake --install build --prefix=$out
          cp LICENSE $out/
        '';
      };

      devShells.${system}.default = pkgs.mkShell {
        inherit packages;

        inputsFrom = [ self.packages.${system}.default ];

        shellHook = ''
          git status
        '';
      };

    };
}
