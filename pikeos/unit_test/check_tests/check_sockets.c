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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>

#include "../unit_test_include.h"

#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_buffer.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_sockets_api.h"
#include "sopc_toolkit_config_constants.h"

const char* uri = "opc.tcp://192.168.8.3:4841/myEndPoint";
const uint32_t endpointDescConfigId = 10;
const uint32_t serverSecureChannelConnectionId = 100;
const uint32_t clientSecureChannelConnectionId = 200;

static SOPC_AsyncQueue* socketEvents = NULL;


static void log_UserCallback(const char* timestampUtc,
                            const char* category,
                            const SOPC_Log_Level level,
                            const char* const line)
{
    SOPC_UNUSED_ARG(timestampUtc);
    SOPC_UNUSED_ARG(category);
    SOPC_UNUSED_ARG(level);
    if (NULL != line)
    {
        vm_cprintf("%s\r\n", line);
    }
}

static void onSocketEvent(SOPC_EventHandler* handler, int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_LooperEvent* ev = SOPC_Calloc(1, sizeof(SOPC_LooperEvent));
    SOPC_ASSERT(NULL != ev);

    ev->event = event;
    ev->eltId = id;
    ev->params = params;
    ev->auxParam = auxParam;

    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingEnqueue(socketEvents, ev));
}

static SOPC_LooperEvent* expect_event(int32_t event, uint32_t id)
{
    SOPC_LooperEvent* ev = NULL;
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingDequeue(socketEvents, (void**) &ev));
    SOPC_ASSERT(event == ev->event);
    SOPC_ASSERT(id == ev->eltId);
    return ev;
}

static void expect_events(int32_t event1,
                          uint32_t id1,
                          SOPC_LooperEvent** ev1,
                          int32_t event2,
                          uint32_t id2,
                          SOPC_LooperEvent** ev2)
{
    bool event1_received = false;
    bool event2_received = false;
    SOPC_LooperEvent* ev = NULL;
    while (!event1_received || !event2_received)
    {
        SOPC_ASSERT(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingDequeue(socketEvents, (void**) &ev));
        if (event2 == ev->event && !event2_received)
        {
            SOPC_ASSERT(event2 == ev->event);
            SOPC_ASSERT(id2 == ev->eltId);
            event2_received = true;
            *ev2 = ev;
        }
        else if (!event1_received)
        {
            SOPC_ASSERT(event1 == ev->event);
            SOPC_ASSERT(id1 == ev->eltId);
            event1_received = true;
            *ev1 = ev;
        }
        else
        {
            vm_cprintf("Expected event %d, effective event %d\n", event1, ev->event);
            vm_cprintf("Expected Id %d, effective id %d\n", id1, ev->eltId);
            SOPC_ASSERT(event1 == ev->event);
            SOPC_ASSERT(id1 == ev->eltId);
        }
    }
}

void suite_test_check_sockets(int* index)
{
    vm_cprintf("\nTEST: %d check sockets\n", *index);
    uint32_t serverSocketIdx = 0;
    uint32_t clientSocketIdx = 0;

    SOPC_Buffer* sendBuffer = SOPC_Buffer_Create(1000);
    SOPC_ASSERT(NULL != sendBuffer);
    SOPC_Buffer* receivedBuffer = NULL;
    SOPC_Buffer* accBuffer = SOPC_Buffer_Create(1000);
    SOPC_ASSERT(NULL != accBuffer);
    uint32_t idx = 0;
    uint8_t byte = 0;
    uint32_t receivedBytes = 0;
    uint32_t totalReceivedBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Log_Configuration logConfiguration = {.logLevel = SOPC_LOG_LEVEL_DEBUG,
                                               .logSystem = SOPC_LOG_SYSTEM_USER,
                                               .logSysConfig = {.userSystemLogConfig = {.doLog = (SOPC_Log_UserDoLog*) &log_UserCallback}}};

    SOPC_EventTimer_Initialize();
    SOPC_Sockets_Initialize();

    SOPC_ASSERT(true == SOPC_Logger_Initialize(&logConfiguration));
    SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_DEBUG);

    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_AsyncQueue_Init(&socketEvents, ""));

    SOPC_Looper* looper = SOPC_Looper_Create("test_sockets");
    SOPC_ASSERT(NULL != looper);

    SOPC_EventHandler* event_handler = SOPC_EventHandler_Create(looper, onSocketEvent);
    SOPC_ASSERT(NULL != event_handler);

    SOPC_Sockets_SetEventHandler(event_handler);

    /* SERVER SIDE: listener creation */

    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_LISTENER, endpointDescConfigId, (uintptr_t) uri, (uint32_t) true);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    SOPC_Free(expect_event(SOCKET_LISTENER_OPENED, endpointDescConfigId));

    /* CLIENT SIDE: connection establishment */
    // Create client connection
    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CONNECTION, clientSecureChannelConnectionId, (uintptr_t) uri, 0);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    /*
     * CLIENT SIDE: socket created event
     */
    SOPC_Free(expect_event(SOCKET_CREATED, clientSecureChannelConnectionId));

    /* SERVER SIDE: accepted connection (socket level only)
     * CLIENT SIDE: socket connection done (it does not mean accepted)
     * Note: both events can occur first, therefore check both at same time
     * */
    {
        SOPC_LooperEvent* ev1 = NULL;
        SOPC_LooperEvent* ev2 = NULL;
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
    vm_cprintf("Test 1: ok\n");

    /* CLIENT SIDE: send a msg buffer through connection */
    for (idx = 0; idx < 1000; idx++)
    {
        byte = (uint8_t)(idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager
    vm_cprintf("Test 2: ok\n");

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;

    receivedBytes = 1;

    while (totalReceivedBytes < 1000 && receivedBytes != 0)
    {
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        SOPC_ASSERT(receivedBuffer->length <= 1000);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    SOPC_ASSERT(totalReceivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    status = SOPC_Buffer_SetPosition(accBuffer, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    vm_cprintf("Test 3: ok\n");

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        status = SOPC_Buffer_Read(&byte, accBuffer, 1);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_ASSERT(byte == (idx % 256));
    }
    vm_cprintf("Test 4: ok\n");

    /* SERVER SIDE: send a msg buffer through connection */
    sendBuffer = SOPC_Buffer_Create(1000);
    SOPC_ASSERT(NULL != sendBuffer);
    SOPC_Buffer_Reset(accBuffer);

    for (idx = 0; idx < 1000; idx++)
    {
        byte = (uint8_t)(idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, serverSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager
    vm_cprintf("Test 5: ok\n");

    /* CLIENT SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;
    receivedBytes = 1;
    while (totalReceivedBytes < 1000 && receivedBytes != 0)
    {
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, clientSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        SOPC_ASSERT(receivedBuffer->length <= 1000);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    SOPC_ASSERT(totalReceivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    SOPC_Buffer_SetPosition(accBuffer, 0);
    vm_cprintf("Test 6: ok\n");

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        status = SOPC_Buffer_Read(&byte, accBuffer, 1);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_ASSERT(byte == (idx % 256));
    }
    vm_cprintf("Test 7: ok\n");
    SOPC_Buffer_Delete(accBuffer);

    /* CLIENT SIDE: send a msg buffer through connection with a length greater than maximum buffer size for socket
     * => the socket layer shall provide it in several buffers  */
    sendBuffer = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    SOPC_ASSERT(NULL != sendBuffer);
    // Use a copy of buffer to compare with result to avoid byte-to-byte comparison on long length buffer
    SOPC_Buffer* sendBufferCopy = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    SOPC_ASSERT(NULL != sendBufferCopy);
    accBuffer = SOPC_Buffer_Create(2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    SOPC_ASSERT(NULL != accBuffer);

    for (idx = 0; idx < 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE; idx++)
    {
        byte = (uint8_t)(idx % 256);
        status = SOPC_Buffer_Write(sendBuffer, &byte, 1);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    status = SOPC_Buffer_Copy(sendBufferCopy, sendBuffer);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (uintptr_t) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager
    vm_cprintf("Test 8: ok\n");

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    totalReceivedBytes = 0;
    receivedBytes = 1;
    while (totalReceivedBytes < 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE && receivedBytes != 0)
    {
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) ev->params;
        SOPC_Free(ev);

        SOPC_ASSERT(receivedBuffer->length <= 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
        receivedBytes = receivedBuffer->length;
        totalReceivedBytes = totalReceivedBytes + receivedBytes;
        status = SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Buffer_Delete(receivedBuffer);
    }

    SOPC_ASSERT(totalReceivedBytes == 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE &&
                accBuffer->length == 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    SOPC_ASSERT(receivedBytes != totalReceivedBytes);
    receivedBuffer = NULL;
    status = SOPC_Buffer_SetPosition(accBuffer, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    vm_cprintf("Test 9: ok\n");

    // Check acc buffer content
    int compareResult = memcmp(sendBufferCopy->data, accBuffer->data, 2 * SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE);
    SOPC_ASSERT(0 == compareResult);
    SOPC_Buffer_Delete(sendBufferCopy);
    SOPC_Buffer_Delete(accBuffer);

    /* CLIENT SIDE: receive a msg buffer through connection */
    SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, clientSocketIdx, (uintptr_t) NULL, clientSecureChannelConnectionId);
    vm_cprintf("Test 10: ok\n");

    /* SERVER SIDE: accepted connection (socket level only) */
    {
        SOPC_LooperEvent* ev = expect_event(SOCKET_FAILURE, serverSecureChannelConnectionId);
        SOPC_ASSERT(serverSocketIdx == ev->auxParam);
        SOPC_Free(ev);
    }

    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
    SOPC_Logger_Clear();
    vm_cprintf("Test 11: ok\n");

    *index += 1;
}
