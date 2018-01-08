#!/bin/bash
IFS=$'\n\t'
export DISPLAY=:0.0
#set -x

LOG_FILE=server_acceptance_tests.log
TMP_FILE=`mktemp`
TAP_FILE=server_acceptance_tests.tap
#SELECTION=./Acceptation_INGOPCS/Acceptation_INGOPCS_nonreg.selection.xml
SELECTION=Acceptation_INGOPCS/Acceptation_INGOPCS.selection.xml
UACTT_ERROR_FILE=uactt_error.log


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

    if [ "$desc_file" != "" ]
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

# main script

rm -f $LOG_FILE $TAP_FILE
echo -e "Launching server"
pushd ../bin
./toolkit_test_server 200000&
popd

echo -e "Launching Acceptance Test Tool"

/opt/opcfoundation/uactt/uacompliancetest.sh -s ./Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml --selection $SELECTION -h -c -r ./$LOG_FILE 2>$UACTT_ERROR_FILE&

wait

# test that log file is available
if [ ! -f $LOG_FILE ];then
    echo "UACTT log file hasn't been written."
fi


echo -e "End of acceptance tests - Generating Test Report"

# result analysis
# first step: analyse XML thanks to xlst
saxonb-xslt -xsl:analyse_log.xsl -s:$LOG_FILE -o:$TMP_FILE
# next step: build a tap report
NB_TESTS=`wc -l $TMP_FILE | awk '{print $1}'`
num_tests=0

echo "1..$NB_TESTS" > $TAP_FILE

while read line; do
    let 'num_tests++'
    test_status=`echo $line | awk -F '-' '{print $1}'`
    test_number=`echo $line | awk -F '-' '{print $2}'`
    test_description=`echo $line | awk -F '-' '{print $3}'`

    case $test_status in
        "Error")
        # is it a known bug ?
        if check_test $KNOWN_BUGS_FILES $test_description
        then
            echo "ok - $num_tests - $test_number $test_description - KNOWN BUG" >> $TAP_FILE
        else
        #otherwise log an error in tap report
            echo "not ok - $num_tests - $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Warning")
        echo "ok - $num_tests - $test_number $test_description - $test_status" >> $TAP_FILE
        ;;

        "Not implemented")
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE $test_description
        then
            echo "ok - $num_tests - $test_number $test_description - SKIPPED TEST" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok - $num_tests - $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Skipped")
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE $test_description
        then
            echo "ok - $num_tests - $test_number $test_description - SKIPPED TEST" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok - $num_tests - $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Not supported")
        # is it a skipped test ?
        if check_test $SKIPPED_TESTS_FILE $test_description
        then
            echo "ok - $num_tests - $test_number $test_description - SKIPPED TEST" >> $TAP_FILE
        else
        # otherwise log an error in tap report
            echo "not ok - $num_tests - $test_number $test_description - $test_status" >> $TAP_FILE
        fi
        ;;

        "Ok")
        # was it a known bug ?
        if check_test $KNOWN_BUGS_FILES $test_description
        then
            echo "ok - $num_tests - $test_number $test_description - FIXED KNOWN BUG" >> $TAP_FILE
        else
        #otherwise log ok in tap report
            echo "ok - $num_tests - $test_number $test_description - " >> $TAP_FILE
        fi
        ;;
        \?)
        abort "Invalid status: $test_status";;

    esac

done < $TMP_FILE

rm -f $LOG_FILE
rm -f $TMP_FILE

echo -e "Test report generated"
