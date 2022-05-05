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

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"

#define MCAST_PORT "4840"
#define MCAST_ADDR "232.1.2.100"
#define NB_ADDRS 100

SOPC_Socket_AddressInfo* addressArr[NB_ADDRS];

static int32_t stopPublisher = false;

static void Test_StopSignal(int sig)
{
    SOPC_UNUSED_ARG(sig);

    if (SOPC_Atomic_Int_Get(&stopPublisher) != false)
    {
        exit(1);
    }
    else
    {
        SOPC_Atomic_Int_Set(&stopPublisher, true);
    }
}

static void init_mcast_addrs(void)
{
    char* addr = SOPC_Calloc(1, sizeof(MCAST_ADDR));
    for (int i = 0; i < NB_ADDRS; i++)
    {
        int res = snprintf(addr, sizeof(MCAST_ADDR), "232.1.2.%03d", i + 100);
        assert(sizeof(MCAST_ADDR) - 1 == res);
        addressArr[i] = SOPC_UDP_SocketAddress_Create(false, addr, MCAST_PORT);
        assert(NULL != addressArr[i]);
    }
    SOPC_Free(addr);
}

static void uninit_mcast_addrs(void)
{
    for (int i = 0; i < NB_ADDRS; i++)
    {
        SOPC_UDP_SocketAddress_Delete(&addressArr[i]);
    }
}

int main(void)
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    init_mcast_addrs();
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    Socket sock = SOPC_INVALID_SOCKET;
    status = SOPC_UDP_Socket_CreateToSend(addressArr[0], NULL, true, &sock);
    SOPC_Buffer* buffer = SOPC_Buffer_Create(100);

    if (SOPC_STATUS_OK == status && buffer != NULL)
    {
        while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&stopPublisher) == false)
        {
            for (uint16_t i = 0; i < NB_ADDRS; i++)
            {
                status = SOPC_Buffer_Write(buffer, (uint8_t*) &i, 2);
                SOPC_Buffer_SetPosition(buffer, 0);
                if (SOPC_STATUS_OK != status)
                {
                    return 1;
                }
                status = SOPC_UDP_Socket_SendTo(sock, addressArr[i], buffer);
                if (SOPC_STATUS_OK != status)
                {
                    return 2;
                }
            }
            SOPC_Sleep(100);
        }
    }
    else
    {
        return -1;
    }

    SOPC_Buffer_Delete(buffer);
    SOPC_UDP_Socket_Close(&sock);
    uninit_mcast_addrs();
    return 0;
}
