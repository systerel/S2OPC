#!/bin/bash
# script launched by Jenkins CI system
#

# clean repository before build
git reset --hard
git clean -fd

# run build and tests
export SOPC_DOCKER_NEEDS_SUDO=1
./clean.sh all && ./.pre-build-in-docker.sh ./pre-build.sh
./clean.sh && ./.build-in-docker.sh ./build.sh
./.test-in-docker.sh ./test-all.sh

