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
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include "uam_s_libs.h"

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_S_LIBS_MemZero(void* pAddr,  const UAS_UInt32 sLen)
{
    memset (pAddr, 0, sLen);
    // TODO : to replace memset, we can do a loop with UINT32 filled to 0 and then filling last bytes with 0
    /// to optimize loop
}


void UAM_S_LIBS_HEAP_Init(UAM_LIBS_Heap_type* pzHeap)
{
    assert (pzHeap != 0);
    assert (!pzHeap->initialized);
    pzHeap->pos = 0;
    pzHeap->initialized = true;
}

void* UAM_S_LIBS_HEAP_Malloc(UAM_LIBS_Heap_type* pzHeap, const UAS_UInt32 len)
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

void UAM_S_LIBS_HEAP_Clear(UAM_LIBS_Heap_type* pzHeap)
{
    assert (pzHeap != 0);
    if (pzHeap->initialized)
    {
        pzHeap->initialized = false;
    }
}
