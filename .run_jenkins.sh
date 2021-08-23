#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


# script launched by Jenkins CI system
#
set -e

# clean repository before build
git reset --hard
git clean -fd

# run build and tests
source .docker-images.sh
# Generate C code from B model and for tests (@ space)
PREBUILD=pre-build
TMP_TOOLING_DIR="$(pwd)/$PREBUILD/tooling"
./clean.sh all && ./.pre-build-get-tooling.sh "$TMP_TOOLING_DIR" && sudo /etc/scripts/run-in-docker "$GEN_IMAGE" TOOLING_DIR="$TMP_TOOLING_DIR" ./pre-build.sh
# check that generated C code is up to date in configuration management
./clean.sh && ./.check_generated_code.sh
# Check rules on source code and automatic formatting compliance
sudo /etc/scripts/run-in-docker "$CHECK_IMAGE" DOCKER_IMAGE="$CHECK_IMAGE" ./.check-code.sh
# Build binaries for Linux target
./clean.sh && sudo /etc/scripts/run-in-docker "$BUILD_IMAGE" WITH_NANO_EXTENDED=1 DOCKER_IMAGE="$BUILD_IMAGE" ./build.sh
# Run tests on Linux target
sudo /etc/scripts/run-in-docker "$TEST_IMAGE" "mosquitto -d && ./test-all.sh"
# run acceptance tests on Linux target
pushd tests/ClientServer/acceptance_tools/
sudo /etc/scripts/run-in-docker "$UACTT_WIN_IMAGE" WITH_NANO_EXTENDED=1 ./launch_acceptance_tests.sh
popd
# Build binaries for Windows target on Linux host
sudo /etc/scripts/run-in-docker "$MINGW_IMAGE" S2OPC_CLIENTSERVER_ONLY=1 CMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake BUILD_SHARED_LIBS=true DOCKER_IMAGE="$MINGW_IMAGE" ./build.sh
