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

#include <check.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_* */

#include "sopc_async_queue.h"
#include "sopc_buffer.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_sockets_api.h"
#include "sopc_toolkit_config_constants.h"

const char* uri = "opc.tcp://localhost:4841/myEndPoint";
const uint32_t endpointDescConfigId = 10;
const uint32_t serverSecureChannelConnectionId = 100;
const uint32_t clientSecureChannelConnectionId = 200;

static SOPC_AsyncQueue* socketEvents = NULL;

static void onSocketEvent(SOPC_EventHandler* handler, int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    SOPC_Event* ev = SOPC_Calloc(1, sizeof(SOPC_Event));
    ck_assert_ptr_nonnull(ev);

    ev->event = event;
    ev->eltId = id;
    ev->params = params;
    ev->auxParam = auxParam;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(socketEvents, ev));
}

static SOPC_Event* expect_event(int32_t event, uint32_t id)
{
    SOPC_Event* ev = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingDequeue(socketEvents, (void**) &ev));
    ck_assert_int_eq(event, ev->event);
    ck_assert_uint_eq(id, ev->eltId);
    return ev;
}

static void expect_events(int32_t event1,
                          uint32_t id1,
                          SOPC_Event** ev1,
                          int32_t event2,
                          uint32_t id2,
                          SOPC_Event** ev2)
{
    bool event1_received = false;
    bool event2_received = false;
    SOPC_Event* ev = NULL;
    while (!event1_received || !event2_received)
    {
        ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingDequeue(socketEvents, (void**) &ev));
        if (event2 == ev->event && !event2_received)
        {
            ck_assert_int_eq(event2, ev->event);
            ck_assert_uint_eq(id2, ev->eltId);
            event2_received = true;
            *ev2 = ev;
        }
        else if (!event1_received)
        {
            ck_assert_int_eq(event1, ev->event);
            ck_assert_uint_eq(id1, ev->eltId);
            event1_received = true;
            *ev1 = ev;
        }
        else
        {
            ck_assert_int_eq(event1, ev->event);
            ck_assert_uint_eq(id1, ev->eltId);
        }
    }
}

START_TEST(test_sockets)
{
    uint32_t serverSocketIdx = 0;
    uint32_t clientSocketIdx = 0;

    SOPC_Buffer* sendBuffer = SOPC_Buffer_Create(1000);
    ck_assert_ptr_nonnull(sendBuffer);
    SOPC_Buffer* receivedBuffer = NULL;
    SOPC_Buffer* accBuffer = SOPC_Buffer_Create(1000);
    ck_assert_ptr_nonnull(accBuffer);
    uint32_t idx = 0;
    uint8_t byte = 0;
    uint32_t receivedBytes = 0;
    uint32_t totalReceivedBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_LogSystem_File_Configuration logFileConfiguration = {
        .logDirPath = "./check_sockets_logs/", .logMaxBytes = 1048576, .logMaxFiles = 5};
    SOPC_Log_Configuration logConfiguration = {.logLevel = SOPC_LOG_LEVEL_INFO,
                                               .logSystem = SOPC_LOG_SYSTEM_FILE,
                                               .logSysConfig = {.fileSystemLogConfig = logFileConfiguration}};

    SOPC_EventTimer_Initialize();
    SOPC_Sockets_Initialize();

    SOPC_FileSystem_CreationResult mkdirRes = SOPC_FileSystem_mkdir("./check_sockets_logs/");
    ck_assert(mkdirRes == SOPC_FileSystem_Creation_OK || mkdirRes == SOPC_FileSystem_Creation_Error_PathAlreadyExists);
    ck_assert_int_eq(true, SOPC_Logger_Initialize(&logConfiguration));
    SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_DEBUG);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_Init(&socketEvents, ""));

    SOPC_Looper* looper = SOPC_Looper_Create("test_sockets");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* event_handler = SOPC_EventHandler_Create(looper, onSocketEvent);
    ck_assert_ptr_nonnull(event_handler);

    SOPC_Sockets_SetEventHandler(event_handler);

    /* SERVER SIDE: listener creation */

    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_SERVER, endpointDescConfigId, (uintptr_t) uri, (uint32_t) true);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    SOPC_Free(expect_event(SOCKET_LISTENER_OPENED, endpointDescConfigId));

    /* CLIENT SIDE: connection establishment */
    // Create client connection
    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CLIENT, clientSecureChannelConnectionId, (uintptr_t) uri, 0);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    /* SERVER SIDE: accepted connection (socket level only)
     * CLIENT SIDE: socket connection done (it does not mean accepted)
     * Note: both events can occur first, therefore check both at same time
     * */
    {
        SOPC_Event* ev1 = NULL;
        SOPC_Event* ev2 = NULL;
        expect_events(SOCKET_LISTENER_CONNECTION, endpointDescConfigId, &ev1, SOCKET_CONNECTION,
                      clientSecureChannelConnectionId, &ev2);
        serverSocketIdx = (uint32_t) ev1->auxParam;
        clientSocketIdx = (uint32_t) ev2->auxParam;
        SOPC_Free(ev1);
        SOPC_Free(ev2);
    }

    /* SERVER SIDE: finish accepting connection (secure channel level) */
    // Note: a new secure channel (with associated connection index) has been created and
    //       must be recorded by the socket as the connection Id
    SOPC_Sockets_EnqueueEvent(SOCKET_ACCEPTED_CONNECTION, serverSocketIdx, (uintptr_t) NULL,
                              serverSecureChannelConnectionId);

    /* CLIENT SIDE: send a msg buffer through connection */
    for (idx = 0; idx < 1000; idx++)
    {
        byte = (uint8_t)(idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;

    receivedBytes = 1;

    while (totalReceivedBytes < 1000 && receivedBytes != 0)
    {
        SOPC_Event* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        ck_assert(receivedBuffer->length <= 1000);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        ck_assert(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    ck_assert(totalReceivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    status = SOPC_Buffer_SetPosition(accBuffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        status = SOPC_Buffer_Read(&byte, accBuffer, 1);
        ck_assert(SOPC_STATUS_OK == status);
        ck_assert(byte == (idx % 256));
    }

    /* SERVER SIDE: send a msg buffer through connection */
    sendBuffer = SOPC_Buffer_Create(1000);
    ck_assert_ptr_nonnull(sendBuffer);
    SOPC_Buffer_Reset(accBuffer);

    for (idx = 0; idx < 1000; idx++)
    {
        byte = (uint8_t)(idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, serverSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager

    /* CLIENT SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;
    receivedBytes = 1;
    while (totalReceivedBytes < 1000 && receivedBytes != 0)
    {
        SOPC_Event* ev = expect_event(SOCKET_RCV_BYTES, clientSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        ck_assert(receivedBuffer->length <= 1000);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        ck_assert(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    ck_assert(totalReceivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    SOPC_Buffer_SetPosition(accBuffer, 0);

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        status = SOPC_Buffer_Read(&byte, accBuffer, 1);
        ck_assert(SOPC_STATUS_OK == status);
        ck_assert(byte == (idx % 256));
    }

    SOPC_Buffer_Delete(accBuffer);

    /* CLIENT SIDE: send a msg buffer through connection with a length greater than maximum buffer size for socket
     * => the socket layer shall provide it in several buffers  */
    sendBuffer = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    ck_assert_ptr_nonnull(sendBuffer);
    // Use a copy of buffer to compare with result to avoid byte-to-byte comparison on long length buffer
    SOPC_Buffer* sendBufferCopy = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    ck_assert_ptr_nonnull(sendBufferCopy);
    accBuffer = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    ck_assert_ptr_nonnull(accBuffer);

    for (idx = 0; idx < 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE; idx++)
    {
        byte = (uint8_t)(idx % 256);
        status = SOPC_Buffer_Write(sendBuffer, &byte, 1);
        ck_assert(SOPC_STATUS_OK == status);
    }
    status = SOPC_Buffer_Copy(sendBufferCopy, sendBuffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;
    receivedBytes = 1;
    while (totalReceivedBytes < 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE && receivedBytes != 0)
    {
        SOPC_Event* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        ck_assert(receivedBuffer->length <= 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        ck_assert(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    ck_assert(totalReceivedBytes == 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE &&
              accBuffer->length == 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    ck_assert(receivedBytes != totalReceivedBytes);
    receivedBuffer = NULL;
    status = SOPC_Buffer_SetPosition(accBuffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Check acc buffer content
    int compareResult = memcmp(sendBufferCopy->data, accBuffer->data, 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    ck_assert_int_eq(0, compareResult);
    SOPC_Buffer_Delete(sendBufferCopy);
    SOPC_Buffer_Delete(accBuffer);

    /* CLIENT SIDE: receive a msg buffer through connection */
    SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, clientSocketIdx, (uintptr_t) NULL, clientSecureChannelConnectionId);

    /* SERVER SIDE: accepted connection (socket level only) */
    {
        SOPC_Event* ev = expect_event(SOCKET_FAILURE, serverSecureChannelConnectionId);
        ck_assert_uint_eq(serverSocketIdx, ev->auxParam);
        SOPC_Free(ev);
    }

    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
}
END_TEST

static Suite* tests_make_suite_sockets(void)
{
    Suite* s;
    TCase* tc_sockets;

    s = suite_create("Sockets");
    tc_sockets = tcase_create("Sockets");
    tcase_add_test(tc_sockets, test_sockets);
    suite_add_tcase(s, tc_sockets);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_make_suite_sockets());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
