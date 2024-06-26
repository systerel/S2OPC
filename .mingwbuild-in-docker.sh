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
 # deactivate warnings as errors because of string format issues
"$(dirname "$0")/".run-in-docker.sh "$MINGW_DIGEST" CMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake DOCKER_DIGEST="$MINGW_DIGEST" S2OPC_CLIENTSERVER_ONLY=1 WARNINGS_AS_ERRORS=0 "$@"
