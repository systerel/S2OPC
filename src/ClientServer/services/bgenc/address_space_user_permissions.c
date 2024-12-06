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

 File Name            : address_space_user_permissions.c

 Date                 : 31/10/2024 10:53:56

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_user_permissions.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_user_permissions__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_user_permissions__l_read_AddressSpace_RolePermissionsOrDefault(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   constants__t_RolePermissionTypes_i * const address_space_user_permissions__p_rolePermissions) {
   {
      constants__t_RolePermissionTypes_i address_space_user_permissions__l_nodeRolePermissions;
      constants_statuscodes_bs__t_StatusCode_i address_space_user_permissions__l_sc;
      constants__t_NamespaceIdx address_space_user_permissions__l_nodeNSidx;
      t_bool address_space_user_permissions__l_isvalid;
      constants__t_Node_i address_space_user_permissions__l_node;
      
      address_space_user_permissions__l_sc = constants_statuscodes_bs__e_sc_bad_generic;
      address_space_user_permissions__l_nodeRolePermissions = constants__c_RolePermissionTypes_indet;
      address_space_bs__readall_AddressSpace_Node(address_space_user_permissions__p_nodeId,
         &address_space_user_permissions__l_isvalid,
         &address_space_user_permissions__l_node);
      if (address_space_user_permissions__l_isvalid == true) {
         address_space_bs__read_AddressSpace_RolePermissions(address_space_user_permissions__l_node,
            &address_space_user_permissions__l_sc,
            &address_space_user_permissions__l_nodeRolePermissions);
      }
      if (address_space_user_permissions__l_sc != constants_statuscodes_bs__e_sc_ok) {
         constants__get_NodeId_NamespaceIndex(address_space_user_permissions__p_nodeId,
            &address_space_user_permissions__l_nodeNSidx);
         address_space_namespaces__get_Namespace_DefaultRolePermissions(address_space_user_permissions__l_nodeNSidx,
            address_space_user_permissions__p_rolePermissions);
      }
      else {
         *address_space_user_permissions__p_rolePermissions = address_space_user_permissions__l_nodeRolePermissions;
      }
   }
}

void address_space_user_permissions__has_AddressSpace_RolePermissions(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   t_bool * const address_space_user_permissions__bres) {
   {
      constants__t_NamespaceIdx address_space_user_permissions__l_nodeNSidx;
      
      constants__get_NodeId_NamespaceIndex(address_space_user_permissions__p_nodeId,
         &address_space_user_permissions__l_nodeNSidx);
      address_space_namespaces__has_Namespace_DefaultRolePermissions(address_space_user_permissions__l_nodeNSidx,
         address_space_user_permissions__bres);
   }
}

void address_space_user_permissions__read_AddressSpace_UserRolePermissions(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   const constants__t_user_i address_space_user_permissions__p_user,
   const constants__t_sessionRoles_i address_space_user_permissions__p_roles,
   constants__t_PermissionType_i * const address_space_user_permissions__p_permissions) {
   {
      constants__t_RolePermissionTypes_i address_space_user_permissions__l_rolePermissions;
      t_bool address_space_user_permissions__l_continue;
      constants__t_NodeId_i address_space_user_permissions__l_role;
      constants__t_PermissionType_i address_space_user_permissions__l_user_permissions;
      
      address_space_user_permissions_bs__init_user_permissions(address_space_user_permissions__p_user);
      address_space_user_permissions__l_read_AddressSpace_RolePermissionsOrDefault(address_space_user_permissions__p_nodeId,
         &address_space_user_permissions__l_rolePermissions);
      if (address_space_user_permissions__l_rolePermissions == constants__c_RolePermissionTypes_indet) {
         *address_space_user_permissions__p_permissions = constants__c_PermissionType_none;
      }
      else {
         address_space_authorization_session_roles_it_bs__init_iter_roles(address_space_user_permissions__p_user,
            address_space_user_permissions__p_roles,
            &address_space_user_permissions__l_continue);
         while (address_space_user_permissions__l_continue == true) {
            address_space_authorization_session_roles_it_bs__continue_iter_roles(&address_space_user_permissions__l_continue,
               &address_space_user_permissions__l_role);
            role_permissions_value_eval__get_permissions_of_role(address_space_user_permissions__l_role,
               address_space_user_permissions__l_rolePermissions,
               &address_space_user_permissions__l_user_permissions);
            address_space_user_permissions_bs__add_user_permissions(address_space_user_permissions__p_user,
               address_space_user_permissions__l_user_permissions);
         }
         address_space_namespaces__delete_rolePermissions(address_space_user_permissions__l_rolePermissions);
         address_space_authorization_session_roles_it_bs__clear_iter_roles();
         address_space_user_permissions_bs__get_merged_user_permissions(address_space_user_permissions__p_user,
            address_space_user_permissions__p_permissions);
      }
   }
}

