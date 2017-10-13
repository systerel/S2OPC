#!/bin/bash
# script launched by Jenkins CI system
#

# clean repository before build
git reset --hard
git clean -fd

# run build and tests
./clean.sh && sudo ./.build-in-docker.sh ./build.sh
sudo ./.test-in-docker.sh ./test-all.sh

