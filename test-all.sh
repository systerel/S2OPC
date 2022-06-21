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

MY_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="${MY_DIR}/bin"
BUILD_DIR="${MY_DIR}/build"
PYS2OPC_TESTS_DIR="${MY_DIR}/tests/ClientServer/validation_tests/pys2opc"
PYS2OPC_LIB_IS_PRESENT=$(ls ${BUILD_DIR}/lib/pys2opc*.whl 2> /dev/null | wc -l)
CLIENTSERVER_TEST_DIR=${BUILD_DIR}/tests/ClientServer
CLIENTSERVER_CTEST_FILE="${CLIENTSERVER_TEST_DIR}/CTestTestfile.cmake"
PUBSUB_TEST_DIR=${BUILD_DIR}/tests/PubSub
PUBSUB_CTEST_FILE="${PUBSUB_TEST_DIR}/CTestTestfile.cmake"
TAP_DIR="${BUILD_DIR}/bin"

CLIENTSERVER_TAP_FILES='check_helpers.tap
check_libsub.tap
check_security_policy_config.tap
check_sc_rcv_buffer.tap
check_sc_rcv_encrypted_buffer.tap
check_sockets.tap
check_wrapper.tap
client_server_test.tap
client_service_faults_test.tap
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
toolkit_test_server_client.tap
toolkit_test_server_local_service.tap
toolkit_test_suite_client.tap
validation.tap'

PUBSUB_TAP_FILES='interop_pub_test.tap
interop_sub_test.tap
ll_multi_pub_sub_test.tap
ll_pub_sub_conf_test.tap
ll_pub_sub_test.tap
ll_pub_sub_xml_test.tap
pubsub_scheduler_udp.tap
pubsub_sched_ethernet.tap
pubsub_sched_ethernet_with_recv_itf.tap
pubsub_sched_ethernet_pub_config_err.tap
pubsub_sched_ethernet_sub_config_err.tap
pubsub_modules_test.tap
xml_parser_test.tap'

PUBSUB_CLIENTSERVER_TAP_FILES='pubsub_server_test.tap'


PYS2OPC_TAP_FILES=$'\nvalidation_pys2opc.tap'

rm -f "${TAP_DIR}"/*.tap

if [ -z $S2OPC_PUBSUB_ONLY ]; then
   if [ ! -f "${CLIENTSERVER_CTEST_FILE}" ]; then
       CLIENTSERVER_TEST_DIR=${BIN_DIR}/ClientServer
       CLIENTSERVER_CTEST_FILE="${CLIENTSERVER_TEST_DIR}/CTestTestfile.cmake"
       TAP_DIR="${BIN_DIR}"
       sed -i "s|S2OPC_ROOT_DIR|${MY_DIR}|g" $CLIENTSERVER_CTEST_FILE
   fi

   if [ ! -f "${CLIENTSERVER_CTEST_FILE}" ]; then
       echo "No CTestTestfile in ${BIN_DIR}/ClientServer or ${BUILD_DIR}/tests/ClientServer"
       echo "Is this a tagged release, or has CMake been run?"
       exit 1
   fi

   cd "${CLIENTSERVER_TEST_DIR}"
   if [ "$PYS2OPC_LIB_IS_PRESENT" == "0" ]; then
       EXPECTED_TAP_FILES=$CLIENTSERVER_TAP_FILES
       ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536 -E 'pys2opc*'
       CTEST_RET1=$?
   else
       EXPECTED_TAP_FILES=$CLIENTSERVER_TAP_FILES$PYS2OPC_TAP_FILES
       ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536
       CTEST_RET1=$?
       mv "${PYS2OPC_TESTS_DIR}"/*.tap "${TAP_DIR}"/
   fi
else
   CTEST_RET1=0
fi

if [ -z $S2OPC_CLIENTSERVER_ONLY ]; then
   adduser --system mosquitto
   mosquitto &
   MOSQUITTO_PID=$!

   if [ ! -f "${PUBSUB_CTEST_FILE}" ]; then
       PUBSUB_TEST_DIR=${BIN_DIR}/PubSub
       PUBSUB_CTEST_FILE="${PUBSUB_TEST_DIR}/CTestTestfile.cmake"
       TAP_DIR="${BIN_DIR}"
       sed -i "s|S2OPC_ROOT_DIR|${MY_DIR}|g" $PUBSUB_CTEST_FILE
   fi

   if [ ! -f "${PUBSUB_CTEST_FILE}" ]; then
       echo "No CTestTestfile in ${BIN_DIR}/PubSub or ${BUILD_DIR}/tests/PubSub"
       echo "Is this a tagged release, or has CMake been run?"
       exit 1
   fi

   cd "${PUBSUB_TEST_DIR}"
   EXPECTED_TAP_FILES=$EXPECTED_TAP_FILES$'\n'$PUBSUB_TAP_FILES
   if [ -z $S2OPC_PUBSUB_ONLY ]; then
       EXPECTED_TAP_FILES=$EXPECTED_TAP_FILES$'\n'$PUBSUB_CLIENTSERVER_TAP_FILES
   fi
   ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536
   CTEST_RET2=$?
   kill $MOSQUITTO_PID
else
   CTEST_RET2=0
fi

CTEST_RET=$((CTEST_RET1+CTEST_RET2))
EXPECTED_TAP_FILES=$(grep -v "^$" <<< "$EXPECTED_TAP_FILES") # remove blank lines
EXPECTED_TAP_FILES=$(sort <<< "$EXPECTED_TAP_FILES") # sort TAP files names
ACTUAL_TAP_FILES=$(LANG=C ls "${TAP_DIR}"/*.tap | sed "s|${TAP_DIR}/||")
ACTUAL_TAP_FILES=$(sort <<< "$ACTUAL_TAP_FILES") # sort TAP files names

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
