#!/usr/bin/env bash
set -euo pipefail

# Usage: ./build_and_deploy.sh
# Builds nix images, loads into docker, optionally tags as :latest and updates nix.compose.yml

# Names of attributes / resulting image repo-name fragments we expect from nix build
ATTRS=(tablo-master tablo-node tablo-client)
RESULT_PREFIX="result-"

built_images=()   # will hold "repository:tag" strings reported by docker load

for attr in "${ATTRS[@]}"; do
  attr="${attr}-docker"
  out="${RESULT_PREFIX}${attr}"
  echo "Building ${attr} with nix..."
  nix build ".#${attr}" -o "${out}"
  echo "Build finished, locating image artifact for ${attr}..."
  # try to find a tar* in the output
  tarfile="result"

  if [[ -n "$tarfile" ]]; then
    echo "Found tar artifact: ${tarfile}"
    load_output="$(docker load -i "${tarfile}")"
    echo "${load_output}"
    # Parse "Loaded image: repo:tag"
    image_ref="$(printf '%s\n' "${load_output}" | awk -F': ' '/Loaded image: /{print $2}' | tr -d '\r' | tail -n1 || true)"
    if [[ -z "${image_ref}" ]]; then
      # fallback: try to find any image created very recently by matching repo name
      echo "Warning: docker load did not print image name. Attempting to guess from docker images..."
      image_ref="$(docker images --format '{{.Repository}}:{{.Tag}}' | grep "${attr}" | head -n1 || true)"
    fi
    if [[ -z "${image_ref}" ]]; then
      echo "Could not determine image reference for ${attr}, skipping."
    else
      echo "Registered image: ${image_ref}"
      built_images+=("${image_ref}")
    fi
  else
    echo "No tar artifact found inside ${out}. Skipping docker load for ${attr}."
    # attempt to match an already present image by attr name
    candidate="$(docker images --format '{{.Repository}}:{{.Tag}}' | grep "${attr}" | head -n1 || true)"
    if [[ -n "${candidate}" ]]; then
      echo "Found existing docker image: ${candidate}"
      built_images+=("${candidate}")
    else
      echo "No existing docker image found for ${attr}."
    fi
  fi
  echo
done

if [[ ${#built_images[@]} -eq 0 ]]; then
  echo "No images were loaded or found. Exiting, Sir."
  exit 1
fi

echo "Images available after build/load:"
for img in "${built_images[@]}"; do
  echo " - ${img}"
done
echo

read -r -p "Sir, do you want to tag all of these images as ':latest'? (y/n): " tag_choice
if [[ "${tag_choice}" =~ ^[Yy]$ ]]; then
  echo "Tagging images as :latest..."
  for img in "${built_images[@]}"; do
    repo="${img%:*}"
    # if the image already has tag 'latest' skip
    if [[ "${img##*:}" == "latest" ]]; then
      echo " ${img} is already :latest — skipping."
      continue
    fi
    docker tag "${img}" "${repo}:latest"
    echo " Tagged ${img} -> ${repo}:latest"
  done
  echo "Tagging complete."
else
  echo "Skipping tagging."
fi
echo

TAG="$(nix eval --raw .#packages.x86_64-linux.tablo-full.version)"
read -r -p "Sir, do you want to update 'nix.compose.yml' to use ':${TAG}' for these services? (y/n): " update_compose
if [[ "${update_compose}" =~ ^[Yy]$ ]]; then
  if [[ ! -f nix.compose.yml && ! -f nix.compose.yaml ]]; then
    echo "nix.compose.yml not found in current directory. Aborting compose update."
  else
    compose_file="nix.compose.yml"
    if [[ ! -f "${compose_file}" ]]; then
      compose_file="nix.compose.yaml"
    fi
    echo "Updating image tags inside ${compose_file}..."
    # Replace occurrences like "tablo-master:..." -> "tablo-master:latest"
    # sed -i 's/-docker:[^"]*/-docker:${NEW_TAG}/g' filename
    sed -E -i.bak -e "s/(image: [^:]+):[^ ]*/\1:${TAG}/" "${compose_file}"
    echo "Backup of previous file saved as ${compose_file}.bak"
    echo "Updated ${compose_file}."
  fi
else
  echo "Skipping compose file update."
fi

rm result-*

echo
echo "Done!"
