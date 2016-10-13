#!/bin/bash
WORKSPACE_DIR=.
EXEC_DIR=$WORKSPACE_DIR/out

cd $EXEC_DIR
# Create script for running stub_server in background and store exit code
echo "#!/bin/bash
./stub_server
echo \$? > server.exitcode" > test_server.sh
chmod +x test_server.sh

# Execute server side of the test
./test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./stub_client
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
sleep 10
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 1 - test: stub_client / stub_server: Passed" > client_server_result.tap
else
    echo "not ok 1 - test: stub_client / stub_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" > client_server_result.tap
fi
echo "1..1" >> client_server_result.tap
# Clean created files
rm -f server.exitcode test_server.sh
