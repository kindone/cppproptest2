# Running CI Locally

This directory contains GitHub Actions workflows that can be tested locally using Docker.

## Prerequisites

- Docker installed and running
- [act](https://github.com/nektos/act) - GitHub Actions local runner

Install act:
```bash
# macOS
brew install act

# Linux
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Or via npm
npm install -g @github/act
```

## Quick Start

### Test the CI workflow locally

```bash
# Run the CI workflow (will prompt for Docker image selection)
cd /path/to/cppproptest2
act

# Run specific job
act -j build-and-test

# Run with specific event (push to main)
act push

# Run with verbose output
act -v

# List all jobs
act -l
```

### Using specific Docker images

act uses Docker images that match GitHub Actions runners. You can specify images:

```bash
# Use Ubuntu image for Linux jobs
act -P ubuntu-latest=catthehacker/ubuntu:act-latest

# Use macOS image (note: macOS runners are simulated, not native)
act -P macos-latest=catthehacker/ubuntu:act-latest

# Use Windows image (Windows support in act is limited)
act -P windows-latest=catthehacker/ubuntu:act-latest
```

## Limitations

- **macOS runners**: act simulates macOS on Linux containers, so some macOS-specific behavior may differ
- **Windows runners**: Limited Windows support; Windows-specific paths and tools may not work correctly
- **Performance**: Local runs may be slower than GitHub-hosted runners
- **Secrets**: You'll need to provide secrets manually if your workflow uses them
- **Artifact uploads**: Artifact upload steps will show warnings when running with `act` (they require GitHub Actions runtime tokens). The workflow uses `continue-on-error: true` for artifact uploads, so they won't fail the workflow - you'll just see warnings. Artifacts will still upload correctly when running on GitHub Actions.

- **Setup actions**: Some setup actions (like `jwlawson/actions-setup-cmake@v1.14` and `microsoft/setup-msbuild@v1.1`) may show warnings about missing `ACTIONS_RUNTIME_TOKEN` when running with `act`. These actions have `continue-on-error: true` and fallback steps, so the workflow will continue. The fallback steps will use system-installed tools if available.

**Note**: If you see `ACTIONS_RUNTIME_TOKEN` errors, they're expected when running locally with `act`. The workflow is designed to handle these gracefully and continue execution.

## Testing Specific Matrix Combinations

To test a specific matrix combination, you can use act's matrix filtering:

```bash
# Test only Linux + GCC
act -j build-and-test --matrix os:ubuntu-latest --matrix compiler:gcc

# Test only C++20 builds
act -j build-and-test --matrix cpp_std:20
```

## Alternative: Docker Compose Testing

For more control, use the provided Docker Compose setup:

```bash
# Run all CI test services
./scripts/test-ci-docker.sh

# Run specific service (e.g., Ubuntu + GCC 12)
./scripts/test-ci-docker.sh ubuntu-gcc-12

# Or use docker-compose directly
docker-compose -f docker-compose.ci.yml up --build ubuntu-gcc-12
```

## Alternative: Manual Docker Testing

If you need even more control, you can manually test in Docker containers:

```bash
# Test Linux build interactively
docker run -it -v $(pwd):/workspace -w /workspace ubuntu:22.04 bash
# Inside container:
apt-get update && apt-get install -y gcc-12 g++-12 cmake make
cmake . -BBUILD -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
cmake --build BUILD --parallel
cd BUILD && ./test_proptest
```

## Troubleshooting

- **Docker not running**: Start Docker Desktop or Docker daemon
- **Permission errors**: Ensure Docker is accessible (may need `sudo` on Linux)
- **Image pull errors**: Check internet connection and Docker registry access
- **act version**: Ensure you have a recent version of act (v0.2+)
