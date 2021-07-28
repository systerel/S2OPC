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

#include "assert.h"
#include "uam_s_libs.h"


/**************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************/


/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef UAS_UInt32 UAM_S2NS_Size;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

/*============================================================================
 * DECLARATION OF INTERNAL SERVICES
 *===========================================================================*/
// TODO serialize_UInt32 & co are duplicated with uam_ns.c

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
// TODO uncomment for message sending
///*===========================================================================*/
//static void serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const UAM_S2NS_Size sLen, UAM_S2NS_Size* pPos)
//{
//    UAS_UInt32* puTmp = (UAS_UInt32*) (&pData[*pPos]);
//    *pPos += sizeof(UAS_UInt32);
//
//    if (*pPos <= sLen)
//    {
//        (*puTmp) = UAM_S_LIBS_nothl(uVal);
//    }
//}
//
///*===========================================================================*/
//static void serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const UAM_S2NS_Size sLen, UAM_S2NS_Size* pPos)
//{
//    UAS_UInt8* puTmp = (UAS_UInt8*) (&pData[*pPos]);
//    *pPos += sizeof(UAS_UInt8);
//
//    if (*pPos <= sLen)
//    {
//        (*puTmp) = uVal;
//    }
//}
//
///*===========================================================================*/
//static void serialize_String(const UAS_UInt8* pzVal,
//                             const UAM_S2NS_Size sValLen,
//                             UAS_UInt8* pData,
//                             const UAM_S2NS_Size sLen,
//                             UAM_S2NS_Size* pPos)
//{
//    if (sValLen + (*pPos) <= sLen)
//    {
//        UAM_S_LIBS_MemCopy(&pData[*pPos], pzVal, sValLen);
//        *pPos += sValLen;
//    }
//}

/*===========================================================================*/
static UAS_UInt32 deserialize_UInt32(const UAS_UInt8* pData, const UAM_S2NS_Size sLen, UAM_S2NS_Size* pPos)
{
    const UAS_UInt32* puTmp = (const UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos > sLen)
    {
        return 0;
    }

    return UAM_S_LIBS_nothl(*puTmp);
}

/*===========================================================================*/
static UAS_UInt8 deserialize_UInt8(const UAS_UInt8* pData, const UAM_S2NS_Size sLen, UAM_S2NS_Size* pPos)
{
    const UAS_UInt8* puTmp = (const UAS_UInt8*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt8);

    if (*pPos > sLen)
    {
        return 0;
    }

    return *puTmp;
}

/*===========================================================================*/
static void deserialize_String(const UAS_UInt8* pData,
                               const UAM_S2NS_Size sLen,
                               UAM_S2NS_Size* pPos,
                               UAS_UInt8* pDest,
                               const UAM_S2NS_Size sValLen)
{
    if (sValLen + *pPos <= sLen)
    {
        UAM_S_LIBS_MemCopy (pDest, pData, sValLen);
    }
    *pPos += sValLen;
}


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
void UAM_S2NS_DecodeSpduRequest(const void* pData, UAS_UInt16 sLen, UAS_RequestSpdu_type* pzRequest)
{
    // This function proceeds to the reverse decoding of EncodeSpduRequest()
    assert (pData != NULL);
    assert (pzRequest != NULL);

    UAM_S2NS_Size pos = 0;
    const UAS_UInt8* pBytes = pData;
    static const UAM_S2NS_Size sExpectedLength = 9;

    if (sExpectedLength == sLen)
    {
        pzRequest->dwSafetyConsumerId = deserialize_UInt32(pBytes, sLen, &pos);
        pzRequest->dwMonitoringNumber = deserialize_UInt32(pBytes, sLen, &pos);
        pzRequest->byFlags = deserialize_UInt8(pBytes, sLen, &pos);
        UAM_S_DoLog_UInt32 (UAM_S_LOG_DEBUG, "UAM_S2NS_DecodeSpduRequest => decoding OK. Received len = ", sLen);
        assert (pos == sLen);
    }
    else
    {
        UAM_S_DoLog_UInt32 (UAM_S_LOG_WARN, "Mismatching size in UAM_S2NS_DecodeSpduRequest. Received len = ", sLen);
    }
}


/*===========================================================================*/
void UAM_S2NS_DecodeSpduResponse(const void* pData, UAS_UInt16 sLen, const UAM_SafetyConfiguration_type* pzConfig,
        UAS_ResponseSpdu_type* pzResponse)
{
    // This function proceeds to the reverse decoding of EncodeSpduResponse()
    assert (pData != NULL);
    assert (pzConfig != NULL);
    assert (pzResponse != NULL);

    UAM_S2NS_Size pos = 0;
    const UAS_UInt8* pBytes = pData;
    static const UAM_S2NS_Size sStaticLength = 25u;
    const UAM_S2NS_Size sExpectedLength = sStaticLength + pzConfig->wNonSafetyDataLength + pzConfig->wSafetyDataLength;

    if (sExpectedLength == sLen)
    {
        // de-serialize Safe data
        deserialize_String(pBytes, sLen, &pos, pzResponse->pbySerializedSafetyData, pzConfig->wSafetyDataLength);
        // de-serialize byFlags
        pzResponse->byFlags = deserialize_UInt8(pBytes, sLen, &pos);
        // de-serialize zSpduId
        pzResponse->zSpduId.dwPart1 = deserialize_UInt32(pBytes, sLen, &pos);
        pzResponse->zSpduId.dwPart2 = deserialize_UInt32(pBytes, sLen, &pos);
        pzResponse->zSpduId.dwPart3 = deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwSafetyConsumerId
        pzResponse->dwSafetyConsumerId = deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwMonitoringNumber
        pzResponse->dwMonitoringNumber = deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize dwCrc
        pzResponse->dwCrc = deserialize_UInt32(pBytes, sLen, &pos);
        // de-serialize NonSafe data
        deserialize_String(pBytes, sLen, &pos, pzResponse->pbySerializedNonSafetyData, pzConfig->wNonSafetyDataLength);
        assert (pos == sLen);

        UAM_S_DoLog_UInt32 (UAM_S_LOG_DEBUG, "UAM_S2NS_DecodeSpduRequest => decoding OK. Received len = ", sLen);
    }
    else
    {
        UAM_S_DoLog_UInt32 (UAM_S_LOG_WARN, "Mismatching size in UAM_S2NS_DecodeSpduRequest. Received len = ", sLen);
    }
}

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

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static const UAS_UInt16 iPortNS2S = 8888u;
static const UAS_UInt16 iPortS2NS = iPortNS2S + 1u;

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
void UAM_S2NS_Initialize(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Initialize-In");
    // TODO : init socket
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Initialize-Out");
}

/*===========================================================================*/
void UAM_S2NS_InitializeSpdu(const UAM_SessionId dwSessionId)
{
    UAM_S_DoLog_UInt32(UAM_S_LOG_SEQUENCE, "UAM_S2NS_InitializeSpdu-In, HDL=", dwSessionId);
    // TODO : init socket
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_InitializeSpdu-Out");
}

/*===========================================================================*/
void UAM_S2NS_SendSpduImpl(const UAM_SessionId dwSessionId, const void* const pData, const UAS_UInt16 sLen)
{
    UAM_S_DoLog_UInt32(UAM_S_LOG_SEQUENCE, "UAM_S2NS_SendSpduImpl-In, HDL=", dwSessionId);
    // TODO : send socket
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_SendSpduImpl-Out");
}

/*===========================================================================*/
void UAM_S2NS_ReceiveAllSpdusFromNonSafe(UAM_S2NS_SpduReceptionEvent pfMessageProcess)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-In");
    // TODO : read socket
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_ReceiveAllSpdusFromNonSafe-Out");
}

/*===========================================================================*/
void UAM_S2NS_Clear(void)
{
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Clear-In");
    // TODO : clear socket
    UAM_S_DoLog(UAM_S_LOG_SEQUENCE, "UAM_S2NS_Clear-Out");
}

