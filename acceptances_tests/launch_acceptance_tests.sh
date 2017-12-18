LOG_FILE=log.txt

rm -f $LOG_FILE
/opt/opcfoundation/uactt/uacompliancetest.sh -s ./Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml --selection ./Acceptation_INGOPCS/Acceptation_INGOPCS_nonreg.selection.xml -h -c -r ./$LOG_FILE

echo -e "End of acceptance tests"

