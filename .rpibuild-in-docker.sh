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


#  Run the script given parameters in the docker
#
set -e

source "$(dirname "$0")/".docker-images.sh
# Note: activate the Python3 cross_venv to be able to build the wheel for RPi with correct information
"$(dirname "$0")/".run-in-docker.sh "$RPI_DIGEST" ". /usr/local/cross_venv/bin/activate && " PYS2OPC_WHEEL_NAME=pys2opc-1.6.0-cp39-cp39-linux_arm.whl CMAKE_TOOLCHAIN_FILE=/toolchain-rpi.cmake DOCKER_DIGEST="$RPI_DIGEST" "$@"
