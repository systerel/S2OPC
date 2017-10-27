#!/bin/bash
# Script to check properties on code of the INGOPCS project:

CSRC=csrc

LOGPATH=pre-build-check.log

# 
CHECK_ABSENCE="(restrict|fgets|fgetws|getc|putc|getwc|putwc|fsetpos|rand|readlink|vfork|putenv|lstat|setuid|setgid|getuid|getgid|seteuid|geteuid|fork|pthread_kill|pthread_cancel|pthread_exit|signal.h|stdarg.h)"

# Redirect all output and errors to log file
echo "Pre-build-check log" > $LOGPATH

echo "Clang automatic formatting check" | tee -a $LOGPATH
find $CSRC -name "*.c" -or -name "*.h" | xargs clang-format -style=file -i >> $LOGPATH
ALREADY_FORMAT=`git ls-files -m $CSRC`

if [[ -z $ALREADY_FORMAT ]]; then
    echo "C source code formatting already done" | tee -a $LOGPATH
else
    echo "ERROR: C source code code formatting not done or not committed" | tee -a $LOGPATH
    exit 1
fi

echo "Checking specific functions or headers not used in code" | tee -a $LOGPATH
find $CSRC -name "*.c" -or -name "*.h" | xargs grep -wiEc $CHECK_ABSENCE | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of functions or headers: $CHECK_ABSENCE" | tee -a $LOGPATH
    exit 1
fi

echo "Terminated with SUCCESS" | tee -a $LOGPATH
