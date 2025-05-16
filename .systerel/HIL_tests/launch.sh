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
# tests with executor.py
# Returns 0 and OK in case of success
# Returns 1 and FAIL in case of failures

cd $(dirname $0)
HIL_DIR=$(pwd)
cd ../../
HOST_DIR=$(pwd)
EMB_DIR=${HOST_DIR}/samples/embedded


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

# Identify the tests to run
TEST_NAME_LIST=$(jq -r ".launch_tests[]" "$TEST_NAME_CFG")
[ -z "${TEST_NAME_LIST}" ] && fail "Missing 'test_name' in test_to_launch.json ($TEST_NAME_LIST)"
mkdir $HOST_DIR/tests_log

#Launch the test
for TEST in $TEST_NAME_LIST; do
    echo "Running test '$TEST'"
    LOG_FILE=$HOST_DIR/tests_log/execute_${TEST}.log
    python3 ${HIL_DIR}/executor.py "$TEST" "$BUILD_CFG_LIST" "$HARDWARE_CAPACITY" "$LOG_FILE"
    RET=$?
    if [ $RET -ne 0 ]; then
        tail -n 20 ${LOG_FILE}
        echo "ERRORS detected, full results available in ${LOG_FILE}"
        exit 1
    else
        echo "Test '$TEST' OK!"
    fi
done

echo "All tests OK"
