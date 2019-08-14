/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file
 *
 * \brief Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <assert.h>
#include <check.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_* */

#include "check_sc_rcv_helpers.h"
#include "hexlify.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encoder.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_secure_channels_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"
#include "stub_sc_sopc_sockets_api.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

static const char* sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";

static const uint32_t pendingRequestHandle = 1000;

static SOPC_SecureChannel_Config scConfig;

// Configuration SC idx provided on configuration (used also as socket / scIdx)
uint32_t scConfigIdx = 0;

static SOPC_ReturnStatus Check_Client_Closed_SC_Helper(SOPC_StatusCode status)
{
    return Check_Client_Closed_SC(scConfigIdx, scConfigIdx, scConfigIdx, pendingRequestHandle, status);
}

static void clearToolkit(void)
{
    SOPC_Toolkit_Clear();
    Check_SC_Clear();
}

static void establishSC(void)
{
    printf("\nSTART UNIT TEST\n");

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Event* serviceEvent = NULL;
    SOPC_Event* socketEvent = NULL;
    int res = 0;
    SOPC_Buffer* buffer = NULL;
    char hexOutput[512];
    uint32_t requestId = 0;

    // Endpoint URL
    SOPC_String stEndpointUrl;
    SOPC_String_Initialize(&stEndpointUrl);
    // Policy security:
    char* pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_None_URI;

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_None;

    // Init toolkit configuration
    status = SOPC_Toolkit_Initialize(NULL);
    if (SOPC_STATUS_OK != status)
    {
        printf("SC_Rcv_Buffer Init: Failed initializing toolkit\n");
    }
    else
    {
        printf("SC_Rcv_Buffer Init: Toolkit initialized\n");
    }
    ck_assert(SOPC_STATUS_OK == status);

    Check_SC_Init();

    scConfig.isClientSc = true;
    scConfig.msgSecurityMode = messageSecurityMode;
    scConfig.reqSecuPolicyUri = pRequestedSecurityPolicyUri;
    scConfig.crt_cli = NULL;
    scConfig.crt_srv = NULL;
    scConfig.key_priv_cli = NULL;
    scConfig.pki = NULL;
    scConfig.requestedLifetime = 100000;
    scConfig.url = sEndpointUrl;

    scConfigIdx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
    ck_assert(scConfigIdx != 0);

    SOPC_ToolkitConfig_SetCircularLogPath("./check_sc_rcv_buffer_logs/", true);
    ck_assert(SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG) == SOPC_STATUS_OK);
    ck_assert(SOPC_STATUS_OK == SOPC_Toolkit_Configured());

    printf("SC_Rcv_Buffer Init: request connection to SC layer\n");
    SOPC_SecureChannels_EnqueueEvent(SC_CONNECT, scConfigIdx, NULL, 0);

    // Retrieve socket event
    printf("SC_Rcv_Buffer Init: Checking correct socket creation event received\n");
    socketEvent = Check_Socket_Event_Received(SOCKET_CREATE_CLIENT, scConfigIdx,
                                              0); // scConfigIdx == scIdx since there is only 1 SC

    ck_assert(socketEvent != NULL);

    res = strcmp(sEndpointUrl, (char*) socketEvent->params);
    if (res != 0)
    {
        status = SOPC_STATUS_NOK;
        printf("SC_Rcv_Buffer: Unexpected SOCKET_CREATE_CLIENT params\n");
    }
    SOPC_Free(socketEvent);
    socketEvent = NULL;

    // Simulate event from socket
    SOPC_EventHandler_Post(socketsEventHandler, SOCKET_CONNECTION, scConfigIdx, NULL, scConfigIdx);
    printf("SC_Rcv_Buffer Init: Simulating socket connection\n");

    printf("SC_Rcv_Buffer Init: Checking correct HEL message requested to be sent\n");

    // Check expected HEL message requested to be sent on socket
    status = Check_Expected_Sent_Message(scConfigIdx,
                                         // Expected HEL message
                                         "48454c464300000000000000ffff0000ffff0000fbff040005000000230000006f70632e74637"
                                         "03a2f2f6c6f63616c686f73743a383838382f6d79456e64506f696e74",
                                         false, 0, 0);
    ck_assert(SOPC_STATUS_OK == status);

    printf("SC_Rcv_Buffer Init: Simulate correct ACK message received\n");

    // Simulate ACK message received on socket
    status = Simulate_Received_Message(scConfigIdx,
                                       // Expected ACK message received
                                       "41434b461c00000000000000ffff0000ffff0000fbff040005000000");

    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Checking correct OPN message requested to be sent\n");

    // Check expected OPN message requested to be sent on socket
    status = Check_Expected_Sent_Message(
        scConfigIdx,
        // Expected OPN message
        "4f504e4684000000000000002f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff01000000010000000100be01000000000000000000000100000000000000ffffffff00"
        "000000000000000000000000000001000000ffffffffa0860100",
        true, 85, 8);

    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Simulate correct OPN message response received\n");

    // Simulate OPN resp. message received on socket
    status = Simulate_Received_Message(
        scConfigIdx,
        /* Expected OPN resp. message received:
         * - SC id = 833084066 <=> 0xa2daa731 received on socket
         * - secu token = 1778696511 <=> 0x3fc1046a
         * - SeqNum = 1
         * - RequestId = 1
         */
        "4f504e4687000000a2daa7312f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff01000000010000000100c1018ea1ed5700bad301010000000000000000000000000000"
        "0000000000a2daa7313fc1046a8ba1ed5700bad301a0860100ffffffff");

    // Retrieve expected service event
    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Checking correct connection established event received by services\n");
    serviceEvent = Check_Service_Event_Received(SC_CONNECTED, scConfigIdx, scConfigIdx);

    ck_assert(NULL != serviceEvent);
    SOPC_Free(serviceEvent);
    serviceEvent = NULL;

    printf("SC_Rcv_Buffer: request to send an empty MSG and retrieve requestId associated\n");
    buffer = SOPC_Buffer_Create(24); // Let 24 bytes reserved for the header
    ck_assert(buffer != NULL);

    status = SOPC_Buffer_SetDataLength(buffer, 24);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, scConfigIdx, (void*) buffer, pendingRequestHandle);

    socketEvent = Check_Socket_Event_Received(SOCKET_WRITE, scConfigIdx, 0);
    ck_assert(socketEvent != NULL && socketEvent->params != NULL);

    buffer = (SOPC_Buffer*) socketEvent->params;
    res = hexlify(buffer->data, hexOutput, buffer->length);

    ck_assert((uint32_t) res == buffer->length);

    // Check typ = MSG final = F
    res = memcmp(hexOutput, "4d534746", 8);
    ck_assert(res == 0);

    // retrieve requestId in MSG
    status = SOPC_Buffer_SetPosition(buffer, 20);
    ck_assert(SOPC_STATUS_OK == status);

    status = SOPC_UInt32_Read(&requestId, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(2 == requestId); // Expected as valid request Id in unit test

    SOPC_Buffer_Delete(buffer);
    SOPC_Free(socketEvent);
    socketEvent = NULL;

    ck_assert(SOPC_STATUS_OK == status);
}

START_TEST(test_unexpected_hel_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected HEL message received on socket\n");

    // Simulate HEL msg received on connected SC
    status = Simulate_Received_Message(scConfigIdx,
                                       "48454c464300000000000000ffff0000ffff0000ffff000001000000230000006f70632e7463703"
                                       "a2f2f6c6f63616c686f73743a383838382f6d79456e64506f696e74");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_ack_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected ACK message received on socket\n");

    // Simulate ACK msg received on connected SC
    status = Simulate_Received_Message(scConfigIdx, "41434b461c00000000000000ffff0000ffff0000ffff000001000000");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_opn_req_msg_replay)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected OPN req message received on socket (replay client msg)\n");

    // Simulate OPN req msg received on connected SC (replay: same as sent by client)
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e4684000000000000002f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff01000000010000000100be01000000000000000000000100000000000000ffffffff00"
        "000000000000000000000000000001000000ffffffffa0860100");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpSecureChannelUnknown); // invalid SC ID == 0
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_opn_req_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected OPN req message received on socket (valid SC ID / SN / req Id)\n");

    // Simulate OPN req msg received on connected SC (valid SC ID / SN / req Id)
    // SC id = 833084066 / SN = 2 / requestId = 2
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e4684000000a2daa7312f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff02000000020000000100be01000000000000000000000100000000000000ffffffff00"
        "000000000000000000000000000001000000ffffffffa0860100");
    ck_assert(SOPC_STATUS_OK == status);

    // invalid message type (MSG type expected for given request Id)
    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_opn_resp_msg_replay)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected OPN resp message received on socket (replay server msg)\n");

    // Simulate OPN resp msg received on connected SC (replay: same as sent by server the first time)
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e4687000000a2daa7312f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff01000000010000000100c1018ea1ed5700bad301010000000000000000000000000000"
        "0000000000a2daa7313fc1046a8ba1ed5700bad301a0860100ffffffff");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed); // invalid SN / request Id
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_opn_resp_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf(
        "SC_Rcv_Buffer: Simulate unexpected OPN resp message received on socket (valid SC ID / token / SN / req Id)\n");

    // Simulate OPN resp msg received on connected SC (valid SC ID / token / SN / req Id)
    // SC id = 833084066 / tokenId = 1778696511 / SN = 2 / requestId = 2 received on socket
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e4687000000a2daa7312f000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f536563757269747950"
        "6f6c696379234e6f6e65ffffffffffffffff02000000020000000100c1018ea1ed5700bad301010000000000000000000000000000"
        "0000000000a2daa7313fc1046a8ba1ed5700bad301a0860100ffffffff");
    ck_assert(SOPC_STATUS_OK == status);

    // invalid message type (MSG type expected for given request Id)
    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_msg_typ)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid message type response received\n");

    // Simulate MIL / F / size 28 / SC id = 833084066 <=> 0xa2daa731 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d494c461c000000a2daa7310123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST
/*
START_TEST(test_unexpected_multi_chunks_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Only since we are not managing multi chunk messages
    printf("SC_Rcv_Buffer: Simulate unexpected intermediate chunk message received\n");

    // Simulate MSG / C / size 28 / SC id = 833084066 <=> 0xa2daa731 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347431c000000a2daa7310123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_abort_chunk_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected abort chunk message received\n");

    // Simulate MSG / A / size 28 / SC id = 833084066 <=> 0xa2daa731 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347411c000000a2daa7310123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST
*/
START_TEST(test_invalid_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid isFinal value received\n");

    // Simulate MSG / M / size 28 / SC id = 833084066 <=> 0xa2daa731 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d53474d1c000000a2daa7310123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_too_large_msg_size)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t byte = 0;
    uint32_t ui32 = 0;

    printf("SC_Rcv_Buffer: Simulate too large message size indicated (without associated content)\n");

    // Simulate MSG / F / size SOPC_TCP_UA_MAX_BUFFER_SIZE + 1 / SC id = 833084066 received on socket
    SOPC_Buffer* buffer = SOPC_Buffer_Create(1000);
    ck_assert(buffer != NULL);
    byte = 'M';
    status = SOPC_Byte_Write(&byte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'S';
    status = SOPC_Byte_Write(&byte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'G';
    status = SOPC_Byte_Write(&byte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'F';
    status = SOPC_Byte_Write(&byte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    ui32 = SOPC_TCP_UA_MAX_BUFFER_SIZE + 1;
    status = SOPC_UInt32_Write(&ui32, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    ui32 = 833084066;
    status = SOPC_UInt32_Write(&ui32, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_Buffer_SetDataLength(buffer, 500);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_EventHandler_Post(socketsEventHandler, SOCKET_RCV_BYTES, scConfigIdx, (void*) buffer, 0);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTooLarge);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid SC ID\n");

    // Simulate MSG / F / size 28 / SC id = 1 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000010000000123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpSecureChannelUnknown);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_token_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid SC TOKEN ID\n");

    // Simulate MSG / F / size 28 / SC id = 833084066 <=> 0xa2daa731 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000a2daa7310123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecureChannelTokenUnknown);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_sequence_number)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid SN\n");

    // Simulate MSG / F / size 28 / SC id = 833084066 / tokenId = 1778696511 <=> 3fc1046a received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000a2daa7313fc1046a89abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_request_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate incorrect MSG message response received with an invalid REQUEST ID\n");

    // Simulate MSG / F / size 28 / SC id = 833084066 / tokenId = 1778696511 / SN = 2 / RequestId !=2 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000a2daa7313fc1046a020000000000000089abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_valid_sc_request_id)
{
    char msg[57];
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Event* serviceEvent = NULL;
    SOPC_Buffer* buffer = NULL;
    int res = 0;

    printf("SC_Rcv_Buffer: Simulate correct MSG message response received with an valid REQUEST ID\n");

    // Simulate MSG / F / size 28 / SC id = 833084066 / tokenId = 1778696511 / SN = 2 / requestId = 2 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000a2daa7313fc1046a020000000200000089abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    serviceEvent = Check_Service_Event_Received(SC_SERVICE_RCV_MSG, scConfigIdx, 0);
    ck_assert(NULL != serviceEvent);
    ck_assert(NULL != serviceEvent->params);
    buffer = (SOPC_Buffer*) serviceEvent->params;

    // msg[57] max size but strlen("89abcdef") == 8 <=> buffer->length - buffer->position == 4
    ck_assert((buffer->length - buffer->position) * 2 <= 57);
    res = hexlify(&buffer->data[buffer->position], msg, buffer->length - buffer->position);

    ck_assert((uint32_t) res == buffer->length - buffer->position);
    ck_assert(2 * res == strlen("89abcdef"));

    res = memcmp(msg, "89abcdef", strlen("89abcdef"));
    ck_assert(res == 0);

    SOPC_Buffer_Delete(serviceEvent->params);
    SOPC_Free(serviceEvent);
}
END_TEST

static Suite* tests_make_suite_invalid_buffers(void)
{
    Suite* s;
    TCase* tc_invalid_buf;

    s = suite_create("SC layer: receive invalid buffers");
    tc_invalid_buf = tcase_create("Invalid buffers received");
    tcase_add_checked_fixture(tc_invalid_buf, establishSC, clearToolkit);
    tcase_add_test(tc_invalid_buf, test_unexpected_hel_msg);
    tcase_add_test(tc_invalid_buf, test_unexpected_ack_msg);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_req_msg_replay);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_req_msg);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_resp_msg_replay);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_resp_msg);
    tcase_add_test(tc_invalid_buf, test_invalid_msg_typ);
    /*tcase_add_test(tc_invalid_buf, test_unexpected_multi_chunks_final_value);
    tcase_add_test(tc_invalid_buf, test_unexpected_abort_chunk_final_value);*/
    tcase_add_test(tc_invalid_buf, test_invalid_final_value);
    tcase_add_test(tc_invalid_buf, test_too_large_msg_size);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_id);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_token_id);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_sequence_number);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_request_id);
    tcase_add_test(tc_invalid_buf, test_valid_sc_request_id);

    suite_add_tcase(s, tc_invalid_buf);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_make_suite_invalid_buffers());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
