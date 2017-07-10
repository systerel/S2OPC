#!/bin/bash
# script launched by Jenkins CI system
#

# clean repository before build
git reset --hard
git clean -fd

# run build and tests
./test-all.sh

