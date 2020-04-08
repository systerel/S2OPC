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

#include <assert.h>
#include "sopc_helper_uri.h"
#include "sopc_mem_alloc.h"

#include "config.h"
#include "config_cyclic.h"

SOPC_NodeId* g_nodeIdArray[CONF_NB_NODE_TO_WRITE];

SOPC_BuiltinId CONF_DV_TYPE_ARRAY[CONF_NB_NODE_TO_WRITE] = {SOPC_SByte_Id,  SOPC_Byte_Id,  SOPC_Int16_Id,
                                                            SOPC_UInt16_Id, SOPC_Int32_Id, SOPC_UInt32_Id,
                                                            SOPC_Int64_Id,  SOPC_UInt64_Id};

char* CONF_NODE_ID_ARRAY[CONF_NB_NODE_TO_WRITE] = {
    "ns=1;i=1007", // SByte
    "ns=1;i=1008", // Byte
    "ns=1;i=1009", // Int16
    "ns=1;i=1010", // UInt16
    "ns=1;i=1011", // Int32
    "ns=1;i=1002", // UInt32
    "ns=1;i=1001", // Int64
    "ns=1;i=1012"  // UInt64
};

SOPC_ReturnStatus conf_initialize_nodeid_array()
{
    memset(g_nodeIdArray, 0, CONF_NB_NODE_TO_WRITE * sizeof(SOPC_NodeId*));

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
    {
        assert(strlen(CONF_NODE_ID_ARRAY[i]) <= INT32_MAX);

        g_nodeIdArray[i] = SOPC_NodeId_FromCString(CONF_NODE_ID_ARRAY[i], (int32_t) strlen(CONF_NODE_ID_ARRAY[i]));
        if (NULL == g_nodeIdArray[i])
        {
            printf("# Error: nodeid %d not recognized: \"%s\"\n", i, CONF_NODE_ID_ARRAY[i]);
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}
