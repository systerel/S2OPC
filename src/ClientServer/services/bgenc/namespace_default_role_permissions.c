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

/******************************************************************************

 File Name            : namespace_default_role_permissions.c

 Date                 : 31/10/2024 15:35:56

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "namespace_default_role_permissions.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespace_default_role_permissions__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespace_default_role_permissions__l_ref_check_isForward_and_RefType(
   const constants__t_Reference_i namespace_default_role_permissions__p_ref,
   const constants__t_NodeId_i namespace_default_role_permissions__p_RefType_NodeId,
   t_bool * const namespace_default_role_permissions__p_bres) {
   {
      t_bool namespace_default_role_permissions__l_IsForward;
      constants__t_NodeId_i namespace_default_role_permissions__l_RefType_NodeId;
      t_bool namespace_default_role_permissions__l_nodeIdsEqual;
      
      *namespace_default_role_permissions__p_bres = false;
      address_space_bs__get_Reference_IsForward(namespace_default_role_permissions__p_ref,
         &namespace_default_role_permissions__l_IsForward);
      if (namespace_default_role_permissions__l_IsForward == true) {
         address_space_bs__get_Reference_ReferenceType(namespace_default_role_permissions__p_ref,
            &namespace_default_role_permissions__l_RefType_NodeId);
         address_space_bs__is_NodeId_equal(namespace_default_role_permissions__l_RefType_NodeId,
            namespace_default_role_permissions__p_RefType_NodeId,
            &namespace_default_role_permissions__l_nodeIdsEqual);
         if (namespace_default_role_permissions__l_nodeIdsEqual == true) {
            *namespace_default_role_permissions__p_bres = true;
         }
      }
   }
}

void namespace_default_role_permissions__check_reference_isForward_and_RefType(
   const constants__t_Reference_i namespace_default_role_permissions__p_ref,
   const constants__t_NodeId_i namespace_default_role_permissions__p_RefType_NodeId,
   t_bool * const namespace_default_role_permissions__p_bres) {
   namespace_default_role_permissions__l_ref_check_isForward_and_RefType(namespace_default_role_permissions__p_ref,
      namespace_default_role_permissions__p_RefType_NodeId,
      namespace_default_role_permissions__p_bres);
}

void namespace_default_role_permissions__namespacemetadata_and_uri_match(
   const constants__t_NamespaceUri namespace_default_role_permissions__p_namespaceUri,
   const constants__t_Node_i namespace_default_role_permissions__p_namespacemetadata_Node,
   t_bool * const namespace_default_role_permissions__p_bres,
   constants__t_RolePermissionTypes_i * const namespace_default_role_permissions__p_maybe_val_DefaultRolePermissions) {
   {
      t_bool namespace_default_role_permissions__l_continue;
      constants__t_Reference_i namespace_default_role_permissions__l_ref;
      t_bool namespace_default_role_permissions__l_bres;
      constants__t_RolePermissionTypes_i namespace_default_role_permissions__l_val_DefaultRolePermissions;
      constants__t_Variant_i namespace_default_role_permissions__l_val_NamespaceUri;
      
      *namespace_default_role_permissions__p_maybe_val_DefaultRolePermissions = constants__c_RolePermissionTypes_indet;
      namespace_default_role_permissions__l_val_DefaultRolePermissions = constants__c_RolePermissionTypes_indet;
      *namespace_default_role_permissions__p_bres = false;
      namespace_metadata_refs_it__init_iter_namespacemetadata_refs(namespace_default_role_permissions__p_namespacemetadata_Node,
         &namespace_default_role_permissions__l_continue);
      while (namespace_default_role_permissions__l_continue == true) {
         namespace_metadata_refs_it__continue_iter_namespacemetadata_refs(&namespace_default_role_permissions__l_continue,
            &namespace_default_role_permissions__l_ref);
         namespace_default_role_permissions__l_ref_check_isForward_and_RefType(namespace_default_role_permissions__l_ref,
            constants__c_HasPropertyType_NodeId,
            &namespace_default_role_permissions__l_bres);
         if (namespace_default_role_permissions__l_bres == true) {
            if (namespace_default_role_permissions__l_val_DefaultRolePermissions == constants__c_RolePermissionTypes_indet) {
               namespace_default_role_permissions_value__ref_maybe_get_DefaultRolePermissions(namespace_default_role_permissions__l_ref,
                  &namespace_default_role_permissions__l_val_DefaultRolePermissions);
            }
            namespace_uri__ref_maybe_get_NamespaceUri(namespace_default_role_permissions__l_ref,
               &namespace_default_role_permissions__l_val_NamespaceUri);
            if (namespace_default_role_permissions__l_val_NamespaceUri != constants__c_Variant_indet) {
               namespaces_uri_eval_bs__compare_namespaceUri_and_value_uri(namespace_default_role_permissions__p_namespaceUri,
                  namespace_default_role_permissions__l_val_NamespaceUri,
                  namespace_default_role_permissions__p_bres);
            }
            address_space_bs__read_AddressSpace_free_variant(namespace_default_role_permissions__l_val_NamespaceUri);
         }
      }
      if (*namespace_default_role_permissions__p_bres == true) {
         *namespace_default_role_permissions__p_maybe_val_DefaultRolePermissions = namespace_default_role_permissions__l_val_DefaultRolePermissions;
      }
      else {
         namespace_default_role_permissions_value__delete_rolePermissions(namespace_default_role_permissions__l_val_DefaultRolePermissions);
      }
   }
}

