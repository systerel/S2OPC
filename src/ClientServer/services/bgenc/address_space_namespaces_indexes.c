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

 File Name            : address_space_namespaces_indexes.c

 Date                 : 29/07/2025 17:01:20

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_namespaces_indexes.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 address_space_namespaces_indexes__a_nsIdxMax_i;
t_bool address_space_namespaces_indexes__has_NamespaceIndexMax_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_namespaces_indexes__INITIALISATION(void) {
   address_space_namespaces_indexes__has_NamespaceIndexMax_i = false;
   address_space_namespaces_indexes__a_nsIdxMax_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_namespaces_indexes__may_initialize_max_namespace_idx(void) {
   {
      t_bool address_space_namespaces_indexes__l_nid_valid;
      constants__t_Node_i address_space_namespaces_indexes__l_Server_NamespaceArray_node;
      constants_statuscodes_bs__t_StatusCode_i address_space_namespaces_indexes__l_sc;
      constants__t_Variant_i address_space_namespaces_indexes__l_val;
      constants__t_RawStatusCode address_space_namespaces_indexes__l_val_sc;
      constants__t_Timestamp address_space_namespaces_indexes__l_val_ts_src;
      
      if (address_space_namespaces_indexes__has_NamespaceIndexMax_i == false) {
         address_space_bs__readall_AddressSpace_Node(constants__c_Server_NamespaceArray_NodeId,
            &address_space_namespaces_indexes__l_nid_valid,
            &address_space_namespaces_indexes__l_Server_NamespaceArray_node);
         if (address_space_namespaces_indexes__l_nid_valid == true) {
            address_space_bs__read_AddressSpace_Raw_Node_Value_value(address_space_namespaces_indexes__l_Server_NamespaceArray_node,
               constants__c_Server_NamespaceArray_NodeId,
               constants__e_aid_Value,
               &address_space_namespaces_indexes__l_sc,
               &address_space_namespaces_indexes__l_val,
               &address_space_namespaces_indexes__l_val_sc,
               &address_space_namespaces_indexes__l_val_ts_src);
            if (address_space_namespaces_indexes__l_sc == constants_statuscodes_bs__e_sc_ok) {
               address_space_namespaces_indexes_bs__read_variant_max_namespaceIndex(address_space_namespaces_indexes__l_val,
                  &address_space_namespaces_indexes__a_nsIdxMax_i);
               address_space_namespaces_indexes__has_NamespaceIndexMax_i = true;
               address_space_bs__read_AddressSpace_free_variant(address_space_namespaces_indexes__l_val);
            }
         }
      }
   }
}

void address_space_namespaces_indexes__has_NamespaceIndex_index(
   const constants__t_NamespaceIdx address_space_namespaces_indexes__p_idx,
   t_bool * const address_space_namespaces_indexes__bres) {
   {
      t_entier4 address_space_namespaces_indexes__l_natIdx;
      
      constants__get_reverse_cast_t_NamespaceIdx(address_space_namespaces_indexes__p_idx,
         &address_space_namespaces_indexes__l_natIdx);
      *address_space_namespaces_indexes__bres = (address_space_namespaces_indexes__l_natIdx <= address_space_namespaces_indexes__a_nsIdxMax_i);
   }
}

