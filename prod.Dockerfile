FROM nixos/nix:latest
LABEL authors="Ture Bentzin <ture.bentzin@alumni.fh-aachen.de>"

# Install dependencies
RUN  nix-env -iA nixpkgs.git \
    && nix-env -iA nixpkgs.cacert

# Build Tablo-full as base layer
RUN nix --extra-experimental-features "nix-command flakes" build github:TureBentzin/Tablo --out-link tablo-full

