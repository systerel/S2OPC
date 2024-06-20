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


set -o nounset
set -o pipefail

cd -- "$(dirname $0)"
SCRIPT_DIR=`pwd`
cd ../../../../..
SOPC_DIR=`pwd`

OPT_EXEC=/s2opc/samples/embedded/platform_dep/pikeos/ci/test-embedded-docker.sh

DOCKER_USER=pikeos_user

function _help() {
    echo "$1 setup the environment required to execute the PikeOS tests in the dedicated docker. This scirpt mainly calls test-embedded.sh"
    echo "Usage: $1 [--help] [-it] [--root]"
    echo "    -it : Start the docker in interactive mode rather than building the sample"
    echo "    -root : Start the docker with root user"
}

while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" =~ --h(elp)? ]] && _help $0 && exit 0
[[ "${PARAM-}" =~ --?it ]] && OPT_EXEC= && continue
[[ "${PARAM-}" == --root ]] && DOCKER_USER=root && continue
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done


echo "SOPC_DIR=${SOPC_DIR}"
[[ -z "${SOPC_DIR-}" ]] && echo 'SOPC_DIR must be set!' && exit 1
! [[ -d  "${SOPC_DIR-}/src/Common" ]] && echo 'SOPC_DIR is invalid!' && exit 1

. ${SOPC_DIR}/.docker-images.sh
echo "Using image PIKEOS_DIGEST=${PIKEOS_DIGEST-}"

docker run --rm -ti -u ${DOCKER_USER} --cap-add=NET_ADMIN --device /dev/net/tun:/dev/net/tun  --mac-address=f8:ca:b8:53:4e:67 --hostname=pikeos_machine -v ${SOPC_DIR}:/s2opc ${PIKEOS_DIGEST} ${OPT_EXEC}