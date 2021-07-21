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

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam_ns.h"
#include "uam_ns2s_itf.h"
#include "uam.h"
#include "uas.h"

#include "sopc_dict.h"
#include "sopc_mem_alloc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef int HANDLE;
typedef struct
{
    HANDLE hFileNs2S;
    HANDLE hFileS2Ns;
} UAM_NS2S_FifosFiles_type;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
/**
 * A dictionary object { UAM_SessionHandle : UAM_NS2S_FifosFles_type* }
 */
static SOPC_Dict* gFifos = NULL;
static const UAS_UInt16 iPort = 8888u;

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static void fifoFilesFree (void* data)
{
    UAM_NS2S_FifosFiles_type* pFiles = (UAM_NS2S_FifosFiles_type*) data;
    if (pFiles)
    {
        if (pFiles->hFileNs2S > 0)
        {
            close(pFiles->hFileNs2S);
        }
        if (pFiles->hFileS2Ns > 0)
        {
            close(pFiles->hFileS2Ns);
        }
    }
}

/*===========================================================================*/
static uint64_t fifo_KeyHash_Fct(const void* pKey)
{
    return (const UAM_SessionHandle)(const UAS_INVERSE_PTR)pKey;
}

/*===========================================================================*/
static bool fifo_KeyEqual_Fct (const void* a, const void* b)
{
    return a == b;
}

/*===========================================================================*/
static UAM_NS2S_FifosFiles_type* fifo_Get (const UAM_SessionHandle key)
{
    if (gFifos == NULL)
    {
        return NULL;
    }
    return (UAM_NS2S_FifosFiles_type*) SOPC_Dict_Get (gFifos, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static HANDLE socket_Create (const UAM_SessionHandle dwHandle, bool isNs2S)
{
    (void) isNs2S;
    HANDLE iHandle = 0;
    int iResult =  0;

    const HANDLE hSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);



    // Ensure the handle is not already used!
    UAM_NS2S_FifosFiles_type* pzPrevHandle = fifo_Get (dwHandle);
    if (pzPrevHandle == 0 && hSock >= 0)
    {
        if (iResult == 0)
        {
            printf ("Created SOCKET %d for port %d\n", hSock, (unsigned) iPort); // TODO remove
            LOG_Trace (LOG_DEBUG, "Created SOCKET %d for port %d\n", hSock, (unsigned) iPort);
            iHandle= hSock;
        }
    }
    return iHandle;
}


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
bool UAM_NS2S_Initialize(const UAM_SessionHandle dwHandle)
{
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_Initialize (%u)", (unsigned) dwHandle);
    bool bResult = false;
    void * key  = (void*)(UAS_INVERSE_PTR)dwHandle;

    if (gFifos == NULL)
    {
        gFifos = SOPC_Dict_Create (NULL, fifo_KeyHash_Fct, fifo_KeyEqual_Fct, NULL, &fifoFilesFree);
        assert (gFifos != NULL);
    }

    UAM_NS2S_FifosFiles_type* pFiles = (UAM_NS2S_FifosFiles_type*) SOPC_Malloc( sizeof(* pFiles));
    assert (pFiles != NULL);

    pFiles->hFileNs2S = socket_Create (dwHandle, true);
    if (pFiles->hFileNs2S > 0)
    {
        pFiles->hFileS2Ns = socket_Create (dwHandle, false);
        if (pFiles->hFileNs2S > 0)
        {
            bResult = SOPC_Dict_Insert (gFifos, key, (void*)pFiles);
        }
    }

    return  bResult;
}

/*===========================================================================*/
void UAM_NS2S_SendSpduImpl(const void* const pData, const size_t sLen, const UAM_SessionHandle dwHandle)
{
    assert (dwHandle == 0x010203u); // TODO remove
    ssize_t iNbWritten = 0;
    UAM_NS2S_FifosFiles_type* pzFifos = fifo_Get (dwHandle);
    if (pData == NULL || sLen == 0 || pzFifos == NULL)
    {
        return;
    }
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_SendSpduImpl(%p, %u, %u)\n", pData, (unsigned)sLen, (unsigned) dwHandle);

    struct sockaddr_in zSAddrTo;
    static const socklen_t siLen = sizeof(zSAddrTo);

    memset((char *) &zSAddrTo, 0, siLen);

    zSAddrTo.sin_family = AF_INET;
    zSAddrTo.sin_port = htons(iPort);
    zSAddrTo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    iNbWritten = sendto (pzFifos->hFileNs2S, pData, sLen, 0, (struct sockaddr *)&zSAddrTo, siLen);

    if (iNbWritten < 0 || ((size_t)iNbWritten) < sLen)
    {
        // TODO
        printf ("UAM_NS2S_SendSpduImpl failed to send %u bytes (res=%d)\n", (unsigned)sLen, (int)iNbWritten);
        assert (false); // TODO
    }
}

/*===========================================================================*/
void UAM_NS2S_Clear(void)
{
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_Clear ()");

    SOPC_Dict_Delete(gFifos);
    gFifos = NULL;
}

