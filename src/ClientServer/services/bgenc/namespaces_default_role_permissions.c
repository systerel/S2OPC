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

 File Name            : namespaces_default_role_permissions.c

 Date                 : 31/10/2024 15:35:57

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "namespaces_default_role_permissions.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespaces_default_role_permissions__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespaces_default_role_permissions__l_ref_get_node(
   const constants__t_Reference_i namespaces_default_role_permissions__p_ref,
   constants__t_Node_i * const namespaces_default_role_permissions__p_node) {
   {
      constants__t_ExpandedNodeId_i namespaces_default_role_permissions__l_ref_target;
      t_bool namespaces_default_role_permissions__l_local_server;
      constants__t_NodeId_i namespaces_default_role_permissions__l_ref_target_NodeId;
      t_bool namespaces_default_role_permissions__l_isvalid;
      constants__t_Node_i namespaces_default_role_permissions__l_ref_target_Node;
      
      *namespaces_default_role_permissions__p_node = constants__c_Node_indet;
      address_space_bs__get_Reference_TargetNode(namespaces_default_role_permissions__p_ref,
         &namespaces_default_role_permissions__l_ref_target);
      constants__getall_conv_ExpandedNodeId_NodeId(namespaces_default_role_permissions__l_ref_target,
         &namespaces_default_role_permissions__l_local_server,
         &namespaces_default_role_permissions__l_ref_target_NodeId);
      if (namespaces_default_role_permissions__l_local_server == true) {
         address_space_bs__readall_AddressSpace_Node(namespaces_default_role_permissions__l_ref_target_NodeId,
            &namespaces_default_role_permissions__l_isvalid,
            &namespaces_default_role_permissions__l_ref_target_Node);
         if (namespaces_default_role_permissions__l_isvalid == true) {
            *namespaces_default_role_permissions__p_node = namespaces_default_role_permissions__l_ref_target_Node;
         }
      }
   }
}

void namespaces_default_role_permissions__l_node_check_NodeClass_and_TypeDef(
   const constants__t_Node_i namespaces_default_role_permissions__p_node,
   t_bool * const namespaces_default_role_permissions__p_bres) {
   {
      constants__t_NodeClass_i namespaces_default_role_permissions__l_NodeClass;
      constants__t_ExpandedNodeId_i namespaces_default_role_permissions__l_typeDefinition;
      t_bool namespaces_default_role_permissions__l_local_server;
      constants__t_NodeId_i namespaces_default_role_permissions__l_NodeId;
      t_bool namespaces_default_role_permissions__l_nodeIdsEqual;
      
      *namespaces_default_role_permissions__p_bres = false;
      address_space_bs__get_NodeClass(namespaces_default_role_permissions__p_node,
         &namespaces_default_role_permissions__l_NodeClass);
      if (namespaces_default_role_permissions__l_NodeClass == constants__e_ncl_Object) {
         address_space_bs__get_TypeDefinition(namespaces_default_role_permissions__p_node,
            &namespaces_default_role_permissions__l_typeDefinition);
         constants__getall_conv_ExpandedNodeId_NodeId(namespaces_default_role_permissions__l_typeDefinition,
            &namespaces_default_role_permissions__l_local_server,
            &namespaces_default_role_permissions__l_NodeId);
         address_space_bs__is_NodeId_equal(namespaces_default_role_permissions__l_NodeId,
            constants__c_NamespaceMetadataType_NodeId,
            &namespaces_default_role_permissions__l_nodeIdsEqual);
         if (namespaces_default_role_permissions__l_nodeIdsEqual == true) {
            *namespaces_default_role_permissions__p_bres = true;
         }
      }
   }
}

void namespaces_default_role_permissions__get_DefaultRolePermissions(
   const constants__t_NamespaceUri namespaces_default_role_permissions__p_namespaceUri,
   constants__t_RolePermissionTypes_i * const namespaces_default_role_permissions__p_val_DefaultRolePermissions) {
   {
      t_bool namespaces_default_role_permissions__l_nid_valid;
      constants__t_Node_i namespaces_default_role_permissions__l_Server_Namespaces_node;
      t_bool namespaces_default_role_permissions__l_continue;
      constants__t_Reference_i namespaces_default_role_permissions__l_ref;
      t_bool namespaces_default_role_permissions__l_bres;
      constants__t_RolePermissionTypes_i namespaces_default_role_permissions__l_maybe_val_DefaultRolePermissions;
      t_bool namespaces_default_role_permissions__l_bValidRef;
      constants__t_Node_i namespaces_default_role_permissions__l_ref_target_Node;
      t_bool namespaces_default_role_permissions__l_bValidNode;
      
      *namespaces_default_role_permissions__p_val_DefaultRolePermissions = constants__c_RolePermissionTypes_indet;
      namespaces_default_role_permissions__l_bValidNode = false;
      address_space_bs__readall_AddressSpace_Node(constants__c_Server_Namespaces_NodeId,
         &namespaces_default_role_permissions__l_nid_valid,
         &namespaces_default_role_permissions__l_Server_Namespaces_node);
      namespaces_default_role_permissions__l_continue = false;
      if (namespaces_default_role_permissions__l_nid_valid == true) {
         namespaces_refs_it__init_iter_namespaces_refs(namespaces_default_role_permissions__l_Server_Namespaces_node,
            &namespaces_default_role_permissions__l_continue);
      }
      while (namespaces_default_role_permissions__l_continue == true) {
         namespaces_refs_it__continue_iter_namespaces_refs(&namespaces_default_role_permissions__l_continue,
            &namespaces_default_role_permissions__l_ref);
         namespace_default_role_permissions__check_reference_isForward_and_RefType(namespaces_default_role_permissions__l_ref,
            constants__c_HasComponentType_NodeId,
            &namespaces_default_role_permissions__l_bValidRef);
         if (namespaces_default_role_permissions__l_bValidRef == true) {
            namespaces_default_role_permissions__l_ref_get_node(namespaces_default_role_permissions__l_ref,
               &namespaces_default_role_permissions__l_ref_target_Node);
            if (namespaces_default_role_permissions__l_ref_target_Node != constants__c_Node_indet) {
               namespaces_default_role_permissions__l_node_check_NodeClass_and_TypeDef(namespaces_default_role_permissions__l_ref_target_Node,
                  &namespaces_default_role_permissions__l_bValidNode);
            }
            if (namespaces_default_role_permissions__l_bValidNode == true) {
               namespace_default_role_permissions__namespacemetadata_and_uri_match(namespaces_default_role_permissions__p_namespaceUri,
                  namespaces_default_role_permissions__l_ref_target_Node,
                  &namespaces_default_role_permissions__l_bres,
                  &namespaces_default_role_permissions__l_maybe_val_DefaultRolePermissions);
               if (namespaces_default_role_permissions__l_bres == true) {
                  *namespaces_default_role_permissions__p_val_DefaultRolePermissions = namespaces_default_role_permissions__l_maybe_val_DefaultRolePermissions;
               }
            }
         }
      }
   }
}

