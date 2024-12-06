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

 File Name            : session_roles.c

 Date                 : 30/09/2024 13:04:32

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_roles.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_roles__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_roles__compute_user_roles(
   const constants__t_user_i session_roles__p_user,
   constants__t_sessionRoles_i * const session_roles__p_roles) {
   {
      t_bool session_roles__l_nid_valid;
      constants__t_Node_i session_roles__l_roleSet_node;
      t_bool session_roles__l_continue;
      constants__t_Reference_i session_roles__l_ref;
      t_bool session_roles__l_bValidRoleRef;
      constants__t_Node_i session_roles__l_maybe_role_node;
      constants__t_NodeId_i session_roles__l_maybe_role_nodeId;
      t_bool session_roles__l_bres;
      
      *session_roles__p_roles = constants__c_sessionRoles_empty;
      address_space_itf__readall_AddressSpace_Node(constants__c_Server_ServerCapabilities_RoleSet_NodeId,
         &session_roles__l_nid_valid,
         &session_roles__l_roleSet_node);
      if (session_roles__l_nid_valid == true) {
         session_roles_granted_bs__initialize_session_roles();
         roleset_references_it__init_iter_roleset_references(session_roles__l_roleSet_node,
            &session_roles__l_continue);
         while (session_roles__l_continue == true) {
            roleset_references_it__continue_iter_roleset_references(&session_roles__l_continue,
               &session_roles__l_ref);
            session_role_eval__is_ref_role(session_roles__l_ref,
               &session_roles__l_bValidRoleRef,
               &session_roles__l_maybe_role_node,
               &session_roles__l_maybe_role_nodeId);
            if (session_roles__l_bValidRoleRef == true) {
               session_role_eval__role_eval_user(session_roles__p_user,
                  session_roles__l_maybe_role_node,
                  &session_roles__l_bres);
               if (session_roles__l_bres == true) {
                  session_roles_granted_bs__add_role_to_session(session_roles__l_maybe_role_nodeId);
               }
            }
         }
         session_roles_granted_bs__pop_session_roles(session_roles__p_roles);
      }
   }
}

