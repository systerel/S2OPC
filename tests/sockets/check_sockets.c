/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <stdlib.h>

#include "sopc_buffer.h"

#include "sopc_event_timer_manager.h"
#include "sopc_secure_channels_api.h"
#include "sopc_sockets_api.h"
#include "stub_sockets_sopc_secure_channels_api.h"

const char* uri = "opc.tcp://localhost:8888/myEndPoint";
const uint32_t endpointDescConfigId = 10;
const uint32_t serverSecureChannelConnectionId = 100;
const uint32_t clientSecureChannelConnectionId = 200;

START_TEST(test_sockets)
{
    uint32_t serverSocketIdx = 0;
    uint32_t clientSocketIdx = 0;

    SOPC_StubSockets_SecureChannelsEventParams* scEventParams = NULL;

    SOPC_Buffer* sendBuffer = SOPC_Buffer_Create(1000);
    SOPC_Buffer* receivedBuffer = NULL;
    SOPC_Buffer* accBuffer = SOPC_Buffer_Create(1000);
    uint32_t idx = 0;
    uint8_t byte = 0;
    uint32_t receivedBytes = 0;
    uint8_t attempts = 0;

#ifdef __TRUSTINSOFT_BUGFIX__
    ck_assert(accBuffer != NULL);
#endif
    SOPC_EventTimer_Initialize();
    SOPC_SecureChannels_Initialize();
    SOPC_Sockets_Initialize();
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    bool SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(uint32_t msecTimeout);
    void TIS_Sockets_Dispatch (void);
#endif

    /* SERVER SIDE: listener creation */
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_CREATE_SERVER(%d) -> endpointDescConfigId(%u)\n", SOCKET_CREATE_SERVER, endpointDescConfigId);
#endif

    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_SERVER, endpointDescConfigId, (void*) uri, (uint32_t) true);
    SOPC_GCC_DIAGNOSTIC_RESTORE
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch ();
#endif

    // Retrieve event of listener creation
    SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d\n", scEventParams->eltId, scEventParams->event);
#endif
    // Check event
    ck_assert(scEventParams->event == SOCKET_LISTENER_OPENED);
    // Check configuration index is preserved
    ck_assert(scEventParams->eltId == endpointDescConfigId);

    free(scEventParams);
    scEventParams = NULL;
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: SOCKET_LISTENER_OPENED step OK\n");
#endif

    /* CLIENT SIDE: connection establishment */
    // Create client connection
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_CREATE_CLIENT(%d) -> clientSecureChannelConnectionId(%u)\n", SOCKET_CREATE_CLIENT, clientSecureChannelConnectionId);
#endif
    // const URI is not modified but generic API cannot guarantee it
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CLIENT, clientSecureChannelConnectionId, (void*) uri, 0);
    SOPC_GCC_DIAGNOSTIC_RESTORE
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch ();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
    TIS_Sockets_Dispatch ();
#endif

    /* SERVER SIDE: accepted connection (socket level only) */
    SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d\n", scEventParams->eltId, scEventParams->event);
#endif
    // Check event
    ck_assert(scEventParams->event == SOCKET_LISTENER_CONNECTION);
    // Check configuration index is preserved
    ck_assert(scEventParams->eltId == endpointDescConfigId);
    serverSocketIdx = scEventParams->auxParam;

    free(scEventParams);
    scEventParams = NULL;

#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: SOCKET_LISTENER_CONNECTION step OK\n");
#endif
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
     SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
     TIS_Sockets_Dispatch ();
#endif
    /* CLIENT SIDE: accepted socket connection */
    SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d (expected = %d)\n", scEventParams->eltId, scEventParams->event, SOCKET_CONNECTION);
#endif
    // Check event
    ck_assert(scEventParams->event == SOCKET_CONNECTION);
    // Check configuration index is preserved
    ck_assert(scEventParams->eltId == clientSecureChannelConnectionId);
    clientSocketIdx = scEventParams->auxParam;

    free(scEventParams);
    scEventParams = NULL;
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: SOCKET_CONNECTION step OK\n");
    // OK for ti_socket
#endif

    /* SERVER SIDE: finish accepting connection (secure channel level) */
    // Note: a new secure channel (with associated connection index) has been created and
    //       must be recorded by the socket as the connection Id
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_ACCEPTED_CONNECTION(%d) -> serverSocketIdx(%u)\n", SOCKET_ACCEPTED_CONNECTION, serverSocketIdx);
#endif
    SOPC_Sockets_EnqueueEvent(SOCKET_ACCEPTED_CONNECTION, serverSocketIdx, NULL, serverSecureChannelConnectionId);
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch ();
#endif

    /* CLIENT SIDE: send a msg buffer through connection */
    for (idx = 0; idx < 1000; idx++)
    {
        byte = (idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_WRITE(%d) -> clientSocketIdx(%u)\n", SOCKET_WRITE, clientSocketIdx);
#endif
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (void*) sendBuffer, 0);
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
#endif
    sendBuffer = NULL; // deallocated by Socket event manager

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    receivedBytes = 0;
    // Let 5 attempts to retrieve all the bytes
    attempts = 0;
    while (receivedBytes < 1000 && attempts < 5)
    {
#ifdef __TRUSTINSOFT_NO_MTHREAD__
      // call TIS_Sockets_Dispatch and TreatSocketsEvents
      TIS_Sockets_Dispatch();
#endif
        SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d\n", scEventParams->eltId, scEventParams->event);
#endif
        // Check event
        ck_assert(scEventParams->event == SOCKET_RCV_BYTES);
        // Check configuration index is preserved
        ck_assert(scEventParams->eltId == serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) scEventParams->params;

        free(scEventParams);
        scEventParams = NULL;
#ifdef __TRUSTINSOFT_DEBUG__
        printf ("TIS: SOCKET_RCV_BYTES step(1-%d) OK\n", attempts);
#endif

        ck_assert(receivedBuffer->length <= 1000);
        receivedBytes = receivedBytes + receivedBuffer->length;
        SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_Buffer_Delete(receivedBuffer);

        attempts++;
    }

    ck_assert(receivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    SOPC_Buffer_SetPosition(accBuffer, 0);

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        SOPC_Buffer_Read(&byte, accBuffer, 1);
        ck_assert(byte == (idx % 256));
    }

    /* SERVER SIDE: send a msg buffer through connection */
    sendBuffer = SOPC_Buffer_Create(1000);
    SOPC_Buffer_Reset(accBuffer);

    for (idx = 0; idx < 1000; idx++)
    {
        byte = (idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_WRITE(%d) -> clientSocketIdx(%u)\n", SOCKET_WRITE, clientSocketIdx);
#endif
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, serverSocketIdx, (void*) sendBuffer, 0);
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
    TIS_Sockets_Dispatch();
#endif
    sendBuffer = NULL; // deallocated by Socket event manager

    /* CLIENT SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    receivedBytes = 0;
    // Let 5 attempts to retrieve all the bytes
    attempts = 0;
    while (receivedBytes < 1000 && attempts < 5)
    {
        SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d\n", scEventParams->eltId, scEventParams->event);
#endif
        // Check event
        ck_assert(scEventParams->event == SOCKET_RCV_BYTES);
        // Check configuration index is preserved
        ck_assert(scEventParams->eltId == clientSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) scEventParams->params;

        free(scEventParams);
        scEventParams = NULL;
#ifdef __TRUSTINSOFT_DEBUG__
        printf ("TIS: SOCKET_RCV_BYTES step(2-%d) OK\n", attempts);
#endif

        ck_assert(receivedBuffer->length <= 1000);
        receivedBytes = receivedBytes + receivedBuffer->length;
        SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_Buffer_Delete(receivedBuffer);

        attempts++;
    }

    ck_assert(receivedBytes == 1000 && accBuffer->length == 1000);
    receivedBuffer = NULL;
    SOPC_Buffer_SetPosition(accBuffer, 0);

    // Check acc buffer content
    for (idx = 0; idx < 1000; idx++)
    {
        SOPC_Buffer_Read(&byte, accBuffer, 1);
        ck_assert(byte == (idx % 256));
    }

    SOPC_Buffer_Delete(accBuffer);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: received buffer OK\n");
#endif
#ifdef __TRUSTINSOFT_HELPER__
    // set slevel to 0 to allocate only one base.
    //@ slevel 0;
#endif

    /* CLIENT SIDE: send a msg buffer through connection with a length greater than maximum message size
     * => the socket layer shall provide it in several buffers  */
    sendBuffer = SOPC_Buffer_Create(2 * SOPC_MAX_MESSAGE_LENGTH);
    accBuffer = SOPC_Buffer_Create(2 * SOPC_MAX_MESSAGE_LENGTH);
#ifdef __TRUSTINSOFT_HELPER__
    // reset slevel
    //@ slevel default;
#endif

    for (idx = 0; idx < 2 * SOPC_MAX_MESSAGE_LENGTH; idx++)
    {
        byte = (idx % 256);
        SOPC_Buffer_Write(sendBuffer, &byte, 1);
    }
    SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, clientSocketIdx, (void*) sendBuffer, 0);
    sendBuffer = NULL; // deallocated by Socket event manager

    /* SERVER SIDE: receive a msg buffer through connection */
    // Accumulate received bytes in a unique buffer
    receivedBytes = 0;
    // Let 5 attempts to retrieve all the bytes
    attempts = 0;
    while (receivedBytes < 2 * SOPC_MAX_MESSAGE_LENGTH && attempts < 5)
    {
        SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
        // Check event
        ck_assert(scEventParams->event == SOCKET_RCV_BYTES);
        // Check configuration index is preserved
        ck_assert(scEventParams->eltId == serverSecureChannelConnectionId);
        receivedBuffer = (SOPC_Buffer*) scEventParams->params;

        free(scEventParams);
        scEventParams = NULL;

        ck_assert(receivedBuffer->length <= 2 * SOPC_MAX_MESSAGE_LENGTH);
        receivedBytes = receivedBytes + receivedBuffer->length;
        SOPC_Buffer_Write(accBuffer, receivedBuffer->data, receivedBuffer->length);
        SOPC_Buffer_Delete(receivedBuffer);

        attempts++;
    }

    ck_assert(receivedBytes == 2 * SOPC_MAX_MESSAGE_LENGTH && accBuffer->length == 2 * SOPC_MAX_MESSAGE_LENGTH);
    ck_assert(attempts > 1);
    receivedBuffer = NULL;
    SOPC_Buffer_SetPosition(accBuffer, 0);

    // Check acc buffer content
    for (idx = 0; idx < 2 * SOPC_MAX_MESSAGE_LENGTH; idx++)
    {
        SOPC_Buffer_Read(&byte, accBuffer, 1);
        ck_assert(byte == (idx % 256));
    }

    SOPC_Buffer_Delete(accBuffer);

    /* CLIENT SIDE: receive a msg buffer through connection */
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: call EnqueueEvent: SOCKET_CLOSE(%d) -> clientSocketIdx(%u)\n", SOCKET_CLOSE, clientSocketIdx);
#endif
    SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, clientSocketIdx, NULL, 0);
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // call TIS_Sockets_Dispatch and TreatSocketsEvents
    TIS_Sockets_Dispatch();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
    TIS_Sockets_Dispatch();
#endif

    /* SERVER SIDE: accepted connection (socket level only) */
    SOPC_AsyncQueue_BlockingDequeue(secureChannelsEvents, (void**) &scEventParams);
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: Dequeue -> id=%u - event=%d\n", scEventParams->eltId, scEventParams->event);
#endif
    // Check event
    ck_assert(scEventParams->event == SOCKET_FAILURE);
    // Check configuration index is preserved
    ck_assert(scEventParams->eltId == serverSecureChannelConnectionId);
    ck_assert(scEventParams->auxParam == serverSocketIdx);

    free(scEventParams);
    scEventParams = NULL;
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS: SOCKET_FAILURE step OK\n");
#endif

    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
    SOPC_SecureChannels_Clear();
}
END_TEST

Suite* tests_make_suite_sockets(void)
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
