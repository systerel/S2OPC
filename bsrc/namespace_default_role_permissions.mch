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
    namespace_default_role_permissions

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    a_ns_uri,
    a_ns_default_role_permissions

INVARIANT
    a_ns_uri : t_Node +-> t_NamespaceUri &
    a_ns_default_role_permissions : t_Node +-> t_RolePermissionTypes_i

INITIALISATION
    a_ns_uri :: t_Node +-> t_NamespaceUri ||
    a_ns_default_role_permissions :: t_Node +-> t_RolePermissionTypes_i

OPERATIONS

    // Check that ref is forward and that RefType is p_RefType_NodeId
    p_bres <-- check_reference_isForward_and_RefType(p_ref, p_RefType_NodeId) =
    PRE
        p_ref : t_Reference_i &
        p_ref : t_Reference &
        p_RefType_NodeId : t_NodeId_i &
        p_RefType_NodeId : t_NodeId
    THEN
        p_bres := bool(Reference_IsForward(p_ref) = TRUE & Reference_ReferenceType(p_ref) = p_RefType_NodeId)
    END
    ;

    p_bres, p_maybe_val_DefaultRolePermissions <-- namespacemetadata_and_uri_match(p_namespaceUri, p_namespacemetadata_Node) =
    PRE
        p_namespaceUri : t_NamespaceUri &
        p_namespacemetadata_Node : t_Node_i &
        p_namespacemetadata_Node : t_Node &
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(p_namespacemetadata_Node)) = c_NamespaceMetadataType_NodeId
    THEN
        p_bres,
        p_maybe_val_DefaultRolePermissions :(p_bres : BOOL &
            p_maybe_val_DefaultRolePermissions : t_RolePermissionTypes_i &
            p_bres = bool(a_ns_uri(p_namespacemetadata_Node) = p_namespaceUri) &
            (p_bres = TRUE => p_maybe_val_DefaultRolePermissions = a_ns_default_role_permissions(p_namespacemetadata_Node)) &
            (p_bres = FALSE => p_maybe_val_DefaultRolePermissions = c_RolePermissionTypes_indet))
    END
    ;

    delete_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        skip
    END

END
