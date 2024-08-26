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

#include <string.h>

#include "unit_test_include.h"

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

static void cb_client_tcp(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Socket_AddressInfo* addrServer = SOPC_Malloc(sizeof(SOPC_Socket_AddressInfo));
    SOPC_Socket_AddressInfo* addrClient = SOPC_Malloc(sizeof(SOPC_Socket_AddressInfo));
    const char* nodeServ = "192.168.8.3";
    const char* portServ = "80";
    const char* nodeClient = "192.168.8.3";
    const char* portClient = "81";
    Socket sockClient = NULL;
    const bool reuseAddr = false;
    const bool nonBlockingSock = false;

    status = SOPC_Socket_AddrInfo_Get(nodeServ, portServ, &addrServer);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_AddrInfo_Get(nodeClient, portClient, &addrClient);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_CreateNew(addrClient, reuseAddr, nonBlockingSock, &sockClient);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_Connect(sockClient, addrServer);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Socket_AddrInfoDelete(&addrServer);
    SOPC_Socket_AddrInfoDelete(&addrClient);
}

static void cb_server_tcp(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Socket_AddressInfo* addr = SOPC_Malloc(sizeof(SOPC_Socket_AddressInfo));
    const char* node = "192.168.8.3";
    const char* port = "80";
    Socket sockClient, sockServer = NULL;
    const bool reuseAddr = false;
    const bool nonBlockingSock = false;

    status = SOPC_Socket_AddrInfo_Get(node, port, &addr);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_CreateNew(addr, reuseAddr, nonBlockingSock, &sockServer);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_Listen(sockServer, addr);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Socket_Accept(sockServer, nonBlockingSock, &sockClient);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Socket_AddrInfoDelete(&addr);
}

static void cb_sender_udp(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const bool useIPv6 = false;
    const bool nonBlockingSocket = true;
    const uint32_t bufferSize = 1000;
    const char* node = "192.168.8.3";
    const char* portReceiver = "80";
    Socket s = 0;
    const char* msg = "A wonderful message";

    SOPC_Buffer* buff = SOPC_Buffer_Create(bufferSize);
    SOPC_ASSERT(NULL != buff);

    status = SOPC_Buffer_Write(buff, (const uint8_t*) msg, strlen(msg) * sizeof(char));
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Buffer_SetPosition(buff, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Socket_AddressInfo* addrDest = SOPC_UDP_SocketAddress_Create(useIPv6, node, portReceiver);
    SOPC_ASSERT(NULL != addrDest);

    status = SOPC_UDP_Socket_CreateToSend(addrDest, NULL, nonBlockingSocket, &s);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_UDP_Socket_SendTo(s, addrDest, buff);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_UDP_SocketAddress_Delete(&addrDest);
    SOPC_UDP_Socket_Close(&s);
    SOPC_Buffer_Delete(buff);
}

static void cb_receiver_udp(void)
{
    const bool useIPv6 = false;
    const bool reuseAddr = false;
    const bool nonBlockingSocket = true;
    const uint32_t bufferSize = 1500;
    const char* node = "192.168.8.3";
    const char* port1 = "80";
    int nbReady = 0;
    SOPC_SocketSet readSet, writeSet, exceptSet;
    Socket sock1 = 0;

    SOPC_Buffer* buff = SOPC_Buffer_Create(bufferSize);

    SOPC_Socket_AddressInfo* addr = SOPC_UDP_SocketAddress_Create(useIPv6, node, port1);
    SOPC_ASSERT(NULL != addr);

    SOPC_ReturnStatus status = SOPC_UDP_Socket_CreateToReceive(addr, NULL, reuseAddr, nonBlockingSocket, &sock1);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_SocketSet_Clear(&readSet);
    SOPC_SocketSet_Clear(&writeSet);
    SOPC_SocketSet_Clear(&exceptSet);

    SOPC_SocketSet_Add(sock1, &readSet);

    nbReady = SOPC_Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, 0);
    SOPC_ASSERT(nbReady > 0);
    status = SOPC_UDP_Socket_ReceiveFrom(sock1, buff);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_UDP_SocketAddress_Delete(&addr);
    SOPC_UDP_Socket_Close(&sock1);
    SOPC_Buffer_Delete(buff);
}

static void cb_multicast_sender(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const bool useIPv6 = false;
    const bool nonBlockingSocket = true;
    const uint32_t bufferSize = 1000;
    const char* multicastGroup = "232.1.2.100";
    const char* portReceiver = "80";
    Socket sock = 0;
    const char* msg = "A second wonderful message";
    const uint8_t ttlScope = 1;
    SOPC_Buffer* buff = SOPC_Buffer_Create(bufferSize);
    SOPC_ASSERT(NULL != buff);

    status = SOPC_Buffer_Write(buff, (const uint8_t*) msg, strlen(msg) * sizeof(char));
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Buffer_SetPosition(buff, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Socket_AddressInfo* addrMulticast = SOPC_UDP_SocketAddress_Create(useIPv6, multicastGroup, portReceiver);
    SOPC_ASSERT(NULL != addrMulticast);

    status = SOPC_UDP_Socket_CreateToSend(addrMulticast, NULL, nonBlockingSocket, &sock);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_UDP_Socket_Set_MulticastTTL(sock, ttlScope);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_UDP_Socket_SendTo(sock, addrMulticast, buff);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_UDP_SocketAddress_Delete(&addrMulticast);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_Buffer_Delete(buff);
}

static void cb_multicast_receiver(void)
{
    const bool useIPv6 = false;
    const bool reuseAddr = false;
    const bool nonBlockingSocket = true;
    const uint32_t bufferSize = 1500;
    const char* multicastGroup = "232.1.2.100";
    const char* port = "80";
    int nbReady = 0;
    const uint8_t ttlScope = 1;
    SOPC_SocketSet readSet, writeSet, exceptSet;
    Socket sock = 0;

    SOPC_Buffer* buff = SOPC_Buffer_Create(bufferSize);

    SOPC_Socket_AddressInfo* addrMulticast = SOPC_UDP_SocketAddress_Create(useIPv6, multicastGroup, port);
    SOPC_ASSERT(NULL != addrMulticast);

    SOPC_ReturnStatus status =
        SOPC_UDP_Socket_CreateToReceive(addrMulticast, NULL, reuseAddr, nonBlockingSocket, &sock);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_UDP_Socket_Set_MulticastTTL(sock, ttlScope);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_SocketSet_Clear(&readSet);
    SOPC_SocketSet_Clear(&writeSet);
    SOPC_SocketSet_Clear(&exceptSet);

    SOPC_SocketSet_Add(sock, &readSet);

    // Wait for 5 second then timeout
    nbReady = SOPC_Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, 5000);
    SOPC_ASSERT(nbReady > 0);
    status = SOPC_UDP_Socket_ReceiveFrom(sock, buff);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_UDP_SocketAddress_Delete(&addrMulticast);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_Buffer_Delete(buff);
}

void suite_test_raw_sockets(int* index)
{
    PRINT("\n TEST %d: sopc_raw_sockets.h \n", *index);
    SOPC_Thread p0, p1 = SOPC_INVALID_THREAD;
    const char* node = "192.168.8.3";
    const char* port = "80";
    const bool useIPv6 = false;
    char** nodeRes = SOPC_Calloc(1, sizeof(*nodeRes));
    char** portRes = SOPC_Calloc(1, sizeof(*portRes));

    SOPC_ReturnStatus status = SOPC_Thread_CreatePrioritized(&p0, (void*) cb_server_tcp, NULL, 60, "TCP_Server");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Thread_CreatePrioritized(&p1, (void*) cb_client_tcp, NULL, 60, "TCP_Client");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Thread_Join(&p0);
    SOPC_Thread_Join(&p1);
    PRINT("Test 1 : ok\n");

    SOPC_Socket_AddressInfo* addrinfo = SOPC_UDP_SocketAddress_Create(useIPv6, node, port);
    status = SOPC_SocketAddress_GetNameInfo(addrinfo, nodeRes, portRes);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_ASSERT(0 == strcmp(node, *nodeRes));
    SOPC_ASSERT(0 == strcmp(port, *portRes));
    SOPC_Free(nodeRes);
    SOPC_Free(portRes);
    PRINT("Test 2 : ok\n");

    *index += 1;
}

void suite_test_udp_sockets(int* index)
{
    PRINT("\n TEST %d: sopc_udp_socket.h \n", *index);

    SOPC_Thread p0, p1, p2, p3 = SOPC_INVALID_THREAD;

    SOPC_ReturnStatus status = SOPC_Thread_CreatePrioritized(&p0, (void*) cb_receiver_udp, NULL, 60, "UDP_Receiver");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Thread_CreatePrioritized(&p1, (void*) cb_sender_udp, NULL, 60, "UDP_Sender");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Thread_Join(&p0);
    SOPC_Thread_Join(&p1);
    PRINT("Test 1 : ok\n");

    status = SOPC_Thread_CreatePrioritized(&p2, (void*) cb_multicast_receiver, NULL, 60, "Multicast_Receiver");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Thread_CreatePrioritized(&p3, (void*) cb_multicast_sender, NULL, 60, "Multicast_Sender");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Thread_Join(&p2);
    SOPC_Thread_Join(&p3);
    PRINT("Test 2 : ok\n");

    *index += 1;
}
