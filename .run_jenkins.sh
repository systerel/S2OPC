#!/bin/bash

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


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
# run acceptance tests on Linux target
pushd acceptances_tests/
../.run-uactt-in-docker.sh ./launch_acceptance_tests.sh
popd
# Build binaries for Windows target on Linux host
./.mingwbuild-in-docker.sh CROSS_COMPILE_MINGW=true ./build.sh
