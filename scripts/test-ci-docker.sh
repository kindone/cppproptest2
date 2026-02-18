#!/bin/bash
# Script to test CI workflow using Docker Compose
# Alternative to act for more control

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "Testing CI workflow with Docker Compose..."
echo "Project root: $PROJECT_ROOT"
echo ""

# Check if docker-compose is available
if ! command -v docker-compose &> /dev/null && ! command -v docker compose &> /dev/null; then
    echo "Error: 'docker-compose' is not installed."
    echo "Install Docker Desktop or docker-compose."
    exit 1
fi

# Check if docker is running
if ! docker info &> /dev/null; then
    echo "Error: Docker is not running."
    echo "Please start Docker Desktop or Docker daemon."
    exit 1
fi

# Use docker compose if available, otherwise docker-compose
DOCKER_COMPOSE_CMD="docker compose"
if ! command -v docker &> /dev/null || ! docker compose version &> /dev/null; then
    DOCKER_COMPOSE_CMD="docker-compose"
fi

# Default to running all services
SERVICE="${1:-}"

if [ -z "$SERVICE" ]; then
    echo "Running all CI test services..."
    echo ""
    $DOCKER_COMPOSE_CMD -f docker-compose.ci.yml up --build --abort-on-container-exit
else
    echo "Running service: $SERVICE"
    echo ""
    $DOCKER_COMPOSE_CMD -f docker-compose.ci.yml up --build --abort-on-container-exit "$SERVICE"
fi

echo ""
echo "CI test completed!"
