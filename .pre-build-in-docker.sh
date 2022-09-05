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


#  Prepare tooling dependency repository & run the script given parameters in the docker
#
# Options
# - TOOLING_DIR can be defined to point to the local tooling repository root directory

set -e

PREBUILD=pre-build
TMP_TOOLING_DIR="$(pwd)/$PREBUILD/tooling"

source "$(dirname "$0")/".docker-images.sh

"$(dirname "$0")/".pre-build-get-tooling.sh "$TMP_TOOLING_DIR"

"$(dirname "$0")/".run-in-docker.sh "$GEN_IMAGE" \
    TOOLING_DIR="$TMP_TOOLING_DIR" RUN_INTERACTIVELY="$RUN_INTERACTIVELY" "$@"
