#!/usr/bin/env bash

# WARNING: This is not a nix generated script. Its just a simplification for running manual nix builds!

echo "building tablo-node image"
nix build -v .#tablo-node-docker && docker load < result

echo "building tablo-master image"
nix build -v .#tablo-master-docker && docker load < result

echo "building tablo-client image"
nix build -v .#tablo-client-docker && docker load < result

rm result
