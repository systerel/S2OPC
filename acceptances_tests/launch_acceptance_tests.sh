#!/bin/bash
IFS=$'\n\t'

LOG_FILE=server_acceptance_tests.log
TMP_FILE=`mktemp`
TAP_FILE=server_acceptance_tests.tap
#SELECTION=./Acceptation_INGOPCS/Acceptation_INGOPCS_nonreg.selection.xml
SELECTION=Acceptation_INGOPCS/Acceptation_INGOPCS.selection.xml

rm -f $LOG_FILE $TAP_FILE
echo -e "Launching Acceptance Test Tool"

/opt/opcfoundation/uactt/uacompliancetest.sh -s ./Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml --selection $SELECTION -h -c -r ./$LOG_FILE 2>/dev/null

echo -e "End of acceptance tests - Generating Test Report"

saxonb-xslt -xsl:analyse_log.xsl -s:$LOG_FILE -o:$TMP_FILE

NB_TESTS=`wc -l $TMP_FILE | awk '{print $1}'`

echo "1..$NB_TESTS" > $TAP_FILE
cat $TMP_FILE >> $TAP_FILE

rm -f $LOG_FILE
rm -f $TMP_FILE

echo -e "Test report generated"
