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
    msg_node_management_add_nodes

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_inv == (
        nb_nodes_to_add : NAT &
        nb_nodes_to_add <= k_n_nodesToAdd_max &
        AddNodeItemReq_ParentNodeId       : t_nodeToAddReqIndex +-> t_ExpandedNodeId &
        AddNodeItemReq_ReferenceTypeId    : t_nodeToAddReqIndex +-> t_NodeId &
        AddNodeItemReq_RequestedNewNodeId : t_nodeToAddReqIndex +-> t_ExpandedNodeId_i & // Might be NULL ExpandedNodeId if server assigns a fresh one
        AddNodeItemReq_BrowseName         : t_nodeToAddReqIndex +-> t_QualifiedName &
        AddNodeItemReq_NodeClass          : t_nodeToAddReqIndex +-> t_NodeClass_i & // Validity not guaranteed by message decoding
        AddNodeItemReq_NodeAttributes     : t_nodeToAddReqIndex +-> t_NodeAttributes &
        AddNodeItemReq_TypeDefinition     : t_nodeToAddReqIndex +-> t_ExpandedNodeId_i &

        /* Response content */

        AddNodeItemResp_Allocated : BOOL &
        AddNodeItemResp_StatusCode : t_nodeToAddReqIndex +-> t_StatusCode &
        AddNodeItemResp_AddedNodeId : t_nodeToAddReqIndex +-> t_NodeId_i
    );

    d_var ==
    nb_nodes_to_add,
    AddNodeItemReq_ParentNodeId,
    AddNodeItemReq_ReferenceTypeId,
    AddNodeItemReq_RequestedNewNodeId,
    AddNodeItemReq_BrowseName,
    AddNodeItemReq_NodeClass,
    AddNodeItemReq_NodeAttributes,
    AddNodeItemReq_TypeDefinition,

    AddNodeItemResp_Allocated,
    AddNodeItemResp_AddedNodeId,
    AddNodeItemResp_StatusCode

ABSTRACT_VARIABLES
    d_var

INVARIANT
    d_inv

INITIALISATION
    d_var :( d_inv )

OPERATIONS

    p_sc, p_nb_nodes_to_add <-- getall_msg_add_nodes_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_add_nodes_req
    THEN
        p_sc,
        p_nb_nodes_to_add,
        nb_nodes_to_add,
        AddNodeItemReq_ParentNodeId,
        AddNodeItemReq_ReferenceTypeId,
        AddNodeItemReq_RequestedNewNodeId,
        AddNodeItemReq_BrowseName,
        AddNodeItemReq_NodeClass,
        AddNodeItemReq_NodeAttributes,
        AddNodeItemReq_TypeDefinition
        :(  p_sc                 : t_StatusCode &
            p_nb_nodes_to_add : NAT &
            (p_sc = e_sc_ok
                =>
                p_nb_nodes_to_add > 0 &
                p_nb_nodes_to_add <= k_n_nodesToAdd_max &
                p_nb_nodes_to_add : t_nodeToAddReqIndex &
                nb_nodes_to_add = p_nb_nodes_to_add &
                AddNodeItemReq_ParentNodeId       : 1..nb_nodes_to_add --> t_ExpandedNodeId &
                AddNodeItemReq_ReferenceTypeId    : 1..nb_nodes_to_add --> t_NodeId &
                AddNodeItemReq_RequestedNewNodeId : 1..nb_nodes_to_add --> t_ExpandedNodeId_i &
                AddNodeItemReq_BrowseName         : 1..nb_nodes_to_add --> t_QualifiedName &
                AddNodeItemReq_NodeClass          : 1..nb_nodes_to_add --> t_NodeClass_i &
                AddNodeItemReq_NodeAttributes     : 1..nb_nodes_to_add --> t_NodeAttributes &
                AddNodeItemReq_TypeDefinition     : 1..nb_nodes_to_add --> t_ExpandedNodeId_i
            ) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops} &
                d_inv
            )
        )
    END
    ;

    p_sc, p_parentExpNid, p_refTypeId, p_reqExpNodeId, p_browseName, p_nodeClass, p_nodeAttributes, p_typeDefId
      <-- getall_add_node_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_node_add_nodes_req &
        p_index : NAT &
        p_index : t_nodeToAddReqIndex &
        p_index > 0 &
        p_index <= nb_nodes_to_add
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
            p_sc : t_StatusCode &
            p_parentExpNid : t_ExpandedNodeId_i &
            p_parentExpNid = AddNodeItemReq_ParentNodeId(p_index) &
            p_refTypeId : t_NodeId_i &
            p_refTypeId = AddNodeItemReq_ReferenceTypeId(p_index) &
            p_reqExpNodeId : t_ExpandedNodeId_i &
            p_reqExpNodeId = AddNodeItemReq_RequestedNewNodeId(p_index) &
            p_browseName : t_QualifiedName_i &
            p_browseName = AddNodeItemReq_BrowseName(p_index) &
            p_nodeClass : t_NodeClass_i &
            p_nodeClass = AddNodeItemReq_NodeClass(p_index) &
            p_nodeAttributes : t_NodeAttributes_i &
            p_nodeAttributes = AddNodeItemReq_NodeAttributes(p_index) &
            p_typeDefId : t_ExpandedNodeId_i &
            p_typeDefId = AddNodeItemReq_TypeDefinition(p_index) &
            ((/* We only check local constraints on requested NodeId (serverIndex == 0), BrowseName (not empty),
                 NodeClass (valid) and NodeAttributes (decoded object).
                 The rest is checked in AddNode item treatment using address space. */
                p_sc = e_sc_ok =>
                p_nodeClass : t_NodeClass &
                /* for objects and variables node classes: TypeDefinition is always defined */
                c_ExpandedNodeId_indet /: AddNodeItemReq_TypeDefinition[AddNodeItemReq_NodeClass~[{e_ncl_Object, e_ncl_Variable}]] &
                /* for other node classes: TypeDefinition is never defined */
                AddNodeItemReq_TypeDefinition[dom(AddNodeItemReq_NodeClass |>> {e_ncl_Object, e_ncl_Variable})] <: {c_ExpandedNodeId_indet}
            ) &
            (not (p_sc = e_sc_ok) => p_sc : {e_sc_bad_node_id_rejected, e_sc_bad_node_class_invalid, e_sc_bad_node_attributes_invalid,
                                             e_sc_bad_browse_name_invalid, e_sc_bad_type_definition_invalid}))

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
        p_nb_results > 0 &
        p_nb_results = nb_nodes_to_add
    THEN
        bres, AddNodeItemResp_Allocated
        :(bres : BOOL &
          AddNodeItemResp_Allocated : BOOL &
          AddNodeItemResp_Allocated = bres)
    END
    ;

    setall_msg_add_nodes_item_resp_params (p_resp_msg, p_index, p_sc, p_nid) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_node_add_nodes_resp &
        p_index : NAT &
        p_index : t_nodeToAddReqIndex &
        p_index > 0 &
        p_index <= nb_nodes_to_add &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_nid : t_NodeId_i &
        (p_sc = e_sc_ok => p_nid : t_NodeId) &
        (p_sc /= e_sc_ok => p_nid = c_NodeId_indet)
    THEN
        AddNodeItemResp_AddedNodeId(p_index) := p_nid ||
        AddNodeItemResp_StatusCode(p_index) := p_sc
    END

END
