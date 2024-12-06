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
    namespace_uri

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    a_ref_namespaceUri

INVARIANT
    a_ref_namespaceUri : t_Reference +-> t_Variant

INITIALISATION
    a_ref_namespaceUri :: t_Reference +-> t_Variant

OPERATIONS

    p_maybe_val_NamespaceUri <-- ref_maybe_get_NamespaceUri(p_ref) =
    PRE
        p_ref : t_Reference_i &
        Reference_IsForward(p_ref) = TRUE &
        Reference_ReferenceType(p_ref) = c_HasPropertyType_NodeId
    THEN
        p_maybe_val_NamespaceUri :(
            p_maybe_val_NamespaceUri : t_Variant_i &
            (p_ref : dom(a_ref_namespaceUri) <=>
                (a_NodeClass(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = e_ncl_Variable &
                 a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = c_NamespaceUri_QualifiedName)) &
            (p_ref : dom(a_ref_namespaceUri) =>
                a_ref_namespaceUri(p_ref) = p_maybe_val_NamespaceUri &
                p_maybe_val_NamespaceUri = a_Value(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref))))) &
            (p_ref /: dom(a_ref_namespaceUri) => p_maybe_val_NamespaceUri = c_Variant_indet))
    END

END
