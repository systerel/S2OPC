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

# set -o errexit  # Cannot be used here since scirpt wont work
set -o nounset
set -o pipefail

function _help() {
    echo "$1 setup the environment required to build the FreeRTOS sample in the dedicated docker. This scirpt mainly calls build-freertos-samples-docker.sh"
    echo "Usage: $1 [--help] [-it] -- [any options to pass to build script]"
    echo "    -it : Start the docker in interactive mode rather than building the sample"
    echo "Example:"
    echo "     $1 -- --nocrypto"
}

OPT_ADD=
OPT_EXEC=/sopc/samples/embedded/platform_dep/freertos/ci/build-freertos-samples-docker.sh
while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" =~ --h(elp)? ]] && _help $0 && exit 0
[[ "${PARAM-}" =~ --?it ]] && OPT_EXEC= && continue
[[ "${PARAM-}" == "--" ]] && OPT_ADD="-- $*" && break
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done

cd `dirname $0`/..
SAMPLE_PTF_DIR=`pwd`
cd ../../../..
SOPC_DIR=`pwd`

echo ${SOPC_DIR-}
[[ -z "${SOPC_DIR-}" ]] && echo 'SOPC_DIR must be set!' && exit 1
! [[ -d  "${SOPC_DIR-}/src/Common" ]] && echo 'SOPC_DIR is invalid!' && exit 1

. ${SOPC_DIR}/.docker-images.sh
echo "Using image FREERTOS_IMAGE=${FREERTOS_IMAGE-}"

rm -rf ${SOPC_DIR}/build_freertos/* 2> /dev/null
mkdir -p ${SOPC_DIR}/build_freertos && chmod 777 ${SOPC_DIR}/build_freertos || exit 2

docker run --rm -ti -v ${SOPC_DIR}:/sopc -u root ${FREERTOS_IMAGE} ${OPT_EXEC} ${OPT_ADD}