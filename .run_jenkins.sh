#!/bin/bash
# script launched by Jenkins CI system
#
set -e

# clean repository before build
git reset --hard
git clean -fd

# run build and tests
export SOPC_DOCKER_NEEDS_SUDO=1
# Generate C code from B model and for tests (@ space)
./clean.sh all && ./.pre-build-in-docker.sh ./pre-build.sh
# Check rules on source code and automatic formatting compliance
./.check-in-docker.sh ./.check-code.sh
# Build binaries for Linux target
./clean.sh && ./.build-in-docker.sh ./build.sh
# Run tests on Linux target
./.test-in-docker.sh ./test-all.sh
# Build binaries for Windows target on Linux host
./.mingwbuild-in-docker.sh CROSS_COMPILE_MINGW=true ./build.sh
