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
    constants_statuscodes_bs,
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
        a_IsAbstract,
        a_EventNotifier,
        a_DataType,
        a_ValueRank,
        a_ArrayDimensions,
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

        IsRangeDefined, /* <=> range string != NULL */
        IsValid_VariantIndex, /* Valid (or none) index range value for Variant */
        HasRange_VariantIndex, /* Valid index range but has no range in  Variant */

        /* LocalizedText supported locales */
        IsSupportedLocales_Variant

    ;

    d_inv ==
    (
        /* Attributes */
        a_NodeId      : t_Node >-> t_NodeId        &
        a_NodeClass   : t_Node --> t_NodeClass     &
        a_BrowseName  : t_Node --> t_QualifiedName &
        a_DisplayName : t_Node --> t_LocalizedText &

        a_Value       : t_Node +-> t_Variant   &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) & /* All Variable have value but not all VariableType */
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        /* + Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_RawStatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &
        a_Value_SourceTimestamp : t_Node +-> t_Timestamp &
        dom(a_Value_SourceTimestamp) = dom(a_NodeClass|>{e_ncl_Variable}) &

        a_IsAbstract : t_Node +-> BOOL &
        dom(a_IsAbstract) = dom(a_NodeClass|>{e_ncl_ObjectType, e_ncl_VariableType, e_ncl_ReferenceType, e_ncl_DataType}) &

        a_EventNotifier : t_Node +-> t_Byte &
        dom(a_EventNotifier) = dom(a_NodeClass|>{e_ncl_Object, e_ncl_View}) &

        a_DataType : t_Node +-> t_NodeId &
        dom(a_DataType) = dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_ValueRank : t_Node +-> t_ValueRank &
        dom(a_ValueRank) = dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_ArrayDimensions : t_Node +-> t_ArrayDimensions &
        dom(a_ArrayDimensions) = dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

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

        IsRangeDefined : t_IndexRange_i --> BOOL &
        IsValid_VariantIndex : t_Variant * t_IndexRange +-> BOOL &
        HasRange_VariantIndex  : t_Variant * t_IndexRange +-> BOOL &

        IsSupportedLocales_Variant : t_Variant * t_LocaleIds --> BOOL
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

    sc, variant <-- read_AddressSpace_AccessLevel_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_AccessLevel)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
              sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_AccessLevel_Variant(a_AccessLevel(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_BrowseName_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_BrowseName)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_QualifiedName_Variant(a_BrowseName(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_ContainsNoLoops_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_ContainsNoLoops)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_ContainsNoLoops(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_DataType_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_DataType)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_NodeId_Variant(a_DataType(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_DisplayName_value(p_locales, p_node) =
    PRE
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_DisplayName)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_LocalizedText_Variant(a_DisplayName(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_EventNotifier_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_EventNotifier)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Byte_Variant(a_EventNotifier(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_Executable_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_Executable)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_Executable(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_Historizing_value(p_node) =
    PRE
        p_node : t_Node_i          &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_Historizing)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_Historizing(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_IsAbstract_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom((a_IsAbstract))
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_IsAbstract(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_NodeClass_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_NodeClass_Variant(a_NodeClass(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_NodeId_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_NodeId_Variant(a_NodeId(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_Symmetric_value(p_node) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_Symmetric)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
              sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_Symmetric(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_UserAccessLevel_value(p_node, p_is_user_read_auth, p_is_user_write_auth) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_UserAccessLevel) &
        p_node : dom(a_AccessLevel) & /* AccessLevel used to generate value */
        p_is_user_read_auth : BOOL &
        p_is_user_write_auth : BOOL
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
              sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_AccessLevel_Variant(a_UserAccessLevel(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_UserExecutable_value(p_node, p_is_user_executable_auth) =
    PRE
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_UserExecutable) &
        p_is_user_executable_auth : BOOL
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
              sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_Boolean_Variant(a_UserExecutable(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant, val_sc, val_ts_src <-- read_AddressSpace_Value_value(p_locales, p_node, index_range) =
    PRE
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        p_node : t_Node_i         &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_Value) &
        index_range : t_IndexRange_i
    THEN
        CHOICE
            variant := c_Variant_indet ||
            val_sc := c_RawStatusCode_BadInvalidState ||
            val_ts_src := c_Timestamp_null ||
            sc := e_sc_bad_out_of_memory
        OR
            IF IsValid_VariantIndex(a_Value(p_node) |-> index_range) = TRUE &
                HasRange_VariantIndex(a_Value(p_node) |-> index_range) = TRUE
            THEN
                variant :(variant : t_Variant_i & variant = a_Value(p_node)) ||
                sc := e_sc_ok ||
                IF a_NodeClass(p_node) = e_ncl_Variable THEN
                    val_sc := a_Value_StatusCode(p_node) ||
                    val_ts_src := a_Value_SourceTimestamp(p_node)
                ELSE
                    /* VariableType */
                    val_sc := c_RawStatusCode_Good ||
                    val_ts_src := c_Timestamp_null
                END
            ELSIF IsValid_VariantIndex(a_Value(p_node) |-> index_range) = FALSE
            THEN
                variant := c_Variant_indet ||
                val_sc := c_RawStatusCode_BadInvalidState ||
                val_ts_src := c_Timestamp_null ||
                sc := e_sc_bad_index_range_invalid
            ELSE
                variant := c_Variant_indet ||
                val_sc := c_RawStatusCode_BadInvalidState ||
                val_ts_src := c_Timestamp_null ||
                sc := e_sc_bad_index_range_no_data
            END
        END
    END
    ;

    sc, variant <-- read_AddressSpace_ValueRank_value(p_node) =
    PRE
        p_node : t_Node_i          &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_ValueRank)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_ValueRank_Variant(a_ValueRank(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    sc, variant <-- read_AddressSpace_ArrayDimensions_value(p_node) =
    PRE
        p_node : t_Node_i          &
        p_node : t_Node           &
        p_node : dom(a_NodeId)    &
        p_node : dom(a_NodeClass) &
        p_node : dom(a_ArrayDimensions)
    THEN
        CHOICE
            variant := c_Variant_indet ||
            sc
            :(sc : t_StatusCode_i &
                sc = e_sc_bad_out_of_memory)
        OR
            variant :(variant : t_Variant_i & variant = conv_ArrayDimensions_Variant(a_ArrayDimensions(p_node))) ||
            sc := e_sc_ok
        END
    END
    ;

    /* a_Value, setter */
    serviceStatusCode, prev_dataValue <-- set_Value(p_user, p_locales, node, variant, toConvert, index_range) =
    PRE
        p_user : t_user_i   &
        p_user : t_user     &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        node  : t_Node_i    &
        node  : t_Node      &
        a_NodeClass(node) = e_ncl_Variable &
        node : dom(a_Value) &
        node : dom(a_Value_StatusCode) &
        variant : t_Variant_i &
        variant : t_Variant   &
        toConvert : BOOL & /* Variant need conversion */
        index_range : t_IndexRange_i &
        e_operation_type_write |-> a_NodeId(node) |-> e_aid_Value |-> p_user : s_AuthorizedOperations
    THEN
        CHOICE
            IF IsValid_VariantIndex(variant |-> index_range) = TRUE &
                HasRange_VariantIndex(variant |-> index_range) = TRUE &
                IsSupportedLocales_Variant(variant |-> p_locales) = TRUE
            THEN
                serviceStatusCode := e_sc_ok ||
                prev_dataValue :(prev_dataValue : t_DataValue_i &
                                 prev_dataValue : t_DataValue &
                                 DataValue_Variant(prev_dataValue) = a_Value(node) &
                                 DataValue_StatusCode(prev_dataValue) = a_Value_StatusCode(node) &
                                 DataValue_SourceTimestamp(prev_dataValue) = a_Value_SourceTimestamp(node)) ||
                a_Value(node) := variant
            ELSIF IsValid_VariantIndex(variant |-> index_range) = FALSE
            THEN
                serviceStatusCode := e_sc_bad_index_range_invalid ||
                prev_dataValue := c_DataValue_indet
            ELSIF HasRange_VariantIndex(variant |-> index_range) = FALSE
            THEN
                serviceStatusCode := e_sc_bad_index_range_no_data ||
                prev_dataValue := c_DataValue_indet
            ELSE /* Locale unsupported: note that it should be Bad_LocaleNotSupported but it is not defined in schemas */
                serviceStatusCode := e_sc_bad_invalid_argument ||
                prev_dataValue := c_DataValue_indet
            END
        OR
            serviceStatusCode
            :(serviceStatusCode : t_StatusCode_i &
              serviceStatusCode = e_sc_bad_out_of_memory) ||
            prev_dataValue := c_DataValue_indet
        END
    END
    ;

    set_Value_StatusCode(p_user, p_node, p_sc) =
    PRE
        p_user : t_user_i   &
        p_user : t_user     &
        p_node  : t_Node_i    &
        p_node  : t_Node      &
        p_node : dom(a_Value_StatusCode) &
        p_sc : t_RawStatusCode &
        e_operation_type_write |-> a_NodeId(p_node) |-> e_aid_Value |-> p_user : s_AuthorizedOperations
    THEN
        a_Value_StatusCode(p_node) := p_sc
    END
    ;

    set_Value_SourceTimestamp(p_user, p_node, p_ts) =
    PRE
        p_user : t_user_i   &
        p_user : t_user     &
        p_node  : t_Node_i    &
        p_node  : t_Node      &
        p_node : dom(a_Value_StatusCode) &
        p_ts : t_Timestamp &
        e_operation_type_write |-> a_NodeId(p_node) |-> e_aid_Value |-> p_user : s_AuthorizedOperations
    THEN
        a_Value_SourceTimestamp(p_node) := p_ts
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
        sc := a_Value_StatusCode(node)
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

    p_bool <-- get_Executable(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : dom(a_Executable)
    THEN
        p_bool := a_Executable(p_node)
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
        /* Valid indexes > 0 */
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
        p_ref_index : t_RefIndex &
        Node_RefIndexEnd(p_node) >= p_ref_index
    THEN
        p_ref :(p_ref : t_Reference_i &
                p_ref = RefIndex_Reference(p_node |-> p_ref_index))
    END
    ;

    bres <-- is_IndexRangeDefined(p_index_range) =
    PRE
        p_index_range : t_IndexRange_i
    THEN
        bres := IsRangeDefined(p_index_range)
    END
    ;

    p_input_arg <-- get_InputArguments(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : dom(a_NodeClass) &
        a_NodeClass(p_node) = e_ncl_Method
    THEN
        p_input_arg :: t_Variant_i
    END
    ;

    p_valueRank <-- get_conv_Variant_ValueRank(p_variant) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant
    THEN
        p_valueRank :: t_ConcreteValueRank
    END
    ;

    p_type <-- get_conv_Variant_Type(p_variant) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant
    THEN
        p_type :: t_NodeId_i
    END

END
