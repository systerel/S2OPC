/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    service_browse

SEES
    constants,
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

    p_isvalid, p_nb_ref, p_src_node <-- get_SourceNode_NbRef(p_src_nodeid) =
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

END
