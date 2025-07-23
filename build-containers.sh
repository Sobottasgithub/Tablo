# This script builds the Docker container for this project and tags them
#!/env bash
set -e

source ./.env

# Check for env variables (DOCKER_REGISTRY, DOCKER_REPOSITORY)

if [ -z "$DOCKER_REGISTRY" ]; then
  read -p "Enter Docker registry (e.g., docker.io): " DOCKER_REGISTRY
  export DOCKER_REGISTRY
fi

if [ -z "$DOCKER_REPOSITORY" ]; then
  read -p "Enter Docker repository (e.g., myusername/myrepository): " DOCKER_REPOSITORY
  export DOCKER_REPOSITORY
fi

# attempt to eval flake for version
if [ -z "$VERSION" ]; then
  VERSION=$(nix eval --raw .#packages.x86_64-linux.default.version 2>/dev/null || true)
  if [ -z "$VERSION" ]; then
    echo "Unable to eval version from flake, please set VERSION manually."
    read -p "Enter version (e.g., 1.0.0): " VERSION
  else
    echo "Using version: $VERSION"
  fi
  export VERSION
fi

echo "Building Images with the following parameters:"
echo "DOCKER_REGISTRY=$DOCKER_REGISTRY"
echo "DOCKER_REPOSITORY=$DOCKER_REPOSITORY"
echo "VERSION=$VERSION"

# Build the Docker image

docker build -t "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:$VERSION" -f dev.Dockerfile .

# Tag the image with latest
docker tag "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:$VERSION" "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:latest"

# Push the image to the registry
docker push "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:$VERSION"
docker push "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:latest"
echo "Docker images built and pushed successfully!"

# Save the image to a tar file

docker save "$DOCKER_REGISTRY/$DOCKER_REPOSITORY/tablo-full:$VERSION" -o "outputs/docker/tablo-full-$VERSION.tar"

echo "Docker image saved to outputs/docker/tablo-full-$VERSION.tar"
