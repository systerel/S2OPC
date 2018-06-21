#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


#  Check toolkit tests binaries are present and run them

MY_DIR=$(cd $(dirname $0) && pwd)
BIN_DIR="${MY_DIR}/bin"
BUILD_DIR="${MY_DIR}/build"
VALIDATION_DIR="${MY_DIR}/validation"
TEST_DIR=${BUILD_DIR}
CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"
TAP_DIR="${BUILD_DIR}/bin"

if [ ! -f "${CTEST_FILE}" ]; then
	TEST_DIR=${BIN_DIR}
	CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"
	TAP_DIR="${BIN_DIR}"
fi

if [ ! -f "${CTEST_FILE}" ]; then
	echo "No CTestTestfile in ${BIN_DIR} or ${BUILD_DIR}"
	echo "Is this a tagged release, or has CMake been run?"
	exit 1
fi

rm -f "${TAP_DIR}"/*.tap

cd "${TEST_DIR}" && ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536
CTEST_RET=$?

ls "${VALIDATION_DIR}"/*.tap >/dev/null 2>&1 && mv "${VALIDATION_DIR}"/*.tap "${TAP_DIR}"/

EXPECTED_TAP_FILES="check_helpers.tap
check_libsub.tap
check_sc_rcv_buffer.tap
check_sc_rcv_encrypted_buffer.tap
check_sockets.tap
client_server_test.tap
sc_establish_timeout.tap
sc_renew.tap
secure_channel_level_None.tap
secure_channel_level_SignAndEncrypt_B256Sha256_2048bit.tap
secure_channel_level_SignAndEncrypt_B256Sha256_2048bit_server_vs_4096bit_client.tap
secure_channel_level_SignAndEncrypt_B256Sha256_4096bit.tap
secure_channel_level_SignAndEncrypt_B256Sha256_4096bit_server_vs_2048bit_client.tap
secure_channel_level_Sign_B256Sha256_2048bit.tap
secure_channel_level_Sign_B256_2048bit.tap
session_timeout.tap
toolkit_test_server_local_service.tap
toolkit_test_suite_client.tap
validation.tap"

ACTUAL_TAP_FILES=$(LANG=C ls "${TAP_DIR}"/*.tap | sed "s|${TAP_DIR}/||")

if [ "$ACTUAL_TAP_FILES" != "$EXPECTED_TAP_FILES" ]; then
	echo "Missing or extra TAP files detected"
	echo
	echo "List of expected TAP files:"
	echo "$EXPECTED_TAP_FILES"
	echo
	echo "Actual list of TAP files:"
	echo "$ACTUAL_TAP_FILES"

	exit 1
fi

${MY_DIR}/tests/scripts/check-tap.py ${TAP_DIR}/*.tap && echo "All TAP files are well formed and free of failed tests" || exit 1

exit $CTEST_RET
