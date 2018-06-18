#!/bin/bash

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
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
TMP_TOOLING_DIR=$(pwd)/$PREBUILD/tooling

DOCKER_IMAGE=ca487e4c9dde

# Create dir to store tooling directory for generation
\rm -rf $PREBUILD
mkdir -p $TMP_TOOLING_DIR > /dev/null
cd $TMP_TOOLING_DIR > /dev/null

# Copy tooling repository with git archive from server repository or local one
if [[ -z $TOOLING_DIR ]]; then
    echo "Environment variable TOOLING_DIR not set => copy of tooling.git through SSH"
    ssh pcb git --git-dir /git/c765/tooling.git archive master bin | tar x
else
    echo "Environment variable TOOLING_DIR set to '$TOOLING_DIR' => copy of repository"
    git --git-dir $TOOLING_DIR/.git archive master bin | tar x
fi
cd - > /dev/null


if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE "TOOLING_DIR=$TMP_TOOLING_DIR $@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE "TOOLING_DIR=$TMP_TOOLING_DIR $@"
fi

