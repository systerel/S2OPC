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

 File Name            : namespaces_refs_it.c

 Date                 : 14/08/2024 16:16:28

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "namespaces_refs_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_Node_i namespaces_refs_it__Node;
t_entier4 namespaces_refs_it__RefIndex;
t_entier4 namespaces_refs_it__RefIndexEnd;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespaces_refs_it__INITIALISATION(void) {
   namespaces_refs_it__Node = constants__c_Node_indet;
   namespaces_refs_it__RefIndex = 0;
   namespaces_refs_it__RefIndexEnd = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespaces_refs_it__init_iter_namespaces_refs(
   const constants__t_Node_i namespaces_refs_it__p_node,
   t_bool * const namespaces_refs_it__p_continue) {
   namespaces_refs_it__Node = namespaces_refs_it__p_node;
   namespaces_refs_it__RefIndex = 1;
   address_space_bs__get_Node_RefIndexEnd(namespaces_refs_it__p_node,
      &namespaces_refs_it__RefIndexEnd);
   *namespaces_refs_it__p_continue = (namespaces_refs_it__RefIndexEnd >= namespaces_refs_it__RefIndex);
}

void namespaces_refs_it__continue_iter_namespaces_refs(
   t_bool * const namespaces_refs_it__p_continue,
   constants__t_Reference_i * const namespaces_refs_it__p_ref) {
   address_space_bs__get_RefIndex_Reference(namespaces_refs_it__Node,
      namespaces_refs_it__RefIndex,
      namespaces_refs_it__p_ref);
   namespaces_refs_it__RefIndex = namespaces_refs_it__RefIndex +
      1;
   *namespaces_refs_it__p_continue = (namespaces_refs_it__RefIndex <= namespaces_refs_it__RefIndexEnd);
}

