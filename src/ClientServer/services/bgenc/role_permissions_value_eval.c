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

 File Name            : role_permissions_value_eval.c

 Date                 : 30/10/2024 16:34:22

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "role_permissions_value_eval.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void role_permissions_value_eval__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void role_permissions_value_eval__get_permissions_of_role(
   const constants__t_NodeId_i role_permissions_value_eval__p_role_nodeId,
   const constants__t_RolePermissionTypes_i role_permissions_value_eval__p_rolesPermissions,
   constants__t_PermissionType_i * const role_permissions_value_eval__p_role_permissions) {
   {
      t_entier4 role_permissions_value_eval__l_nb_rolePermissions;
      t_bool role_permissions_value_eval__l_continue;
      t_entier4 role_permissions_value_eval__l_rolePermissionIdx;
      constants__t_NodeId_i role_permissions_value_eval__l_roleId;
      t_bool role_permissions_value_eval__l_roleIds_match;
      constants__t_PermissionType_i role_permissions_value_eval__l_role_permissions;
      
      role_permissions_value_eval_bs__init_user_role_permissions(role_permissions_value_eval__p_role_nodeId);
      role_permissions_value_eval_bs__read_variant_rolePermissions(role_permissions_value_eval__p_rolesPermissions,
         &role_permissions_value_eval__l_nb_rolePermissions);
      role_permissions_value_it__init_iter_rolePermissions(role_permissions_value_eval__l_nb_rolePermissions,
         &role_permissions_value_eval__l_continue);
      *role_permissions_value_eval__p_role_permissions = constants__c_PermissionType_none;
      while (role_permissions_value_eval__l_continue == true) {
         role_permissions_value_it__continue_iter_rolePermissions(&role_permissions_value_eval__l_continue,
            &role_permissions_value_eval__l_rolePermissionIdx);
         role_permissions_value_eval_bs__read_rolePermissions_roleId(role_permissions_value_eval__p_rolesPermissions,
            role_permissions_value_eval__l_rolePermissionIdx,
            &role_permissions_value_eval__l_roleId);
         address_space_bs__is_NodeId_equal(role_permissions_value_eval__p_role_nodeId,
            role_permissions_value_eval__l_roleId,
            &role_permissions_value_eval__l_roleIds_match);
         if (role_permissions_value_eval__l_roleIds_match == true) {
            role_permissions_value_eval_bs__read_rolePermissions_permissions(role_permissions_value_eval__p_rolesPermissions,
               role_permissions_value_eval__l_rolePermissionIdx,
               &role_permissions_value_eval__l_role_permissions);
            role_permissions_value_eval_bs__add_user_role_permissions(role_permissions_value_eval__p_role_nodeId,
               role_permissions_value_eval__l_role_permissions);
         }
      }
      role_permissions_value_eval_bs__get_merged_user_role_permissions(role_permissions_value_eval__p_role_nodeId,
         role_permissions_value_eval__p_role_permissions);
   }
}

