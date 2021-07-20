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
#include "uam_ns2s_itf.h"
#include "uam.h"
#include "uas.h"


#include "sopc_common.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_dict.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
/**
 * A dictionary object { UAM_SessionHandle : UAM_NS_Configuration_type* }
 */
static SOPC_Dict* gSessions = NULL; // TODO : not sure that is really useful!

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
static uint64_t Session_KeyHash_Fct(const void* pKey)
{
    return (const UAM_SessionHandle)(const UAS_INVERSE_PTR)pKey;
}

/*===========================================================================*/
static bool Session_KeyEqual_Fct (const void* a, const void* b)
{
    return a == b;
}

/*===========================================================================*/
static void Session_CloseFcn (const void* key, const void* value, void* user_data)
{
    assert (user_data == NULL);
    assert (value != NULL);
    UAM_NS2S_Clear((const UAM_SessionHandle)(const UAS_INVERSE_PTR)key);
}

/*===========================================================================*/
static UAM_NS_Configuration_type* Session_Get (const UAM_SessionHandle key)
{
    return SOPC_Dict_Get (gSessions, (void*) (UAS_INVERSE_PTR) key, NULL);
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
void UAM_NS_Initialize(void)
{
    assert (gSessions == NULL);
    gSessions = SOPC_Dict_Create (NULL, Session_KeyHash_Fct, Session_KeyEqual_Fct, NULL, SOPC_Free);
    assert (gSessions != NULL);
    LOG_Trace (LOG_DEBUG, "UAM_NS_Initialize OK!");
}


/*===========================================================================*/
bool UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig)
{
    assert (gSessions != NULL); // TODO Remove
    bool bResult = false;
    if  (gSessions != NULL &&  pzConfig != NULL)
    {
        bResult = Session_Add (pzConfig);
        if (bResult && pzConfig->pfSetup != NULL)
        {
            LOG_Trace (LOG_DEBUG, "UAM_NS_CreateSpdu, HDL=%u",(unsigned) pzConfig->dwHandle);
            bResult = (*pzConfig->pfSetup) (pzConfig->dwHandle, pzConfig->pUserParams);
        }
        if (bResult)
        {
            bResult = UAM_NS2S_Initialize(pzConfig->dwHandle);
            printf("UAM_NS2S_Initialize ret= %d\n", bResult); // TODO
        }
    }
    return bResult;
}

/*===========================================================================*/
void UAM_NS_MessageReceived (UAM_SessionHandle dwHandle, const void* pData, const size_t sLen)
{
    const UAM_NS_Configuration_type* pzConfig  = Session_Get (dwHandle);

    if (pzConfig != NULL)
    {
        LOG_Trace (LOG_DEBUG, "Received message on HDL=%u (len=%u)", (unsigned) dwHandle, (unsigned) sLen);
        UAM_NS2S_SendSpduImpl (pData, sLen, dwHandle);
    }
}

/*===========================================================================*/
void UAM_NS_Clear(void)
{
    assert (gSessions != NULL);
    SOPC_Dict_ForEach(gSessions, &Session_CloseFcn, NULL);
    SOPC_Dict_Delete(gSessions);
    gSessions = NULL;
}
