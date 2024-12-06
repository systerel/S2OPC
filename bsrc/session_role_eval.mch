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
    session_role_eval

SEES
    constants,
    address_space_itf

ABSTRACT_VARIABLES
    a_user_role_match,
    s_role_node_refs

INVARIANT
    a_user_role_match : t_user <-> t_Node &
    s_role_node_refs <: t_Reference

INITIALISATION
    a_user_role_match :: t_user <-> t_Node ||
    s_role_node_refs :(s_role_node_refs <: t_Reference)

OPERATIONS
    p_bres <-- role_eval_user(p_user, p_role_node) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_role_node : t_Node_i &
        p_role_node : s_Node &
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(p_role_node)) = c_RoleType_NodeId
    THEN
        p_bres := bool(p_user |-> p_role_node : a_user_role_match)
    END
    ;

    p_bres, p_maybe_role_node, p_maybe_role_nodeId <-- is_ref_role(p_ref) =
    PRE
        p_ref : t_Reference_i &
        p_ref : t_Reference
    THEN
        p_bres,p_maybe_role_node, p_maybe_role_nodeId
        :(
            p_bres : BOOL & p_bres = bool(p_ref : s_role_node_refs) &
            p_maybe_role_node : t_Node_i & p_maybe_role_nodeId : t_NodeId_i &
            (p_bres = TRUE =>
                Reference_IsForward(p_ref) = TRUE &
                Reference_ReferenceType(p_ref) = c_HasComponentType_NodeId &
                p_maybe_role_nodeId = conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)) &
                p_maybe_role_nodeId = a_NodeId(p_maybe_role_node) &
                c_RoleType_NodeId = conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(p_maybe_role_node))
            ) &
            (p_bres = FALSE =>
                p_maybe_role_node = c_Node_indet &
                p_maybe_role_nodeId = c_NodeId_indet
            )
        )
    END

END
