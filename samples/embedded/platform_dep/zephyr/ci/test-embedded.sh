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

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
S2OPC_ROOT_DIR="$(cd ${SCRIPT_DIR}/../../../../../ && pwd)"
EXIT_STATUS=""

function test_cli()
{
    export CLI_BIN=$1

    [ ! -f ${CLI_BIN} ] && echo " File ${CLI_BIN} not found. Are you sure having built S2OPC embedded samples ?" && exit 1

    echo "launch ${CLI_BIN}"
    timeout --preserve-status -s SIGTERM -k 10s 1 ${CLI_BIN}
    exit_status=$?

    if [ "$exit_status" -eq "0" ]; then
        echo -e "\n SUCCESS executing file ${CLI_BIN} \n"
    else
        echo -e "\n ERROR executing file ${CLI_BIN} \n"
        echo -e " ${CLI_BIN} exit with status ${exit_status}"
    fi
    EXIT_STATUS="$EXIT_STATUS $exit_status"

}

test_cli "${S2OPC_ROOT_DIR}/build_zephyr/cli_client_native_posix_64.bin"
test_cli "${S2OPC_ROOT_DIR}/build_zephyr/cli_pubsub_server_native_posix_64.bin"

# check if there has been an error
for result in $EXIT_STATUS; do
    [ "$result" -ne "0" ] && exit 1
done

exit 0