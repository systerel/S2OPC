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
PROJECT_DIR=`pwd`
SOPC_INCLUDE_DIR=${PROJECT_DIR}/sopc_incldue
mkdir -p ${SOPC_INCLUDE_DIR}

SRC_DIR=${SOPC_INCLUDE_DIR-}/sopc
mkdir -p ${SRC_DIR}

MAIN_FILE=${PROJECT_DIR-}/source/lwip_ipv4_ipv6_echo_freertos.c

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
[[ -z ${SOPC_INCLUDE_DIR-} ]] && echo "[EE] SOPC_INCLUDE_DIR must be set to Core folder of FREERTOS installation dir" && exit 1

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
find  "${SRC_DIR-}" -name "*.*" ! -path "*/sample_inc/*" -exec mv {} ${SOPC_INCLUDE_DIR-}/ \;
cp -ur ${S2OPC_SAMPLE-}/platform_dep/include/*.* ${SOPC_INCLUDE_DIR-}/ || exit 14
find "${SRC_DIR-}/" -mindepth 1 -type d -exec rm -rf {} +
echo "[II] All copied to ${SOPC_INCLUDE_DIR}/"

# Patch LWIP example
FILE=${PROJECT_DIR-}/source/lwipopts_gen.h
sed -i 's/^#define[ \t]\+LWIP_PROVIDE_ERRNO[ \t]\+[0-9]\+/#define LWIP_PROVIDE_ERRNO 0/' "${FILE-}"
sed -i 's/^#define[ \t]\+LWIP_SINGLE_NETIF[ \t]\+[0-9]\+/#define LWIP_SINGLE_NETIF 0/' "${FILE-}"
sed -i 's/^#define[ \t]\+MEMP_NUM_NETCONN[ \t]\+[0-9]\+U\?\([ \t]\|$\)/#define MEMP_NUM_NETCONN 16U\1/' "${FILE-}"
sed -i 's/^#define[ \t]\+MEMP_NUM_NETBUF[ \t]\+[0-9]\+U\?\([ \t]\|$\)/#define MEMP_NUM_NETBUF 8U\1/' "${FILE-}"
sed -i 's/^#define[ \t]\+MEMP_NUM_NETDB[ \t]\+[0-9]\+U\?\([ \t]\|$\)/#define MEMP_NUM_NETDB 4U\1/' "${FILE-}"
sed -i 's/^#define[ \t]\+TCPIP_THREAD_STACKSIZE[ \t]\+[0-9]\+U\?\([ \t]\|$\)/#define TCPIP_THREAD_STACKSIZE 2048U\1/' "${FILE-}"
sed -i 's/^#define[ \t]\+DEFAULT_THREAD_STACKSIZE[ \t]\+[0-9]\+U\?\([ \t]\|$\)/#define DEFAULT_THREAD_STACKSIZE 2048U\1/' "${FILE-}"
echo "[II] LWIP conf_gen patched"

if ! grep -qF "sopc_main();" "${MAIN_FILE-}"; then
  sed -i '/vTaskDelete(NULL);/i\sopc_main();' "${MAIN_FILE-}"
fi
echo "[II] patched sopc_main"

function add_init {
  if ! grep -qF "$1" "$3"; then
    # remove escape for use in sed
    esc_1=$(printf '%s\n' "$1" | sed -e 's/[&/\]/\\&/g')
    esc_2=$(printf '%s\n' "$2" | sed -e 's/[&/\]/\\&/g')

    sed -i "/$esc_2/{
      N
      s|$esc_2\n|$esc_2\n$esc_1\n|
    }" "$3"
  fi
}

#Add include
FILE=${MAIN_FILE}
INCLUDE="#include \"lwip/sockets.h\""
add_init "#include \"freertos_platform_dep.h\"" "${INCLUDE}" "${FILE}"
echo "[II] lwip_ipv4_ipv6_echo_freertos_cm33.c patched include to main.c"

#Remove unused files
rm -rf "${PROJECT_DIR-}/source/shell_task.c"
rm -rf "${PROJECT_DIR-}/source/shell_task.h"
rm -rf "${PROJECT_DIR-}/source/shell_task_mode.h"
rm -rf "${PROJECT_DIR-}/source/socket_task.c"
rm -rf "${PROJECT_DIR-}/source/socket_task.h"
FILE=${MAIN_FILE}
sed -i '/^#include[ \t]*"shell_task\.h"/d' "${FILE-}"
echo "[II] ${MAIN_FILE} patched for unused files"