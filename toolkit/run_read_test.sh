#!/bin/bash
WORKSPACE_DIR=.
if [[ -n $1 ]]; then
    EXEC_DIR=$1
else
    EXEC_DIR=$WORKSPACE_DIR/out
fi

cd $EXEC_DIR
./toolkit_test_read

# Fullfil TAP result
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: toolkit_test_read: Passed" > toolkit_read_result.tap
else
    echo "not ok 1 - test: toolkit_test_read: exit code '$?'" > toolkit_read_result.tap
fi
echo "1..1" >> toolkit_read_result.tap
