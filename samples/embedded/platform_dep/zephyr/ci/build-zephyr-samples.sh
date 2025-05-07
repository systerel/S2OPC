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
export SCRIPT="$(pwd)/build-zephyr-samples-west.sh"
cd "$(dirname "$0")"/../../../../..
HOST_DIR=$(pwd)

function usage() {
    echo "By default builds a predefined set of Zephyr applications"
    echo "To build a specific Zephyr applications use -b <BOARD> -a <APPLICATION>"
    echo "Usage : $0 [-h] [-i] [-a <APPLICATION>] [-b <BOARD>] [--ip <IP_ADDRESS>] [--log <PATH>] [--bin <PATH>] [--docker]"
    echo "  $0                      : build predefined application/boards"
    echo "  $0 -i                   : Start an interactive session in zephyr docker. see ${SCRIPT} for more help"
    echo "  $0 -b                   : Allows user to specify a <BOARD>"
    echo "  $0 -a                   : Allows user to specify an <APPLICATION>"
    echo "  $0 --ip <IP_ADDRESS>    : Configure IP Adress of ethernet interface"
    echo "--log <PATH>              : give a specific path to store the logs"
    echo "--bin <PATH>              : give a specific path to store the bins"
    echo "--docker                  : run the build script in docker"
    echo "  $0 -h                   : This help"
    exit 0
}

# interactive?
IS_INTERACTIVE=false
OPT_IP_ADDRESS=
GET_BOARD=
GET_APP=
LOG_PATH=
BIN_PATH=
DOCKER=false

while [[ ! -z $1 ]]; do
    PARAM=$1
    shift
    [[ ${PARAM} == "-h" ]] && usage && exit 1
    [[ ${PARAM} == "-i" ]] && IS_INTERACTIVE=true && continue
    [[ ${PARAM} == "-b" ]] && GET_BOARD=$1 && shift && continue
    [[ ${PARAM} == "-a" ]] && GET_APP=$1 && shift && continue
    [[ ${PARAM-} == "--ip" ]] && OPT_IP_ADDRESS="--ip $1" && shift && continue
    [[ ${PARAM-} == "--log" ]] && LOG_PATH="--log $1" && shift && continue
    [[ ${PARAM-} == "--bin" ]] && BIN_PATH="--bin $1" && shift && continue
    [[ ${PARAM-} == "--docker" ]] && DOCKER=true && continue

    echo "Unknown parameter : ${PARAM}"
    exit 2
done

[[ ! -z $LOG_PATH ]] && LOG_PATH="--log ${LOG_PATH}"
[[ ! -z $BIN_PATH ]] && BIN_PATH="--bin ${BIN_PATH}"


if [ "$DOCKER" = true ]; then
    source .docker-images.sh
    docker inspect ${ZEPHYR_DIGEST} 2>/dev/null >/dev/null  || fail "Docker image not installed: ${ZEPHYR_DIGEST}"

    echo "Mapping ${HOST_DIR} to DOCKER '/workdir'"
    $IS_INTERACTIVE && echo "Running an interactive session on ${ZEPHYR_DIGEST}" && \
        (docker run -it --rm -v ${HOST_DIR}:/zephyrproject/modules/lib/s2opc -w /zephyrproject/modules/lib/s2opc ${ZEPHYR_DIGEST})
    $IS_INTERACTIVE && exit 1
fi

function build() {
  export BOARD=$1
  shift
  export APP=$1
  shift
  export ADD_CONF=$*
  
  echo "Starting docker to build ${APP} for ${BOARD}"
  
  if [ "$DOCKER" = true ]; then
  (docker run --rm -v ${HOST_DIR}:/zephyrproject/modules/lib/s2opc -w /zephyrproject/modules/lib/s2opc ${ZEPHYR_DIGEST}\
    samples/embedded/platform_dep/zephyr/ci/${SCRIPT} ${BOARD} ${APP} ${OPT_IP_ADDRESS} ${LOG_PATH} ${BIN_PATH} ${ADD_CONF})& 
    wait $!
  else
    (${SCRIPT} ${BOARD} ${APP} ${OPT_IP_ADDRESS} ${LOG_PATH} ${BIN_PATH} --nodocker ${ADD_CONF})& wait $!
  fi
  echo "Result = $?"
}


if [ -z "${GET_BOARD}" ] && [ -z "${GET_APP}" ]; then
    rm -rf build_zephyr/* 2>/dev/null
    mkdir -p build_zephyr
    # local user is different from docker container user
    # therefore, access rights issues can occur.
    chmod a+rw build_zephyr
    chmod a+rw samples/embedded/cli_client/
    chmod a+rw samples/embedded/cli_pubsub_server/

    build stm32h735g_disco cli_client
    build nucleo_h745zi_q/stm32h745xx/m7 cli_client -DCONFIG_NET_GPTP=y
    build mimxrt1064_evk cli_pubsub_server
    build stm32h747i_disco/stm32h747xx/m7 cli_pubsub_server --nocrypto
    build stm32h747i_disco/stm32h747xx/m7 cli_pubsub_server

    # Check results
    EXPECTED_FILES="cli_client_stm32h735g_disco.bin cli_client_nucleo_h745zi_q_stm32h745xx_m7.bin cli_pubsub_server_mimxrt1064_evk.bin  cli_pubsub_server_stm32h747i_disco_stm32h747xx_m7.bin"
    RESULT=true
    if ! ${RESULT} ; then
        echo "Build failed. To run docker manually: $0 -i"
        exit 200
    fi
elif [ -z "${GET_BOARD}" ] || [ -z "${GET_APP}" ]; then
    echo "Missing argument board or app see ./build-zephyr-samples.sh -h" && exit 1
else
    printf "\n\e[0;36mChosen boards : $GET_BOARD Chosen app :$GET_APP \033[39m"
    build $GET_BOARD $GET_APP
fi

for f in ${EXPECTED_FILES} ; do
    [ ! -f build_zephyr/${f} ] && echo "File not build : ${f}" && RESULT=false
    [ -f build_zephyr/${f} ] && echo "File OK : ${f}"
done

echo "Build Successful"

