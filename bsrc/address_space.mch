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
    address_space

SEES
    constants,
    service_write_decode_bs /* TODO: move to refinment */

DEFINITIONS
    "definitions.def";
    d_variables ==
        /* Attributes */
        a_NodeId,
        a_NodeClass,
        a_BrowseName,
        a_DisplayName,
        a_Value,
        a_Value_StatusCode,

        /* References */
        Reference_SourceNode,
        Reference_ReferenceType,
        Reference_TargetNode,
        Reference_IsForward,

        Node_Reference,
        Node_RefIndexEnd,
        RefIndex_Reference,

        IsValid_VariantIndex,
        HasRange_VariantIndex,

        /* Service Write */
        ResponseWrite_allocated,
        ResponseWrite_StatusCode,

        /* A special reference, that we model because it is used by Browse.
         * This is the content of the TargetNode of all Variables and Objects. */
        Ref_HasTypeDefinition,

        /* User authorization */
        s_AuthorizedOperations
    ;

    d_inv ==
    (
        /* Node attributes */
        a_NodeId      : t_Node >-> t_NodeId        &
        a_NodeClass   : t_Node --> t_NodeClass     &
        a_BrowseName  : t_Node --> t_QualifiedName &
        a_DisplayName : t_Node --> t_LocalizedText &

        a_Value : t_Node +-> t_Variant &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        /* Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_StatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &

        Ref_HasTypeDefinition : t_Node +-> t_ExpandedNodeId &
        dom(Ref_HasTypeDefinition) = dom(a_NodeClass |> {e_ncl_Variable, e_ncl_Object}) &

        /* References */
        Reference_SourceNode    : t_Reference --> t_NodeId         &
        Reference_ReferenceType : t_Reference --> t_NodeId         &
        Reference_TargetNode    : t_Reference --> t_ExpandedNodeId &
        Reference_IsForward     : t_Reference --> BOOL             &

        Node_Reference          : t_Node     <-> t_Reference &
        Node_RefIndexEnd        : t_Node     --> t_RefIndex  &
        RefIndex_Reference      : t_Node * t_RefIndex --> t_Reference &

        IsValid_VariantIndex : t_Variant * t_IndexRange --> BOOL &
        HasRange_VariantIndex  : t_Variant * t_IndexRange --> BOOL &

        /* Service Write */
        ResponseWrite_allocated : BOOL &
        ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
        (ResponseWrite_allocated = FALSE
         =>
         ResponseWrite_StatusCode = {}) &

        /* User authorization */
        s_AuthorizedOperations <: t_operation_type * t_NodeId * t_AttributeId * t_user
    )

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
    /* a_NodeClass(node) and any_attribute(node) */
    sc, ncl, val <-- read_NodeClass_Attribute(p_user, node, aid, index_range) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        node : t_Node_i &
        node : t_Node &
        aid : t_AttributeId_i &
        aid : t_AttributeId &
        index_range : t_IndexRange_i &
        index_range : t_IndexRange
    THEN
        /* TODO: define cases of failure (sc /= e_sc_ok): value requested on non Variable* type */
        sc,
        ncl,
        val:(sc : t_StatusCode_i &
             sc : t_StatusCode &
             ncl : t_NodeClass_i &
             ncl = a_NodeClass(node) &
             val : t_Variant_i & /* TODO: must not be left undeterministic in concretisation, so there must be an _indet */
            (
            node : dom(AttributeID_Node_Variant(aid))
            =>
            val = AttributeID_Node_Variant(aid)(node)
            )
        )
    END
    ;

    /* Promoted */
    /* a_NodeId~ */
    nid_valid, node <-- readall_AddressSpace_Node(nid) =
    PRE
        nid : t_NodeId_i
    THEN
        IF  nid : ran(a_NodeId) THEN
            nid_valid := TRUE ||
            node :( node : t_Node_i & node|->nid : a_NodeId )
        ELSE
            nid_valid := FALSE ||
            node := c_Node_indet
        END
    END
    ;

    p_node_class <-- get_NodeClass(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_node_class :(p_node_class : t_NodeClass_i &
                       p_node_class = a_NodeClass(p_node))
    END
    ;

    p_browse_name <-- get_BrowseName(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_browse_name :(p_browse_name : t_QualifiedName_i &
                        p_browse_name = a_BrowseName(p_node))
    END
    ;

    p_display_name <-- get_DisplayName(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_display_name :(p_display_name : t_LocalizedText_i &
                         p_display_name = a_DisplayName(p_node))
    END
    ;

    p_type_def <-- get_TypeDefinition(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(Ref_HasTypeDefinition)
    THEN
        p_type_def :(p_type_def : t_ExpandedNodeId_i &
                     p_type_def = Ref_HasTypeDefinition(p_node))
    END
    ;

    p_RefType <-- get_Reference_ReferenceType(p_ref) =
    PRE
        p_ref : t_Reference_i &
        p_ref : t_Reference
    THEN
        p_RefType
        :(  p_RefType : t_NodeId_i &
            p_RefType = Reference_ReferenceType(p_ref)
         )
    END
    ;

    p_TargetNode <-- get_Reference_TargetNode(p_ref) =
    PRE
        p_ref : t_Reference_i &
        p_ref : t_Reference
    THEN
        p_TargetNode
        :(
            p_TargetNode : t_ExpandedNodeId_i &
            p_TargetNode = Reference_TargetNode(p_ref)
         )
    END
    ;

    p_IsForward <-- get_Reference_IsForward(p_ref) =
    PRE
        p_ref : t_Reference_i &
        p_ref : t_Reference
    THEN
        p_IsForward := Reference_IsForward(p_ref)
    END
    ;

    p_ref_index <-- get_Node_RefIndexEnd(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_ref_index :(
            p_ref_index : NAT &
            p_ref_index = Node_RefIndexEnd(p_node)
        )
    END
    ;

    p_ref <-- get_RefIndex_Reference(p_node, p_ref_index) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node &
        p_ref_index : NAT &
        p_ref_index : t_RefIndex
    THEN
        p_ref :(p_ref : t_Reference_i &
                p_ref = RefIndex_Reference(p_node |-> p_ref_index))
    END
    ;

    /* Promoted */
    sc <-- get_Value_StatusCode(p_user, node) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        node : t_Node_i &
        node : t_Node &
        a_NodeClass(node) = e_ncl_Variable
    THEN
        sc :(sc : t_StatusCode_i &
             sc = a_Value_StatusCode(node)
            )
    END
    ;

    /* Promoted */
    /* Frees a value previously malloced with read_AddressSpace_Attribute_value */
    read_AddressSpace_free_value(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END
    ;

    /* --------------------------------------------------------------------- */
    /* Service Write */

    StatusCode_service <-- treat_write_request_WriteValues (p_user) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        ResponseWrite_allocated = TRUE
    THEN
        StatusCode_service,
        a_Value,
        ResponseWrite_StatusCode
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            a_Value : t_Node +-> t_Variant &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &
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

    p_authorized <-- get_user_authorization (p_operation_type, p_node_id, p_attribute_id, p_user) =
    PRE
        p_operation_type : t_operation_type_i &
        p_operation_type : t_operation_type   &
        p_node_id        : t_NodeId_i         &
        p_node_id        : t_NodeId           &
        p_attribute_id   : t_AttributeId_i    &
        p_attribute_id   : t_AttributeId      &
        p_user           : t_user_i           &
        p_user           : t_user
    THEN
        p_authorized := bool(p_operation_type |-> p_node_id |-> p_attribute_id |-> p_user : s_AuthorizedOperations)
    END

END
