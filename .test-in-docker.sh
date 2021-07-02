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

DOCKER_IMAGE=sha256:d7e092f270d02864e09475c60757f51491f8f7eb3b66f0dcb5d587c7c44e385a # test:2.9

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    "$(dirname $0)/".run-in-docker.sh $DOCKER_IMAGE "$@"
else
    sudo "$(dirname $0)/".run-in-docker.sh $DOCKER_IMAGE "$@"
fi
