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

 File Name            : browse_treatment_target_it.c

 Date                 : 04/08/2022 14:53:03

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_target_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_Node_i browse_treatment_target_it__Node;
t_entier4 browse_treatment_target_it__RefIndex;
t_entier4 browse_treatment_target_it__RefIndexEnd;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_target_it__INITIALISATION(void) {
   browse_treatment_target_it__Node = constants__c_Node_indet;
   browse_treatment_target_it__RefIndex = 0;
   browse_treatment_target_it__RefIndexEnd = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_target_it__init_iter_reference(
   const constants__t_Node_i browse_treatment_target_it__p_node,
   const t_entier4 browse_treatment_target_it__p_startIndex,
   t_bool * const browse_treatment_target_it__p_continue) {
   browse_treatment_target_it__Node = browse_treatment_target_it__p_node;
   browse_treatment_target_it__RefIndex = browse_treatment_target_it__p_startIndex;
   address_space_itf__get_Node_RefIndexEnd(browse_treatment_target_it__p_node,
      &browse_treatment_target_it__RefIndexEnd);
   *browse_treatment_target_it__p_continue = (browse_treatment_target_it__RefIndexEnd >= browse_treatment_target_it__RefIndex);
}

void browse_treatment_target_it__continue_iter_reference(
   t_bool * const browse_treatment_target_it__p_continue,
   constants__t_Reference_i * const browse_treatment_target_it__p_ref,
   t_entier4 * const browse_treatment_target_it__p_nextRefIndex) {
   address_space_itf__get_RefIndex_Reference(browse_treatment_target_it__Node,
      browse_treatment_target_it__RefIndex,
      browse_treatment_target_it__p_ref);
   browse_treatment_target_it__RefIndex = browse_treatment_target_it__RefIndex +
      1;
   *browse_treatment_target_it__p_continue = (browse_treatment_target_it__RefIndex <= browse_treatment_target_it__RefIndexEnd);
   *browse_treatment_target_it__p_nextRefIndex = browse_treatment_target_it__RefIndex;
}

