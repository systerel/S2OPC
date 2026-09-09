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

# Verification of suspect cases raised by .fuzzysully.sh
# Each case is replayed with a large timeout: experience shows that most suspects come from monitor timeout and not from an actual server problem


set -o nounset

# arguments
BIN_DIR=${1:-}
[ -z "${BIN_DIR}" ] && echo "The first argument must indicate the server bin folder" && exit 1
[ ! -x "${BIN_DIR}/toolkit_demo_server" ] && echo "${BIN_DIR}/toolkit_demo_server not found or not executable. Provide a valid path to the OPC UA server binaries" && exit 2
BIN_DIR="$(cd "${BIN_DIR}" && pwd)"

S2OPC_DIR="$(cd "$(dirname "$0")" && pwd)"
WITH_SERVER="${S2OPC_DIR}/tests/ClientServer/scripts/with-opc-server.py"
[ ! -f "${WITH_SERVER}" ] && echo "${WITH_SERVER} not found" && exit 3
[ -z "$(command -v fuzzysully)" ] && echo "fuzzysully not found in PATH" && exit 4

RESULTS_DIR="${S2OPC_DIR}/fuzzysully-results"
SUSPECTS="${RESULTS_DIR}/SUSPECTS.log"
if [ ! -f "${SUSPECTS}" ]; then
    echo "SUSPECTS.log missing: the fuzzing job artifact was not retrieved"
    exit 1
fi
if [ ! -s "${SUSPECTS}" ]; then
    echo "No suspect case to verify"
    exit 0
fi
 
# Timeouts intentionally larger than those used during fuzzing
FUZZ_HOST=${FUZZ_HOST:-localhost}
FUZZ_PORT=${FUZZ_PORT:-4841}
FUZZ_SEND_TIMEOUT=${FUZZ_SEND_TIMEOUT:-1.0}
FUZZ_RECV_TIMEOUT=${FUZZ_RECV_TIMEOUT:-1.0}
 
export TEST_SERVER_XML_CONFIG=../../tests/ClientServer/data/config/S2OPC_Server_Demo_Config_Fuzzer.xml
export TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml
export TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml
 
VERIF_DIR="${RESULTS_DIR}/verification"
mkdir -p "${VERIF_DIR}"
cd "${VERIF_DIR}"

CMD_FILE="${VERIF_DIR}/fuzzer_commands.txt"
CONFIRMES="${VERIF_DIR}/CONFIRMES.log"
FAUX_POSITIFS="${VERIF_DIR}/FAUX_POSITIFS.log"
NON_CONCLUANTS="${VERIF_DIR}/NON_CONCLUANTS.log"
rm -f "${CONFIRMES}" "${FAUX_POSITIFS}" "${NON_CONCLUANTS}"

TRACE_BRUT="${VERIF_DIR}/fuzzowski-results/OPCUA_${FUZZ_HOST}_${FUZZ_PORT}__TCP_server_None_all.log"

echo "Verifying $(wc -l < "${SUSPECTS}") suspect cases"

while read -r CAS; do
    # We position ourselves just before the case to verify (case - 1). The goto command is the only way to replay a case.
    DEBUT=$((CAS - 1))

    # The goto is slow: about 30s per 100000 cases. We add a duration to allow the few following cases to execute
    DELAI_GOTO=$(( DEBUT / 100000 * 30 ))
    DUREE=$(( DELAI_GOTO + 30 ))
    SERVER_TIMEOUT=$(( DUREE + 120 ))

    echo "goto ${DEBUT}" > "${CMD_FILE}"
    echo "c" >> "${CMD_FILE}"

    # Each case is replayed in a fresh fuzzy session and on a fresh server, in case the server crashes or fuzzy crashes because the tested case is actually suspect
    rm -f "${TRACE_BRUT}"
    echo "Case ${CAS} : replay for ${DUREE}s"

    python3 "${WITH_SERVER}" --server-wd "${BIN_DIR}" --server-cmd ./toolkit_demo_server --wait-timeout ${SERVER_TIMEOUT} bash -c "{ cat ${CMD_FILE}; sleep ${DUREE}; } | timeout ${DUREE} fuzzysully server ${FUZZ_HOST} ${FUZZ_PORT} '' -st ${FUZZ_SEND_TIMEOUT} -rt ${FUZZ_RECV_TIMEOUT}"

    if [ ! -f "${TRACE_BRUT}" ]; then
        echo "No trace produced"
        echo "${CAS}" >> "${NON_CONCLUANTS}"
        continue
    fi

    TRACE="${VERIF_DIR}/cas_${CAS}_trace.txt"
    strings "${TRACE_BRUT}" > "${TRACE}"
    rm -f "${TRACE_BRUT}"

    if grep -q "Added test case ${CAS} as a suspect" "${TRACE}"; then
        echo "Case ${CAS} is still suspect with a timeout of ${FUZZ_SEND_TIMEOUT}s"
        echo "${CAS}" >> "${CONFIRMES}"
    elif grep -q "Test Case: ${CAS}:" "${TRACE}"; then
        echo " False positive, the server responded"
        echo "${CAS}" >> "${FAUX_POSITIFS}"
        rm -f "${TRACE}"
    else
        # The case was not reached or the goto did not have time to complete, which can create false positives
        echo " case not reached: goto delay too short"
        echo "${CAS}" >> "${NON_CONCLUANTS}"
    fi
done < "${SUSPECTS}"

echo " False positives : $([ -f "${FAUX_POSITIFS}" ] && wc -l < "${FAUX_POSITIFS}" || echo 0)"
echo "Inconclusive : $([ -f "${NON_CONCLUANTS}" ] && wc -l < "${NON_CONCLUANTS}" || echo 0)"

if [ -s "${CONFIRMES}" ]; then
    echo "Confirmed suspect cases :"
    cat "${CONFIRMES}"
    exit 1
fi

echo "No confirmed suspect case"
exit 0