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


set -e
export SCRIPT=build-zephyr-samples-docker.sh
cd "$(dirname "$0")"/../../../../..
HOST_DIR=$(pwd)

function usage() {
    echo "Builds a predefined set of Zephyr applications"
    echo "Usage:"
    echo "  $0    : build predefined application/boards"
    echo "  $0 -i : Start an interactive session in zephyr docker. see ${SCRIPT} for more help"
    echo "  $0 -h : This help"
    exit 0
}

# interactive?
IS_INTERACTIVE=false
[ "$1" == "-h" ] && usage
[ "$1" == "-i" ] && IS_INTERACTIVE=true

source .docker-images.sh
docker inspect ${ZEPHYR_IMAGE} 2>/dev/null >/dev/null  || fail "Docker image not installed: ${ZEPHYR_IMAGE}" 

rm -rf build_zephyr/* 2>/dev/null
mkdir -p build_zephyr
# local user is different from docker container user
# therefore, access rights issues can occur.
chmod a+rw build_zephyr
chmod a+rw samples/embedded/cli_client/
chmod a+rw samples/embedded/cli_pubsub_server/

echo "Mapping ${HOST_DIR} to DOCKER '/workdir'"
$IS_INTERACTIVE && echo "Running an interactive session on ${ZEPHYR_IMAGE}" && \
    (docker run -it --rm -v ${HOST_DIR}:/zephyrproject/s2opc -w /zephyrproject/s2opc ${ZEPHYR_IMAGE})
$IS_INTERACTIVE && exit 1

function build() {
  export BOARD=$1
  export APP=$2
  echo "Starting docker to build ${APP} for ${BOARD}"

  (docker run --rm -v ${HOST_DIR}:/zephyrproject/s2opc -w /zephyrproject/s2opc ${ZEPHYR_IMAGE}\
     samples/embedded/platform_dep/zephyr/ci/${SCRIPT} ${BOARD} ${APP})&
  wait $!
  echo "Result = $?"
}
build stm32h735g_disco cli_client
build mimxrt1064_evk cli_pubsub_server
build native_posix_64 cli_pubsub_server

# Check results
EXPECTED_FILES="cli_client_stm32h735g_disco.bin cli_pubsub_server_mimxrt1064_evk.bin  cli_pubsub_server_native_posix_64.bin"
RESULT=true
for f in ${EXPECTED_FILES} ; do
	[ ! -f build_zephyr/${f} ] && echo "File not build : ${f}" && RESULT=false
	[ -f build_zephyr/${f} ] && echo "File OK : ${f}"
done

if ! ${RESULT} ; then
	echo "Build failed. To run docker manually: $0 -i"
	exit 200
fi
echo "Build Successful"

