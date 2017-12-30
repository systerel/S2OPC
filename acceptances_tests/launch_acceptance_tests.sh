#!/bin/bash
set -x

IFS=$'\n\t'
DEFAULT_PROJECT_LINE="<project>"

LOG_FILE=server_acceptance_tests.log
TMP_FILE=`mktemp`
TAP_FILE=server_acceptance_tests.tap

rm -f $LOG_FILE $TAP_FILE
echo -e "Launching Acceptance Test Tool"

/opt/opcfoundation/uactt/uacompliancetest.sh -s ./Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml --selection ./Acceptation_INGOPCS/Acceptation_INGOPCS_nonreg.selection.xml -h -c -r ./$LOG_FILE 2>/dev/null

echo -e "End of acceptance tests - Generating Test Report"

saxonb-xslt -xsl:analyse_log.xsl -s:$LOG_FILE -o:$TMP_FILE

NB_TESTS=`wc -l $TMP_FILE | awk '{print $1}'`

echo "1..$NB_TESTS" > $TAP_FILE
cat $TMP_FILE >> $TAP_FILE

rm -f $LOG_FILE
rm -f $TMP_FILE

echo -e "Test report generated"
