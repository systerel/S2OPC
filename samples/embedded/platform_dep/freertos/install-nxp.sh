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



cd `dirname $0`/../../../..
S2OPC_BASE=`pwd`
S2OPC_SRC=${S2OPC_BASE-}/src
S2OPC_SAMPLE=${S2OPC_BASE-}/samples/embedded

cd `dirname $0`/..
FREERTOS_DIR=`pwd`
FREERTOS_CORE_DIR=${FREERTOS_DIR}/temp
mkdir -p ${FREERTOS_CORE_DIR}

SRC_DIR=${FREERTOS_CORE_DIR-}/sopc
mkdir -p ${SRC_DIR}

GET_BOARD=
OPT_LAZY=false
OPT_CRYPTO="mbedtls"
while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" == "-b" ]] && GET_BOARD=$1 && shift && continue
[[ "${PARAM-}" == "--lazy" ]] && OPT_LAZY=true  && continue
[[ "${PARAM-}" == "--nocrypto" ]] && OPT_CRYPTO="nocrypto"  && continue
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done

[[ -z ${GET_BOARD-} ]] && echo "[EE] GET_BOARD must be set with -b [board-name]" && exit 1

echo "Installing S2OPC for freeRTOS samples..."

# Check FreeRTOS
[[ -z ${FREERTOS_CORE_DIR-} ]] && echo "[EE] FREERTOS_CORE_DIR must be set to Core folder of FREERTOS installation dir" && exit 1

# Check sample
[[ -z ${FREERTOS_SAMPLE-} ]] && echo "[WW] FREERTOS_SAMPLE not defined, using default cli_pubsub_server" && FREERTOS_SAMPLE=cli_pubsub_server
! [[ -d ${S2OPC_SAMPLE-}/${FREERTOS_SAMPLE-} ]] && echo "[EE] Sample '${FREERTOS_SAMPLE-}' not found. Check env var FREERTOS_SAMPLE"

echo "[II] Sample ${FREERTOS_SAMPLE-} found!"

$OPT_LAZY || rm -rf  ${SRC_DIR-} 2> /dev/null
mkdir -p ${SRC_DIR-}/sample_src
mkdir -p ${SRC_DIR-}/sample_inc

# copy source files to Core/Src
cp -ur ${S2OPC_SAMPLE-}/${FREERTOS_SAMPLE-}/src/* "${SRC_DIR-}/sample_src" || exit 10
cp -ur ${S2OPC_SAMPLE-}/platform_dep/freertos/src/* "${SRC_DIR-}/sample_src" || exit 10
[[ ${OPT_CRYPTO} = "mbedtls" ]] && (cp -ur ${S2OPC_SAMPLE-}/platform_dep/mbedtls_config "${SRC_DIR-}" || exit 11)
cp -ur ${S2OPC_SRC-}/Common "${SRC_DIR-}" || exit 12
cp -ur ${S2OPC_SRC-}/PubSub "${SRC_DIR-}" || exit 13
cp -ur ${S2OPC_SRC-}/ClientServer "${SRC_DIR-}" || exit 14

mv ${SRC_DIR-}/Common/helpers_platform_dep/freertos/s2opc_common_export.h_ ${SRC_DIR-}/Common/helpers_platform_dep/freertos/s2opc_common_export.h

# Remove other platform ports
( cd ${SRC_DIR-}/Common/helpers_platform_dep && for f in * ; do [ -d ${f-} ] && [[ ${f-} != freertos ]] && rm -rf ${f-} ; done )
# Remove EXPAT-related files
find  ${SRC_DIR-} -name "*expat*.c" -or -name "*config_xml*.c" -or -path "*xml_expat*.c" |xargs rm -fv
# Remove CRYPTO-related files
( cd ${SRC_DIR-}/Common/crypto/lib_dep && for f in * ; do [ -d ${f-} ] && [[ ${f-} != ${OPT_CRYPTO-} ]] && rm -rf ${f-} && echo "removed '${SRC_DIR-}/Common/crypto/lib_dep/${f}/**'" ; done )

find "${SRC_DIR-}/sample_src/boards/" -type f ! -name "${GET_BOARD-}.h" -delete

echo "[II] Using crypto library '${OPT_CRYPTO-}'"
echo "[II] Source copied to ${S2OPC_SRC-}"

# Move .h files to include folder
find  "${SRC_DIR-}" -name "*.*" ! -path "*/sample_inc/*" -exec mv {} ${FREERTOS_CORE_DIR-}/ \;
cp -ur ${S2OPC_SAMPLE-}/platform_dep/include/*.* ${FREERTOS_CORE_DIR-}/ || exit 14
find "${SRC_DIR-}/" -mindepth 1 -type d -exec rm -rf {} +
echo "[II] All copied to ${FREERTOS_CORE_DIR}/"

