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

      version = "1.2";

      packages = with pkgs; [
        cmake
        gcc
        gnumake
      ];
    in
    {

      packages.${system} =
        let
          tablo-full = pkgs.stdenv.mkDerivation {
            pname = "tablo-full";
            inherit version;
            src = ./.;

            meta = {
              description = "Full tablo package";
              mainProgram = "tablo-node";
            };

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

          mkTabloDocker =
            name: extraConfig:
            pkgs.dockerTools.buildImage {
              inherit name;
              tag = version;
              config = {
                Cmd = [ "${tablo-full}/bin/${name}" ];
              } // extraConfig;
            };
        in
        {
          inherit tablo-full;
          default = tablo-full;
          tablo-node-docker = mkTabloDocker "tablo-node" {};
          tablo-master-docker = mkTabloDocker "tablo-master" {};

          # used for client containers not deployed via swarm
          tablo-client-docker = mkTabloDocker "tablo-client" {};
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
