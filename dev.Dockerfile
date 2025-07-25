FROM nixos/nix:latest
LABEL authors="Ture Bentzin <ture.bentzin@alumni.fh-aachen.de>"

# Install dependencies
RUN nix-env -iA nixpkgs.dos2unix \
    && nix-env -iA nixpkgs.cacert \
    && nix-env -iA nixpkgs.bashInteractive

# Copy source code
COPY . /tablo

# Fix line endings when using Windows
RUN find ./tablo -type f -exec dos2unix {} +

# Build Tablo-full as base layer
RUN nix --extra-experimental-features "nix-command flakes" build tablo/ --out-link tablo-full

EXPOSE 8080
