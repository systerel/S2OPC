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

#include "msg_register_nodes_bs.h"
#include "sopc_mem_alloc.h"

#include <assert.h>

void msg_register_nodes_bs__INITIALISATION(void) {}

void msg_register_nodes_bs__alloc_msg_register_nodes_resp_results(
    const constants__t_msg_i msg_register_nodes_bs__p_resp_msg,
    const t_entier4 msg_register_nodes_bs__p_nb_results,
    t_bool* const msg_register_nodes_bs__bres)
{
    assert(msg_register_nodes_bs__p_nb_results > 0);

    OpcUa_RegisterNodesResponse* response = msg_register_nodes_bs__p_resp_msg;
    response->RegisteredNodeIds = SOPC_Calloc((size_t) msg_register_nodes_bs__p_nb_results, sizeof(SOPC_NodeId));

    if (response->RegisteredNodeIds != NULL)
    {
        for (int32_t i = 0; i < msg_register_nodes_bs__p_nb_results; ++i)
        {
            SOPC_NodeId_Initialize(&response->RegisteredNodeIds[i]);
        }

        response->NoOfRegisteredNodeIds = msg_register_nodes_bs__p_nb_results;
        *msg_register_nodes_bs__bres = true;
    }
    else
    {
        response->NoOfRegisteredNodeIds = 0;
        *msg_register_nodes_bs__bres = false;
    }
}

void msg_register_nodes_bs__get_msg_register_nodes_req_nb_nodes(
    const constants__t_msg_i msg_register_nodes_bs__p_req_msg,
    t_entier4* const msg_register_nodes_bs__p_nb_nodes)
{
    OpcUa_RegisterNodesRequest* request = msg_register_nodes_bs__p_req_msg;
    *msg_register_nodes_bs__p_nb_nodes = request->NoOfNodesToRegister;
}

void msg_register_nodes_bs__get_msg_register_nodes_req_node_id(
    const constants__t_msg_i msg_register_nodes_bs__p_req_msg,
    const t_entier4 msg_register_nodes_bs__p_index,
    constants__t_NodeId_i* const msg_register_nodes_bs__p_node_id)
{
    OpcUa_RegisterNodesRequest* request = msg_register_nodes_bs__p_req_msg;
    assert(msg_register_nodes_bs__p_index > 0 && msg_register_nodes_bs__p_index <= request->NoOfNodesToRegister);
    *msg_register_nodes_bs__p_node_id = &request->NodesToRegister[msg_register_nodes_bs__p_index - 1];
}

void msg_register_nodes_bs__setall_msg_register_nodes_resp_node_id(
    const constants__t_msg_i msg_register_nodes_bs__p_resp_msg,
    const t_entier4 msg_register_nodes_bs__p_index,
    const constants__t_NodeId_i msg_register_nodes_bs__p_node_id,
    t_bool* const msg_register_nodes_bs__bres)
{
    OpcUa_RegisterNodesResponse* response = msg_register_nodes_bs__p_resp_msg;
    assert(msg_register_nodes_bs__p_index > 0 && msg_register_nodes_bs__p_index <= response->NoOfRegisteredNodeIds);

    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&response->RegisteredNodeIds[msg_register_nodes_bs__p_index - 1],
                                                msg_register_nodes_bs__p_node_id);
    *msg_register_nodes_bs__bres = (status == SOPC_STATUS_OK);
}
