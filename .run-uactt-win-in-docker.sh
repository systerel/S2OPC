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

# com2.systerel.fr:5000/c838/uactt-win: 1.8
DOCKER_IMAGE=sha256:6c5d97dd84cf85269c4f89250e6b12601fdd0298eeee14675c8600e0972d953e

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    "$(dirname $0)/".run-in-docker.sh $DOCKER_IMAGE "$@"
else
    sudo "$(dirname $0)/".run-in-docker.sh $DOCKER_IMAGE "$@"
fi
