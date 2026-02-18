# Contributing to cppproptest2

## Testing CI Locally

Before pushing your changes, you can test the CI workflow locally using containers.

### Quick Start

**Option 1: Using act (recommended for testing GitHub Actions workflow)**
```bash
# Run all jobs
./scripts/test-ci-local.sh

# Or use act directly
act -j build-and-test
```

**Option 2: Using Docker Compose (more control)**
```bash
# Run all test configurations
./scripts/test-ci-docker.sh

# Run specific configuration
./scripts/test-ci-docker.sh ubuntu-gcc-12
```

**Option 3: Manual Docker (for debugging)**
```bash
docker run -it -v $(pwd):/workspace -w /workspace ubuntu:22.04 bash
# Then follow build steps inside container
```

### Prerequisites

- Docker installed and running
- For act: `brew install act` (or see [act installation](https://github.com/nektos/act))
- For docker-compose: Usually included with Docker Desktop

### More Information

See [.github/workflows/README.md](.github/workflows/README.md) for detailed documentation.
