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

#include <assert.h>
#include <string.h>

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
typedef struct UAM_NS_DynamicConfig_struct
{
    UAM_NS_SpduHandle hNextFreeHandle;
} UAM_NS_DynamicConfig_type;

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static UAM_NS_DynamicConfig_type zConfig;
static bool bInitialized = false;

/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
void UAM_NS_Initialize(void)
{
    assert (!bInitialized);
    zConfig.hNextFreeHandle = 0;
    //TODO

    bInitialized = true;
}


/*===========================================================================*/
UAM_NS_SpduHandle UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig)
{
    UAM_NS_SpduHandle hResult = UAM_NoHandle;
    if  (bInitialized &&  pzConfig != NULL)
    {
        hResult = zConfig.hNextFreeHandle;
        zConfig.hNextFreeHandle++;
        //TODO
    }
    return hResult;
}

/*===========================================================================*/
void UAM_NS_Clear(void)
{
    //TODO
    bInitialized = false;
}
