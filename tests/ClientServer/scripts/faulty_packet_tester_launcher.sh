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

set -o nounset
set +e

if [[ "${TRACE-0}" == "1" ]]; then set -o xtrace; fi # debugging by running the script with the flag TRACE=1

# Set Path to server binary
PARAM=$1
[ -z  ${PARAM} ] && echo "First argument must set the path to opc ua server binary" && exit 1
[ ! -f ${PARAM} ] && echo " ${PARAM} file doesn't exist. Please give a path to a valid opc ua server binary" && exit 2
SERVER_CMD_WD="$(cd "$(dirname "${PARAM}")" && pwd)"
# Set absolute path to Server command
SERVER_CMD=$SERVER_CMD_WD/$(basename -- ${PARAM})
echo "Server test ${SERVER_CMD}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)" # Path to the directory of the script
cd ${SCRIPT_DIR}
S2OPC_DIR="$(cd ../../.. && pwd)" # Path to the root directory of the project

NB_SCENARIOS=5
STATUS=()
for scenario in $(seq 1 ${NB_SCENARIOS})
do
    SERVER_CONFIG="TEST_SERVER_XML_CONFIG=./S2OPC_Server_Test_Faulty_Packet_Config.xml TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    test $scenario -eq 5 && SERVER_CONFIG="TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml TEST_PASSWORD_PRIVATE_KEY=password"

    python3 ./with-opc-server.py --server-cmd ${SERVER_CMD} --server-wd ${SERVER_CMD_WD} --server-env "${SERVER_CONFIG}" "python3" "${S2OPC_DIR}/tests/ClientServer/interop_tools/faulty_packet.py" "--scenario" "$scenario"
    STATUS+=($?)
done

for RES in ${STATUS[@]}; do
    [[ 0 -ne $RES ]] && echo "Test Failed check tap files" && exit 1
done

echo -e "\nTest Succeed"
exit 0


