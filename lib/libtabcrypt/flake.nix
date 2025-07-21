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

      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "libtabcrypt";
        version = "1.0";
        src = ./.;

        buildInputs = packages;

        configurePhase = ''
          cmake -B build -S $src
        '';

        buildPhase = ''
          cd build
          make
        '';

        installPhase = ''
          mkdir -p $out/lib
          cp -r $src/include $out/
          cp libtabcrypt.* $out/lib/
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
