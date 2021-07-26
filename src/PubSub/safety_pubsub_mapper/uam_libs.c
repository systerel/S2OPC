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
 * INCLUDES
 *===========================================================================*/
#include "uam_libs.h"

// TODO : remove all and replace by existing PikeOs or user functions
#include <netinet/in.h>
#include <string.h>
#include <assert.h>

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_LIBS_MemZero(void* pAddr,  const UAS_UInt32 sLen)
{
    memset (pAddr, 0, sLen);
    // TODO : to replace memset, we can do a loop with UINT32 filled to 0 and then filling last bytes with 0
    /// to optimize loop
}

/*===========================================================================*/
void UAM_LIBS_serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos)
{
    UAS_UInt32* puTmp = (UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos <= sLen)
    {
        (*puTmp) = ntohl(uVal);
    }
}

/*===========================================================================*/
void UAM_LIBS_serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos)
{
    UAS_UInt8* puTmp = (UAS_UInt8*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt8);

    if (*pPos <= sLen)
    {
        (*puTmp) = uVal;
    }
}

/*===========================================================================*/
void UAM_LIBS_serialize_String(const UAS_UInt8* pzVal,
                             const UAS_UInt32 sValLen,
                             UAS_UInt8* pData,
                             const UAS_UInt32 sLen,
                             UAS_UInt32* pPos)
{
    if (sValLen + (*pPos) <= sLen)
    {
        memcpy(&pData[*pPos], pzVal, sValLen);
        *pPos += sValLen;
    }
}

/*===========================================================================*/
UAS_UInt32 UAM_LIBS_deserialize_UInt32(const UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos)
{
    const UAS_UInt32* puTmp = (const UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos > sLen)
    {
        return 0;
    }

    return ntohl(*puTmp);
}

/*===========================================================================*/
UAS_UInt8 UAM_LIBS_deserialize_UInt8(const UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos)
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
void UAM_LIBS_deserialize_String(const UAS_UInt8* pData,
                               const UAS_UInt32 sLen,
                               UAS_UInt32* pPos,
                               SOPC_ByteString* pzDest,
                               const UAS_UInt32 sValLen)
{
    if (sValLen + *pPos <= sLen)
    {
        SOPC_ByteString_CopyFromBytes(pzDest, pData + *pPos, (int32_t) sValLen);
    }
    else
    {
        //        printf("deserialize_String fail : sValLen= %u, pos=%u, sLen=%u\n",
        //                (unsigned) sValLen, (unsigned) *pPos, (unsigned) sLen); // TODO clean
    }
    *pPos += sValLen;
}


void UAM_LIBS_HEAP_Init(UAM_LIBS_Heap_type* pzHeap)
{
    assert (pzHeap != 0);
    assert (!pzHeap->initialized);
    pzHeap->pos = 0;
    pzHeap->initialized = true;
}

void* UAM_LIBS_HEAP_Malloc(UAM_LIBS_Heap_type* pzHeap, const size_t len)
{
    void* pResult = NULL;
    if (pzHeap != 0 && pzHeap->initialized &&
            (pzHeap->pos + len <= sizeof(pzHeap->buffer)))
    {
        pResult = pzHeap->buffer + pzHeap->pos;
        pzHeap->pos += len;
    }
    return pResult;
}

void UAM_LIBS_HEAP_Clear(UAM_LIBS_Heap_type* pzHeap)
{
    assert (pzHeap != 0);
    if (pzHeap->initialized)
    {
        pzHeap->initialized = false;
    }
}
