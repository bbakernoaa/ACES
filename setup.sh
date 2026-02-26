#!/bin/bash
# setup.sh
#
# This script sets up the ACES development environment using Docker.
# It pulls the official JCSDA image and drops you into a bash shell.
#
# Usage: ./setup.sh

set -e

# Define the container image
IMAGE="jcsda/docker-gnu-openmpi-dev:1.9"

# Ensure docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed or not in PATH."
    exit 1
fi

echo "Pulling Docker image: $IMAGE"
docker pull "$IMAGE"

echo "Launching ACES Development Container..."
# Mount the current directory to /work in the container
docker run -it --rm \
    -v "$(pwd):/work" \
    -w /work \
    "$IMAGE" \
    /bin/bash -c "source /opt/spack-environment/activate.sh && exec bash"
