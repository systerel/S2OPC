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


echo "PWD=$(pwd) P='$0'"
cd $(dirname $0) || exit 1
cd ..
HOSTDIR=$(pwd)
CURDIR=$(pwd)/zephyr/samples
S2OPCDIR=/workdir/modules/lib/s2opc
ZEPHYRDIR=${S2OPCDIR}/zephyr/samples
OUTDIR=${HOSTDIR}/build_zephyr

mkdir -p ${OUTDIR}

cd ${CURDIR}

#Just ensure it is the right folder
[ -d "zephyr_common_src" ] || exit 1
[ `whoami` != 'user' ] && echo "Unexpected user `whoami`. Is this script executed within the docker?" && exit 2

# Replace S2OPC content by host
cd ${S2OPCDIR}/..
rm -rf s2opc
ln -s ${HOSTDIR} s2opc || exit 3

cd s2opc || exit 3


cd ${CURDIR}
build_app() {
	BOARD=$1
	APP=$2
	rm -f build/zephyr/zephyr.bin
	cd ${ZEPHYRDIR}/${APP}
	rm -rf build && west build -b ${BOARD} . 2>&1 |tee ${OUTDIR}/${APP}_${BOARD}.log
	if ! [ -f build/zephyr/zephyr.bin ] ; then
		echo " ** Build ${APP} failed " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
		exit 1
	fi
	cp build/zephyr/zephyr.bin ${OUTDIR}/${APP}_${BOARD}.bin 2>&1 |tee -a ${OUTDIR}/${APP}_${BOARD}.log
	echo " ** Build ${APP} OK " |tee -a ${OUTDIR}/${APP}_${BOARD}.log
}

# Build SERVER demo on stm32h735g_disco
build_app stm32h735g_disco zephyr_server || exit 10

# Build SERVER demo on mimxrt1064_evk
build_app mimxrt1064_evk zephyr_server || exit 11

# Build PUBSUB demo on stm32h735g_disco
build_app stm32h735g_disco zephyr_pubsub || exit 12

# Build CLIENTdemo on stm32h735g_disco
build_app stm32h735g_disco zephyr_client || exit 13

ls -l ${OUTDIR}/

