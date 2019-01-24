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
    constants,
    user_authorization_bs,
    data_value_pointer_bs

DEFINITIONS

    d_variables ==
        a_NodeId,
        a_NodeClass,
        a_BrowseName,
        a_DisplayName,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp,
        a_Value_ServerTimestamp,
        a_IsAbstract,
        a_EventNotifier,
        a_DataType,
        a_ValueRank,
        a_AccessLevel,
        a_UserAccessLevel,
        a_Historizing,
        a_Executable,
        a_UserExecutable,
        a_ContainsNoLoops,
        a_Symmetric,

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
        a_NodeId      : t_Node >-> t_NodeId        &
        a_NodeClass   : t_Node --> t_NodeClass     &
        a_BrowseName  : t_Node --> t_QualifiedName &
        a_DisplayName : t_Node --> t_LocalizedText &

        a_Value       : t_Node +-> t_Variant   &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) & /* All Variable have value but not all VariableType */

        /* + Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_StatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &
        a_Value_SourceTimestamp : t_Node +-> t_DateTime &
        dom(a_Value_SourceTimestamp) = dom(a_NodeClass|>{e_ncl_Variable}) &
        a_Value_ServerTimestamp : t_Node +-> t_DateTime &
        dom(a_Value_ServerTimestamp) = dom(a_NodeClass|>{e_ncl_Variable}) &

        a_IsAbstract : t_Node +-> BOOL &
        dom(a_IsAbstract) = dom(a_NodeClass|>{e_ncl_ObjectType, e_ncl_VariableType, e_ncl_ReferenceType, e_ncl_DataType}) &

        a_EventNotifier : t_Node +-> BOOL &
        dom(a_EventNotifier) = dom(a_NodeClass|>{e_ncl_Object, e_ncl_View}) &

        a_DataType : t_Node +-> t_NodeId &
        dom(a_DataType) = dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_ValueRank : t_Node +-> t_ValueRank &
        dom(a_ValueRank) = dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_AccessLevel : t_Node +-> t_access_level &
        dom(a_AccessLevel) = dom(a_NodeClass|>{e_ncl_Variable}) &

        a_UserAccessLevel : t_Node +-> t_access_level &
        dom(a_UserAccessLevel) = dom(a_NodeClass|>{e_ncl_Variable}) & /* Dynamic value evaluation depending on authorization of user */

        a_Historizing : t_Node +-> BOOL &
        dom(a_Historizing) = dom(a_NodeClass|>{e_ncl_Variable}) &

        a_Executable : t_Node +-> BOOL &
        dom(a_Executable) = dom(a_NodeClass|>{e_ncl_Method}) &

        a_UserExecutable : t_Node +-> BOOL &
        dom(a_UserExecutable) = dom(a_NodeClass|>{e_ncl_Method}) &

        a_ContainsNoLoops : t_Node +-> BOOL &
        dom(a_ContainsNoLoops) = dom(a_NodeClass|>{e_ncl_View}) &

        a_Symmetric : t_Node +-> BOOL &
        dom(a_Symmetric) = dom(a_NodeClass|>{e_ncl_ReferenceType}) &

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
    );

    d_init ==
        d_inv

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    /* This is where the Address Space is filled */
    d_variables :( d_init )

OPERATIONS

    bres <-- is_NodeId_equal(nid1, nid2) =
    PRE
        nid1 : t_NodeId_i &
        nid1 : t_NodeId &
        nid2 : t_NodeId_i &
        nid2 : t_NodeId
    THEN
        bres := bool(nid1 = nid2)
    END
    ;

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
    sc, variant <-- read_AddressSpace_Attribute_value(p_user, node, ncl, aid, index_range) =
    PRE
        p_user : t_user_i       &
        p_user : t_user         &
        node : t_Node_i         &
        node : t_Node           &
        node : dom(a_NodeId)    &
        node : dom(a_NodeClass) &
        ncl : t_NodeClass_i &
        ncl = a_NodeClass(node) &
        aid  : t_AttributeId_i  &
        aid  : t_AttributeId    &
        index_range : t_IndexRange_i &
        e_operation_type_read |-> a_NodeId(node) |-> aid |-> p_user : s_AuthorizedOperations
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc : {e_sc_bad_out_of_memory, e_sc_bad_internal_error})
        OR
            IF aid : {e_aid_NodeId, e_aid_NodeClass, e_aid_BrowseName, e_aid_DisplayName, e_aid_Value, e_aid_AccessLevel} THEN /* TODO: rest of the attributes */
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
    serviceStatusCode, prev_dataValue <-- set_Value(p_user, node, dataValue, index_range) =
    PRE
        p_user : t_user_i   &
        p_user : t_user     &
        node  : t_Node_i    &
        node  : t_Node      &
        node : dom(a_Value) &
        node : dom(a_Value_StatusCode) &
        dataValue : t_DataValue_i &
        dataValue : t_DataValue   &
        index_range : t_IndexRange_i &
        e_operation_type_write |-> a_NodeId(node) |-> e_aid_Value |-> p_user : s_AuthorizedOperations
    THEN
        CHOICE
            IF IsValid_VariantIndex(DataValue_Variant(dataValue) |-> index_range) = TRUE &
                HasRange_VariantIndex(DataValue_Variant(dataValue) |-> index_range) = TRUE
            THEN
                serviceStatusCode := e_sc_ok ||
                prev_dataValue :(prev_dataValue : t_DataValue_i &
                                 prev_dataValue : t_DataValue &
                                 DataValue_Variant(prev_dataValue) = a_Value(node) &
                                 DataValue_StatusCode(prev_dataValue) = a_Value_StatusCode(node)) ||
                a_Value(node) := DataValue_Variant(dataValue)
            ELSIF IsValid_VariantIndex(DataValue_Variant(dataValue) |-> index_range) = FALSE
            THEN
                serviceStatusCode := e_sc_bad_index_range_invalid ||
                prev_dataValue := c_DataValue_indet
            ELSE
                serviceStatusCode := e_sc_bad_index_range_no_data ||
                prev_dataValue := c_DataValue_indet
            END
        OR
            serviceStatusCode
            :(serviceStatusCode : t_StatusCode_i &
                serviceStatusCode : {e_sc_bad_out_of_memory, e_sc_bad_internal_error}) ||
            prev_dataValue := c_DataValue_indet
        END
    END
    ;

    /* a_Value_StatusCode */
    sc <-- get_Value_StatusCode(p_user, node) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        node : t_Node_i &
        node : t_Node   &
        node : dom(a_Value_StatusCode) &
        e_operation_type_read |-> a_NodeId(node) |-> e_aid_Value |-> p_user : s_AuthorizedOperations
    THEN
        sc :(sc : t_StatusCode_i &
             sc = a_Value_StatusCode(node))
    END
    ;

    read_AddressSpace_clear_value(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END
    ;

    /* Frees a value previously malloced with read_AddressSpace_Attribute_value */
    read_AddressSpace_free_variant(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END
    ;

    /* Frees a value previously malloced with set_Value */
    write_AddressSpace_free_dataValue(data) =
    PRE
        data : t_DataValue_i
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

    p_data_type <-- get_DataType(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_DataType)
    THEN
        p_data_type :(p_data_type : t_NodeId_i &
                      p_data_type = a_DataType(p_node))
    END
    ;

    p_value_rank <-- get_ValueRank(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_ValueRank)
    THEN
        p_value_rank :(p_value_rank = a_ValueRank(p_node))
    END
    ;

    p_access_level <-- get_AccessLevel(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_node : dom(a_AccessLevel)
    THEN
        p_access_level :(p_access_level : t_access_level &
                         p_access_level = a_AccessLevel(p_node))
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
