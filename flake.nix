{
  description = "Tablo";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

    ttp2 = {
      url = "github:Sobottasgithub/ttp2";
    };

    tud = {
      url = "github:Sobottasgithub/tud";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      ttp2,
      tud,
    }:
    let
      system = "x86_64-linux";

      pkgs = import nixpkgs {
        inherit system;
      };

      version = "1.3";

      libttp2 = ttp2.packages.${system}.lib;
      libtud = tud.packages.${system}.lib;

      packages = with pkgs; [
        cmake
        gcc
        gnumake
        protobuf
        libttp2
        libtud
      ];
    in
    {
      packages.${system} =
        let
          mkTabloPackage =
            {
              pname,
              buildTarget ? pname,

              enableLibtabcrypt ? false,
              enableLibtabnet ? false,

              enableNode ? false,
              enableClient ? false,
              enableMaster ? false,

              extraInputs ? [ ],
            }:
            pkgs.stdenv.mkDerivation {
              inherit pname version;

              src = ./.;

              meta = {
                description = "${pname} package";
                mainProgram = pname;
              };

              buildInputs = packages ++ extraInputs;

              configurePhase = ''
                cmake -B build -S $src \
                  -DCMAKE_BUILD_TYPE=Release \
                  -DDEF_LIBTABCRYPT=${if enableLibtabcrypt then "ON" else "OFF"} \
                  -DDEF_LIBTABNET=${if enableLibtabnet then "ON" else "OFF"} \
                  -DDEF_NODE=${if enableNode then "ON" else "OFF"} \
                  -DDEF_CLIENT=${if enableClient then "ON" else "OFF"} \
                  -DDEF_MASTER=${if enableMaster then "ON" else "OFF"}
              '';

              buildPhase = ''
                cmake --build build \
                  --target ${buildTarget} \
                  -j$NIX_BUILD_CORES
              '';

              installPhase = ''
                cmake --install build --prefix=$out
                cp LICENSE $out/
              '';
            };

          libtabcrypt = mkTabloPackage {
            pname = "libtabcrypt";
            buildTarget = "tabcrypt";

            enableLibtabcrypt = true;
          };

          libtabnet = mkTabloPackage {
            pname = "libtabnet";
            buildTarget = "tabnet";

            enableLibtabnet = true;

            extraInputs = [
              libtabcrypt
              # include libttp2 just here for later setup!
            ];
          };

          tablo-node = mkTabloPackage {
            pname = "tablo-node";

            enableNode = true;

            extraInputs = [
              libtabcrypt
              libtabnet
            ];
          };

          tablo-client = mkTabloPackage {
            pname = "tablo-client";

            enableClient = true;
            enableLibtabcrypt = true;
            enableLibtabnet = true;

            extraInputs = [
              libtabcrypt
              libtabnet
            ];
          };

          tablo-master = mkTabloPackage {
            pname = "tablo-master";

            enableMaster = true;

            extraInputs = [
              libtabcrypt
              libtabnet
            ];
          };

          tablo-full = mkTabloPackage {
            pname = "tablo-full";
            buildTarget = "all";

            enableLibtabcrypt = true;
            enableLibtabnet = true;

            enableNode = true;
            enableClient = true;
            enableMaster = true;

            extraInputs = [
              libtabcrypt
              libtabnet
            ];
          };

          tablo = pkgs.symlinkJoin {
            name = "tablo-${version}";

            paths = [
              libtabcrypt
              libtabnet
              tablo-node
              tablo-client
              tablo-master
            ];
          };

          mkTabloDocker =
            package: binary:
            pkgs.dockerTools.buildImage {
              name = binary;
              tag = version;

              config = {
                Cmd = [ "${package}/bin/${binary}" ];
              };
            };
        in
        {
          inherit
            tablo
            tablo-node
            tablo-client
            tablo-master
            tablo-full
            libtabcrypt
            libtabnet
            libttp2
            libtud
            ;

          default = tablo;

          tablo-node-docker = mkTabloDocker tablo-node "tablo-node";

          tablo-master-docker = mkTabloDocker tablo-master "tablo-master";

          tablo-client-docker = mkTabloDocker tablo-client "tablo-client";
        };

      devShells.${system}.default =
        let
          devPackages = packages ++ [
            pkgs.bridge-utils
            pkgs.clang-tools
            pkgs.protobuf
          ];
        in
        pkgs.mkShell {
          packages = devPackages;

          inputsFrom = [
            self.packages.${system}.default
          ];

          shellHook = ''
            git status
            cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          '';
        };
    };
}
