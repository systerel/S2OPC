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
 * Creates an asynchronous queue (pzQueue) which are dequeued in dedicated thread (gThread).
 * When a Request or Response message is received, the event is enqueued. When processed by the task,
 * the message is encoded in a raw buffer which is then sent to SAFE using interface (UAM_NS2S_xxx)
 * UAM_NS_CheckSpduReception shall be called periodically to check the reception of a SPDU from SAFE.
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam_s.h"
#include "uam_s2ns_itf.h"
#include "uas.h"

#include "uam_s_libs.h"


/**************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************/
#define USE_SELECT 0

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

/*============================================================================
 * DECLARATION OF INTERNAL SERVICES
 *===========================================================================*/

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
/*                     SOCKETS-SPECIFIC PART                                 */
/*===========================================================================*/
/*===========================================================================*/


// TODO : remove these specific includes for another implementation than sockets!
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
#define MAX_CONNECTIONS 20

typedef UAS_Int32 HANDLE;
typedef struct
{
    UAM_SessionId dwSessionId;
    HANDLE hSocketWriteNS2S;
    HANDLE hSocketWriteS2NS;
    struct sockaddr_in zSAddrTo;
} UAM_NS2S_Impl_type;

static const UAS_UInt16 iPortNS2S = 8888u;
static const UAS_UInt16 iPortS2NS = iPortNS2S + 1u;

static UAM_NS2S_Impl_type gSockets [MAX_CONNECTIONS];
/** Index of next free slot in gSockets */
static UAS_UInt32 gNextFreeSocket = 0;

#if USE_SELECT
static fd_set gReadFdSet;
static int gMaxFd = 0;
#endif

#define MAX_UDP_SOCKET_RCV_SIZE (1600u)

/*===========================================================================*/
static UAM_NS2S_Impl_type* socket_Get(UAM_SessionId dwSessionId)
{
    UAM_NS2S_Impl_type* pzResult = NULL;
    UAS_UInt32 uIndex = 0;
    for (uIndex = 0 ; uIndex < gNextFreeSocket && pzResult == NULL ; uIndex ++)
    {
        UAM_NS2S_Impl_type* pzSocket = &gSockets[uIndex];
        if (pzSocket->dwSessionId == dwSessionId)
        {
            pzResult = pzSocket;
        }
    }

    return pzResult;
}

/*===========================================================================*/
static HANDLE socket_Create(void)
{
    const HANDLE hSock = (HANDLE) socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return hSock;
}


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
void UAM_S2NS_Initialize(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Initialize-In");
    UAS_UInt32 uIndex = 0;
    for (uIndex = 0 ; uIndex < MAX_CONNECTIONS ; uIndex ++)
    {
        UAM_NS2S_Impl_type* pzSocket = &gSockets[uIndex];
        pzSocket->dwSessionId = UAM_NoHandle;
        pzSocket->hSocketWriteNS2S = -1;
        pzSocket->hSocketWriteS2NS = -1;
        UAM_S_LIBS_VarZero (pzSocket->zSAddrTo);
    }
    gNextFreeSocket = 0;
#if USE_SELECT
    gMaxFd = 0;
    FD_ZERO(&gReadFdSet);
#else
#endif
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Initialize-Out");
}

/*===========================================================================*/
void UAM_S2NS_InitializeSpdu(const UAM_SessionId dwSessionId)
{
    UAM_S_DoLog_UInt(UAM_S_LOG_SEQUENCE, "UAM_S2NS_InitializeSpdu-In, HDL=", dwSessionId);
    UAM_S_LIBS_ASSERT (gNextFreeSocket < MAX_CONNECTIONS);
    UAM_NS2S_Impl_type* pzSocket = &gSockets[gNextFreeSocket];

    UAM_NS2S_Impl_type * pzPrevConfig = socket_Get (dwSessionId);
    UAM_S_LIBS_ASSERT (pzPrevConfig == NULL && "This UAM_SessionId is already used");

    gNextFreeSocket ++;

    pzSocket->hSocketWriteNS2S = socket_Create();
    pzSocket->hSocketWriteS2NS = socket_Create();
    pzSocket->dwSessionId = dwSessionId;

    UAM_S_LIBS_VarZero (pzSocket->zSAddrTo);

    pzSocket->zSAddrTo.sin_family = AF_INET;
    pzSocket->zSAddrTo.sin_port = htons(iPortS2NS);
    pzSocket->zSAddrTo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (pzSocket->hSocketWriteNS2S > 0 && pzSocket->hSocketWriteS2NS > 0)
    {
        // Bind read socket to input port.
        static const int trueInt = true;
        struct sockaddr_in server_addr;
        int iResult = -1;

        UAM_S_LIBS_VarZero (server_addr);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(iPortNS2S);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        fcntl(pzSocket->hSocketWriteNS2S, F_SETFL, fcntl(pzSocket->hSocketWriteNS2S, F_GETFL) | O_NONBLOCK);

        setsockopt(pzSocket->hSocketWriteNS2S, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));

        iResult = bind(pzSocket->hSocketWriteNS2S, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));

        if (iResult < 0)
        {
            UAM_S_DoLog_UInt (UAM_S_LOG_ERROR, "UAM_S2NS_InitializeSpdu failed to bind port ", iPortNS2S);
        }
        else
        {
            UAM_S_DoLog_UInt(UAM_S_LOG_DEBUG, "UAM_S2NS_InitializeSpdu bound port %u", iPortNS2S);
        }

#if USE_SELECT
        // Will be listening to UDP frames from NON SAFE
        if (pzSocket->hSocketWriteNS2S > gMaxFd)
        {
            gMaxFd = pzSocket->hSocketWriteNS2S;
        }
        FD_SET(pzSocket->hSocketWriteNS2S, &gReadFdSet);
#endif
    }

    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_InitializeSpdu-Out");
}

/*===========================================================================*/
void UAM_S2NS_SendSpduImpl(const UAM_SessionId dwSessionId, const void* const pData, const UAM_S_Size sLen)
{
    UAM_S_DoLog_UInt(UAM_S_LOG_SEQUENCE, "UAM_S2NS_SendSpduImpl-In, HDL=", dwSessionId);
    UAS_Int64 iNbWritten = 0;

    UAM_NS2S_Impl_type * pzSocket = socket_Get (dwSessionId);
    UAM_S_LIBS_ASSERT (pzSocket != NULL && "This UAM_SessionId is undefined");
    UAM_S_LIBS_ASSERT (pData!= NULL);

    if (sLen > 0)
    {
        static const socklen_t siLen = sizeof(struct sockaddr_in);

        iNbWritten = sendto(pzSocket->hSocketWriteNS2S, pData, sLen, 0, (struct sockaddr*) &pzSocket->zSAddrTo, siLen);

        if (iNbWritten < 0 || ((size_t) iNbWritten) < sLen)
        {
            UAM_S_DoLog_UInt(UAM_S_LOG_WARN, "UAM_S2NS_SendSpduImpl failed to send all bytes on SESSION = ", dwSessionId);
        }
    }
    else
    {
        UAM_S_DoLog_UInt(UAM_S_LOG_WARN, "UAM_S2NS_SendSpduImpl, sending empty message on SESSION = ", dwSessionId);
    }

    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_SendSpduImpl-Out");
}

/*===========================================================================*/
void UAM_S2NS_ReceiveAllSpdusFromNonSafe(UAM_S2NS_SpduReceptionEvent pfMessageProcess)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-In");
    UAM_S_LIBS_ASSERT (pfMessageProcess!= NULL);

    UAS_UInt32 uIndex = 0;
    UAS_Int64 iResult = 0;
#if USE_SELECT

    static struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

    if (gMaxFd > 0)
    {
        iResult = select (gMaxFd + 1, &gReadFdSet, NULL, NULL, &timeout);
        UAM_S_DoLog_Int(UAM_S_LOG_INFO, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-select=", iResult);
        if (iResult > 0)
        {
            for (uIndex = 0 ; uIndex < gNextFreeSocket ; uIndex ++)
            {
                UAM_NS2S_Impl_type* pzSocket = &gSockets[uIndex];
                if (FD_ISSET (pzSocket->hSocketWriteNS2S, &gReadFdSet))
                {
                    UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Read from SESSION=", pzSocket->dwSessionId);
                    // note that using a static allocated buffer works because we are sure there is no
                    // concurrent access in SAFE program (no threads/interruptions)
                    static UAS_UInt8 aUdpBuffer[MAX_UDP_SOCKET_RCV_SIZE];
                    iResult = recv(pzSocket->hSocketWriteNS2S, aUdpBuffer, MAX_UDP_SOCKET_RCV_SIZE, 0);
                    UAM_S_DoLog_Int(UAM_S_LOG_INFO, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Read from NB Read=", iResult);
                    if (iResult > 0)
                    {
                        (*pfMessageProcess)(pzSocket->dwSessionId, (const void*)aUdpBuffer, (UAS_UInt64) iResult);
                    }
                }
            }
        }
        else if (iResult != EWOULDBLOCK && iResult < 0)
        {
            UAM_S_DoLog_Int(UAM_S_LOG_WARN, "UAM_S2NS_ReceiveAllSpdusFromNonSafe, SELECT failed with code = ", iResult);
        }

    }
#else
    for (uIndex = 0 ; uIndex < gNextFreeSocket ; uIndex ++)
    {
        UAM_NS2S_Impl_type* pzSocket = &gSockets[uIndex];
        // note that using a static allocated buffer works because we are sure there is no
        // concurrent access in SAFE program (no threads/interruptions)
        static UAS_UInt8 aUdpBuffer[MAX_UDP_SOCKET_RCV_SIZE];
        iResult = recv(pzSocket->hSocketWriteNS2S, aUdpBuffer, MAX_UDP_SOCKET_RCV_SIZE, 0);
        if (iResult > 0)
        {
            UAM_S_DoLog_Int(UAM_S_LOG_INFO, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Read from NB Read=", iResult);
            UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Read from SESSION=", pzSocket->dwSessionId);
            (*pfMessageProcess)(pzSocket->dwSessionId, (const void*)aUdpBuffer, (UAS_UInt64) iResult);
        }
    }
#endif
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Out");
}

/*===========================================================================*/
void UAM_S2NS_Clear(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Clear-In");
    UAS_UInt32 uIndex = 0;
    for (uIndex = 0 ; uIndex < gNextFreeSocket ; uIndex ++)
    {
        UAM_NS2S_Impl_type* pzSocket = &gSockets[uIndex];
        if (pzSocket->hSocketWriteNS2S > 0)
        {
            close (pzSocket->hSocketWriteNS2S);
        }
        if (pzSocket->hSocketWriteS2NS > 0)
        {
            close (pzSocket->hSocketWriteS2NS);
        }
        pzSocket->hSocketWriteNS2S = -1;
        pzSocket->hSocketWriteS2NS = -1;
        pzSocket->dwSessionId = UAM_NoHandle;
    }
    gNextFreeSocket = 0;
#if USE_SELECT
    gMaxFd = 0;
#endif
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Clear-Out");
}

