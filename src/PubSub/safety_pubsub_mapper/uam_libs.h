/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 *
 * Modifications: adaptation for S2OPC project
 * ======================================================================*/

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/
/** \file This file contains the UAM utility functions. They can be used by both
 * SAFE and NON-SAFE softwares (no system depandancies
 */

#ifndef SOPC_UAM_LIBS_H_
#define SOPC_UAM_LIBS_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/
#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

void UAM_LIBS_MemZero(void* pAddr,  const UAS_UInt32 sLen);

/*===========================================================================*/
void UAM_LIBS_serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos);

/*===========================================================================*/
void UAM_LIBS_serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos);
/*===========================================================================*/
void UAM_LIBS_serialize_String(const UAS_UInt8* pzVal,
                             const UAS_UInt32 sValLen,
                             UAS_UInt8* pData,
                             const UAS_UInt32 sLen,
                             UAS_UInt32* pPos);

/*===========================================================================*/
UAS_UInt32 UAM_LIBS_deserialize_UInt32(const UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos);

/*===========================================================================*/
UAS_UInt8 UAM_LIBS_deserialize_UInt8(const UAS_UInt8* pData, const UAS_UInt32 sLen, UAS_UInt32* pPos);


/*===========================================================================*/
void UAM_LIBS_deserialize_String(const UAS_UInt8* pData,
                               const UAS_UInt32 sLen,
                               UAS_UInt32* pPos,
                               SOPC_ByteString* pzDest,
                               const UAS_UInt32 sValLen);

/*===========================================================================*/

#ifndef HEAP_SIZE
#define HEAP_SIZE 1024ul * 128u
#endif

typedef struct UAM_LIBS_Heap_struct
{
    bool initialized;
    UAS_UInt64 pos;
    char buffer[HEAP_SIZE];
} UAM_LIBS_Heap_type;


void UAM_LIBS_HEAP_Init(UAM_LIBS_Heap_type* pzHeap);
void* UAM_LIBS_HEAP_Malloc(UAM_LIBS_Heap_type* pzHeap, const size_t len);
void UAM_LIBS_HEAP_Clear(UAM_LIBS_Heap_type* pzHeap);


#endif
