#!/bin/bash
WORKSPACE_DIR=.
if [[ -n $1 ]]; then
    EXEC_DIR=$1
else
    EXEC_DIR=$WORKSPACE_DIR/out
fi

cd $EXEC_DIR
./toolkit_test_write

# Fullfil TAP result
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: toolkit_test_write: Passed" > toolkit_write_result.tap
else
    echo "not ok 1 - test: toolkit_test_write: exit code '$?'" > toolkit_write_result.tap
fi
echo "1..1" >> toolkit_write_result.tap
