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

#include "node_id_pointer_bs.h"
#include "sopc_mem_alloc.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void node_id_pointer_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void node_id_pointer_bs__copy_node_id_pointer_content(const constants__t_NodeId_i node_id_pointer_bs__p_node_id,
                                                      t_bool* const node_id_pointer_bs__bres,
                                                      constants__t_NodeId_i* const node_id_pointer_bs__p_node_id_copy)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    *node_id_pointer_bs__p_node_id_copy = constants__c_NodeId_indet;
    if (constants__c_NodeId_indet != node_id_pointer_bs__p_node_id)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        SOPC_NodeId* nodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        if (NULL != nodeId)
        {
            SOPC_NodeId_Initialize(nodeId);
            status = SOPC_NodeId_Copy(nodeId, node_id_pointer_bs__p_node_id);
            if (SOPC_STATUS_OK == status)
            {
                *node_id_pointer_bs__p_node_id_copy = nodeId;
            }
            else
            {
                SOPC_Free(nodeId);
            }
        }
    }
    *node_id_pointer_bs__bres = (SOPC_STATUS_OK == status);
}

void node_id_pointer_bs__free_node_id_pointer(const constants__t_NodeId_i node_id_pointer_bs__p_node_id)
{
    SOPC_NodeId_Clear(node_id_pointer_bs__p_node_id);
    SOPC_Free(node_id_pointer_bs__p_node_id);
}
