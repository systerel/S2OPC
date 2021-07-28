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
 * SAFE and NON-SAFE softwares (no system dependencies)
 */

#ifndef SOPC_UAM_S_LIBS_H_
#define SOPC_UAM_S_LIBS_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/
#include "uas.h"
#include "uam_s.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

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


void UAM_S_LIBS_HEAP_Init(UAM_LIBS_Heap_type* pzHeap);
void* UAM_S_LIBS_HEAP_Malloc(UAM_LIBS_Heap_type* pzHeap, const UAM_S_Size len);
void UAM_S_LIBS_HEAP_Clear(UAM_LIBS_Heap_type* pzHeap);
void UAM_S_LIBS_MemZero(void* pAddr,  const UAM_S_Size sLen);
#define UAM_S_LIBS_VarZero(v) UAM_S_LIBS_MemZero (&(v) , sizeof(v) );
void UAM_S_LIBS_MemCopy(void* pDest,  const void* pSource,  const UAM_S_Size sLen);
UAS_UInt32 UAM_S_LIBS_nothl(const UAS_UInt32 uLong);
UAS_UInt16 UAM_S_LIBS_noths(const UAS_UInt16 ushort);
void UAM_S_LIBS_ExitFailure (const UAS_Int32 sCode);


#define _UAM_S_LIBS_XSTRINGIFY(s) _UAM_S_LIBS_STRINGIFY(s)
#define _UAM_S_LIBS_STRINGIFY(s) #s
#define UAM_S_LIBS_ASSERT(c) do \
    { \
        if (! (c)) \
        {\
        	UAM_S_DoLog (UAM_S_LOG_ERROR, "ASSERT failed from file "__FILE__" line " _UAM_S_LIBS_XSTRINGIFY(__LINE__) ", assert = <" _UAM_S_LIBS_XSTRINGIFY(c) ">");\
        	UAM_S_LIBS_ExitFailure (1); \
        }\
    } while (0)

/*===========================================================================*/
void UAM_S_LIBS_serialize_UInt32(const UAS_UInt32 uVal, UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos);
void UAM_S_LIBS_serialize_UInt8(const UAS_UInt8 uVal, UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos);
void UAM_S_LIBS_serialize_String(const UAS_UInt8* pzVal,
                             const UAM_S_Size sValLen,
                             UAS_UInt8* pData,
                             const UAM_S_Size sLen,
                             UAM_S_Size* pPos);
UAS_UInt32 UAM_S_LIBS_deserialize_UInt32(const UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos);
UAS_UInt8 UAM_S_LIBS_deserialize_UInt8(const UAS_UInt8* pData, const UAM_S_Size sLen, UAM_S_Size* pPos);
void UAM_S_LIBS_deserialize_String(const UAS_UInt8* pData,
                               const UAM_S_Size sLen,
                               UAM_S_Size* pPos,
                               UAS_UInt8* pDest,
                               const UAM_S_Size sValLen);


// TODO : include somewhere in the build environment
#define IS_LITTLE_ENDIAN


#endif
