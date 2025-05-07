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
    msg_node_management_delete_nodes

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_inv == (
        nb_nodes_to_delete : NAT &
        nb_nodes_to_delete <= k_n_nodesToDelete_max &
        DeleteNodeItemReq_nodeId    : t_nodeToDeleteReqIndex +-> t_NodeId &
        DeleteNodeItemReq_deleteTargetReferences : t_nodeToDeleteReqIndex +-> BOOL &

        /* Response content */
        DeleteNodeItemResp_Allocated : BOOL &
        DeleteNodeItemResp_StatusCode : t_nodeToDeleteReqIndex +-> t_StatusCode
    );

    d_var ==
    nb_nodes_to_delete,
    DeleteNodeItemReq_nodeId,
    DeleteNodeItemReq_deleteTargetReferences,

    DeleteNodeItemResp_Allocated,
    DeleteNodeItemResp_StatusCode

ABSTRACT_VARIABLES
    d_var

INVARIANT
    d_inv

INITIALISATION
    d_var :( d_inv )

OPERATIONS

    p_sc, p_nb_nodes_to_delete <-- getall_msg_delete_nodes_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_delete_nodes_req
    THEN
        p_sc,
        p_nb_nodes_to_delete,
        nb_nodes_to_delete,
        DeleteNodeItemReq_nodeId,
        DeleteNodeItemReq_deleteTargetReferences
        :(  p_sc                 : t_StatusCode &
            p_nb_nodes_to_delete : NAT &
            nb_nodes_to_delete = p_nb_nodes_to_delete &
            (p_sc = e_sc_ok
                =>
                p_nb_nodes_to_delete > 0 &
                p_nb_nodes_to_delete <= k_n_nodesToDelete_max &
                p_nb_nodes_to_delete : t_nodeToDeleteReqIndex &
                DeleteNodeItemReq_nodeId                    : 1..nb_nodes_to_delete --> t_NodeId &
                DeleteNodeItemReq_deleteTargetReferences    : 1..nb_nodes_to_delete --> BOOL
            ) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops} &
                p_nb_nodes_to_delete = 0 &
                d_inv
            )
        )
    END
    ;

    p_nodeId, p_deleteTargetReferences <-- getall_delete_node_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_delete_nodes_req &
        p_index : NAT &
        p_index : t_nodeToDeleteReqIndex &
        p_index > 0 &
        p_index <= nb_nodes_to_delete
    THEN
        p_nodeId,
        p_deleteTargetReferences
        :(
            p_nodeId : t_NodeId_i &
            p_nodeId = DeleteNodeItemReq_nodeId(p_index) &
            p_deleteTargetReferences : BOOL &
            p_deleteTargetReferences = DeleteNodeItemReq_deleteTargetReferences(p_index)
        )
    END
    ;

    bres <-- alloc_msg_delete_nodes_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_delete_nodes_resp &
        p_nb_results : NAT &
        p_nb_results > 0 &
        p_nb_results = nb_nodes_to_delete
    THEN
        bres, DeleteNodeItemResp_Allocated
        :(bres : BOOL &
          DeleteNodeItemResp_Allocated : BOOL &
          DeleteNodeItemResp_Allocated = bres)
    END
    ;

    setall_msg_delete_nodes_item_resp_params (p_resp_msg, p_index, p_sc) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_delete_nodes_resp &
        p_index : NAT &
        p_index : t_nodeToDeleteReqIndex &
        p_index > 0 &
        p_index <= nb_nodes_to_delete &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        DeleteNodeItemResp_StatusCode(p_index) := p_sc
    END

END
