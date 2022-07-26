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
    service_write_1

SEES
    constants,
    constants_statuscodes_bs,
    service_write_decode_bs /* TODO: move to refinment */

DEFINITIONS
    "address_space.def";

    d_variables ==
        ResponseWrite_allocated,
        ResponseWrite_StatusCode
    ;

    d_inv ==
    (
        ResponseWrite_allocated : BOOL &
        ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
        (ResponseWrite_allocated = FALSE
         =>
         ResponseWrite_StatusCode = {})
    )

ABSTRACT_VARIABLES
    d_adsp_variables,
    d_variables

INVARIANT
    d_adsp_invariant &
    d_inv

INITIALISATION
    d_adsp_variables,
    d_variables
    :(
        d_adsp_invariant &
        d_inv
    )

OPERATIONS

    d_adsp_operations;

    StatusCode_service <-- treat_write_request_WriteValues (p_user, p_locales) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        ResponseWrite_allocated = TRUE
    THEN
        StatusCode_service,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp,
        ResponseWrite_StatusCode
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &

            a_Value : s_Node +-> t_Variant &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            a_Value_StatusCode : s_Node +-> t_RawStatusCode &
            dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &
            a_Value_SourceTimestamp : s_Node +-> t_Timestamp &
            dom(a_Value_SourceTimestamp) = dom(a_NodeClass|>{e_ncl_Variable}) &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode
        )
    END
    ;

    bret <-- alloc_write_request_responses(nb_req) =
    PRE
        nb_req : NAT &
        ResponseWrite_allocated = FALSE
    THEN
        bret,
        ResponseWrite_StatusCode,
        ResponseWrite_allocated
        :(
            bret : BOOL &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
            ResponseWrite_allocated : BOOL &
            (ResponseWrite_allocated = FALSE
             =>
             ResponseWrite_StatusCode = {}) &
            bret = ResponseWrite_allocated
        )
    END
    ;

    dealloc_write_request_responses =
    PRE
        ResponseWrite_allocated = TRUE
    THEN
        ResponseWrite_allocated  := FALSE ||
        ResponseWrite_StatusCode := {}
    END
    ;

    /* Promoted */
    write_WriteResponse_msg_out(msg_out) =
    PRE
        msg_out : t_msg_i &
        msg_out : t_msg
    THEN
        skip
    END
    ;

    /* Services Node Management */
    sc_addnode <-- addNode_AddressSpace(p_parentNid, p_refTypeId, p_newNodeId, p_browseName, p_nodeClass, p_nodeAttributes, p_typeDefId) =
    PRE
        p_parentNid : t_ExpandedNodeId_i &
        p_parentNid : dom(conv_ExpandedNodeId_NodeId) &
        conv_ExpandedNodeId_NodeId(p_parentNid) : ran(a_NodeId) &
        p_refTypeId : t_NodeId_i &
        p_refTypeId : t_NodeId &
        p_newNodeId : t_NodeId_i &
        p_newNodeId : t_NodeId &
        p_newNodeId /: ran(a_NodeId) &
        p_browseName : t_QualifiedName_i &
        p_browseName : t_QualifiedName &
        p_nodeClass : t_NodeClass_i &
        p_nodeClass = e_ncl_Variable &
        p_nodeAttributes : t_NodeAttributes_i &
        p_nodeAttributes : t_NodeAttributes &
        p_typeDefId : t_ExpandedNodeId_i &
        p_typeDefId : t_ExpandedNodeId &
        (p_nodeClass : {e_ncl_Object, e_ncl_Variable} =>
            p_typeDefId : dom(conv_ExpandedNodeId_NodeId) &
            conv_ExpandedNodeId_NodeId(p_typeDefId) : ran(a_NodeId)) &
        (p_nodeClass /: {e_ncl_Object, e_ncl_Variable} =>
            p_typeDefId = c_ExpandedNodeId_indet)
    THEN
        sc_addnode,
        d_adsp_nodes_variables
        :(
            sc_addnode : t_StatusCode_i &
            sc_addnode : {e_sc_ok, e_sc_bad_browse_name_duplicated, e_sc_bad_reference_not_allowed,
                          e_sc_bad_type_definition_invalid, e_sc_bad_out_of_memory,
                          e_sc_bad_node_class_invalid} & // BadNodeClassInvalid used for unsupported NodeClass for now
            d_adsp_nodes_invariant
        )
    END

END
