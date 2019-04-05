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

 File Name            : browse_treatment_1.c

 Date                 : 05/04/2019 14:46:17

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_1__getall_SourceNode_NbRef(
   const constants__t_NodeId_i browse_treatment_1__p_src_nodeid,
   t_bool * const browse_treatment_1__p_isvalid,
   t_entier4 * const browse_treatment_1__p_nb_ref,
   constants__t_Node_i * const browse_treatment_1__p_src_node) {
   address_space__readall_AddressSpace_Node(browse_treatment_1__p_src_nodeid,
      browse_treatment_1__p_isvalid,
      browse_treatment_1__p_src_node);
   if (*browse_treatment_1__p_isvalid == true) {
      address_space__get_Node_RefIndexEnd(*browse_treatment_1__p_src_node,
         browse_treatment_1__p_nb_ref);
   }
   else {
      *browse_treatment_1__p_nb_ref = 0;
   }
}

void browse_treatment_1__Is_RefTypes_Compatible(
   const t_bool browse_treatment_1__p_is_ref_type1,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type1,
   const t_bool browse_treatment_1__p_inc_subtypes,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type2,
   t_bool * const browse_treatment_1__p_ref_types_compat) {
   {
      t_bool browse_treatment_1__l_node_ids_equal;
      
      if (browse_treatment_1__p_is_ref_type1 == true) {
         address_space__is_NodeId_equal(browse_treatment_1__p_ref_type1,
            browse_treatment_1__p_ref_type2,
            &browse_treatment_1__l_node_ids_equal);
         if (browse_treatment_1__l_node_ids_equal == true) {
            *browse_treatment_1__p_ref_types_compat = true;
         }
         else if (browse_treatment_1__p_inc_subtypes == true) {
            address_space__is_transitive_subtype(browse_treatment_1__p_ref_type2,
               browse_treatment_1__p_ref_type1,
               browse_treatment_1__p_ref_types_compat);
         }
         else {
            *browse_treatment_1__p_ref_types_compat = false;
         }
      }
      else {
         *browse_treatment_1__p_ref_types_compat = true;
      }
   }
}

