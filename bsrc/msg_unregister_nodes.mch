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
    msg_unregister_nodes

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_inv == (
        nb_nodes : NAT &
        nb_nodes <= k_n_unregisterNodes_max &

        UnregisterNodesReq_NodeId      : t_unregisterNodesReqIndex +-> t_NodeId
    )

ABSTRACT_VARIABLES
    nb_nodes,
    UnregisterNodesReq_NodeId

INVARIANT
    d_inv

INITIALISATION
    nb_nodes           := 0  ||
    UnregisterNodesReq_NodeId      := {}

OPERATIONS

    p_sc, p_nb_nodes <-- getall_msg_unregister_nodes_req_nb_nodes (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_unregister_nodes_req
    THEN
        p_sc,
        p_nb_nodes,
        nb_nodes,
        UnregisterNodesReq_NodeId
        :(  p_sc       : t_StatusCode &
            p_nb_nodes : NAT &
            nb_nodes <= k_n_unregisterNodes_max &
            UnregisterNodesReq_NodeId : t_unregisterNodesReqIndex +-> t_NodeId &

            (p_sc = e_sc_ok
                =>
                p_nb_nodes : t_unregisterNodesReqIndex &
                nb_nodes = p_nb_nodes &
                UnregisterNodesReq_NodeId : 1..nb_nodes --> t_NodeId) &
            (p_sc /= e_sc_ok
                => p_sc : {e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops} &
                   p_nb_nodes = 0)
        )
    END
    ;

    p_node_id  <-- get_msg_unregister_nodes_req_node_id (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_unregister_nodes_req &
        p_index : NAT &
        p_index : t_unregisterNodesReqIndex &
        p_index > 0 &
        p_index <= nb_nodes
    THEN
        p_node_id:(p_node_id : t_NodeId_i &  p_node_id = UnregisterNodesReq_NodeId(p_index))
    END

END
