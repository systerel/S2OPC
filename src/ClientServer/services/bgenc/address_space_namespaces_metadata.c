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

 File Name            : address_space_namespaces_metadata.c

 Date                 : 02/12/2024 16:44:52

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_namespaces_metadata.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_namespaces_metadata__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_namespaces_metadata__l_fill_default_role_permissions(
   const constants__t_Variant_i address_space_namespaces_metadata__p_val) {
   {
      t_bool address_space_namespaces_metadata__l_continue;
      t_entier4 address_space_namespaces_metadata__l_namespaceUriIdx;
      constants__t_NamespaceUri address_space_namespaces_metadata__l_namespaceUri;
      constants__t_RolePermissionTypes_i address_space_namespaces_metadata__l_val_DefaultRolePermissions;
      constants__t_NamespaceIdx address_space_namespaces_metadata__l_namespaceUriIdxNS;
      t_entier4 address_space_namespaces_metadata__l_nb_namespaces;
      
      namespace_array_bs__read_variant_nb_namespaceUris(address_space_namespaces_metadata__p_val,
         &address_space_namespaces_metadata__l_nb_namespaces);
      default_role_permissions_array_bs__init_array_of_DefaultRolePermissions(address_space_namespaces_metadata__l_nb_namespaces);
      namespace_array_it__init_iter_namespaceUris(address_space_namespaces_metadata__l_nb_namespaces,
         &address_space_namespaces_metadata__l_continue);
      while (address_space_namespaces_metadata__l_continue == true) {
         namespace_array_it__continue_iter_namespaceUris(&address_space_namespaces_metadata__l_continue,
            &address_space_namespaces_metadata__l_namespaceUriIdx);
         namespace_array_bs__read_variant_namespaceUri(address_space_namespaces_metadata__p_val,
            address_space_namespaces_metadata__l_namespaceUriIdx,
            &address_space_namespaces_metadata__l_namespaceUri);
         namespaces_default_role_permissions__get_DefaultRolePermissions(address_space_namespaces_metadata__l_namespaceUri,
            &address_space_namespaces_metadata__l_val_DefaultRolePermissions);
         constants__get_cast_t_NamespaceIdx(address_space_namespaces_metadata__l_namespaceUriIdx,
            &address_space_namespaces_metadata__l_namespaceUriIdxNS);
         default_role_permissions_array_bs__add_DefaultRolePermissions_at_idx(address_space_namespaces_metadata__l_namespaceUri,
            address_space_namespaces_metadata__l_namespaceUriIdxNS,
            address_space_namespaces_metadata__l_val_DefaultRolePermissions);
      }
   }
}

void address_space_namespaces_metadata__may_initialize_default_role_permissions(void) {
   {
      t_bool address_space_namespaces_metadata__l_nid_valid;
      constants__t_Node_i address_space_namespaces_metadata__l_Server_NamespaceArray_node;
      constants_statuscodes_bs__t_StatusCode_i address_space_namespaces_metadata__l_sc;
      constants__t_Variant_i address_space_namespaces_metadata__l_val;
      constants__t_RawStatusCode address_space_namespaces_metadata__l_val_sc;
      constants__t_Timestamp address_space_namespaces_metadata__l_val_ts_src;
      t_bool address_space_namespaces_metadata__l_b_is_initialized;
      
      default_role_permissions_array_bs__is_default_role_permissions_initialized(&address_space_namespaces_metadata__l_b_is_initialized);
      if (address_space_namespaces_metadata__l_b_is_initialized == false) {
         address_space_bs__readall_AddressSpace_Node(constants__c_Server_NamespaceArray_NodeId,
            &address_space_namespaces_metadata__l_nid_valid,
            &address_space_namespaces_metadata__l_Server_NamespaceArray_node);
         if (address_space_namespaces_metadata__l_nid_valid == true) {
            address_space_bs__read_AddressSpace_Raw_Node_Value_value(address_space_namespaces_metadata__l_Server_NamespaceArray_node,
               constants__c_Server_NamespaceArray_NodeId,
               constants__e_aid_Value,
               &address_space_namespaces_metadata__l_sc,
               &address_space_namespaces_metadata__l_val,
               &address_space_namespaces_metadata__l_val_sc,
               &address_space_namespaces_metadata__l_val_ts_src);
            if (address_space_namespaces_metadata__l_sc == constants_statuscodes_bs__e_sc_ok) {
               address_space_namespaces_metadata__l_fill_default_role_permissions(address_space_namespaces_metadata__l_val);
               address_space_bs__read_AddressSpace_free_variant(address_space_namespaces_metadata__l_val);
            }
         }
      }
   }
}

