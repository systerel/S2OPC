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

# Options: --lazy : do not reinstall sources

OPT_LAZY=false
while [ "$#" -gt 0 ] ; do
PARAM=$1
shift
[ "$PARAM" == "--lazy" ] && OPT_LAZY=true && OPT_INSTALL="--lazy" && continue
echo "Unexpected parameter : $PARAM" && exit 127
done

export GCC=arm-none-eabi-gcc 

cd `dirname $0`/../../../../..
export SOPC_ROOT=`pwd`
echo "SOPC_ROOT=$SOPC_ROOT"
export WS=/tmp/ws_freertos
export OUT_DIR=${SOPC_ROOT}/build_freertos
export OUT_ELF=${OUT_DIR}/freertos-sopc.elf
export BUILD_DIR=${WS}/build
export FREERTOS_CORE_DIR=${WS}/Core
mkdir -p ${FREERTOS_CORE_DIR}/Src ${BUILD_DIR} ${OUT_DIR}
cd ${FREERTOS_CORE_DIR}

$OPT_LAZY &&  echo "Lazy build => not reinstalling sources"
$OPT_LAZY || cp -rf /stmcube_ws/* ${WS}
${SOPC_ROOT}/samples/embedded/platform_dep/freertos/install.sh $OPT_INSTALL ||  exit $?

echo "Installation OK, starting compile step"

C_OPT_ERR='-Werror -Wall'
C_OPT_DBG='-g -DDEBUG_ -DDEBUG'
C_OPT_CPU='-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb'
C_OPT_CSTD='-std=gnu11 '
C_OPT_SPECS='--specs=nano.specs '
C_OPT_F='-ffunction-sections -fdata-sections  -fstack-usage'
C_DEFS='-DUSE_HAL_DRIVER -DSTM32H723xx '
C_DEFS+=' -DMBEDTLS_CONFIG_FILE="mbedtls_config.h" '
C_DEFS+=' -DSTM32_THREAD_SAFE_STRATEGY=4 -DSOPC_PTR_SIZE=4 -DWITH_USER_ASSERT=1'
C_DEFS+=' -D_RETARGETABLE_LOCKING=1' # Necessary since configuration differs in newlib.h
C_DEFS+=' -DSOPC_BOARD_TARGET_INCLUDE="NUCLEO-H723ZG.h"' # Indirection to board-specific header file if required
C_INCS='-I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include'
C_INCS+=' -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../MBEDTLS/App -I../Middlewares/Third_Party/LwIP/src/include'
C_INCS+=' -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2'
C_INCS+=' -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp'
C_INCS+=' -I../Middlewares/Third_Party/mbedTLS/include/mbedtls -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps'
C_INCS+=' -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif'
C_INCS+=' -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa'
C_INCS+=' -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys'
C_INCS+=' -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Middlewares/Third_Party/mbedTLS/include'
C_INCS+=' -I../Core/ThreadSafe '
C_INCS+=" -I${FREERTOS_CORE_DIR}/Src/sopc/sample_inc"


function build_c_file() {
    FILE=$1
    FILENAME=`basename ${f}`
    O_FILE=${BUILD_DIR}/${FILENAME}.o
    [ "${O_FILE}" -nt "${FILE}" ] && echo "Not rebuilding $O_FILE (up to date)" && return 0
    echo "# Build file $FILE"
    DEPENDANCIES= # '-MMD -MP -MF"'${BUILD_DIR}/${FILENAME}.d'" -MT"'${BUILD_DIR}/${FILENAME}.o'"'
    echo ${GCC} "${FILE}" ${C_OPT_DBG}  ${C_OPT_CPU} ${C_OPT_CSTD} ${C_OPT_SPECS} ${C_DEFS} -c ${C_INCS} -O1 ${C_OPT_F} ${C_OPT_ERR} ${DEPENDANCIES} -o ${O_FILE}
    ${GCC} "${FILE}" ${C_OPT_DBG}  ${C_OPT_CPU} ${C_OPT_CSTD} ${C_OPT_SPECS} ${C_DEFS} -c ${C_INCS} -O1 ${C_OPT_F} ${C_OPT_ERR} ${DEPENDANCIES} -o ${O_FILE} 2>> ${OUT_DIR}/build.err
}

function build_asm_file() {
    FILE=$1
    FILENAME=`basename ${f}`
    O_FILE=${BUILD_DIR}/${FILENAME}.o
    [ "${O_FILE}" -nt "${FILE}" ] && echo "Not rebuilding $O_FILE (up to date)" && return 0
    echo "# Build file $FILE"
    DEPENDANCIES= # '-MMD -MP -MF"'${BUILD_DIR}/${FILENAME}.d'" -MT"'${BUILD_DIR}/${FILENAME}.o'"'
    echo ${GCC} ${C_OPT_CPU} ${C_OPT_DBG} -c -x assembler-with-cpp ${DEPENDANCIES} ${C_OPT_SPECS} -o ${O_FILE} "${FILE}"
    ${GCC} ${C_OPT_CPU} ${C_OPT_DBG} -c -x assembler-with-cpp ${DEPENDANCIES} ${C_OPT_SPECS} -o ${O_FILE} "${FILE}" 2>> ${OUT_DIR}/build.err
}

function build_link() {
    L_OPT_CPU='-mcpu=cortex-m7 --specs=nosys.specs  --specs=nano.specs  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb' 
    L_LINK_SCRIPT=-T"${WS}/STM32H723ZGTX_FLASH.ld"
    L_LINK_MAP='-Wl,-Map="freertos-sopc.map"'
    
    # Create list of .o files
    O_LIST=${OUT_DIR}/objects.list
    find ${WS}/build/ -name '*.o' |sed 's/^\(.*\)$/"\1"/g' | sort > ${O_LIST}
    NB_O_FOUND=$(cat ${O_LIST} |wc -l)
    echo "Found ${NB_O_FOUND} object files..."
    [ "${NB_O_FOUND}" != "${NB_O_FILES}" ] && echo "Mismatching number of 'obj' files: ${NB_O_FOUND} (but ${NB_O_FILES} were built)" && exit 4
    
    # Link
    # Original line from StmCubeIde:
    #   arm-none-eabi-gcc -o "H723-SOPC2.elf" @"objects.list"   -mcpu=cortex-m7 -T"C:\Users\jeremie\STM32CubeIDE\workspace_1.8.0\H723-SOPC2\STM32H723ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="H723-SOPC2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
    # Adapted with no changes, except names
    # ${GCC} -o ${OUT_ELF} @"objects.list"   -mcpu=cortex-m7 -T"C:\Users\jeremie\STM32CubeIDE\workspace_1.8.0\H723-SOPC2\STM32H723ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="H723-SOPC2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
    echo ${GCC} -o ${OUT_ELF} @"${O_LIST}" ${L_OPT_CPU} ${L_LINK_SCRIPT} ${L_LINK_MAP} -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group
    ${GCC} -o ${OUT_ELF} @"${O_LIST}" ${L_OPT_CPU} ${L_LINK_SCRIPT} ${L_LINK_MAP} -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group 2>> ${OUT_DIR}/build.err

    ! ls -gh "${OUT_ELF}" 2>/dev/null && echo "Output file '${OUT_ELF}' not found!" &&  exit 99
}

rm -f ${OUT_DIR}/build.log ${OUT_DIR}/build.err
cd ${BUILD_DIR} || exit 3

NB_O_FILES=0

# Note : could use a Makefile to handle "newer" and parallelize build automatically instead of this...
C_FILES=`find ${WS} -name "*.c"`
echo $C_FILES >  ${OUT_DIR}/c_sources.list
for f in $C_FILES ; do
    NB_O_FILES=$((NB_O_FILES + 1))
    build_c_file $f 2>&1  |tee -a ${OUT_DIR}/build.log || exit $?
done

S_FILES=`find ${WS} -name "*.s"`
for f in $S_FILES ; do
    echo $f >>  ${OUT_DIR}/c_sources.list
    NB_O_FILES=$((NB_O_FILES + 1))
    build_asm_file $f 2>&1  |tee -a ${OUT_DIR}/build.log || exit $?
done

echo "Finished compiling ${NB_O_FILES} object files..."

build_link 2>&1 |tee -a ${OUT_DIR}/build.log

echo
cat ${OUT_DIR}/build.err
