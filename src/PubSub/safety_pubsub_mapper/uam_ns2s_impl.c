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
 *      Implementation of services defined in "uam_ns2s_itf.h" using "Named pipes":
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

#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
/**
 * A dictionary object { UAM_SessionHandle : FifoFileHandle }
 */
static SOPC_Dict* gFifos = NULL;

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
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
static int fifo_Get (const UAM_SessionHandle key)
{
    return (int)(UAS_INVERSE_PTR) SOPC_Dict_Get (gFifos, (void*) (UAS_INVERSE_PTR) key, NULL);
}

/*===========================================================================*/
static bool fifo_Add (const UAM_SessionHandle dwHandle, const char* const sFilename)
{
    bool bResult = false;
    assert (NULL != sFilename);

    int pzPrevHandle = fifo_Get (dwHandle);
    if (pzPrevHandle == 0)
    {
        int handle= TODO (open file);
        // TODO WIP
        UAM_NS_Configuration_type* pzNewConfig = SOPC_Malloc (sizeof(*pzNewConfig));
        bResult = (NULL != pzNewConfig);
        if (bResult)
        {
            memcpy(pzNewConfig, pzConfig, sizeof(*pzConfig));
            // Note : Values and Keys are freed
            bResult = SOPC_Dict_Insert (gSessions, (void*)(UAS_INVERSE_PTR)pzConfig->dwHandle, pzNewConfig);
        }
    }
    return bResult;
}


/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
bool UAM_NS2S_Initialize(const UAM_SessionHandle dwHandle)
{
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_Initialize (%u)", (unsigned) dwHandle);
    char filename [50];
    int iResult =  -1;

    if (gFifos == NULL)
    {
        gFifos = SOPC_Dict_Create (NULL, fifo_KeyHash_Fct, fifo_KeyEqual_Fct, NULL, NULL);
        assert (gFifos != NULL);
    }
    snprintf(filename, sizeof(filename), "/tmp/uas-sh%u.fifo", dwHandle);
    iResult = mkfifo (filename, 0666);
    // Todo : store in gFifos

    return  iResult == 0;
}

/*===========================================================================*/
void UAM_NS2S_SendSpduImpl(const void* const pData, const size_t sLen, const UAM_SessionHandle dwHandle)
{
    assert (dwHandle == 0x010203u); // TODO remove
    if (pData == NULL || sLen == 0)
    {
        return;
    }
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_SendSpduImpl(%p, %u, %u)\n", pData, (unsigned)sLen, (unsigned) dwHandle);
    // TODO

}

/*===========================================================================*/
void UAM_NS2S_Clear(const UAM_SessionHandle dwHandle)
{
    LOG_Trace (LOG_DEBUG, "UAM_NS2S_Clear (%u)", (unsigned) dwHandle);
    // TODO
}

