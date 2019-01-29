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
    service_browse

SEES
    constants,
    constants_statuscodes_bs,
    address_space

DEFINITIONS
    d_variables ==
        ResponseBrowse_BrowseStatus,
        ResponseBrowse_ContinuationPoint,
        ResponseBrowse_Res_ReferenceTypeId,
        ResponseBrowse_Res_Forwards,
        ResponseBrowse_Res_NodeId,
        ResponseBrowse_Res_BrowseName,
        ResponseBrowse_Res_DisplayName,
        ResponseBrowse_Res_NodeClass,
        ResponseBrowse_Res_TypeDefinition
    ;

    d_inv ==
        ResponseBrowse_BrowseStatus        : t_BrowseValue +-> t_StatusCode                          &
        ResponseBrowse_ContinuationPoint   : t_BrowseValue +-> t_Reference                           &
        ResponseBrowse_Res_ReferenceTypeId : t_BrowseValue --> (t_BrowseResult +-> t_NodeId)         &
        ResponseBrowse_Res_Forwards        : t_BrowseValue --> (t_BrowseResult +-> BOOL)             &
        ResponseBrowse_Res_NodeId          : t_BrowseValue --> (t_BrowseResult +-> t_ExpandedNodeId) &
        ResponseBrowse_Res_BrowseName      : t_BrowseValue --> (t_BrowseResult +-> t_QualifiedName)  &
        ResponseBrowse_Res_DisplayName     : t_BrowseValue --> (t_BrowseResult +-> t_LocalizedText)  &
        ResponseBrowse_Res_NodeClass       : t_BrowseValue --> (t_BrowseResult +-> t_NodeClass)      &
        ResponseBrowse_Res_TypeDefinition  : t_BrowseValue --> (t_BrowseResult +-> t_ExpandedNodeId)

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables
    :(
        d_inv
     )

OPERATIONS

    p_isvalid, p_nb_ref, p_src_node <-- getall_SourceNode_NbRef(p_src_nodeid) =
    PRE
        p_src_nodeid : t_NodeId_i &
        p_src_nodeid : t_NodeId
    THEN
        p_isvalid,
        p_nb_ref,
        p_src_node
        :(  p_isvalid  : BOOL &
            p_nb_ref   : NAT  &
            p_src_node : t_Node_i
         )
    END
    ;

    p_isallocated <-- alloc_browse_response(p_nb_bvi) =
    PRE
        p_nb_bvi : NAT
    THEN
        p_isallocated :: BOOL
    END
    ;

    p_isallocated, p_nb_bri <-- alloc_browse_result(p_bvi, p_nb_target_max, p_nb_target) =
    PRE
        p_bvi           : t_BrowseValue_i &
        p_bvi           : t_BrowseValue   &
        p_nb_target_max : INT             &
        p_nb_target     : NAT
    THEN
        p_isallocated :: BOOL ||
        p_nb_bri      :: NAT
    END
    ;

    p_ref_types_compat <-- Is_RefTypes_Compatible(p_is_ref_type1, p_ref_type1, p_inc_subtypes, p_ref_type2) =
    PRE
        p_is_ref_type1 : BOOL       &
        p_ref_type1    : t_NodeId_i &
        p_inc_subtypes : BOOL       &
        p_ref_type2    : t_NodeId_i &
        p_ref_type2    : t_NodeId
    THEN
        p_ref_types_compat :: BOOL
    END
    ;

    p_res <-- copy_target_node_browse_result(p_bvi, p_bri, p_RefType, p_NodeId, p_IsForward) =
    PRE
        p_bvi       : t_BrowseValue_i    &
        p_bvi       : t_BrowseValue      &
        p_bri       : t_BrowseResult_i   &
        p_bri       : t_BrowseResult     &
        p_RefType   : t_NodeId_i         &
        p_RefType   : t_NodeId           &
        p_NodeId    : t_ExpandedNodeId_i &
        p_NodeId    : t_ExpandedNodeId   &
        p_IsForward : BOOL
    THEN
        p_res,
        ResponseBrowse_Res_ReferenceTypeId,
        ResponseBrowse_Res_Forwards,
        ResponseBrowse_Res_NodeId,
        ResponseBrowse_Res_BrowseName,
        ResponseBrowse_Res_DisplayName,
        ResponseBrowse_Res_NodeClass,
        ResponseBrowse_Res_TypeDefinition
        :(
            p_res : BOOL &
            ResponseBrowse_Res_ReferenceTypeId : t_BrowseValue --> (t_BrowseResult +-> t_NodeId)         &
            ResponseBrowse_Res_Forwards        : t_BrowseValue --> (t_BrowseResult +-> BOOL)             &
            ResponseBrowse_Res_NodeId          : t_BrowseValue --> (t_BrowseResult +-> t_ExpandedNodeId) &
            ResponseBrowse_Res_BrowseName      : t_BrowseValue --> (t_BrowseResult +-> t_QualifiedName)  &
            ResponseBrowse_Res_DisplayName     : t_BrowseValue --> (t_BrowseResult +-> t_LocalizedText)  &
            ResponseBrowse_Res_NodeClass       : t_BrowseValue --> (t_BrowseResult +-> t_NodeClass)      &
            ResponseBrowse_Res_TypeDefinition  : t_BrowseValue --> (t_BrowseResult +-> t_ExpandedNodeId)
         )
    END
    ;

    fill_continuation_point(p_bvi, p_continue_ref, p_ref) =
    PRE
        p_bvi          : t_BrowseValue_i &
        p_bvi          : t_BrowseValue   &
        p_continue_ref : BOOL            &
        p_ref          : t_Reference_i   &
        p_ref          : t_Reference
    THEN
        ResponseBrowse_ContinuationPoint :: t_BrowseValue +-> t_Reference
    END
    ;

    /* Promoted */
    p_isvalid <-- write_BrowseResponse_msg_out(p_msg_out) =
    PRE
        p_msg_out : t_msg_i &
        p_msg_out : t_msg
    THEN
        p_isvalid :: BOOL
    END
    ;

    /* Promoted */
    free_browse_result =
    BEGIN
        skip
    END
    ;

    /* Promoted from msg_browse_response_bs */
    set_ResponseBrowse_BrowseStatus(p_bvi, p_sc) =
    PRE
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        skip
    END

END
