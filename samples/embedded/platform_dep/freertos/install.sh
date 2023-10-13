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
SRC_DIR=${FREERTOS_CORE_DIR-}/Src/sopc

OPT_LAZY=false
OPT_CRYPTO="mbedtls"
while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" == "--lazy" ]] && OPT_LAZY=true  && continue
[[ "${PARAM-}" == "--nocrypto" ]] && OPT_CRYPTO="nocrypto"  && continue
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done

echo "Installing S2OPC for freeRTOS samples..."

# Check FreeRTOS

[[ -z ${FREERTOS_CORE_DIR-} ]] && echo "[EE] FREERTOS_CORE_DIR must be set to Core folder of FREERTOS installation dir" && exit 1
! [[ -f ${FREERTOS_CORE_DIR-}/Src/freertos.c ]] && echo "[EE] FREERTOS_CORE_DIR must point to Core folder of FREERTOS installation dir" && echo FREERTOS_CORE_DIR=${FREERTOS_CORE_DIR-} && exit 2
echo "[II] FreeRTOS found!"

# Check sample
[[ -z ${FREERTOS_SAMPLE-} ]] && echo "[WW] FREERTOS_SAMPLE not defined, using default cli_pubsub_server" && FREERTOS_SAMPLE=cli_pubsub_server
! [[ -d ${S2OPC_SAMPLE-}/${FREERTOS_SAMPLE-} ]] && echo "[EE] Sample '${FREERTOS_SAMPLE-}' not found. Check env var FREERTOS_SAMPLE"

echo "[II] Sample ${FREERTOS_SAMPLE-} found!"

$OPT_LAZY || rm -fr  ${SRC_DIR-} 2> /dev/null
mkdir -p ${SRC_DIR-}/sample_src
mkdir -p ${SRC_DIR-}/sample_inc

# copy source files to Core/Src
cp -ur ${S2OPC_SAMPLE-}/${FREERTOS_SAMPLE-}/src/* "${SRC_DIR-}/sample_src" || exit 10
cp -ur ${S2OPC_SAMPLE-}/platform_dep/freertos/src/* "${SRC_DIR-}/sample_src" || exit 10
cp -ur ${S2OPC_SAMPLE-}/platform_dep/mbedtls_config "${SRC_DIR-}" || exit 11
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

echo "[II] Using crypto library '${OPT_CRYPTO-}'"
echo "[II] Source copied to ${S2OPC_SRC-}"

# Move .h files to include folder
find  "${SRC_DIR-}" -name "*.h" ! -path "*/sample_inc/*" -exec mv {} ${SRC_DIR-}/sample_inc \;
cp -ur ${S2OPC_SAMPLE-}/platform_dep/include/*.h ${SRC_DIR-}/sample_inc || exit 14
echo "[II] Headers copied to ${SRC_DIR}/sample_inc"

# Patch main entry
FILE=${FREERTOS_CORE_DIR-}/Src/main.c 
sed -i 's/^.*USER CODE BEGIN 5.*$/  extern void sopc_main_entry\(void *\); sopc_main_entry\(NULL\);/g' ${FILE-} || exit 3
# unix2dos.exe ${FILE-} >/dev/null 2>/dev/null  ## Only used to avoid EOL issue if needed on Windows
echo "[II] Main entry patched to call S2OPC code"

# Patch freertos hooks
FILE=${FREERTOS_CORE_DIR-}/Src/freertos.c 
sed -i 's/\bvApplicationTickHook\b/__vApplicationTickHook__/g' ${FILE-}
echo "[II] FreeRTOS hooks patched for 'vApplicationTickHook'"
sed -i 's/\bvApplicationStackOverflowHook\b/__vApplicationStackOverflowHook__/g'  ${FILE-}
# unix2dos.exe ${FILE-} >/dev/null 2>/dev/null  ## Only used to avoid EOL issue if needed on Windows
echo "[II] FreeRTOS hooks patched for 'vApplicationStackOverflowHook'"

# Patch mbedtls_hardware_poll
# Patch:
# -      memset(&(Output[index * 4]), (int)randomValue, 4);
# +      memcpy(&(Output[index * 4]), &(int)randomValue, 4);
FILE=${FREERTOS_CORE_DIR-}/../MBEDTLS/Target/hardware_rng.c
sed -i 's/memset\((&(Output\[index[^,]*, *\)(int)\(randomValue\)/memcpy\1\&\2/g' ${FILE-}
# unix2dos.exe ${FILE-} >/dev/null 2>/dev/null  ## Only used to avoid EOL issue if needed on Windows
echo "[II] '`basename ${FILE-}`' bug patched for 'mbedtls_hardware_poll'"

# Patch ETH MAC filter
# Replace 
# ETH_MACDMAConfig(heth); 
#  by
# ETH_MACDMAConfig(heth); extern SOPC_ETH_MAC_Filter_Config(ETH_HandleTypeDef*); SOPC_ETH_MAC_Filter_Config(heth);
FILE=${FREERTOS_CORE_DIR-}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c
mv ${FILE-} ${FILE-}.crlf
tr -d '\r' <${FILE-}.crlf >${FILE-}
rm -f ${FILE-}.crlf
res=false
sed -i 's/^\( *ETH_MACDMAConfig(heth); *\)$/\1 extern void SOPC_ETH_MAC_Filter_Config(ETH_HandleTypeDef*); SOPC_ETH_MAC_Filter_Config(heth);/g' ${FILE-} 
! grep -q '^ *ETH_MACDMAConfig(heth); *extern void' ${FILE-} && echo "[II] '`basename ${FILE-}`' patched failed for 'ETH MAC filter'"  && exit 1
unix2dos.exe ${FILE-} >/dev/null 2>/dev/null
echo "[II] '`basename ${FILE-}`' patched for 'ETH MAC filter'"


# Patches below are not used anymore since this is in USER BEGIN/END tags (not lost when regenerating code from STMCube Ide)
# This is kept for traceability of updates in environments

#sed -i 's/configUSE_TICK_HOOK *0/configUSE_TICK_HOOK 1/g' ${FREERTOS_CORE_DIR-}/Inc/FreeRTOSConfig.h 
#echo "[II] Patched configUSE_TICK_HOOK for FreeRTOS options"

# Patch LWIP_RAM_HEAP_POINTER for LwIP opts (https://github.com/STMicroelectronics/STM32CubeF4/issues/123)
# now useless since MEM_LIBC_MALLOC is used and no more LWIP specific Malloc exists
# f=$(find $(dirname ${FREERTOS_CORE_DIR-}) -name 'lwipopts.h')
#! [ -f "$f" ] && echo "could not find 'lwipopts.h'" && exit 20
#sed -i 's/^\( *#define LWIP_RAM_HEAP_POINTER\)/\/\/\1/g' "${f-}]f"
#echo "[II] LWIP_RAM_HEAP_POINTER patched in LwIP_opts"


