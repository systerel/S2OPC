#!/bin/bash
# Script to check properties on code of the INGOPCS project:

CSRC=csrc

LOGPATH=pre-build-check.log

# 
CHECK_ABSENCE="(restrict|fgets|fgetws|getc|putc|getwc|putwc|fsetpos|rand|readlink|vfork|putenv|lstat|setuid|setgid|getuid|getgid|seteuid|geteuid|fork|pthread_kill|pthread_cancel|pthread_exit|signal.h|stdarg.h)"

# Redirect all output and errors to log file
echo "Pre-build-check log" > $LOGPATH

echo "Checking specific functions or headers not used in code" | tee -a $LOGPATH
find $CSRC -name "*.c" -or -name "*.h" | xargs grep -wiEc $CHECK_ABSENCE | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of functions or headers: $CHECK_ABSENCE" | tee -a $LOGPATH
    exit 1
fi

echo "Checking specific CERT rules using clang-tidy tool"
# CERT rules to verify
CERT_RULES=cert-flp30-c,cert-fio38-c,cert-env33-c,cert-err34-c,cert-msc30-c
# Define include directories
SRC_DIRS=(`find $CSRC -not -path "*windows*" -type d`)
SRC_INCL=${SRC_DIRS[@]/#/-I}
CLANG_TIDY_LOG=cert_rules_clang_tidy.log
# Run clang-tidy removing default checks (-*) and adding CERT rules verification
find $CSRC -not -path "*windows*" -name "*.c" -or -not -path "*windows*" -name "*.h" -exec clang-tidy {} -checks=-*,$CERT_RULES -- $SRC_INCL \; &> $CLANG_TIDY_LOG
# Check if resulting log contains error or warnings 
grep -wiEc "(error|warning)" cert_rules_clang_tidy.log | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking CERT rules $CERT_RULES with clang-tidy: see log $CLANG_TIDY_LOG" | tee -a $LOGPATH
    exit 1
else
    \rm $CLANG_TIDY_LOG
fi


echo "Clang automatic formatting check" | tee -a $LOGPATH
find $CSRC -name "*.c" -or -name "*.h" | xargs clang-format -style=file -i >> $LOGPATH
ALREADY_FORMAT=`git ls-files -m $CSRC`

if [[ -z $ALREADY_FORMAT ]]; then
    echo "C source code formatting already done" | tee -a $LOGPATH
else
    echo "ERROR: C source code code formatting not done or not committed" | tee -a $LOGPATH
    exit 1
fi

echo "Terminated with SUCCESS" | tee -a $LOGPATH
