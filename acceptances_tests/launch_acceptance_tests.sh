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
# Start a virtual display 99 to could run the acceptance test tools which use it
Xvfb :99 -screen 0 800x600x16 -ac &
# Retrieve PID to kill virtual display after running acceptance tool
XVFB_PID=$!
export DISPLAY=:99
#set -x

ROOT_DIR=$(cd $(dirname $0)/.. && pwd)
LOG_FILE=server_acceptance_tests.log
TMP_FILE=`mktemp`
TAP_FILE=server_acceptance_tests.tap
SELECTION=Acceptation_INGOPCS/Acceptation_INGOPCS.selection.xml
UACTT_ERROR_FILE=uactt_error.log
SERVER_ERROR=server_error.log


SKIPPED_TESTS_FILE=skipped_tests.cfg
KNOWN_BUGS_FILES=known_bugs.cfg

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

# check if all known bugs and skipped tests are disjoint
both_skipped_known_bugs=$(cat <(cut -d "|" -f 2- $KNOWN_BUGS_FILES) <(cut -d "|" -f 2- $SKIPPED_TESTS_FILE) | sort | uniq -d)
if [[ $both_skipped_known_bugs ]]
then
    echo "ERROR, the following lines are both in $KNOWN_BUGS_FILES and $SKIPPED_TESTS_FILE:"
    echo $both_skipped_known_bugs
    exit 1
fi

# main script

rm -f $LOG_FILE $TAP_FILE
echo "Launching server"
pushd ${ROOT_DIR}/build/bin
./toolkit_test_server 2> $SERVER_ERROR &
SERVER_PID=$!
# wait for server to be up
${ROOT_DIR}/tests/scripts/wait_server.py
popd

echo "Launching Acceptance Test Tool"

/opt/opcfoundation/uactt/uacompliancetest.sh -s ./Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml --selection $SELECTION -h -c -r ./$LOG_FILE 2>$UACTT_ERROR_FILE

echo "Closing Acceptance Test Tool"

# kill virtual display since not necessary anymore
kill -9 $XVFB_PID
# kill server not necessary anymore
kill $SERVER_PID
wait $SERVER_PID
mv ${ROOT_DIR}/build/bin/$SERVER_ERROR .

# test that log file is available
if [ ! -f $LOG_FILE ];then
    echo "UACTT log file hasn't been written."
fi


echo "End of acceptance tests - Generating Test Report"

# result analysis
# first step: analyse XML thanks to xlst
saxonb-xslt -xsl:analyse_log.xsl -s:$LOG_FILE -o:$TMP_FILE
# next step: build a tap report
num_tests=0

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

n_err=$(grep -c "^not ok" $TAP_FILE)
echo "There were $n_err not oks"

# check TAP file
mv $TAP_FILE ${ROOT_DIR}/build/bin/
${ROOT_DIR}/tests/scripts/check-tap.py ${ROOT_DIR}/build/bin/$TAP_FILE && echo "TAP file is well formed and free of failed tests" || exit 1

if grep -q '==' $SERVER_ERROR; then
    echo "ERROR: Asan issues detected"
    exit 2
fi



