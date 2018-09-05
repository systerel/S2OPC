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

MACHINE
    msg_unregister_nodes_bs

SEES
    constants,
    message_in_bs,
    message_out_bs

OPERATIONS

    p_nb_nodes <-- get_msg_unregister_nodes_req_nb_nodes (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_unregister_nodes_req
    THEN
        p_nb_nodes :: INT
    END
    ;

    p_node_id <-- get_msg_unregister_nodes_req_node_id (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_unregister_nodes_req &
        p_index : NAT &
        p_index : t_unregisterNodesReqIndex
    THEN
        p_node_id
        :(p_node_id : t_NodeId_i & p_node_id : t_NodeId)
    END

END
