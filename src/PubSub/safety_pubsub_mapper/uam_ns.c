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
 * TODO:
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam_ns.h"
#include "uam.h"
#include "uas.h"

#include "sopc_common.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_dict.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
/**
 * A dictionary object { UAM_SessionHandle* : UAM_NS_Configuration_type* }
 */
static SOPC_Dict* gSessions = NULL; // TODO : not sure that is really useful!

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static uint64_t Session_KeyHash_Fct(const void* pKey)
{
    uint64_t result = 0xFFFFFFFFFFFFFFFF;
    if (pKey != 0)
    {
        const UAM_SessionHandle* pHandle = (const UAM_SessionHandle*)pKey;
        return *pHandle;
    }
    return result;
}

/*===========================================================================*/
static bool Session_KeyEqual_Fct (const void* a, const void* b)
{
    const UAM_SessionHandle h1 = *(const UAM_SessionHandle*)a;
    const UAM_SessionHandle h2 = *(const UAM_SessionHandle*)b;
    return h1 == h2;
}

/*===========================================================================*/
static UAM_NS_Configuration_type* Session_Get (const UAM_SessionHandle key)
{
    return SOPC_Dict_Get (gSessions, &key, NULL);
}

/*===========================================================================*/
static bool Session_Add (const UAM_NS_Configuration_type* const pzConfig)
{
    bool bResult = false;
    assert (NULL != pzConfig);

    UAM_NS_Configuration_type* pzPrevConfig = Session_Get (pzConfig->dwHandle);
    if (pzPrevConfig == NULL)
    {
        UAM_NS_Configuration_type* pzNewConfig = SOPC_Malloc (sizeof(*pzNewConfig));
        UAM_SessionHandle* pKey = NULL;
        bResult = (NULL != pzNewConfig);
        if (bResult)
        {
            memcpy(pzNewConfig, pzConfig, sizeof(*pzConfig));
            pKey = SOPC_Malloc(sizeof(*pKey));
            bResult = (NULL != pKey);
        }
        if (bResult)
        {
            // Note : Values and Keys are freed
            SOPC_Dict_Insert (gSessions, pKey, pzNewConfig);
        }
    }
    return bResult;
}

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_NS_Initialize(void)
{
    assert (gSessions == NULL);
    gSessions = SOPC_Dict_Create (NULL, Session_KeyHash_Fct, Session_KeyEqual_Fct, SOPC_Free, SOPC_Free);
    assert (gSessions != NULL);
}


/*===========================================================================*/
bool UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig)
{
    bool bResult = false;
    if  (gSessions != NULL &&  pzConfig != NULL)
    {
        bResult = Session_Add (pzConfig);
        if (bResult && pzConfig->pfSetup != NULL)
        {
            bResult = (*pzConfig->pfSetup) (pzConfig->dwHandle, pzConfig->pUserParams);
        }
    }
    return bResult;
}

/*===========================================================================*/
void UAM_NS_Clear(void)
{
    SOPC_Dict_Delete(gSessions);
    gSessions = NULL;
}
