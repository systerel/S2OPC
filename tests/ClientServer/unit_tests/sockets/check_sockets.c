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
#include <string.h>

#include "sopc_async_queue.h"
#include "sopc_atomic.h"
#include "sopc_buffer.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_raw_sockets.h"
#include "sopc_sockets_api.h"
#include "sopc_threads.h"
#include "sopc_toolkit_config_constants.h"

const char* uri = "opc.tcp://localhost:4841/myEndPoint";
const uint32_t endpointDescConfigId = 10;
const uint32_t serverSecureChannelConnectionId = 100;
const uint32_t clientSecureChannelConnectionId = 200;

static SOPC_AsyncQueue* socketEvents = NULL;

static void onSocketEvent(SOPC_EventHandler* handler, int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    SOPC_LooperEvent* ev = SOPC_Calloc(1, sizeof(SOPC_LooperEvent));
    ck_assert_ptr_nonnull(ev);

    ev->event = event;
    ev->eltId = id;
    ev->params = params;
    ev->auxParam = auxParam;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(socketEvents, ev));
}

static SOPC_LooperEvent* expect_event(int32_t event, uint32_t id)
{
    SOPC_LooperEvent* ev = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingDequeue(socketEvents, (void**) &ev));
    ck_assert_int_eq(event, ev->event);
    ck_assert_uint_eq(id, ev->eltId);
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
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_LISTENER, endpointDescConfigId, (uintptr_t) SOPC_strdup(uri),
                              (uint32_t) true);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    SOPC_Free(expect_event(SOCKET_LISTENER_OPENED, endpointDescConfigId));

    /* CLIENT SIDE: connection establishment */
    // Create client connection
    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CONNECTION, clientSecureChannelConnectionId, (uintptr_t) SOPC_strdup(uri),
                              0);
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
        SOPC_Free((void*) ev1->params);
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
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
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
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, clientSecureChannelConnectionId);
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
        SOPC_LooperEvent* ev = expect_event(SOCKET_RCV_BYTES, serverSecureChannelConnectionId);
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
        SOPC_LooperEvent* ev = expect_event(SOCKET_FAILURE, serverSecureChannelConnectionId);
        ck_assert_uint_eq(serverSocketIdx, ev->auxParam);
        SOPC_Free(ev);
    }

    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
    SOPC_Logger_Clear();
}
END_TEST

/* Number of occurrences of log lines of interest for test_listener_max_connections_no_busy_loop */
static int32_t nbRefusedConnectionLogs = 0;
static int32_t nbListenerSuspendedLogs = 0;
static int32_t nbListenerResumedLogs = 0;

static void onLog(const char* timestampUtc, const char* category, const SOPC_Log_Level level, const char* const line)
{
    SOPC_UNUSED_ARG(timestampUtc);
    SOPC_UNUSED_ARG(category);
    SOPC_UNUSED_ARG(level);
    if (NULL != strstr(line, "refusing new connection since maximum number of socket reached"))
    {
        SOPC_Atomic_Int_Add(&nbRefusedConnectionLogs, 1);
    }
    else if (NULL != strstr(line, "stops accepting new connections"))
    {
        SOPC_Atomic_Int_Add(&nbListenerSuspendedLogs, 1);
    }
    else if (NULL != strstr(line, "accepts new connections again"))
    {
        SOPC_Atomic_Int_Add(&nbListenerResumedLogs, 1);
    }
}

/* Opens a blocking raw TCP connection to the given host and port, trying each resolved address */
static SOPC_Socket raw_connect(const char* host, const char* port)
{
    SOPC_Socket sock = SOPC_INVALID_SOCKET;
    SOPC_Socket_AddressInfo* addrs = NULL;
    SOPC_ReturnStatus status = SOPC_Socket_AddrInfo_Get(host, port, &addrs);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_STATUS_NOK;
    for (SOPC_Socket_AddressInfo* p = addrs; NULL != p && SOPC_STATUS_OK != status;
         p = SOPC_Socket_AddrInfo_IterNext(p))
    {
        status = SOPC_Socket_CreateNew(p, false, false, &sock);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Socket_Connect(sock, p);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Socket_Close(&sock);
            }
        }
    }
    SOPC_Socket_AddrInfoDelete(&addrs);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    return sock;
}

/* Non-regression test: when a listener cannot accept new connections (SOPC_MAX_SOCKETS_CONNECTIONS reached or no free
 * socket), a pending connection attempt shall not make the sockets thread loop on the listener read event (100% CPU and
 * "refusing new connection" warning flooding). The pending connection shall be accepted once a socket is freed. */
START_TEST(test_listener_max_connections_no_busy_loop)
{
    // Number of connections the listener can accept: limited by the connections per listener or by the free sockets
    // (index 0 is reserved and 1 socket is used by the listener itself)
    const uint32_t maxAccepted =
        (SOPC_MAX_SOCKETS - 2 < SOPC_MAX_SOCKETS_CONNECTIONS) ? SOPC_MAX_SOCKETS - 2 : SOPC_MAX_SOCKETS_CONNECTIONS;
    SOPC_Socket clientSockets[SOPC_MAX_SOCKETS_CONNECTIONS + 1];
    uint32_t firstServerSocketIdx = 0;
    SOPC_Log_Configuration logConfiguration = {.logLevel = SOPC_LOG_LEVEL_INFO,
                                               .logSystem = SOPC_LOG_SYSTEM_USER,
                                               .logSysConfig = {.userSystemLogConfig = {.doLog = onLog}}};

    SOPC_EventTimer_Initialize();
    SOPC_Sockets_Initialize();
    ck_assert(SOPC_Logger_Initialize(&logConfiguration));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_Init(&socketEvents, ""));
    SOPC_Looper* looper = SOPC_Looper_Create("test_listener");
    ck_assert_ptr_nonnull(looper);
    SOPC_EventHandler* event_handler = SOPC_EventHandler_Create(looper, onSocketEvent);
    ck_assert_ptr_nonnull(event_handler);
    SOPC_Sockets_SetEventHandler(event_handler);

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_LISTENER, endpointDescConfigId, (uintptr_t) SOPC_strdup(uri),
                              (uint32_t) true);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_Free(expect_event(SOCKET_LISTENER_OPENED, endpointDescConfigId));

    /* Reach the maximum number of connections accepted by the listener */
    for (uint32_t i = 0; i < maxAccepted; i++)
    {
        clientSockets[i] = raw_connect("localhost", "4841");
        SOPC_LooperEvent* ev = expect_event(SOCKET_LISTENER_CONNECTION, endpointDescConfigId);
        SOPC_Free((void*) ev->params);
        if (0 == i)
        {
            firstServerSocketIdx = (uint32_t) ev->auxParam;
        }
        SOPC_Free(ev);
    }

    /* One more connection: established by the OS (listen backlog) but cannot be accepted by the listener */
    clientSockets[maxAccepted] = raw_connect("localhost", "4841");
    SOPC_Sleep(1000);

    // Before fix: the listener stayed readable, connection was refused in loop and logged thousands of times
    ck_assert_int_eq(0, SOPC_Atomic_Int_Get(&nbRefusedConnectionLogs));
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&nbListenerSuspendedLogs));
    ck_assert_int_eq(0, SOPC_Atomic_Int_Get(&nbListenerResumedLogs));

    /* Free a socket: the pending connection shall now be accepted */
    // Note: connection id is 0 since SOCKET_ACCEPTED_CONNECTION was not sent for the accepted sockets
    SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, firstServerSocketIdx, (uintptr_t) NULL, 0);
    {
        SOPC_LooperEvent* ev = expect_event(SOCKET_LISTENER_CONNECTION, endpointDescConfigId);
        SOPC_Free((void*) ev->params);
        SOPC_Free(ev);
    }
    ck_assert_int_eq(0, SOPC_Atomic_Int_Get(&nbRefusedConnectionLogs));
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&nbListenerResumedLogs));

    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
    for (uint32_t i = 0; i <= maxAccepted; i++)
    {
        SOPC_Socket_Close(&clientSockets[i]);
    }
    SOPC_Looper_Delete(looper);
    SOPC_AsyncQueue_Free(&socketEvents);
    SOPC_Logger_Clear();
}
END_TEST

/* Non-regression test for the cross-platform uniformization of display of IPv4-mapped addresses into IPv6 addresses */
START_TEST(test_socket_address_getnameinfo_ipv4_mapped)
{
    static const struct
    {
        const char* input;        /* numeric address parsed by getaddrinfo */
        const char* port;         /* numeric service */
        const char* expectedHost; /* expected rendered host (after normalization) */
    } cases[] = {
        {"::ffff:192.168.56.1", "55218", "192.168.56.1"}, /* IPv4-mapped IPv6 -> plain IPv4 (the case being fixed) */
        {"192.168.0.55", "55524", "192.168.0.55"},        /* genuine IPv4 -> unchanged */
        {"fe80::1", "4840", "fe80::1"},                   /* genuine IPv6 -> unchanged */
    };

    /* getaddrinfo() (used by SOPC_Socket_AddrInfo_Get) requires the network stack to be initialized: on Windows this
       performs the mandatory WSAStartup, without which getaddrinfo fails with WSANOTINITIALISED. It is a no-op on POSIX
       platforms. */
    ck_assert(SOPC_Socket_Network_Initialize());

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    {
        SOPC_Socket_AddressInfo* ai = NULL;
        SOPC_ReturnStatus status = SOPC_Socket_AddrInfo_Get(cases[i].input, cases[i].port, &ai);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_ptr_nonnull(ai);

        SOPC_Socket_Address* addr = SOPC_Socket_CopyAddress(ai);
        ck_assert_ptr_nonnull(addr);

        char* host = NULL;
        char* serv = NULL;
        status = SOPC_SocketAddress_GetNameInfo(addr, &host, &serv);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_ptr_nonnull(host);
        ck_assert_ptr_nonnull(serv);
        ck_assert_str_eq(cases[i].expectedHost, host);
        ck_assert_str_eq(cases[i].port, serv);

        SOPC_Free(host);
        SOPC_Free(serv);
        SOPC_SocketAddress_Delete(&addr);
        SOPC_Socket_AddrInfoDelete(&ai);
    }

    SOPC_Socket_Network_Clear();
}
END_TEST

static Suite* tests_make_suite_sockets(void)
{
    Suite* s;
    TCase* tc_sockets;
    TCase* tc_socket_address;

    s = suite_create("Sockets");
    tc_sockets = tcase_create("Sockets");
    tcase_add_test(tc_sockets, test_sockets);
    tcase_add_test(tc_sockets, test_listener_max_connections_no_busy_loop);
    suite_add_tcase(s, tc_sockets);

    tc_socket_address = tcase_create("SocketAddress");
    tcase_add_test(tc_socket_address, test_socket_address_getnameinfo_ipv4_mapped);
    suite_add_tcase(s, tc_socket_address);

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
