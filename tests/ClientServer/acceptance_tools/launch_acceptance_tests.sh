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

IFS=$'\n\t'
# Start a virtual display 99 to run the acceptance test tools which use it
Xvfb :99 -screen 0 800x600x16 -ac &
# Retrieve PID to kill virtual display after running acceptance tool
XVFB_PID=$!
export DISPLAY=:99
#set -x

ROOT_DIR=$(cd $(dirname $0)/../../.. && pwd)
LOG_FILE=server_acceptance_tests.log
TMP_FILE=`mktemp`
TAP_FILE=server_acceptance_tests.tap
UACTT_ERROR_FILE=uactt_error.log
SERVER_ERROR=server_error.log

# Maximum duration of the UACTT run: kill it from inside the script so that logs
# can still be collected and analyzed afterwards. Must remain below the CI caps
# (RUNNER_SCRIPT_TIMEOUT=50m / job timeout=1h), otherwise the runner kills the
# whole script and no artifact is uploaded at all.
UACTT_TIMEOUT=${UACTT_TIMEOUT:-45m}


SKIPPED_TESTS_FILE=skipped_tests.cfg
KNOWN_BUGS_FILES=known_bugs.cfg

S2OPC_UACTT_CONFIG=Acceptation_S2OPC
S2OPC_CERTS_CONFIG=../../../samples/ClientServer/data/cert
UACTT_PROJECT_PATH=/UACTT_project
CONFIGURATION=$UACTT_PROJECT_PATH/Acceptation_S2OPC.ctt.xml

NODE_MGT_SELECTION_FILE=Acceptation_S2OPC.node_mgt.selection.xml

if [[ -z "$S2OPC_NANO_PROFILE" ]] || [[ $S2OPC_NANO_PROFILE -eq  0 ]]
then
    if [[ -z "$S2OPC_NODE_MANAGEMENT" ]] || [[ $S2OPC_NODE_MANAGEMENT -eq 0 ]]
    then
        TOOLKIT_TEST_SERVER=./toolkit_test_server
        SELECTION=$UACTT_PROJECT_PATH/Acceptation_S2OPC.selection.xml    
    else
        TOOLKIT_TEST_SERVER=./toolkit_test_server
        SELECTION=$UACTT_PROJECT_PATH/$NODE_MGT_SELECTION_FILE
    fi
else
    TOOLKIT_TEST_SERVER=./toolkit_test_nano_server
    SELECTION=$UACTT_PROJECT_PATH/Acceptation_S2OPC.nano.selection.xml
    CONFIGURATION=$UACTT_PROJECT_PATH/Acceptation_S2OPC.nano.ctt.xml
fi

# Allow a CI job to override the default UACTT selection
if [[ -n "${S2OPC_UACTT_SELECTION_FILE:-}" ]]
then
    SELECTION=$UACTT_PROJECT_PATH/$S2OPC_UACTT_SELECTION_FILE
fi

# Copy the local S2OPC UACTT configuration files to the UACTT project (overwrite ones in docker)
echo "Copy the S2OPC repository configuration files into UACTT project (overwrite)"
pushd $S2OPC_UACTT_CONFIG/
./gen_other_configs.sh
popd
cp -v $S2OPC_UACTT_CONFIG/* $UACTT_PROJECT_PATH/
if [ $? -ne 0 ]
then
    echo "Copy failed"
    exit -1
fi

# Copy the CA of S2OPC to UACTT certificates
echo "Copy the S2OPC CA certificates into UACTT PKI configuration"
cp -v $S2OPC_CERTS_CONFIG/cacert.der $UACTT_PROJECT_PATH/PKI/CA/certs/
if [ $? -ne 0 ]
then
    echo "Copy failed"
    exit -1
fi
cp -v $S2OPC_CERTS_CONFIG/cacrl.der $UACTT_PROJECT_PATH/PKI/CA/crl/
if [ $? -ne 0 ]
then
    echo "Copy failed"
    exit -1
fi

# this function takes two arguments:
# - one test description file
# - one test name
# It returns 0 if test name is contained into test
# description file and 1 otherwise.
function check_test {

    local desc_file=$1
    local test_name=$2
    test_name=`echo $test_name | sed -e  "s/\[.*//"`

    if [[ ("$desc_file" != "" ) && ("$test_name" != "") ]]
    then
	grep "$test_name" $desc_file > /dev/null
	grep_status=$?
    else
	grep_status=1
    fi

    if [ $grep_status -ne 0 ]
    then
	return 1
    else
	return 0
    fi

}

function server_has_sanitizer_error {
    grep -Eq "ERROR: (Address|Leak|Memory|Thread|UndefinedBehavior)Sanitizer|runtime error:|SUMMARY: .*Sanitizer" "$SERVER_ERROR"
}

# check if all known bugs and skipped tests are disjoint
both_skipped_known_bugs=$(cat <(cut -d "|" -f 2- $KNOWN_BUGS_FILES) <(cut -d "|" -f 2- $SKIPPED_TESTS_FILE) | sort | uniq -d)
if [[ $both_skipped_known_bugs ]]
then
    echo "WARNING, the following lines are both in $KNOWN_BUGS_FILES and $SKIPPED_TESTS_FILE:"
    echo $both_skipped_known_bugs
fi

# main script

rm -f $LOG_FILE $TAP_FILE
echo "Launching server $TOOLKIT_TEST_SERVER"
pushd ${ROOT_DIR}/build/bin
./$TOOLKIT_TEST_SERVER 2> $SERVER_ERROR &
SERVER_PID=$!
# wait for server to be up
${ROOT_DIR}/tests/ClientServer/scripts/wait_server.py
popd

# Heartbeat in CI trace: the UACTT only writes its result file at the end of the
# run, so without this the job trace stays silent for the whole run and a killed
# job gives no clue of where it was stuck (nor whether the server died meanwhile).
(
    while sleep 60
    do
        if kill -0 $SERVER_PID 2> /dev/null
        then
            server_state="alive"
        else
            server_state="DEAD"
        fi
        echo "[heartbeat] $(date '+%Y-%m-%d %H:%M:%S') server: $server_state - UACTT stderr: $(wc -c 2> /dev/null < $UACTT_ERROR_FILE || echo 0) bytes - last line: $(tail -n 1 $UACTT_ERROR_FILE 2> /dev/null)"
    done
) &
HEARTBEAT_PID=$!

if [[ -z "$LINUX_UACTT" ]] || [[ "$LINUX_UACTT" -eq 0 ]]
then
    mkdir -p /tmp/wineprefix
    export WINEPREFIX="/tmp/wineprefix"
    export WINEARCH="win32"
    # Run windows UACTT with wine: see <uactt>/help/index.htm#t=command_line_interface.htm for parameters
    echo "Launching Acceptance Test Tool: wine /opt/uactt/uacompliancetest.exe --settings $(winepath -w $CONFIGURATION) --selection $(winepath -w $SELECTION) --hidden --close --result $(winepath -w ./$LOG_FILE) 2>$UACTT_ERROR_FILE"
    timeout --signal=TERM --kill-after=30s $UACTT_TIMEOUT wine /opt/uactt/uacompliancetest.exe --settings $(winepath -w $CONFIGURATION) --selection $(winepath -w $SELECTION) --hidden --close --result $(winepath -w ./$LOG_FILE) 2>$UACTT_ERROR_FILE
else
    # Run linux UACTT
    echo "Launching Acceptance Test Tool: uacompliancetest -s $CONFIGURATION --selection $SELECTION -h -c -r ./$LOG_FILE 2>$UACTT_ERROR_FILE"
    timeout --signal=TERM --kill-after=30s $UACTT_TIMEOUT /opt/opcfoundation/uactt/bin/uacompliancetest -s $CONFIGURATION --selection $SELECTION -h -c -r ./$LOG_FILE 2>$UACTT_ERROR_FILE
fi
UACTT_STATUS=$?

echo "Closing Acceptance Test Tool (exit status: $UACTT_STATUS)"
# timeout(1) exits with 124 on timeout, 137 (128+SIGKILL) if --kill-after was needed
if [ $UACTT_STATUS -eq 124 ] || [ $UACTT_STATUS -eq 137 ]
then
    echo "ERROR: UACTT did not terminate within $UACTT_TIMEOUT and was killed"
fi

# kill heartbeat and virtual display since not necessary anymore
kill $HEARTBEAT_PID 2> /dev/null
kill -9 $XVFB_PID
# kill server not necessary anymore (2 times to avoid OPCUA shutdown phase)
kill $SERVER_PID
kill $SERVER_PID
wait $SERVER_PID
mv ${ROOT_DIR}/build/bin/$SERVER_ERROR .

# test that log file is available: the UACTT only writes it on normal termination,
# so it is missing when the UACTT was killed on timeout or crashed
if [ ! -f $LOG_FILE ];then
    echo "ERROR: UACTT log file hasn't been written: no test report can be generated."
    echo "Last lines of $UACTT_ERROR_FILE:"
    tail -n 20 $UACTT_ERROR_FILE
    if server_has_sanitizer_error; then
        echo "ERROR: sanitizer issue detected in server stderr ($SERVER_ERROR)"
        exit 2
    fi
    exit 3
fi


echo "End of acceptance tests - Generating Test Report"

# result analysis
# first step: analyse XML thanks to xlst
saxonb-xslt -xsl:analyse_log.xsl -s:$LOG_FILE -o:$TMP_FILE
# next step: build a tap report
num_tests=0

# Note: codes corresponding to result in XML (.log here)
# 0: Fail/Error
# 1: Warning
# 2: Not Implemented
# 3: Skipped
# 4: Not Supported
# 5: OK / Log
# 6: Back-trace

while read line; do
    test_status=`echo $line | awk -F '|' '{print $1}'`
    test_number=`echo $line | awk -F '|' '{print $2}'`
    test_description=`echo $line | awk -F '|' '{print $3}'`

    case $test_status in
        "Error")
        let 'num_tests++'
        # is it a known bug ?
        if check_test $KNOWN_BUGS_FILES "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description # TODO Known bug" >> $TAP_FILE
        else
        #otherwise log an error in tap report
            echo "not ok $num_tests $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Warning")
        let 'num_tests++'
        echo "ok $num_tests $test_number $test_description - $test_status" >> $TAP_FILE
        ;;

        "Not implemented")
        let 'num_tests++'
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description # skip Known bug" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok $num_tests $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Skipped")
        let 'num_tests++'
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description # skip Known bug" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok $num_tests $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Not supported")
        let 'num_tests++'
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description # skip Known bug" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok $num_tests $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Ok")
        let 'num_tests++'
        # was it a known bug ?
        if check_test $KNOWN_BUGS_FILES "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description - FIXED KNOWN BUG" >> $TAP_FILE
        elif check_test $SKIPPED_TESTS_FILE "${test_number}|${test_description}"
        then
            echo "ok $num_tests $test_number $test_description - NEW SUPPORTED / NEW NO SKIP" >> $TAP_FILE
        else
        #otherwise log ok in tap report
            echo "ok $num_tests $test_number $test_description" >> $TAP_FILE
        fi
        ;;

    esac

done < $TMP_FILE

# add number of tests at the beginning of TAP file
sed -i "1s/^/1..$num_tests\n/" $TAP_FILE

rm -f $TMP_FILE

echo "Test report generated"

n_ok=$(grep -c "^ok" $TAP_FILE)
n_err=$(grep -c "^not ok" $TAP_FILE)
echo "
-----------------------------------"
if [ $n_err -ne 0 ]
then
    echo "TESTS KO:"
    grep "^not ok" $TAP_FILE
else
    echo "Remaining known bugs:"
    grep "TODO Known bug" $TAP_FILE
    echo "-----------------------------------"
    echo "FIXED known bugs / skipped tests:"
    grep "FIXED" $TAP_FILE
    grep "NEW SUPPORTED" $TAP_FILE
fi
echo "-----------------------------------
"
echo "
There were $n_ok OKs and $n_err KOs.
"
echo "-----------------------------------"

# check TAP file
mv $TAP_FILE ${ROOT_DIR}/build/bin/
${ROOT_DIR}/tests/scripts/check-tap.py ${ROOT_DIR}/build/bin/$TAP_FILE && echo "TAP file is well formed and free of failed tests" || exit 1

if server_has_sanitizer_error; then
    echo "ERROR: sanitizer issue detected in server stderr ($SERVER_ERROR)"
    exit 2
fi
