#!/bin/bash
# Script to test CI workflow locally using act

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "Testing CI workflow locally with act..."
echo "Project root: $PROJECT_ROOT"
echo ""

# Check if act is installed
if ! command -v act &> /dev/null; then
    echo "Error: 'act' is not installed."
    echo "Install it with: brew install act"
    echo "Or visit: https://github.com/nektos/act"
    exit 1
fi

# Check if docker is running
if ! docker info &> /dev/null; then
    echo "Error: Docker is not running."
    echo "Please start Docker Desktop or Docker daemon."
    exit 1
fi

# Default to running the build-and-test job
JOB="${1:-build-and-test}"

echo "Running job: $JOB"
echo ""

# Run act with appropriate settings
act -j "$JOB" \
    -P ubuntu-latest=catthehacker/ubuntu:act-latest \
    -P macos-latest=catthehacker/ubuntu:act-latest \
    -P windows-latest=catthehacker/ubuntu:act-latest \
    --container-architecture linux/amd64 \
    "$@"

echo ""
echo "CI test completed!"
