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

DOCKER_IMAGE=sha256:9b7787b1150e97bc112f98d912694025eb57038b56062d516a8b022de153654d # build:1.10

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE DOCKER_IMAGE=$DOCKER_IMAGE "$@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE DOCKER_IMAGE=$DOCKER_IMAGE "$@"
fi
