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

# set -o errexit  # Cannot be used here since script wont work
set -o nounset
set -o pipefail

declare -A HAL
declare -A HAL_Driver
declare -A LD
declare -A CPU
declare -A FPU
declare -A LAN

# add new supported board here
# NUCLEO-H723ZG
HAL[NUCLEO-H723ZG]="STM32H723xx"
HAL_Driver[NUCLEO-H723ZG]="STM32H7xx"
LD[NUCLEO-H723ZG]="STM32H723ZGTX_FLASH.ld"
CPU[NUCLEO-H723ZG]="cortex-m7"
FPU[NUCLEO-H723ZG]="fpv5-d16"
LAN[NUCLEO-H723ZG]="lan8742"

function _help() {
    echo "Builds a given FreeRTOS application"
    echo "Usage: $0 [BOARD] [APP] [--ip <IP_ADDRESS>] [--nocrypto] [--log <PATH>] [--bin <PATH>] [--lazy]"
    echo " <BOARD> <APP> : build the <APP> sample application (default 'cli_pubsub_server') for board <BOARD> (default 'NUCLEO-H723ZG')"
    echo " --ip <IP_ADDRESS> : Configure IP Address of ethernet interface"
    echo "--nocrypto : Use nocrypto library instead of MbedTLS"
    echo "--log <PATH> : give a specific path to store the logs"
    echo "--bin <PATH> : give a specific path to store the bin"
    echo "--lazy : do not reinstall S2OPC sources (avoids full rebuild)"
    echo " -h : Print this help and return"
    exit 0
}

function fail() {
    echo "$@" >&2
    exit 1
}

OPT_LAZY=false
IP_ADDRESS=
OPT_CRYPTO=mbedtls
LOG_PATH=
BIN_PATH=
BOARD=
APP=

[[ $1 == --* ]] || { export BOARD=$1; shift;}
[[ $1 == --* ]] || { export APP=$1; shift;}

while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" =~ "-h" ]] && _help $0 && exit 0
[[ "${PARAM-}" =~ "--lazy" ]] && OPT_LAZY=true && OPT_INSTALL="--lazy" && continue
[[ "${PARAM-}" =~ "--nocrypto" ]] && OPT_CRYPTO="nocrypto" && continue
[[ "${PARAM-}" == "--ip" ]] && IP_ADDRESS=$1 && shift && continue
[[ "${PARAM-}" == "--log" ]] && LOG_PATH=$1 && shift && continue
[[ "${PARAM-}" == "--bin" ]] && BIN_PATH=$1 && shift && continue
[[ "${PARAM-}" == "--" ]] && break
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done

[[ -z $BOARD ]] && export BOARD="NUCLEO-H723ZG" && echo "Using default board ${BOARD}"
[[ -z $APP ]]   && export APP="cli_pubsub_server" && echo "Using default application ${APP}"
[[ -z $IP_ADDRESS ]] && IP_ADDRESS="192.168.42.21" && echo "Using default IP address : ${IP_ADDRESS}"

# TODO : Re-add MbedTLS Support
if [ OPT_CRYPTO == "mbedtls" ]; then
    fail "mbedtls is not supported for now --nocrypto is necessary"
fi

if [[ -z "${HAL[$BOARD]}" ]]; then
    echo "Error: Unsupported board '$BOARD'."
    echo "Supported boards are: ${!HAL[@]}"
    exit 127
fi

BOARD_HAL="${HAL[$BOARD]}"
BOARD_HAL_Driver="${HAL_Driver[$BOARD]}"
BOARD_LD="${LD[$BOARD]}"
BOARD_CPU="${CPU[$BOARD]}"
BOARD_FPU="${FPU[$BOARD]}"
BOARD_LAN="${LAN[$BOARD]}"

export GCC=arm-none-eabi-gcc

cd `dirname $0`/../../../../..
export SOPC_ROOT=`pwd`
echo "SOPC_ROOT=$SOPC_ROOT"
export WS=/tmp/ws_freertos
export OUT_DIR=${SOPC_ROOT-}/build_freertos
export BUILD_DIR=${WS-}/build
export FREERTOS_CORE_DIR=${WS-}/Core
mkdir -p ${FREERTOS_CORE_DIR-}/Src ${BUILD_DIR-} ${OUT_DIR-}
cd ${FREERTOS_CORE_DIR-}

[[ -n $BIN_PATH ]] && case "$BIN_PATH" in
    *.bin) BIN_PATH=${BIN_PATH%????} ;;
esac

[[ -n $LOG_PATH ]] && case "$LOG_PATH" in
    *.log) LOG_PATH=${LOG_PATH%????} ;;
esac

[[ -z $LOG_PATH ]] && LOG_PATH=${OUT_DIR}/${APP}_${BOARD} && echo "Using default log path : ${LOG_PATH}.log"
[[ -z $BIN_PATH ]] && BIN_PATH=${OUT_DIR}/${APP}_${BOARD}  && echo "Using default bin path : ${BIN_PATH}.bin"

export OUT_ELF="${BIN_PATH}.elf"
export OUT_BIN="${BIN_PATH}.bin"

${OPT_LAZY-} &&  echo "Lazy build => not reinstalling sources"
${OPT_LAZY-} || cp -rf /stmcube_ws/* ${WS-}
${SOPC_ROOT-}/samples/embedded/platform_dep/freertos/install-stm.sh --sample ${APP} ${OPT_INSTALL-} $([ "$OPT_CRYPTO" = "nocrypto" ] && echo --nocrypto) "$@" || exit $?

echo "Installation OK, starting compile step"

C_OPT_ERR="-Werror"
C_OPT_DBG="-g -DDEBUG_ -DDEBUG"
C_OPT_CPU="-mcpu=${BOARD_CPU} -mfpu=${BOARD_FPU} -mfloat-abi=hard -mthumb"
C_OPT_CSTD="-std=gnu11 "
C_OPT_SPECS="--specs=nano.specs "
C_OPT_F="-ffunction-sections -fdata-sections -fstack-usage"
C_DEFS="-DUSE_HAL_DRIVER -D${BOARD_HAL} "
C_DEFS+=" -D_RETARGETABLE_LOCKING=1" # Necessary since configuration differs in newlib.h
C_INCS="-I../Core/Inc -I../Drivers/${BOARD_HAL_Driver}_HAL_Driver/Inc -I../Drivers/${BOARD_HAL_Driver}_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/${BOARD_HAL_Driver}/Include"
C_INCS+=" -I../Drivers/CMSIS/Include -I../Core/Src/sopc/sample_inc -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2"
C_INCS+=" -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/Components/${BOARD_LAN} -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys"
C_INCS+=" -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch"
C_DEFS+=" -include../Core/Src/sopc/sample_inc/${BOARD}.h "

function build_c_file() {
    FILE=$1
    FILENAME=`basename ${f-}`
    O_FILE=${BUILD_DIR-}/${FILENAME-}.o
    [[ "${O_FILE-}" -nt "${FILE-}" ]] && echo "Not rebuilding ${O_FILE-} (up to date)" && return 0
    echo "# Build file ${FILE-}"
    DEPENDENCIES= # '-MMD -MP -MF"'${BUILD_DIR-}/${FILENAME-}.d'" -MT"'${BUILD_DIR-}/${FILENAME-}.o'"'
    echo ${GCC-} "${FILE-}" ${C_OPT_DBG-}  ${C_OPT_CPU-} ${C_OPT_CSTD-} ${C_OPT_SPECS-} ${C_DEFS-} -c ${C_INCS-} -O2 ${C_OPT_F-} ${C_OPT_ERR-} ${DEPENDENCIES-} -o ${O_FILE-}
    ${GCC-} "${FILE-}" ${C_OPT_DBG-}  ${C_OPT_CPU-} ${C_OPT_CSTD-} ${C_OPT_SPECS-} ${C_DEFS-} -c ${C_INCS-} -O2 ${C_OPT_F-} ${C_OPT_ERR-} ${DEPENDENCIES-} -o ${O_FILE-} 2>> "${LOG_PATH}.err"
}

function build_asm_file() {
    FILE=$1
    FILENAME=`basename ${f-}`
    O_FILE=${BUILD_DIR-}/${FILENAME-}.o
    [[ "${O_FILE-}" -nt "${FILE-}" ]] && echo "Not rebuilding ${O_FILE-} (up to date)" && return 0
    echo "# Build file ${FILE-}"
    DEPENDENCIES= # '-MMD -MP -MF"'${BUILD_DIR-}/${FILENAME-}.d'" -MT"'${BUILD_DIR-}/${FILENAME-}.o'"'
    echo ${GCC-} ${C_OPT_CPU-} ${C_OPT_DBG-} -c -x assembler-with-cpp ${DEPENDENCIES-} ${C_OPT_SPECS-} -o ${O_FILE-} "${FILE-}"
    ${GCC-} ${C_OPT_CPU-} ${C_OPT_DBG-} -c -x assembler-with-cpp ${DEPENDENCIES-} ${C_OPT_SPECS-} -o ${O_FILE-} "${FILE-}" 2>> "${LOG_PATH}.err"
}

function build_link() {
    L_OPT_CPU='-mcpu=cortex-m7 --specs=nosys.specs  --specs=nano.specs  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb'
    L_LINK_SCRIPT=-T"${WS-}/${BOARD_LD}"
    L_LINK_MAP='-Wl,-Map="freertos-sopc.map"'

    # Create list of .o files
    O_LIST=${OUT_DIR-}/objects.list
    find ${WS-}/build/ -name '*.o' | sed 's/^\(.*\)$/"\1"/g' | sort > ${O_LIST-}
    NB_O_FOUND=$(cat ${O_LIST-} | wc -l)
    echo "Found ${NB_O_FOUND-} object files..."
    [[ "${NB_O_FOUND-}" != "${NB_O_FILES-}" ]] && echo "Mismatching number of 'obj' files: ${NB_O_FOUND-} (but ${NB_O_FILES-} were built)" && exit 4

    # Link
    # Original line from StmCubeIde:
    #   arm-none-eabi-gcc -o "H723-SOPC2.elf" @"objects.list"   -mcpu=cortex-m7 -T"C:\Users\[user]\STM32CubeIDE\workspace_1.8.0\H723-SOPC2\STM32H723ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="H723-SOPC2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
    # Adapted with no changes, except names
    # ${GCC-} -o ${OUT_ELF-} @"objects.list"   -mcpu=cortex-m7 -T"C:\Users\[user]\STM32CubeIDE\workspace_1.8.0\H723-SOPC2\STM32H723ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="H723-SOPC2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
    echo ${GCC-} -o ${OUT_ELF-} @"${O_LIST-}" ${L_OPT_CPU-} ${L_LINK_SCRIPT-} ${L_LINK_MAP-} -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group
    ${GCC-} -o ${OUT_ELF-} @"${O_LIST-}" ${L_OPT_CPU-} ${L_LINK_SCRIPT-} ${L_LINK_MAP-} -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group 2>> "${LOG_PATH}.err"

    ! ls -gh "${OUT_ELF-}" 2>/dev/null && echo "Output file '${OUT_ELF-}' not found!" &&  exit 99
}

rm -f "${LOG_PATH}.log" "${LOG_PATH}.err"
cd ${BUILD_DIR-} || exit 3

# Configure new Ip address if needed
if [ ! -z ${IP_ADDRESS} ]; then
    IFS='.' read -ra PARSED_IP_ADDRESS <<< "$IP_ADDRESS"
    TARGET_PATH=$WS/LWIP/App/lwip.c
    for index in {0..3} ; do
        sed -i "s/^\(\s*IP_ADDRESS\[\s*${index}\s*\]\s*=\s*\)[0-9]\+\(\s*;\)/\1${PARSED_IP_ADDRESS[$index]}\2/g" $TARGET_PATH || exit $?
        # Check that pattern exist
        grep -q "\s*IP_ADDRESS\[\s*${index}\s*\]\s*=\s*[0-9]\+\s*;" $TARGET_PATH || fail "Ip pattern not found in $TARGET_PATH"
    done
fi

NB_O_FILES=0

# Note : could use a Makefile to handle "newer" and parallelize build automatically instead of this...
C_FILES=`find ${WS-} -name "*.c"`
echo ${C_FILES-} >  ${OUT_DIR-}/c_sources.list
for f in ${C_FILES-} ; do
    NB_O_FILES=$((NB_O_FILES + 1))
    build_c_file $f 2>&1  |tee -a "${LOG_PATH}.log" || exit $?
done

S_FILES=`find ${WS-} -name "*.s"`
for f in ${S_FILES-} ; do
    echo ${f-} >>  ${OUT_DIR-}/c_sources.list
    NB_O_FILES=$((NB_O_FILES + 1))
    build_asm_file $f 2>&1  |tee -a "${LOG_PATH}.log" || exit $?
done

echo "Finished compiling ${NB_O_FILES-} object files..."

build_link 2>&1 |tee -a "${LOG_PATH}.log"
arm-none-eabi-objcopy -O binary ${OUT_ELF-} ${OUT_BIN-}

echo
cat "${LOG_PATH}.err"

# Necessary to avoid issue when doing multiple builds
rm -rf ${BUILD_DIR}