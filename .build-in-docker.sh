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

DOCKER_IMAGE=sha256:0bbab53a4c13efb85aaca8fbfddc60e60acb7a776b3491ae75715c21d88b3942 # build:1.26

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    "`dirname $0`/".run-in-docker.sh $DOCKER_IMAGE DOCKER_IMAGE=$DOCKER_IMAGE "$@"
else
    sudo "`dirname $0`/".run-in-docker.sh $DOCKER_IMAGE DOCKER_IMAGE=$DOCKER_IMAGE "$@"
fi
