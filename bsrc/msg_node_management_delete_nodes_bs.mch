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
    msg_node_management_delete_nodes_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_inv == (
        DeleteNodeItemReq_nodeId    : t_nodeToDeleteReqIndex +-> t_NodeId &
        DeleteNodeItemReq_deleteTargetReferences : t_nodeToDeleteReqIndex +-> BOOL &

        /* Response content */
        DeleteNodeItemResp_Allocated : BOOL &
        DeleteNodeItemResp_StatusCode : t_nodeToDeleteReqIndex +-> t_StatusCode
    );

    d_var ==
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

    p_nodeId, p_deleteTargetReferences <-- getall_delete_node_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_delete_nodes_req &
        p_index : NAT &
        p_index : t_nodeToDeleteReqIndex &
        p_index > 0
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
        p_nb_results > 0
    THEN
        bres, DeleteNodeItemResp_Allocated
        :(bres : BOOL &
          DeleteNodeItemResp_Allocated : BOOL &
          DeleteNodeItemResp_Allocated = bres)
    END
    ;

    setall_msg_delete_nodes_item_resp_params(p_resp_msg, p_index, p_sc) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_delete_nodes_resp &
        p_index : NAT &
        p_index > 0 &
        p_index : t_nodeToDeleteReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        DeleteNodeItemResp_StatusCode(p_index) := p_sc
    END
    ;

    p_nb_delete_nodes <-- get_msg_delete_nodes_req_nb_delete_nodes(p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_delete_nodes_req
    THEN
        p_nb_delete_nodes :: INT
    END

END
