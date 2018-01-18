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

WORKSPACE_DIR=.
if [[ -n $1 ]]; then
    EXEC_DIR=$1
else
    EXEC_DIR=$WORKSPACE_DIR/bin
fi

cd $EXEC_DIR
# Create script for running stub_server in background and store exit code
echo "#!/bin/bash
./test_secure_channels_server \$1
echo \$? > server.exitcode" > test_server.sh
chmod +x test_server.sh

echo ""
echo "======================================================================"
echo "Secure channels tests level: SignAndEncrypt B256Sha256 2048 key length"
echo "======================================================================"

## SC tests: SignAndEncrypt
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 1 - test: test_secure_channels_client / test_secure_channels_server: Passed" > client_server_result.tap
else
    echo "not ok 1 - test: test_secure_channels_client / test_secure_channels_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" > client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: Sign B256Sha256 2048 key length"
echo "======================================================================"

## SC tests: Sign
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client sign
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 2 - test: test_secure_channels_client sign / test_secure_channels_server: Passed" >> client_server_result.tap
else
    echo "not ok 2 - test: test_secure_channels_client sign / test_secure_channels_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: None"
echo "======================================================================"

## SC tests: None
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client none
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 3 - test: test_secure_channels_client none / test_secure_channels_server: Passed" >> client_server_result.tap
else
    echo "not ok 3 - test: test_secure_channels_client none / test_secure_channels_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: Sign B256 2048 key length"
echo "======================================================================"

## SC tests: SignAndEncrypt B256
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client encrypt B256
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 4 - test: test_secure_channels_client encrypt B256 / test_secure_channels_server: Passed" >> client_server_result.tap
else
    echo "not ok 4 - test: test_secure_channels_client encrypt B256 / test_secure_channels_server exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: SignAndEncrypt B256Sha256 4096 key length"
echo "======================================================================"

## SC tests: SignAndEncrypt B256Sha256 4096 key length
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh 4096 &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client encrypt B256Sha256 4096
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 5 - test: test_secure_channels_client encrypt B256Sha256 4096 / test_secure_channels_server 4096: Passed" >> client_server_result.tap
else
    echo "not ok 5 - test: test_secure_channels_client encrypt B256Sha256 4096 / test_secure_channels_server 4096 exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: SignAndEncrypt B256Sha256 4096 server vs 2048 client"
echo "======================================================================"

## SC tests: SignAndEncrypt B256Sha256 4096 key length
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh 4096 &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client encrypt B256Sha256 2048 4096
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 6 - test: test_secure_channels_client encrypt B256Sha256 2048 / test_secure_channels_server 4096: Passed" >> client_server_result.tap
else
    echo "not ok 6 - test: test_secure_channels_client encrypt B256Sha256 2048 / test_secure_channels_server 4096 exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo ""
echo "======================================================================"
echo "Secure channels tests level: SignAndEncrypt B256Sha256 2048 server vs 4096 client"
echo "======================================================================"

## SC tests: SignAndEncrypt B256Sha256 4096 key length
# remove precedent exit code file
rm -f server.exitcode
# Execute server side of the test
./test_server.sh 2048 &
sleep 1 # Wait server started

# Execute client side of the test and retrieve exit code
./test_secure_channels_client encrypt B256Sha256 4096 2048
CLIENT_EXITCODE="$?"
# Wait end of server side execution and retrieve exit code
wait
SERVER_EXITCODE=`cat server.exitcode`

# Fullfil TAP result
if [[ $CLIENT_EXITCODE -eq 0 && $SERVER_EXITCODE -eq 0 ]]; then
    echo "ok 7 - test: test_secure_channels_client encrypt B256Sha256 4096 / test_secure_channels_server 2048: Passed" >> client_server_result.tap
else
    echo "not ok 7 - test: test_secure_channels_client encrypt B256Sha256 4096 / test_secure_channels_server 2048 exit codes: $CLIENT_EXITCODE / $SERVER_EXITCODE" >> client_server_result.tap
fi

echo "1..7" >> client_server_result.tap

# Clean created files
rm -f server.exitcode test_server.sh

