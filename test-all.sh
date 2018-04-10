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

MY_DIR=$(cd $(dirname $0) && pwd)
BIN_DIR="${MY_DIR}/bin"
BUILD_DIR="${MY_DIR}/build"
VALIDATION_DIR="${MY_DIR}/validation"
TEST_DIR=${BUILD_DIR}
CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"

if [ ! -f "${CTEST_FILE}" ]; then
	TEST_DIR=${BIN_DIR}
	CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"
fi

if [ ! -f "${CTEST_FILE}" ]; then
	echo "No CTestTestfile in ${BIN_DIR} or ${BUILD_DIR}"
	echo "Is this a tagged release, or has CMake been run?"
	exit 1
fi

rm -f "${TEST_DIR}"/*.tap

cd "${TEST_DIR}" && ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536
CTEST_RET=$?

ls "${VALIDATION_DIR}"/*.tap >/dev/null 2>&1 && mv "${VALIDATION_DIR}"/*.tap "${BIN_DIR}"/

EXPECTED_TAP_FILES="check_helpers.tap
check_ingopcs_addspace.tap
check_sc_rcv_buffer.tap
check_sc_rcv_encrypted_buffer.tap
check_sockets.tap
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
toolkit_test_read.tap
toolkit_test_server_local_service.tap
toolkit_test_suite_client.tap
toolkit_test_write.tap
validation.tap"

ACTUAL_TAP_FILES=$(ls "${BIN_DIR}"/*.tap | sed "s|${BIN_DIR}/||")

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

${MY_DIR}/tests/scripts/check-tap ${BIN_DIR}/*.tap && echo "All TAP files are well formed and free of failed tests" || exit 1

exit $CTEST_RET
