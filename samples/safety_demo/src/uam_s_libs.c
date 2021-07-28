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
#include <string.h> // TODO remove !
#include <stdlib.h> // TODO remove !
#include "uam_s_libs.h"

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_S_LIBS_MemZero(void* pAddr,  const UAM_S_Size sLen)
{
    memset (pAddr, 0, sLen);
    // TODO : to replace memset, we can do a loop with UINT32 filled to 0 and then filling last bytes with 0
    /// to optimize loop
}

/*===========================================================================*/
void UAM_S_LIBS_MemCopy(void* pDest,  const void* pSource,  const UAM_S_Size sLen)
{
    memcpy (pDest, pSource, sLen);
    // TODO : memcpy,
}

#ifdef IS_BIG_ENDIAN
#ifdef IS_LITTLE_ENDIAN
#error "Could not determine Endianness for this architecture. Only one amongst IS_LITTLE_ENDIAN and IS_BIG_ENDIAN must be defined."
#endif
/*===========================================================================*/
UAS_UInt32 UAM_S_LIBS_nothl(const UAS_UInt32 uLong)
{
    return uLong;
}

/*===========================================================================*/
UAS_UInt16 UAM_S_LIBS_noths(const UAS_UInt16 ushort)
{
    return ushort;
}

#elif defined (IS_LITTLE_ENDIAN)
/*===========================================================================*/
UAS_UInt32 UAM_S_LIBS_nothl(const UAS_UInt32 uLong)
{
    const UAS_UInt8* u8map = (const UAS_UInt8*) &uLong;
    UAS_UInt32 uResult = u8map[3];
    uResult += ( (UAS_UInt32) u8map[2]) << 8;
    uResult += ( (UAS_UInt32) u8map[1]) << 16;
    uResult += ( (UAS_UInt32) u8map[0]) << 24;
    return uResult;
}

/*===========================================================================*/
UAS_UInt16 UAM_S_LIBS_noths(const UAS_UInt16 ushort)
{
    const UAS_UInt8* u8map = (const UAS_UInt8*) &ushort;
    UAS_UInt16 uResult = u8map[1];
    uResult += (UAS_UInt16) ( ( (UAS_UInt32) u8map[0]) << 8u);
    return uResult;
}

#else
#error "Could not determine Endianness for this architecture. Either IS_LITTLE_ENDIAN or IS_BIG_ENDIAN must be defined."
#endif

/*===========================================================================*/
void UAM_S_LIBS_ExitFailure (const UAS_Int32 sCode)
{
    exit(sCode); /* TODO: define safe rules! */
}

/*===========================================================================*/
void UAM_S_LIBS_HEAP_Init(UAM_LIBS_Heap_type* pzHeap)
{
    UAM_S_LIBS_ASSERT (pzHeap != 0);
    UAM_S_LIBS_ASSERT (!pzHeap->initialized);
    pzHeap->pos = 0;
    pzHeap->initialized = true;
}

/*===========================================================================*/
void* UAM_S_LIBS_HEAP_Malloc(UAM_LIBS_Heap_type* pzHeap, const UAM_S_Size len)
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

/*===========================================================================*/
void UAM_S_LIBS_HEAP_Clear(UAM_LIBS_Heap_type* pzHeap)
{
    UAM_S_LIBS_ASSERT (pzHeap != 0);
    if (pzHeap->initialized)
    {
        pzHeap->initialized = false;
    }
}
/*===========================================================================*/
void UAM_S_LIBS_serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos)
{
    UAS_UInt32* puTmp = (UAS_UInt32*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt32);

    if (*pPos <= sLen)
    {
        (*puTmp) = UAM_S_LIBS_nothl(uVal);
    }
}

/*===========================================================================*/
void UAM_S_LIBS_serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos)
{
    UAS_UInt8* puTmp = (UAS_UInt8*) (&pData[*pPos]);
    *pPos += sizeof(UAS_UInt8);

    if (*pPos <= sLen)
    {
        (*puTmp) = uVal;
    }
}

/*===========================================================================*/
void UAM_S_LIBS_serialize_String(const UAS_UInt8* pzVal,
                             const UAM_S_Size sValLen,
                             UAS_UInt8* pData,
                             const UAM_S_Size sLen,
                             UAM_S_Size* pPos)
{
    if (sValLen + (*pPos) <= sLen)
    {
        UAM_S_LIBS_MemCopy(&pData[*pPos], pzVal, sValLen);
        *pPos += sValLen;
    }
}

/*===========================================================================*/
UAS_UInt32 UAM_S_LIBS_deserialize_UInt32(const UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos)
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
UAS_UInt8 UAM_S_LIBS_deserialize_UInt8(const UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos)
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
void UAM_S_LIBS_deserialize_String(const UAS_UInt8* pData,
                               const UAM_S_Size sLen,
                               UAM_S_Size* pPos,
                               UAS_UInt8* pDest,
                               const UAM_S_Size sValLen)
{
    if (sValLen + *pPos <= sLen)
    {
        UAM_S_LIBS_MemCopy (pDest, pData, sValLen);
    }
    *pPos += sValLen;
}

