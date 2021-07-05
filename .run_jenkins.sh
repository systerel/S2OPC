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
export SOPC_DOCKER_NEEDS_SUDO=1
export LOCAL_JENKINS_JOB=1
# Generate C code from B model and for tests (@ space)
./clean.sh all && ./.pre-build-in-docker.sh ./pre-build.sh
# check that generated C code is up to date in configuration management
./clean.sh && ./.check_generated_code.sh
# Check rules on source code and automatic formatting compliance
./.check-in-docker.sh ./.check-code.sh
# Build binaries for Linux target
./clean.sh && ./.build-in-docker.sh WITH_NANO_EXTENDED=1 ./build.sh
# Run tests on Linux target
./.test-in-docker.sh "mosquitto -d && ./test-all.sh"
# run acceptance tests on Linux target
pushd tests/ClientServer/acceptance_tools/
../../../.run-uactt-win-in-docker.sh WITH_NANO_EXTENDED=1 ./launch_acceptance_tests.sh
popd
# Build binaries for Windows target on Linux host
./.mingwbuild-in-docker.sh S2OPC_CLIENTSERVER_ONLY=1 ./build.sh
