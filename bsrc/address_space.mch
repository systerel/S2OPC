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
    constants_statuscodes_bs,
    service_write_decode_bs, /* TODO: move to refinment */
    data_value_pointer_bs

DEFINITIONS
    "address_space.def"

ABSTRACT_VARIABLES
    d_adsp_variables

INVARIANT
    d_adsp_invariant

INITIALISATION
    d_adsp_variables
    :(
        d_adsp_invariant
    )

OPERATIONS

    d_adsp_UNINIT;
    d_adsp_operations;

    p_status <-- treat_write_request_WriteValue (p_user, p_locales, p_wvi) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds &
        p_wvi : t_WriteValue_i &
        p_wvi : t_WriteValue
    THEN
        LET dataValue, index_range, nid, aid BE
            dataValue = WriteValue_DataValue(p_wvi) &
            index_range = WriteValue_IndexRange(p_wvi) &
            nid = WriteValue_NodeId(p_wvi) &
            aid = WriteValue_AttributeId(p_wvi)
        IN
        p_status,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp :(
            p_status : t_StatusCode_i &
            p_status : t_StatusCode &

            (aid : t_AttributeId
            =>
                p_status : {
                    e_sc_bad_node_id_unknown,
                    e_sc_bad_not_writable,
                    e_sc_bad_type_mismatch,
                    e_sc_bad_not_writable,
                    e_sc_bad_user_access_denied,
                    e_sc_bad_index_range_invalid,
                    e_sc_bad_index_range_no_data,
                    e_sc_bad_invalid_argument,
                    e_sc_bad_out_of_memory,
                    e_sc_ok
                }
            ) &
            (not(aid : t_AttributeId)
            =>
                p_status = e_sc_bad_attribute_id_invalid
            ) &
            (p_status = e_sc_ok =>
                a_Value = a_Value$0 <+ {a_NodeId~(nid) |-> DataValue_Variant(dataValue)} &

                (local_service_treatment = TRUE
                =>
                    a_Value_StatusCode = a_Value_StatusCode$0
                        <+ {a_NodeId~(nid) |-> DataValue_StatusCode(dataValue)} &
                    a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
                        <+ {a_NodeId~(nid) |-> DataValue_SourceTimestamp(dataValue)}
                ) &
                (local_service_treatment = FALSE
                =>
                    (conv_AccessLevel_StatusWrite(a_AccessLevel(a_NodeId~(nid))) = TRUE
                    =>
                        a_Value_StatusCode = a_Value_StatusCode$0
                            <+ {a_NodeId~(nid) |-> DataValue_StatusCode(dataValue)}
                    ) &
                    (not(conv_AccessLevel_StatusWrite(a_AccessLevel(a_NodeId~(nid))) = TRUE)
                    =>
                        a_Value_StatusCode = a_Value_StatusCode$0) &

                    (conv_AccessLevel_TimestampWrite (a_AccessLevel(a_NodeId~(nid))) = TRUE
                    =>
                        a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
                            <+ {a_NodeId~(nid) |-> DataValue_SourceTimestamp(dataValue)}
                    ) &
                    (not(conv_AccessLevel_TimestampWrite (a_AccessLevel(a_NodeId~(nid))) = TRUE)
                    =>
                        a_Value_SourceTimestamp = a_Value_SourceTimestamp$0)
                )
            ) &
            (not (p_status = e_sc_ok)
            =>
                a_Value = a_Value$0 &
                a_Value_StatusCode = a_Value_StatusCode$0 &
                a_Value_SourceTimestamp = a_Value_SourceTimestamp$0
            )
        )
        END
    END
    ;

    p_rawStatusCode, p_nb_out, p_out_arguments <-- exec_callMethod(p_endpoint_config_idx, p_call_method_pointer) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_call_method_pointer : t_CallMethodPointer_i &
        p_call_method_pointer : t_CallMethodPointer
    THEN
        d_adsp_variables, /* Any address space change is possible */
        p_rawStatusCode,
        p_nb_out,
        p_out_arguments
        :(
            d_adsp_invariant &
            p_rawStatusCode : t_RawStatusCode &
            p_nb_out : NAT &
            p_out_arguments : t_ArgumentsPointer_i &
            (p_rawStatusCode /= c_RawStatusCode_Good =>
                p_nb_out = 0 &
                p_out_arguments = c_ArgumentsPointer_indet
            ) &
            (p_rawStatusCode = c_RawStatusCode_Good =>
                (p_nb_out > 0 =>
                 p_out_arguments : t_ArgumentsPointer)
            )
        )
    END
    ;

    /* Node Management operations */
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
        p_nodeClass : t_NodeClass &
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

        s_Node,
        a_NodeId,
        a_NodeClass,
        a_BrowseName,
        a_DisplayName,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp,
        a_DataType,
        a_ValueRank,
        a_ArrayDimensions,
        a_AccessLevel,
        a_UserAccessLevel,
        a_Historizing,
        Ref_HasTypeDefinition,
        Reference_SourceNode,
        Reference_ReferenceType,
        Reference_TargetNode,
        Reference_IsForward,
        Node_RefIndexEnd,
        RefIndex_Reference
        :(
            sc_addnode : t_StatusCode_i &
            sc_addnode : {e_sc_ok, e_sc_bad_browse_name_duplicated, e_sc_bad_reference_not_allowed,
                          e_sc_bad_type_definition_invalid, e_sc_bad_node_attributes_invalid, e_sc_bad_out_of_memory,
                          e_sc_bad_node_class_invalid} & // BadNodeClassInvalid used for unsupported NodeClass for now
            s_Node <: t_Node &

            /* Attributes */
            a_NodeId      : s_Node >-> t_NodeId        &
            a_NodeClass   : s_Node --> t_NodeClass     &
            a_BrowseName  : s_Node --> t_QualifiedName &
            a_DisplayName : s_Node --> t_LocalizedText &
            a_Value       : s_Node +-> t_Variant   &
            a_Value_StatusCode : s_Node +-> t_RawStatusCode &
            a_Value_SourceTimestamp : s_Node +-> t_Timestamp &
            a_DataType : s_Node +-> t_NodeId &
            a_ValueRank : s_Node +-> t_ValueRank &
            a_ArrayDimensions : s_Node +-> t_ArrayDimensions &
            a_AccessLevel : s_Node +-> t_access_level &
            a_UserAccessLevel : s_Node +-> t_access_level &
            a_Historizing : s_Node +-> BOOL &

            Ref_HasTypeDefinition : s_Node +-> t_ExpandedNodeId &

            Reference_SourceNode    : t_Reference +-> t_NodeId         &
            Reference_ReferenceType : t_Reference +-> t_NodeId         &
            Reference_TargetNode    : t_Reference +-> t_ExpandedNodeId &
            Reference_IsForward     : t_Reference +-> BOOL             &

            Node_RefIndexEnd        : s_Node     +-> t_RefIndex  &
            RefIndex_Reference      : s_Node * t_RefIndex +-> t_Reference &

            (sc_addnode = e_sc_ok =>
                /* We use inclusion since adding 1 node might lead to add several nodes (if implemented) */
                s_Node$0 <: s_Node &
                a_NodeId$0 <: a_NodeId &
                a_NodeClass$0 <: a_NodeClass &
                a_BrowseName$0 <: a_BrowseName &
                a_DisplayName$0 <: a_DisplayName &
                a_Value$0 <: a_Value &
                a_Value_StatusCode$0 <: a_Value_StatusCode &
                a_Value_SourceTimestamp$0 <: a_Value_SourceTimestamp &
                a_DataType$0 <: a_DataType &
                a_ValueRank$0 <: a_ValueRank &
                a_ArrayDimensions$0 <: a_ArrayDimensions &
                a_AccessLevel$0 <: a_AccessLevel &
                a_UserAccessLevel$0 <: a_UserAccessLevel &
                a_Historizing$0 <: a_Historizing &

                Ref_HasTypeDefinition$0 <: Ref_HasTypeDefinition &

                Reference_SourceNode$0    <: Reference_SourceNode &
                Reference_ReferenceType$0 <: Reference_ReferenceType &
                Reference_TargetNode$0    <: Reference_TargetNode &
                Reference_IsForward$0     <: Reference_IsForward &

                Node_RefIndexEnd$0        <: Node_RefIndexEnd &
                RefIndex_Reference$0      <: RefIndex_Reference &
                /* Minimal added references are 2 references for parent / child relationship */
                #(l_node, l_refIndex, l_ref, l_parentNode, l_refParentIndex, l_refParent).(
                    (l_node : s_Node & l_node /: s_Node$0 &
                     l_refIndex : t_RefIndex &
                     l_ref = RefIndex_Reference(l_node |-> l_refIndex) & l_ref /: ran(RefIndex_Reference$0) &
                     l_ref /: dom(Reference_SourceNode$0)    & l_ref : dom(Reference_SourceNode) &
                     l_ref /: dom(Reference_ReferenceType$0) & l_ref : dom(Reference_ReferenceType) &
                     l_ref /: dom(Reference_TargetNode$0)    & l_ref : dom(Reference_TargetNode) &
                     l_ref /: dom(Reference_IsForward$0)     & l_ref : dom(Reference_IsForward)) &
                    (
                       (l_node |-> p_newNodeId) : a_NodeId &
                       (l_node |-> p_nodeClass) : a_NodeClass &
                       (l_node |-> p_browseName) : a_BrowseName &
                       (l_ref |-> p_newNodeId) : Reference_SourceNode &
                       (l_ref |-> p_refTypeId) : Reference_ReferenceType &
                       (l_ref |-> p_parentNid) : Reference_TargetNode &
                       (l_ref |-> FALSE) : Reference_IsForward
                    ) &
                    (l_parentNode : s_Node$0 & a_NodeId(l_parentNode) = conv_ExpandedNodeId_NodeId(p_parentNid) &
                     l_refParentIndex : t_RefIndex &
                     l_refParent = RefIndex_Reference$0(l_parentNode |-> l_refParentIndex) &
                     l_refParent /: ran(RefIndex_Reference$0) &
                     l_refParent /: dom(Reference_SourceNode$0)    & l_refParent : dom(Reference_SourceNode) &
                     l_refParent /: dom(Reference_ReferenceType$0) & l_refParent : dom(Reference_ReferenceType) &
                     l_refParent /: dom(Reference_TargetNode$0)    & l_refParent : dom(Reference_TargetNode) &
                     l_refParent /: dom(Reference_IsForward$0)     & l_refParent : dom(Reference_IsForward)) &
                    (
                        (l_refParent |-> conv_ExpandedNodeId_NodeId(p_parentNid)) : Reference_SourceNode &
                        (l_refParent |-> p_refTypeId) : Reference_ReferenceType &
                        (l_refParent |-> conv_NodeId_ExpandedNodeId(p_newNodeId)) : Reference_TargetNode &
                        (l_refParent |-> TRUE) : Reference_IsForward
                    )
                )
            ) &
            (not (sc_addnode = e_sc_ok) =>
                s_Node = s_Node$0 &

                /* Attributes */
                a_NodeId = a_NodeId$0 &
                a_NodeClass = a_NodeClass$0 &
                a_BrowseName = a_BrowseName$0 &
                a_DisplayName = a_DisplayName$0 &
                a_Value = a_Value$0 &
                a_Value_StatusCode = a_Value_StatusCode$0 &
                a_Value_SourceTimestamp = a_Value_SourceTimestamp$0 &
                a_DataType = a_DataType$0 &
                a_ValueRank = a_ValueRank$0 &
                a_ArrayDimensions = a_ArrayDimensions$0 &
                a_AccessLevel = a_AccessLevel$0 &
                a_UserAccessLevel = a_UserAccessLevel$0 &
                a_Historizing = a_Historizing$0 &

                Ref_HasTypeDefinition = Ref_HasTypeDefinition$0 &

                Reference_SourceNode    = Reference_SourceNode$0 &
                Reference_ReferenceType = Reference_ReferenceType$0 &
                Reference_TargetNode    = Reference_TargetNode$0 &
                Reference_IsForward     = Reference_IsForward$0 &

                Node_RefIndexEnd        = Node_RefIndexEnd$0 &
                RefIndex_Reference      = RefIndex_Reference$0
            )
        )
    END

END
