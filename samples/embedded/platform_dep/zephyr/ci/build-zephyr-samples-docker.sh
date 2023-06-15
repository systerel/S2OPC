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
    echo "Usage:"
    echo "  $0    : build a predefined application/board"
    echo "  $0 <BOARD> <APP> : build the <APP> sample application (default 'cli_pubsub_server') for board <BOARD> (default 'mimxrt1064_evk')"
    echo "  $0 -h : This help"
    exit 0
}

[ "$1" == "-h" ] && usage

P0=$0
cd $(dirname $P0) || exit 1
cd ../../../../..

S2OPCDIR=$(pwd)
SAMPLESDIR=${S2OPCDIR}/samples/embedded
OUTDIR=${S2OPCDIR}/build_zephyr

#echo "OUTDIR=$OUTDIR"
#echo "S2OPCDIR=$S2OPCDIR"
#echo "SAMPLESDIR=$SAMPLESDIR"
#echo "OUTDIR=$OUTDIR"

mkdir -p ${OUTDIR} || exit 2

[ `whoami` != 'user' ] && echo "Unexpected user `whoami`. Is this script executed within the docker?" && exit 2

# Just check that all folders exist!
cd ${S2OPCDIR} || exit 3
cd ${SAMPLESDIR} || exit 4

# Trust user directory in order to get commit signature
# User in docker and outside docker could have different UID which leads to warnings from git
git config --global --add safe.directory ${S2OPCDIR}

export BOARD=$1
export APP=$2
west boards |grep -q ^$BOARD$ || fail "Invalid board $BOARD. Type 'west boards' for the list of supported targets."
[ -d "${SAMPLESDIR}/${APP}" ] || fail "Invalid application $APP"

[[ -z $BOARD ]] && export BOARD=mimxrt1064_evk && echo "Using default board ${BOARD}"
[[ -z $APP ]]   && export APP=cli_pubsub_server && echo "Using default application ${APP}"
 

echo " ** Building ${APP} ... " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
cd ${SAMPLESDIR}/${APP} || return 1
sudo rm -rf build || return  1
west build -b ${BOARD} . 2>&1 |tee ${OUTDIR}/${APP}_${BOARD}.log
mv build/zephyr/zephyr.exe build/zephyr/zephyr.bin 2> /dev/null
if ! [ -f build/zephyr/zephyr.bin ] ; then
  echo " ** Build ${APP} failed " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
  return 1
fi
cp build/zephyr/zephyr.bin ${OUTDIR}/${APP}_${BOARD}.bin 2>&1 |tee -a ${OUTDIR}/${APP}_${BOARD}.log
echo " ** Build ${APP} OK " |tee -a ${OUTDIR}/${APP}_${BOARD}.log

ls -l ${OUTDIR}/

