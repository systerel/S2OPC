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

BUILD_CFG_LIST=$HIL_DIR/config/build_config.json
HARDWARE_CAPACITY=$HIL_DIR/config/hardware_capa.json

TEST=$1
[ -z "${TEST}" ] && fail "Missing 'TEST' for Launch.sh"
LOG_DIR=$2
[ -z "${LOG_DIR}" ] && fail "Missing 'LOG_DIR' for Launch.sh"

LOG_FILE=$LOG_DIR/execute_${TEST}.log
python3 ${HIL_DIR}/executor.py "$TEST" "$BUILD_CFG_LIST" "$HARDWARE_CAPACITY" "$LOG_FILE"
RET=$?
if [ $RET -ne 0 ]; then
    tail -n 20 ${LOG_FILE}
    echo "ERRORS detected, full results available in ${LOG_FILE}"
    exit 1
else
    echo "Test '$TEST' OK!"
fi
