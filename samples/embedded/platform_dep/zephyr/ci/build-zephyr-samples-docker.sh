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

P0=$0
echo "PWD=$(pwd) P0='$P0'"
cd $(dirname $P0) || exit 1
cd ../../../../..

HOSTDIR=$(pwd)
S2OPCDIR=/zephyrproject/modules/lib/s2opc
SAMPLESDIR=${S2OPCDIR}/samples/embedded
OUTDIR=${HOSTDIR}/build_zephyr

echo
echo "HOSTDIR=$HOSTDIR"
echo "S2OPCDIR=$S2OPCDIR"
echo "SAMPLESDIR=$SAMPLESDIR"
echo "OUTDIR=$OUTDIR"

rm -rf ${OUTDIR} 2> /dev/null
mkdir -p ${OUTDIR}

[ `whoami` != 'user' ] && echo "Unexpected user `whoami`. Is this script executed within the docker?" && exit 2

# Replace S2OPC content by host
! [ -d ${S2OPCDIR} ] && echo "S2OPC not found in ${S2OPCDIR}" && exit 2
cd ${S2OPCDIR}/.. || exit 2
mv s2opc s2opc_bak || exit 2
ln -s ${HOSTDIR} s2opc || exit 3

cd ${S2OPCDIR} || exit 3


cd ${SAMPLESDIR} || exit 4
build_app() {
  export BOARD=$1
  export APP=$2
  echo " ** Building ${APP} ... " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
  cd ${SAMPLESDIR}/${APP} || return 1
  rm -rf build && west build -b ${BOARD} . 2>&1 |tee ${OUTDIR}/${APP}_${BOARD}.log
  mv build/zephyr/zephyr.exe build/zephyr/zephyr.bin 2> /dev/null
  if ! [ -f build/zephyr/zephyr.bin ] ; then
    echo " ** Build ${APP} failed " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
    return 1
  fi
  cp build/zephyr/zephyr.bin ${OUTDIR}/${APP}_${BOARD}.bin 2>&1 |tee -a ${OUTDIR}/${APP}_${BOARD}.log
  echo " ** Build ${APP} OK " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
}

# Build CLIENT demo on stm32h735g_disco
build_app stm32h735g_disco cli_client || exit 10

# Build PUBSUB+SERVER demo on mimxrt1064_evk
build_app mimxrt1064_evk cli_pubsub_server || exit 11

# Build PUBSUB+SERVER demo on native_posix_64
build_app native_posix_64 cli_pubsub_server || exit 12

ls -l ${OUTDIR}/

