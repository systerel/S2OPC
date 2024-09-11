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

 File Name            : roleset_references_it.c

 Date                 : 14/08/2024 11:57:54

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "roleset_references_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_Node_i roleset_references_it__Node;
t_entier4 roleset_references_it__RefIndex;
t_entier4 roleset_references_it__RefIndexEnd;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void roleset_references_it__INITIALISATION(void) {
   roleset_references_it__Node = constants__c_Node_indet;
   roleset_references_it__RefIndex = 0;
   roleset_references_it__RefIndexEnd = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void roleset_references_it__init_iter_roleset_references(
   const constants__t_Node_i roleset_references_it__p_node,
   t_bool * const roleset_references_it__p_continue) {
   roleset_references_it__Node = roleset_references_it__p_node;
   roleset_references_it__RefIndex = 1;
   address_space_itf__get_Node_RefIndexEnd(roleset_references_it__p_node,
      &roleset_references_it__RefIndexEnd);
   *roleset_references_it__p_continue = (roleset_references_it__RefIndexEnd >= roleset_references_it__RefIndex);
}

void roleset_references_it__continue_iter_roleset_references(
   t_bool * const roleset_references_it__p_continue,
   constants__t_Reference_i * const roleset_references_it__p_ref) {
   address_space_itf__get_RefIndex_Reference(roleset_references_it__Node,
      roleset_references_it__RefIndex,
      roleset_references_it__p_ref);
   roleset_references_it__RefIndex = roleset_references_it__RefIndex +
      1;
   *roleset_references_it__p_continue = (roleset_references_it__RefIndex <= roleset_references_it__RefIndexEnd);
}

