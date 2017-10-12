#!/bin/bash
WORKSPACE_DIR=.
if [[ -n $1 ]]; then
    EXEC_DIR=$1
else
    EXEC_DIR=$WORKSPACE_DIR/bin
fi

cd $EXEC_DIR
# Create script for running stub_server in background and store exit code
echo "#!/bin/bash
./toolkit_test_server
echo \$? > server.exitcode" > toolkit_test_server.sh
chmod +x toolkit_test_server.sh

## INGOPCS API VERSION
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./toolkit_test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./toolkit_test_client
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
sleep 5
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 1 - test: toolkit_test_client / toolkit_test_server: Passed" > client_server_toolkit_result.tap
else
    echo "not ok 1 - test: toolkit_test_client / toolkit_test_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" > client_server_toolkit_result.tap
fi
echo "1..1" >> client_server_toolkit_result.tap
# Clean created files
rm -f server.exitcode toolkit_test_server.sh
