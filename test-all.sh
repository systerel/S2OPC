#!/bin/bash
#  Check toolkit tests binaries are present and run them
set -e
BIN_DIR=bin

# Check binaries are present Build
echo "Check test binaries are present"
if [ -f "$BIN_DIR/check_helpers" ] &&
   [ -f "$BIN_DIR/check_sockets" ] &&
   [ -f "$BIN_DIR/test_secure_channels_server" ] &&
   [ -f "$BIN_DIR/test_secure_channels_client" ] &&
   [ -f "$BIN_DIR/toolkit_test_read" ] &&
   [ -f "$BIN_DIR/toolkit_test_write" ] &&
   [ -f "$BIN_DIR/toolkit_test_server" ] &&
   [ -f "$BIN_DIR/toolkit_test_client" ]
then
    echo "Test binaries found"
else
    echo "Test binary missing"
    exit 1
fi

# run helpers tests
export CK_TAP_LOG_FILE_NAME=$BIN_DIR/helpers.tap && $BIN_DIR/check_helpers
# run sockets test
export CK_TAP_LOG_FILE_NAME=$BIN_DIR/sockets.tap && $BIN_DIR/check_sockets
# run secure channels client / server test
./run_client_server_test_SC_level.sh

# run services tests
## unitary service tests
./bin/toolkit_test_read
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: read service test: Passed" > $BIN_DIR/service_read.tap
else
    echo "not ok 1 - test: read service test: $?" > $BIN_DIR/service_read.tap
fi
echo "1..1" >> $BIN_DIR/service_read.tap

./bin/toolkit_test_write
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: write service test: Passed" > $BIN_DIR/service_write.tap
else
    echo "not ok 1 - test: write service test: $?" > $BIN_DIR/service_write.tap
fi
echo "1..1" >> $BIN_DIR/service_write.tap

## run toolkit cilent / server test
./run_client_server_test.sh
