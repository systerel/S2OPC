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

# Script to build the S2OPC / ZEPHYR samplesproject:
#
# This script is intended to be run from within the docker.
# It does not require elevation

function fail() {
    echo $*
    exit 1
}

function usage() {
    echo  "Builds a given Zephyr application"
    echo "Usage: $0 [BOARD] [APP] [--ip <IP_ADDRESS>] [--nocrypto] [--log <PATH>] [--bin <PATH>] [--nodocker]"
    echo " <BOARD> <APP> : build the <APP> sample application (default 'cli_pubsub_server') for board <BOARD> (default 'mimxrt1064_evk')"
    echo " --ip <IP_ADDRESS> : Configure IP Adress of ethernet interface"
    echo "--nocrypto : Use nocrypto library instead of MbedTLS"
    echo "--log <PATH> : give a specific path to store the logs"
    echo "--bin <PATH> : give a specific path to store the bin"
    echo "--nodocker : This script is being run outside of a container" 
    echo " -h : Print this help and return"
    exit 0
}

[ "$1" == "-h" ] && usage

P0=$0
cd $(dirname $P0) || exit 1
SCRIPT_DIR=$(pwd)
cd ../../../../..

S2OPCDIR=$(pwd)
SAMPLESDIR=${S2OPCDIR}/samples/embedded
OUTDIR=${S2OPCDIR}/build_zephyr

OPT_IP_ADDRESS=
IP_ADDRESS=
LOG_PATH=
BIN_PATH=
export NO_CRYTO=0
DOCKER=true

export BOARD=$1
shift
export APP=$1
shift
[[ $1 == "--ip" ]] && shift && IP_ADDRESS=$1 && shift
[[ $1 == "--nocrypto" ]] && shift && export NO_CRYTO=1 && shift
[[ $1 == "--log" ]] && shift && LOG_PATH=$1 && shift
[[ $1 == "--bin" ]] && shift && BIN_PATH=$1 && shift
[[ $1 == "--nodocker" ]] && shift && DOCKER=false

mkdir -p ${OUTDIR} || exit 2

if ["{$DOCKER}" = true]; then
  [ `whoami` != 'user' ] && echo "Unexpected user `whoami`. Is this script executed within the docker?" && exit 2
fi

# Just check that all folders exist!
cd ${S2OPCDIR} || exit 3
cd ${SAMPLESDIR} || exit 4

# Trust user directory in order to get commit signature
# User in docker and outside docker could have different UID which leads to warnings from git
git config --global --add safe.directory ${S2OPCDIR}

export ADD_CONF=$*

BOARD_NAME=$(echo "${BOARD}" | tr '/' '_')

[[ -z $BOARD ]] && export BOARD=mimxrt1064_evk && echo "Using default board ${BOARD}"
[[ -z $APP ]]   && export APP=cli_pubsub_server && echo "Using default application ${APP}"
[[ ! -z ${IP_ADDRESS} ]] && OPT_IP_ADDRESS="-DCONFIG_SOPC_ETH_ADDRESS=\"${IP_ADDRESS}\" -DCONFIG_SOPC_ENDPOINT_ADDRESS=\"opc.tcp://${IP_ADDRESS}:4841\"" && echo "Configure new Ip address ${IP_ADDRESS}"
[[ -z $LOG_PATH ]]   && LOG_PATH=${OUTDIR}/${APP}_${BOARD_NAME}.log && echo "Using default log path ${LOG_PATH}"
[[ -z $BIN_PATH ]]   && BIN_PATH=${OUTDIR}/${APP}_${BOARD_NAME}.bin && echo "Using default bin path ${BIN_PATH}"
[[ ! -z ${ADD_CONF} ]] && echo "Using additional extra configuration : '${ADD_CONF}'"

directory_path=$(dirname "$LOG_PATH")
file_name=$(basename "$file_path")


#west boards |grep -q ^$BOARD$ || fail "Invalid board $BOARD. Type 'west boards' for the list of supported targets."
[ -d "${SAMPLESDIR}/${APP}" ] || fail "Invalid application $APP"

echo " ** Building ${APP} ... " | mkdir -p $(dirname "$LOG_PATH") |tee -a ${LOG_PATH}
cd ${SAMPLESDIR}/${APP} || return 1
sudo rm -rf build || return  1

echo "Command : 'west build -b ${BOARD} -- ${OPT_IP_ADDRESS} ${ADD_CONF} . '"
west build -b ${BOARD} -- ${OPT_IP_ADDRESS} ${ADD_CONF} . 2>&1 |tee ${LOG_PATH}
chmod --recursive 777 build
mv build/zephyr/zephyr.exe build/zephyr/zephyr.bin 2> /dev/null
if ! [ -f build/zephyr/zephyr.bin ] ; then
  echo " ** Build ${APP} failed " | mkdir -p $(dirname "$LOG_PATH") | tee -a ${LOG_PATH}
  exit 1
fi
 mkdir -p $(dirname "$BIN_PATH")
cp build/zephyr/zephyr.bin ${BIN_PATH} 2>&1 |tee -a ${LOG_PATH}
echo " ** Build ${APP} OK " | mkdir -p $(dirname "$LOG_PATH") |tee -a ${LOG_PATH}

ls -l $(dirname "$LOG_PATH")/
chmod -R 777 build
ls -l $(dirname "$BIN_PATH")/
chmod -R 777 build


