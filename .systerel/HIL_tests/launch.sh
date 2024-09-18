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

# Main script for HIL testing
# Takes all information needed from build_config.json, test_to_launch.json; hardware_capa.json
# Launches successively ./compile.sh, ./flash_app.sh and then the tests with executor.py
# Returns 0 and OK in case of success
# Returns 1 and FAIL in case of failures

cd $(dirname $0)
HIL_DIR=$(pwd)
cd ../../
HOST_DIR=$(pwd)
EMB_DIR=${HOST_DIR}/samples/embedded

# Check for necessary commands
#**********************************
if ! command -v jq &> /dev/null; then
    echo "'jq' is not installed"
    exit 1
fi

if ! command -v arm-none-eabi-objcopy &> /dev/null; then
    echo "'binutils-arm-none-eabi' is not installed"
    exit 1
fi
cd $HOST_DIR
#**********************************
# Delete the build directories
#**********************************
# rm -rf build_zephyr/* 2>/dev/null # TODO : clean all & avoid rebuilds
mkdir -p build_zephyr
# local user is different from docker container user
# therefore, access rights issues can occur.
chmod a+rw build_zephyr
chmod a+rw samples/embedded/cli_client/
chmod a+rw samples/embedded/cli_pubsub_server/
cd $HIL_DIR
#**********************************

##########################
# Helper function to exit with an error message
# @param $* An error message
function fail() {
    echo -e "[EE] $*" >&2
    exit -1
}

#**********************************
# Delete the build directories
#**********************************

BUILD_CFG_LIST=$HIL_DIR/config/build_config.json
TEST_NAME_CFG=$HIL_DIR/config/test_to_launch.json
HARDWARE_CAPACITY=$HIL_DIR/config/hardware_capa.json
LOG_PATH=$HIL_DIR/logs

rm -rf -- ${LOG_PATH}
mkdir -p ${LOG_PATH}

# Identify the tests to run
TEST_NAME_LIST=$(jq -r ".launch_tests[]" "$TEST_NAME_CFG")
[ -z "${TEST_NAME_LIST}" ] && fail "Missing 'test_name' in test_to_launch.json ($TEST_NAME_LIST)"

for TESTS in $TEST_NAME_LIST; do
index=0
    for BUILD in $(jq -c ".tests.${TESTS}.builds[]" "$BUILD_CFG_LIST"); do
    #jq -c : return a compacted line instead of 1 line per item
        SERIAL=$(jq -r ".tests.${TESTS}.builds[$index].BOARD_SN" "$BUILD_CFG_LIST")
        [ -z "${SERIAL}" ] && fail "Missing 'BOARD_SN' field in 'builds' ($BUILD)"
        BUILD_NAME=$(jq -r ".tests.${TESTS}.builds[$index].build_name" "$BUILD_CFG_LIST")
        [ -z "${BUILD_NAME}" ] && fail "Missing 'BUILD_NAME' field in 'builds' ($BUILD)"
        # Identify the operating system involved in the test
        OS=$(jq -r ".build.${BUILD_NAME}.OS" "$BUILD_CFG_LIST")
        [ -z "${OS}" ] && fail "Missing 'OS' field in 'builds' ($BUILD)"
        [ -d "$EMB_DIR/platform_dep/${OS}" ] || fail "OS '$OS' is not supported on HIL tests"
        # Identify the application involved in the test
        APP=$(jq -r ".build.${BUILD_NAME}.app" "$BUILD_CFG_LIST")
        [ -z "${APP}" ] && fail "Missing 'APP' field in 'builds' ($BUILD)"
        [ -d "$EMB_DIR/${APP}" ] || fail "APP '$APP' is not supported on HIL tests"
        # Identify the board involved in the test
        BOARD=$(jq -r ".build.${BUILD_NAME}.board" "$BUILD_CFG_LIST")
        [ -z "${BOARD}" ] && fail "Missing 'BOARD' field in 'builds' ($BUILD)"
        # Identify the extension (.bin, .elf ...) involved in the test
        # !!!!! Not used yet as only .bin are supported for now !!!!!
        EXTENSION=$(jq -r ".build.${BUILD_NAME}.flash_type " "$BUILD_CFG_LIST")
        [ -z "${EXTENSION}" ] && fail "Missing 'EXTENSION' field in 'builds' ($BUILD)"
        #Compile and flash the right application on the right board according to previous parameters
        
        LOG_FILE=$LOG_PATH/compile_${OS}_${APP}_${BOARD}.log
        OUT_FILE=$HOST_DIR/build_${OS}/${APP}_${BOARD}.${EXTENSION}
        if ! [ -f $OUT_FILE ] ; then
          echo "Building $(basename ${OUT_FILE}) for board $BOARD (see $LOG_FILE)"
           ${HIL_DIR}/compile.sh "$OS" "$BOARD" "$APP" "$EXTENSION" > $LOG_FILE 2>&1
        else
          echo "Not rebuilding $(basename ${OUT_FILE})"
        fi 
        [ -f $OUT_FILE ] || fail "Missing output file ${OUT_FILE}"
        
        LOG_FILE=$LOG_PATH/flash_${OS}_${APP}_${BOARD}.log
        echo "Flash $APP/$OS on board $BOARD #$SERIAL"
        ${HIL_DIR}/flash_app.sh "$SERIAL" "${APP}_${BOARD}.${EXTENSION}" "${OS}" > $LOG_FILE 2>&1
        [ $? != 0 ]  && cat $LOG_FILE && fail "Flashing failed"
        ((index=index+1))
    done
done

for TEST in $TEST_NAME_LIST; do
    #Launch the test
    echo "Running test '$TEST'"
    python3 ${HIL_DIR}/executor.py "$TEST" "$BUILD_CFG_LIST" "$HARDWARE_CAPACITY" "$LOG_PATH"
    RET=$?
    if [ $RET -ne 0 ]; then
        echo "ERRORS detected, results available in ${LOG_PATH}"
        exit 1
    fi
done

echo "All tests OK"
exit 0
