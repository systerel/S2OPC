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
        Node_RefIndexBegin,
        Node_RefIndexEnd,
        RefIndex_Reference,

        /* Service Write */
        ResponseWrite_allocated,
        ResponseWrite_StatusCode,

        /* A special reference, that we model because it is used by Browse.
         * This is the content of the TargetNode of all Variables and Objects. */
        Ref_HasTypeDefinition
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
        Node_RefIndexBegin      : t_Node     --> t_RefIndex  &
        Node_RefIndexEnd        : t_Node     --> t_RefIndex  &
        RefIndex_Reference      : t_Node * t_RefIndex --> t_Reference &

        /* Service Write */
        ResponseWrite_allocated : BOOL &
        ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
        (ResponseWrite_allocated = FALSE
         =>
         ResponseWrite_StatusCode = {})
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
    sc, ncl, val <-- read_NodeClass_Attribute(node, aid) =
    PRE
        node : t_Node_i &
        node : t_Node &
        aid : t_AttributeId_i &
        aid : t_AttributeId
    THEN
        /* TODO: add user-access levels */
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

    /* TODOPAB: Voir conjointement avec les Références à l'entrée */
    /* TODO: On peut mettre un array pour les Begin, pour les End,
       et un 3° un (Reference *) */
    p_ref_index <-- get_Node_RefIndexBegin(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_ref_index :(
            p_ref_index : NAT &
            p_ref_index = Node_RefIndexBegin(p_node)
        )
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
    sc <-- get_Value_StatusCode(node) =
    PRE
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

    StatusCode_service <-- treat_write_request_WriteValues =
    PRE
        ResponseWrite_allocated = TRUE /* TODO & userid /= c_user_local */
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

END
