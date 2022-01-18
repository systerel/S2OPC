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
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "sopc_atomic.h"
#include "sopc_mem_alloc.h"
#include "sopc_sub_sockets_mgr.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"

#define MCAST_PORT "4840"
#define MCAST_ADDR "232.1.2.100"
#define NB_ADDRS 100

SOPC_Socket_AddressInfo* addressArr[NB_ADDRS];
Socket socketArr[NB_ADDRS];
uint16_t sockIdxArr[NB_ADDRS];

static SOPC_Buffer* buffer = NULL;
static int32_t stop = 0;
static int32_t cpt = 0;
static int sleepCount = 20;
static int retCode = 2;

static void init_mcast_addrs(void)
{
    printf("config:");
    SOPC_Socket_AddressInfo* localAddr = SOPC_UDP_SocketAddress_Create(false, NULL, MCAST_PORT);
    assert(localAddr != NULL);
    char* addr = SOPC_Calloc(1, sizeof(MCAST_ADDR));
    for (uint16_t i = 0; i < NB_ADDRS; i++)
    {
        printf("%" PRIx16 "\n", i);
        int res = snprintf(addr, sizeof(MCAST_ADDR), "232.1.2.%03" PRIu16, i + 100);
        assert(sizeof(MCAST_ADDR) - 1 == res);
        addressArr[i] = SOPC_UDP_SocketAddress_Create(false, addr, MCAST_PORT);
        assert(NULL != addressArr[i]);
        SOPC_ReturnStatus status = SOPC_UDP_Socket_CreateToReceive(addressArr[i], NULL, true, true, &socketArr[i]);
        assert(SOPC_STATUS_OK == status);
        status = SOPC_UDP_Socket_AddMembership(socketArr[i], NULL, addressArr[i], localAddr);
        assert(SOPC_STATUS_OK == status);
        sockIdxArr[i] = i;
    }
    SOPC_Free(addr);
    SOPC_UDP_SocketAddress_Delete(&localAddr);
}

static void uninit_mcast_addrs(void)
{
    for (int i = 0; i < NB_ADDRS; i++)
    {
        SOPC_UDP_Socket_Close(&socketArr[i]);
        SOPC_UDP_SocketAddress_Delete(&addressArr[i]);
    }
}

static void readyToReceive(void* sockContext, Socket sock)
{
    uint16_t* pSockIdx = sockContext;
    if (SOPC_Atomic_Int_Get(&stop))
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_UDP_Socket_ReceiveFrom(sock, buffer);

    if (SOPC_STATUS_OK == status && buffer->length > 1)
    {
        uint16_t resi = 0;
        status = SOPC_Buffer_Read((uint8_t*) &resi, buffer, 2);
        if (SOPC_STATUS_OK == status)
        {
            printf("%u, ", resi);
            if (resi != *pSockIdx)
            {
                printf("(error detected), ");
                retCode = -1;
            }
        }
        buffer->position = 0;
    }
    else if (SOPC_STATUS_OK == status && buffer->length == 1)
    {
        assert(buffer->data[0] == 0);
        printf("Received 'empty' data, used to check that subscriber is running !\n");
    }
    else
    {
        retCode = -2;
        printf("Unexpected error\n");
        SOPC_Atomic_Int_Set(&stop, true);
    }

    cpt++;
    if (cpt >= NB_ADDRS)
    {
        if (retCode != -1)
        {
            retCode = 0;
        }
        SOPC_Atomic_Int_Set(&stop, true);
    }

    SOPC_Buffer_SetPosition(buffer, 0);
}

int main(void)
{
    init_mcast_addrs();

    buffer = SOPC_Buffer_Create(100);

    if (buffer != NULL)
    {
        SOPC_Sub_SocketsMgr_Initialize(sockIdxArr, sizeof(*sockIdxArr), socketArr, NB_ADDRS, readyToReceive, NULL, NULL,
                                       0);
        printf("Received from pub:");
        while (false == SOPC_Atomic_Int_Get(&stop) && sleepCount > 0)
        {
            SOPC_Sleep(100);
            sleepCount--;
        }
        printf("\n");
        SOPC_Sub_SocketsMgr_Clear();
    }
    else
    {
        retCode = -1;
    }

    SOPC_Buffer_Delete(buffer);
    uninit_mcast_addrs();
    return retCode;
}
