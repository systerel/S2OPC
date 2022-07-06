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
    service_add_nodes_1

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

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

    d_adsp_operations;

    StatusCode_service <-- treat_write_request(p_user, p_locales, write_msg, resp_msg) =
    PRE
        p_user : t_user_i    &
        p_user : t_user      &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        write_msg : t_msg_i  &
        write_msg = c_msg_in &
        c_msg_in : t_msg     &
        a_msg_in_type = e_msg_attribute_write_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        StatusCode_service,
        a_Value
        :(
            StatusCode_service       : t_StatusCode_i &
            StatusCode_service       : {e_sc_ok, e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops, e_sc_bad_out_of_memory}  &
            a_Value                  : s_Node +-> t_Variant &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType})
        )
    END
    ;

    sc_operation, new_nodeId
      <-- treat_add_nodes_item (p_parentExpNid, p_refTypeId, p_reqExpNodeId, p_browseName, p_nodeClass, p_nodeAttributes, p_typeDefId) =
    PRE
        p_parentExpNid : t_ExpandedNodeId_i &
        p_parentExpNid : t_ExpandedNodeId &
        p_refTypeId : t_NodeId_i &
        p_refTypeId : t_NodeId &
        p_reqExpNodeId : t_ExpandedNodeId_i &
        p_browseName : t_QualifiedName_i &
        p_browseName : t_QualifiedName &
        p_nodeClass : t_NodeClass_i &
        p_nodeClass : t_NodeClass &
        p_nodeAttributes : t_NodeAttributes_i &
        p_nodeAttributes : t_NodeAttributes &
        p_typeDefId : t_ExpandedNodeId_i &
        (p_nodeClass :{e_ncl_Object, e_ncl_Variable} => p_typeDefId : t_ExpandedNodeId)
    THEN
        sc_operation,
        new_nodeId,
        d_adsp_nodes_variables
        :(
            d_adsp_nodes_invariant &
            sc_operation : t_StatusCode_i &
            sc_operation : {e_sc_ok,
                            // temporary to indicate unsupported node classes
                            e_sc_bad_node_class_invalid,
                            e_sc_bad_out_of_memory }  &
            new_nodeId : t_NodeId_i &
            (sc_operation = e_sc_ok =>
                new_nodeId : t_NodeId &
                new_nodeId : ran(a_NodeId) &
                (p_reqExpNodeId : t_ExpandedNodeId =>
                    p_reqExpNodeId : dom(conv_ExpandedNodeId_NodeId) &
                    new_nodeId = conv_ExpandedNodeId_NodeId(p_reqExpNodeId))
          )
        )
    END

END
