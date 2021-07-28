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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/** \file
 *      Implementation of services defined in "uam_ns2s_itf.h" using "UDP sockets" (loopback):
 *      - Sending a SPDU to Safe partition
 *      - Reading a SPDU from Safe partition
 */

/* TODO list
 * - This example implementation will not work with several SPDU couples defined, because all
 *      are using the same port 8888 / 8889. The port must be configured for each session.
 * - Move all utility functions in separate file to ease functional understanding
 * - This file contains both implementations of "uam_ns_impl.h" and "uam_ns2s_itf.h".
 *      This may be separated.
 * - A Trace channel should be added to forward SAFE logs to NON SAFE partition so that
 *      safe logs can be archived or seen at run-time.
 */
/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam.h"
#include "uam_ns2s_itf.h"
#include "uas.h"

#include "sopc_dict.h"
#include "sopc_mem_alloc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "uam_ns_impl.h"

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef int HANDLE;
typedef struct
{
    HANDLE hSocketWriteNS2S;
    HANDLE hSocketWriteS2NS;
    struct sockaddr_in zSAddrTo;
} UAM_NS2S_Impl_type;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
/**
 * A dictionary object { UAM_SessionId : UAM_NS2S_FifosFles_type* }
 */
static SOPC_Dict* gFifos = NULL;
static const UAS_UInt16 iPortNS2S = 8888u;
static const UAS_UInt16 iPortS2NS = iPortNS2S + 1u;

#define MAX_RECEPTION_BUFFER_SIZE (2048u)
/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static void fifoFilesFree(void* data)
{
    UAM_NS2S_Impl_type* pFiles = (UAM_NS2S_Impl_type*) data;
    if (pFiles)
    {
        if (pFiles->hSocketWriteNS2S > 0)
        {
            close(pFiles->hSocketWriteNS2S);
        }
        if (pFiles->hSocketWriteS2NS > 0)
        {
            close(pFiles->hSocketWriteS2NS);
        }
        SOPC_Free(pFiles);
    }
}

/*===========================================================================*/
static uint64_t fifo_KeyHash_Fct(const void* pKey)
{
    return (const UAM_SessionId)(const UAS_INVERSE_PTR) pKey;
}

/*===========================================================================*/
static bool fifo_KeyEqual_Fct(const void* a, const void* b)
{
    return a == b;
}

/*===========================================================================*/
static UAM_NS2S_Impl_type* fifo_Get(const UAM_SessionId key)
{
    if (gFifos == NULL)
    {
        return NULL;
    }
    return (UAM_NS2S_Impl_type*) SOPC_Dict_Get(gFifos, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static HANDLE socket_Create(const UAM_SessionId dwSessionId)
{
    HANDLE iHandle = 0;
    int iResult = 0;

    const HANDLE hSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Ensure the handle is not already used!
    UAM_NS2S_Impl_type* pzPrevHandle = fifo_Get(dwSessionId);
    if (pzPrevHandle == 0 && hSock >= 0)
    {
        if (iResult == 0)
        {
            iHandle = hSock;
        }
    }
    return iHandle;
}

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
bool UAM_NS2S_Initialize(const UAM_SessionId dwSessionId)
{
#ifdef UASDEF_DBG
    LOG_Trace(LOG_DEBUG, "UAM_NS2S_Initialize (%u)", (unsigned) dwSessionId);
#endif
    bool bResult = false;
    void* key = (void*) (UAS_INVERSE_PTR) dwSessionId;

    if (gFifos == NULL)
    {
        gFifos = SOPC_Dict_Create(NULL, fifo_KeyHash_Fct, fifo_KeyEqual_Fct, NULL, &fifoFilesFree);
        assert(gFifos != NULL);
    }

    UAM_NS2S_Impl_type* pFiles = (UAM_NS2S_Impl_type*) SOPC_Malloc(sizeof(*pFiles));
    assert(pFiles != NULL);

    pFiles->hSocketWriteNS2S = socket_Create(dwSessionId);
    pFiles->hSocketWriteS2NS = socket_Create(dwSessionId);

    memset((char*) &pFiles->zSAddrTo, 0, sizeof(pFiles->zSAddrTo));

    pFiles->zSAddrTo.sin_family = AF_INET;
    pFiles->zSAddrTo.sin_port = htons(iPortNS2S);
    pFiles->zSAddrTo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (pFiles->hSocketWriteNS2S > 0 && pFiles->hSocketWriteS2NS)
    {
        // Bind read socket to input port.
        static const int trueInt = true;
        struct sockaddr_in server_addr;
        int iResult = -1;

        memset((char*) &server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(iPortS2NS);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        fcntl(pFiles->hSocketWriteS2NS, F_SETFL, fcntl(pFiles->hSocketWriteS2NS, F_GETFL) | O_NONBLOCK);

        setsockopt(pFiles->hSocketWriteS2NS, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));

        iResult = bind(pFiles->hSocketWriteS2NS, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));

        if (iResult < 0)
        {
            perror("Bind error");
#ifdef UASDEF_DBG
            LOG_Trace(LOG_ERROR, "UAM_NS2S_Initialize failed to bind port %u", (unsigned) iPortS2NS);
#endif
        }
        else
        {
#ifdef UASDEF_DBG
            LOG_Trace(LOG_DEBUG, "UAM_NS2S_Initialize bound HDL=%d to port %u", (int) dwSessionId, (unsigned) iPortS2NS);
#endif
            bResult = SOPC_Dict_Insert(gFifos, key, (void*) pFiles);
        }
    }

    return bResult;
}

/*===========================================================================*/
void UAM_NS2S_SendSpduImpl(const UAM_SessionId dwSessionId, const void* const pData, const size_t sLen)
{
    assert(dwSessionId == 0x010203u); // TODO remove, just for POC
    ssize_t iNbWritten = 0;
    UAM_NS2S_Impl_type* pzFifos = fifo_Get(dwSessionId);
    if (pData == NULL || sLen == 0 || pzFifos == NULL)
    {
        return;
    }

    static const socklen_t siLen = sizeof(struct sockaddr_in);

    iNbWritten = sendto(pzFifos->hSocketWriteNS2S, pData, sLen, 0, (struct sockaddr*) &pzFifos->zSAddrTo, siLen);

    if (iNbWritten < 0 || ((size_t) iNbWritten) < sLen)
    {
        printf("UAM_NS2S_SendSpduImpl failed to send %u bytes (res=%d)\n", (unsigned) sLen, (int) iNbWritten);
        assert(false); // TODO
    }
}

/*===========================================================================*/
void UAM_NS2S_ReceiveSpduImpl(const UAM_SessionId dwSessionId, void* pData, size_t sMaxLen, size_t* sReadLen)
{
    assert(dwSessionId == 0x010203u); // TODO remove, just for POC
    ssize_t iNbRead = 0;
    UAM_NS2S_Impl_type* pzFifos = fifo_Get(dwSessionId);

    if (pData == NULL || sReadLen == NULL || pzFifos == NULL)
    {
        return;
    }
    *sReadLen = 0;

    struct sockaddr_in zSAddrFrom;
    socklen_t siLen = sizeof(zSAddrFrom);

    memset((char*) &zSAddrFrom, 0, siLen);

    zSAddrFrom.sin_family = AF_INET;
    zSAddrFrom.sin_port = htons(iPortS2NS);
    zSAddrFrom.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    iNbRead = recv(pzFifos->hSocketWriteS2NS, pData, sMaxLen, 0);
    if (iNbRead > 0)
    {
        *sReadLen = (size_t) iNbRead;
        printf("UAM_NS2S_ReceiveSpduImpl:Rcvd %u bytes\n", (unsigned) iNbRead); // TODO
    }
}

/*===========================================================================*/
void UAM_NS2S_Clear(void)
{
#ifdef UASDEF_DBG
    LOG_Trace(LOG_DEBUG, "UAM_NS2S_Clear ()");
#endif

    SOPC_Dict_Delete(gFifos);
    gFifos = NULL;
}
