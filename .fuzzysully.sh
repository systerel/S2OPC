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

# Fuzzing an S2OPC server with Fuzzysully (ANSSI).

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

ulimit -n 65535 2>/dev/null || echo "Open file limit not raised (current : $(ulimit -n))"

# Fuzzing parameters
FUZZ_HOST=${FUZZ_HOST:-localhost}
FUZZ_PORT=${FUZZ_PORT:-4841}
FUZZ_SEND_TIMEOUT=${FUZZ_SEND_TIMEOUT:-0.02}
FUZZ_RECV_TIMEOUT=${FUZZ_RECV_TIMEOUT:-0.02}
FUZZ_DURATION=${FUZZ_DURATION:-900}
SERVER_TIMEOUT=$((FUZZ_DURATION + 120))

# Demo server configuration (in None policy)
export TEST_SERVER_XML_CONFIG=../../tests/ClientServer/data/config/S2OPC_Server_Demo_Config_Fuzzer.xml
export TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml
export TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml

# Save session logs/results
RESULTS_DIR="${S2OPC_DIR}/fuzzysully-results"
mkdir -p "${RESULTS_DIR}"
cd "${RESULTS_DIR}"


CMD_FILE="${RESULTS_DIR}/fuzzer_commands.txt"
# We save the last test case
LAST_FILE="${RESULTS_DIR}/LASTTESTCASE.log"
SERVER_LOGS="${BIN_DIR}/toolkit_demo_server_logs"
# Since the server keeps logs from previous runs, we create a marker to only read those produced by the current session
REPERE="${RESULTS_DIR}/run_start"
SUSPECTS="${RESULTS_DIR}/SUSPECTS.log"

compresser_resultats() {
    # Traces are only useful to investigate a suspect or a hang. Without anything to look at, they are not worth the hundreds of MB they weigh. So we delete them.
    if [ -s "${SUSPECTS}" ]; then
        gzip -f "${RESULTS_DIR}"/*_trace_fuzzer.txt 2>/dev/null
    else
        rm -f "${RESULTS_DIR}"/*_trace_fuzzer.txt
    fi
}


# We resume from the last test case reached in the previous fuzzing session, to avoid restarting from the first case indefinitely.
# To restart from the beginning (first testcase) you need to delete LASTTESTCASE.log
CURRENT_CASE=${FUZZ_FIRST_CASE:-1}
if [ -f "${LAST_FILE}" ]; then
    CURRENT_CASE="$(cat "${LAST_FILE}")"
    echo "Restart at case ${CURRENT_CASE}"
fi

# The fuzzer may die before the end of the allotted time (e.g. saturation of open file count in the terminal session). So we relaunch it from its last position until the timeout expires.
FIN=$(( $(date +%s) + FUZZ_DURATION ))
SESSION=0
ECHECS=0

while [ "$(date +%s)" -lt "${FIN}" ]; do
    SESSION=$((SESSION + 1))
    RESTANT=$(( FIN - $(date +%s) ))

    # The goto command of fuzzysully is slow (about 30s per 100000 cases). We therefore add this delay to the fuzzing session time to avoid cutting into the fuzzing itself
    DELAI_GOTO=$(( CURRENT_CASE / 100000 * 30 ))
    DUREE_SESSION=$(( RESTANT + DELAI_GOTO ))
    SERVER_TIMEOUT=$(( DUREE_SESSION + 120 ))
    
    # Command I pass to the fuzzer on its standard input. It positions itself on the first test case, then I launch execution
    echo "goto ${CURRENT_CASE}" > "${CMD_FILE}"
    echo "c" >> "${CMD_FILE}"

    touch "${REPERE}"
    echo "Session ${SESSION} : ${FUZZ_HOST}:${FUZZ_PORT} from ${CURRENT_CASE}, for ${DUREE_SESSION}s"

    # Here the sleep keeps the shell open because without it the fuzzer receives an end-of-file after its two commands (see fuzzysully/fuzzer_commands.txt)
    python3 "${WITH_SERVER}" --server-wd "${BIN_DIR}" --server-cmd ./toolkit_demo_server --wait-timeout ${SERVER_TIMEOUT} bash -c "{ cat ${CMD_FILE}; sleep ${DUREE_SESSION}; } | timeout ${DUREE_SESSION} fuzzysully server ${FUZZ_HOST} ${FUZZ_PORT} '' -st ${FUZZ_SEND_TIMEOUT} -rt ${FUZZ_RECV_TIMEOUT}" > "${RESULTS_DIR}/session_${SESSION}.log" 2>&1
    echo "Return code : $?"

    # The log file resulting from the session contains raw unreadable bytes. So we need to use the "strings" command to make it readable
    TRACE_BRUT="${RESULTS_DIR}/fuzzowski-results/OPCUA_${FUZZ_HOST}_${FUZZ_PORT}__TCP_server_None_all.log"
    if [ ! -f "${TRACE_BRUT}" ]; then
        echo "No trace produced by the fuzzer"
        ECHECS=$((ECHECS + 1))
        [ "${ECHECS}" -ge 3 ] && echo "Three sessions without progress, stopping" && exit 5
        continue
    fi

    TRACE="${RESULTS_DIR}/$(date +%y%m%d_%H%M%S)_trace_fuzzer.txt"
    strings "${TRACE_BRUT}" > "${TRACE}"
    if [ ! -s "${TRACE}" ]; then
        echo "Unreadable or empty trace, the raw log is kept"
        mv "${TRACE_BRUT}" "${RESULTS_DIR}/$(date +%y%m%d_%H%M%S)_trace_brute.log"
        exit 6
    fi
    rm -f "${TRACE_BRUT}"
    echo "Trace : ${TRACE}"
    rm -f "${RESULTS_DIR}/session_${SESSION}.log"

    # We note the last case reached so that the next launch resumes at the same place
    LAST="$(grep 'Test Case' "${TRACE}" | tail -n1 | grep -oP 'Test Case:\s*\K\d+(?=:)')"
    if [ -z "${LAST}" ] || [ "${LAST}" -le "${CURRENT_CASE}" ]; then
        echo "No progress on this session"
        ECHECS=$((ECHECS + 1))
        [ "${ECHECS}" -ge 3 ] && echo "Three sessions without progress, stopping" && exit 5
        continue
    fi

    ECHECS=0
    CURRENT_CASE="${LAST}"
    echo "Last case reached : ${CURRENT_CASE}"
    echo "${CURRENT_CASE}" > "${LAST_FILE}"

    ## Here we collect suspect cases: they do not fail the test but after several runs, we notice they are almost all due to monitor timeout and not a fuzzer bug or vulnerability
    grep 'as a suspect' "${TRACE}" | grep -oP 'Added test case \K\d+' >> "${SUSPECTS}"

    # Check for server hang via logs
    # When the server hangs, it no longer accepts any connection even after stopping the fuzzer.
    BLOCKED="$(find "${SERVER_LOGS}" -name '*.log' -newer "${REPERE}" -exec grep -l 'no more SCs available' {} \;)"
    if [ -n "${BLOCKED}" ]; then
        echo "The server hung during fuzzing, logs:"
        echo "${BLOCKED}"
        cp ${BLOCKED} "${RESULTS_DIR}/"
        exit 1
    fi
done

if [ -f "${SUSPECTS}" ]; then
    sort -u -n "${SUSPECTS}" -o "${SUSPECTS}"
    echo "Suspect cases : $(wc -l < "${SUSPECTS}")"
fi

echo "No server blockage detected."
exit 0
