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

#include "msg_unregister_nodes_bs.h"

#include <assert.h>

void msg_unregister_nodes_bs__INITIALISATION(void) {}

void msg_unregister_nodes_bs__get_msg_unregister_nodes_req_nb_nodes(
    const constants__t_msg_i msg_unregister_nodes_bs__p_req_msg,
    t_entier4* const msg_unregister_nodes_bs__p_nb_nodes)
{
    OpcUa_UnregisterNodesRequest* request = msg_unregister_nodes_bs__p_req_msg;
    *msg_unregister_nodes_bs__p_nb_nodes = request->NoOfNodesToUnregister;
}

void msg_unregister_nodes_bs__get_msg_unregister_nodes_req_node_id(
    const constants__t_msg_i msg_unregister_nodes_bs__p_req_msg,
    const t_entier4 msg_unregister_nodes_bs__p_index,
    constants__t_NodeId_i* const msg_unregister_nodes_bs__p_node_id)
{
    OpcUa_UnregisterNodesRequest* request = msg_unregister_nodes_bs__p_req_msg;
    assert(msg_unregister_nodes_bs__p_index > 0 && msg_unregister_nodes_bs__p_index <= request->NoOfNodesToUnregister);
    *msg_unregister_nodes_bs__p_node_id = &request->NodesToUnregister[msg_unregister_nodes_bs__p_index - 1];
}
