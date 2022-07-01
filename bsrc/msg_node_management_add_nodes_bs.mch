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
    msg_node_management_add_nodes_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    p_sc, p_parentExpNid, p_refTypeId, p_reqExpNodeId, p_browseName, p_nodeClass, p_nodeAttributes, p_typeDefId
      <-- getall_add_node_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_add_nodes_req &
        p_index : NAT &
        p_index : t_nodeToAddReqIndex &
        p_index > 0
    THEN
        p_sc,
        p_parentExpNid,
        p_refTypeId,
        p_reqExpNodeId,
        p_browseName,
        p_nodeClass,
        p_nodeAttributes,
        p_typeDefId
        :(
            p_sc : t_StatusCode & p_sc : {e_sc_ok, e_sc_bad_node_id_rejected, e_sc_bad_node_class_invalid,
                                          e_sc_bad_node_attributes_invalid, e_sc_bad_browse_name_invalid,
                                          e_sc_bad_type_definition_invalid} &
            p_parentExpNid : t_ExpandedNodeId_i & p_parentExpNid : t_ExpandedNodeId &
            p_refTypeId : t_NodeId_i & p_refTypeId : t_NodeId &
            p_reqExpNodeId : t_ExpandedNodeId_i &
            p_browseName : t_QualifiedName_i & p_browseName : t_QualifiedName &
            p_nodeClass : t_NodeClass_i & p_nodeClass : t_NodeClass &
            p_nodeAttributes : t_NodeAttributes_i & p_nodeAttributes : t_NodeAttributes &
            p_typeDefId : t_ExpandedNodeId_i
        )
    END
    ;

    bres <-- alloc_msg_add_nodes_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_add_nodes_resp &
        p_nb_results : NAT &
        p_nb_results > 0
    THEN
        bres :: BOOL
    END
    ;

    setall_msg_add_nodes_item_resp_params (p_resp_msg, p_index, p_sc, p_nid) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_add_nodes_resp &
        p_index : NAT &
        p_index > 0 &
        p_index : t_nodeToAddReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_nid : t_NodeId_i &
        (p_sc = e_sc_ok => p_nid : t_NodeId) &
        (p_sc /= e_sc_ok => p_nid = c_NodeId_indet)
    THEN
        skip
    END
    ;

    p_nb_add_nodes <-- get_msg_create_add_nodes_req_nb_add_nodes (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_nb_add_nodes :: INT
    END

END
