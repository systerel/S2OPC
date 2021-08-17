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

TMP_TOOLING_DIR=$1

# Create dir to store tooling directory for generation
\rm -rf $PREBUILD
mkdir -p "$TMP_TOOLING_DIR" > /dev/null
cd "$TMP_TOOLING_DIR" > /dev/null

# Copy tooling repository with git archive from server repository or local one
if [[ -z "$TOOLING_DIR" ]]; then
    echo "Environment variable TOOLING_DIR not set => copy of tooling.git through SSH"
    ssh pcb git --git-dir /git/c838/tooling.git archive master bin | tar x
else
    echo "Environment variable TOOLING_DIR set to '$TOOLING_DIR' => copy of repository"
    git --git-dir "$TOOLING_DIR"/.git archive master bin | tar x
fi
cd - > /dev/null


