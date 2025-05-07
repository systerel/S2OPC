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

#include "msg_node_management_delete_nodes_bs.h"

#include "sopc_mem_alloc.h"
#include "sopc_types.h"
#include "util_b2c.h"

/*------------------------
 INITIALISATION Clause
------------------------*/
void msg_node_management_delete_nodes_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_node_management_delete_nodes_bs__alloc_msg_delete_nodes_resp_results(
    const constants__t_msg_i msg_node_management_delete_nodes_bs__p_resp_msg,
    const t_entier4 msg_node_management_delete_nodes_bs__p_nb_results,
    t_bool* const msg_node_management_delete_nodes_bs__bres)
{
    *msg_node_management_delete_nodes_bs__bres = false;
    OpcUa_DeleteNodesResponse* deleteNodesResp =
        (OpcUa_DeleteNodesResponse*) msg_node_management_delete_nodes_bs__p_resp_msg;
    if (msg_node_management_delete_nodes_bs__p_nb_results > 0)
    {
        if (SIZE_MAX / (uint32_t) msg_node_management_delete_nodes_bs__p_nb_results > sizeof(*deleteNodesResp->Results))
        {
            deleteNodesResp->NoOfResults = msg_node_management_delete_nodes_bs__p_nb_results;
            deleteNodesResp->Results = SOPC_Calloc((size_t) msg_node_management_delete_nodes_bs__p_nb_results,
                                                   sizeof(*deleteNodesResp->Results));
            if (NULL != deleteNodesResp->Results)
            {
                for (int32_t i = 0; i < deleteNodesResp->NoOfResults; i++)
                {
                    deleteNodesResp->Results[i] = OpcUa_BadInternalError;
                }
                *msg_node_management_delete_nodes_bs__bres = true;
            }
        }
    }
    else
    {
        deleteNodesResp->NoOfResults = 0;
        *msg_node_management_delete_nodes_bs__bres = true;
    }
}

void msg_node_management_delete_nodes_bs__get_msg_delete_nodes_req_nb_delete_nodes(
    const constants__t_msg_i msg_node_management_delete_nodes_bs__p_req_msg,
    t_entier4* const msg_node_management_delete_nodes_bs__p_nb_delete_nodes)
{
    const OpcUa_DeleteNodesRequest* deleteNodesReq =
        (const OpcUa_DeleteNodesRequest*) msg_node_management_delete_nodes_bs__p_req_msg;
    *msg_node_management_delete_nodes_bs__p_nb_delete_nodes = deleteNodesReq->NoOfNodesToDelete;
}

void msg_node_management_delete_nodes_bs__getall_delete_node_item_req_params(
    const constants__t_msg_i msg_node_management_delete_nodes_bs__p_req_msg,
    const t_entier4 msg_node_management_delete_nodes_bs__p_index,
    constants__t_NodeId_i* const msg_node_management_delete_nodes_bs__p_nodeId,
    t_bool* const msg_node_management_delete_nodes_bs__p_deleteTargetReferences)
{
    // index is ensured to be valid by B model and NodesToDelete array to be valid due to decoding message success
    OpcUa_DeleteNodesRequest* deleteNodesReq =
        (OpcUa_DeleteNodesRequest*) msg_node_management_delete_nodes_bs__p_req_msg;
    OpcUa_DeleteNodesItem* deleteNodesItem =
        &deleteNodesReq->NodesToDelete[msg_node_management_delete_nodes_bs__p_index - 1];

    *msg_node_management_delete_nodes_bs__p_deleteTargetReferences = deleteNodesItem->DeleteTargetReferences;
    *msg_node_management_delete_nodes_bs__p_nodeId = &deleteNodesItem->NodeId;
}

void msg_node_management_delete_nodes_bs__setall_msg_delete_nodes_item_resp_params(
    const constants__t_msg_i msg_node_management_delete_nodes_bs__p_resp_msg,
    const t_entier4 msg_node_management_delete_nodes_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_node_management_delete_nodes_bs__p_sc)
{
    OpcUa_DeleteNodesResponse* deleteNodesResp =
        (OpcUa_DeleteNodesResponse*) msg_node_management_delete_nodes_bs__p_resp_msg;
    SOPC_StatusCode* deleteNodesResult = &deleteNodesResp->Results[msg_node_management_delete_nodes_bs__p_index - 1];
    constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(msg_node_management_delete_nodes_bs__p_sc,
                                                                      deleteNodesResult);
}
