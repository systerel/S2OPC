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
    address_space_bs

SEES
    constants

DEFINITIONS

    d_variables ==
        a_NodeId,
        a_NodeClass,
        a_BrowseName,
        a_DisplayName,
        a_Value,
        a_Value_StatusCode,

        Ref_HasTypeDefinition,

        Reference_SourceNode,
        Reference_ReferenceType,
        Reference_TargetNode,
        Reference_IsForward,

        Node_RefIndexEnd,
        RefIndex_Reference,

        Generic_Node_Variant,
        IsValid_VariantIndex, /* Valid (or none) index range value for Variant */
        HasRange_VariantIndex /* Valid index range but has no range in  Variant */
    ;

    d_inv ==
    (
        /* Attributes */
        a_NodeId      : t_Node +-> t_NodeId    &
        a_NodeClass   : t_Node +-> t_NodeClass &
        a_BrowseName  : t_Node +-> t_QualifiedName &
        a_DisplayName : t_Node +-> t_LocalizedText &
        a_Value       : t_Node +-> t_Variant   &

        /* Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_StatusCode &

        Ref_HasTypeDefinition : t_Node +-> t_ExpandedNodeId &

        Reference_SourceNode    : t_Reference +-> t_NodeId         &
        Reference_ReferenceType : t_Reference +-> t_NodeId         &
        Reference_TargetNode    : t_Reference +-> t_ExpandedNodeId &
        Reference_IsForward     : t_Reference +-> BOOL             &

        Node_RefIndexEnd        : t_Node     +-> t_RefIndex  &
        RefIndex_Reference      : t_Node * t_RefIndex +-> t_Reference &

        Generic_Node_Variant : t_NodeClass * t_Node * t_AttributeId +-> t_Variant &
        IsValid_VariantIndex : t_Variant * t_IndexRange +-> BOOL &
        HasRange_VariantIndex  : t_Variant * t_IndexRange +-> BOOL
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    /* This is where the Address Space is filled */
    d_variables
    :(
        a_NodeId : t_Node >-> t_NodeId &
        a_NodeClass : t_Node --> t_NodeClass &

        a_Value : t_Node +-> t_Variant &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_Value_StatusCode : t_Node +-> t_StatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &

        /* References */
        Reference_SourceNode    : t_Reference --> t_NodeId         &
        Reference_ReferenceType : t_Reference --> t_NodeId         &
        Reference_TargetNode    : t_Reference --> t_ExpandedNodeId &
        Reference_IsForward     : t_Reference --> BOOL &

        Generic_Node_Variant : t_NodeClass * t_Node * t_AttributeId --> t_Variant &
        IsValid_VariantIndex : t_Variant * t_IndexRange --> BOOL &
        HasRange_VariantIndex  : t_Variant * t_IndexRange --> BOOL
     )

OPERATIONS

    /* a_NodeId~ */
    nid_valid, node <-- readall_AddressSpace_Node(nid) =
    PRE
        nid : t_NodeId_i
    THEN
        IF nid : ran(a_NodeId) THEN
            nid_valid := TRUE ||
            node :( node : t_Node_i & node|->nid : a_NodeId )
        ELSE
            nid_valid := FALSE ||
            node := c_Node_indet
        END
    END
    ;

    /* Reads any attribute and outputs a variant (valid or not)
     * As this function uses the util_variant__new_Variant_from_* functions, the value must be freed once used
     */
    sc, variant <-- read_AddressSpace_Attribute_value(node, ncl, aid, index_range) =
    PRE
        node : t_Node_i         &
        node : t_Node           &
        node : dom(a_NodeId)    &
        node : dom(a_NodeClass) &
        ncl : t_NodeClass_i &
        ncl = a_NodeClass(node) &
        aid  : t_AttributeId_i  &
        aid  : t_AttributeId    &
        index_range : t_IndexRange_i &
        index_range : t_IndexRange
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc : {e_sc_bad_out_of_memory, e_sc_bad_internal_error})
        OR
            IF aid : t_AttributeId - {e_aid_Description} THEN /* TODO: Description treatment */
                IF IsValid_VariantIndex(Generic_Node_Variant(ncl |-> node |-> aid) |-> index_range) = TRUE &
                    HasRange_VariantIndex(Generic_Node_Variant(ncl |-> node |-> aid) |-> index_range) = TRUE
                THEN
                    SELECT aid = e_aid_NodeId THEN
                        sc,
                        variant
                        :(  sc = e_sc_ok &
                            variant : t_Variant_i &
                            variant = conv_NodeId_Variant(a_NodeId(node))
                        )
                    WHEN aid = e_aid_NodeClass THEN
                        variant := conv_NodeClass_Variant(a_NodeClass(node)) ||
                        sc := e_sc_ok
                    WHEN aid = e_aid_Value &
                        (ncl = e_ncl_Variable or ncl = e_ncl_VariableType) THEN
                        variant := a_Value(node) ||
                        sc := e_sc_ok
                    ELSE
                        /* TODO: add BrowseName, DisplayName, AccessLevel cases (a_NodeClass shall not be used) */
                        variant := conv_NodeClass_Variant(a_NodeClass(node)) ||
                        sc := e_sc_ok
                    END
                ELSIF IsValid_VariantIndex(Generic_Node_Variant(ncl |-> node |-> aid) |-> index_range) = FALSE
                THEN
                    variant := c_Variant_indet ||
                    sc := e_sc_bad_index_range_invalid
                ELSE
                    variant := c_Variant_indet ||
                    sc := e_sc_bad_index_range_no_data
                END
            ELSE

                variant := c_Variant_indet ||
                sc := e_sc_bad_attribute_id_invalid
            END
        END
    END
    ;

    /* a_Value, setter */
    serviceStatusCode, prev_value <-- set_Value(node, value, index_range) =
    PRE
        node  : t_Node_i    &
        node  : t_Node      &
        value : t_Variant_i &
        value : t_Variant   &
        index_range : t_IndexRange_i &
        index_range : t_IndexRange
    THEN
        CHOICE
            IF IsValid_VariantIndex(value |-> index_range) = TRUE &
                HasRange_VariantIndex(value |-> index_range) = TRUE
            THEN
                serviceStatusCode := e_sc_ok ||
                prev_value :(prev_value : t_Variant_i & prev_value = a_Value(node)) ||
                a_Value(node) := value
            ELSIF IsValid_VariantIndex(value |-> index_range) = FALSE
            THEN
                serviceStatusCode := e_sc_bad_index_range_invalid ||
                prev_value := c_Variant_indet
            ELSE
                serviceStatusCode := e_sc_bad_index_range_no_data ||
                prev_value := c_Variant_indet
            END
        OR
            serviceStatusCode
            :(serviceStatusCode : t_StatusCode_i &
                serviceStatusCode : {e_sc_bad_out_of_memory, e_sc_bad_internal_error}) ||
            prev_value := c_Variant_indet
        END
    END
    ;

    /* a_Value_StatusCode */
    sc <-- get_Value_StatusCode(node) =
    PRE
        node : t_Node_i &
        node : t_Node   &
        node : dom(a_Value_StatusCode)
    THEN
        sc :(sc : t_StatusCode_i &
             sc = a_Value_StatusCode(node))
    END
    ;

    /* Frees a value previously malloced with read_AddressSpace_Attribute_value */
    read_AddressSpace_free_value(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END
    ;

    p_browse_name <-- get_BrowseName(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_BrowseName)
    THEN
        p_browse_name :(p_browse_name : t_QualifiedName_i &
                        p_browse_name = a_BrowseName(p_node))
    END
    ;

    p_display_name <-- get_DisplayName(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_DisplayName)
    THEN
        p_display_name :(p_display_name : t_LocalizedText_i &
                         p_display_name = a_DisplayName(p_node))
    END
    ;

    p_node_class <-- get_NodeClass(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_NodeClass)
    THEN
        p_node_class :(p_node_class : t_NodeClass_i &
                       p_node_class = a_NodeClass(p_node))
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

END
