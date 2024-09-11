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
    session_role_identity_node

SEES
    constants,
    address_space_itf

ABSTRACT_VARIABLES
    a_ref_identity

INVARIANT
    a_ref_identity : t_Reference +-> t_Node

INITIALISATION
    a_ref_identity :: t_Reference +-> t_Node

OPERATIONS
    p_maybe_node_Identity, p_maybe_nodeId_Identity <-- ref_maybe_get_Identity(p_ref) =
    PRE
        p_ref : t_Reference_i &
        Reference_IsForward(p_ref) = TRUE &
        Reference_ReferenceType(p_ref) = c_HasPropertyType_NodeId
    THEN
        p_maybe_node_Identity,
        p_maybe_nodeId_Identity
                :(
            p_maybe_nodeId_Identity : t_NodeId_i &
            p_maybe_node_Identity : t_Node_i &

            (Reference_IsForward(p_ref) = TRUE &
            Reference_ReferenceType(p_ref) = c_HasPropertyType_NodeId &
            a_NodeClass(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = e_ncl_Variable &
            a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = c_Identities_QualifiedName
            =>
            a_ref_identity(p_ref) = p_maybe_node_Identity &
            p_maybe_node_Identity = a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) &

            (Reference_IsForward(p_ref) /= TRUE or
            Reference_ReferenceType(p_ref) /= c_HasPropertyType_NodeId or
            a_NodeClass(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) /= e_ncl_Variable or
            a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) /= c_Identities_QualifiedName
            =>
            p_maybe_node_Identity = c_Node_indet))
     END
END
