#!/bin/bash

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


#  Check toolkit tests binaries are present and run them
set -e
BIN_DIR=./bin
TEST_SCRIPTS=./tests/scripts

# Check binaries are present Build
echo "Check test binaries are present"
if [ -f "$BIN_DIR/check_helpers" ] &&
   [ -f "$BIN_DIR/check_sockets" ] &&
   [ -f "$BIN_DIR/test_secure_channels_server" ] &&
   [ -f "$BIN_DIR/test_secure_channels_client" ] &&
   [ -f "$BIN_DIR/toolkit_test_read" ] &&
   [ -f "$BIN_DIR/toolkit_test_write" ] &&
   [ -f "$BIN_DIR/toolkit_test_server_local_service" ] &&
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
$TEST_SCRIPTS/run_client_server_test_SC_level.sh

# run services tests
## unitary service tests
$BIN_DIR/toolkit_test_read
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: read service test: Passed" > $BIN_DIR/service_read.tap
else
    echo "not ok 1 - test: read service test: $?" > $BIN_DIR/service_read.tap
fi
echo "1..1" >> $BIN_DIR/service_read.tap

$BIN_DIR/toolkit_test_write
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: write service test: Passed" > $BIN_DIR/service_write.tap
else
    echo "not ok 1 - test: write service test: $?" > $BIN_DIR/service_write.tap
fi
echo "1..1" >> $BIN_DIR/service_write.tap

pushd $BIN_DIR
./toolkit_test_server_local_service
if [[ $? -eq 0 ]]; then
    echo "ok 1 - test: server local service test: Passed" > ./server_local_service.tap
else
    echo "not ok 1 - test: server local service test: $?" > ./server_local_service.tap
fi
echo "1..1" >> ./server_local_service.tap
popd

## run toolkit client / server test
$TEST_SCRIPTS/run_client_server_test.sh

## run validation tests
pushd $BIN_DIR
./toolkit_test_server&
popd
pushd validation
./client.py&
popd
wait
mv validation/validation.tap bin/

## run validation tests
pushd $BIN_DIR
./toolkit_test_server 20000&
popd
pushd validation
./client_sc_renew.py&
popd
wait
mv validation/sc_renew.tap bin/

## run validation tests
pushd $BIN_DIR
./toolkit_test_server&
popd
pushd validation
./client_session_timeout.py&
popd
wait
mv validation/session_timeout.tap bin/
