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

 File Name            : namespace_array_it.c

 Date                 : 04/11/2024 10:51:48

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "namespace_array_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 namespace_array_it__currentnamespaceUriIdx_i;
t_entier4 namespace_array_it__nb_namespaceUris_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespace_array_it__INITIALISATION(void) {
   namespace_array_it__currentnamespaceUriIdx_i = 0;
   namespace_array_it__nb_namespaceUris_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespace_array_it__init_iter_namespaceUris(
   const t_entier4 namespace_array_it__p_nb_namespaceUris,
   t_bool * const namespace_array_it__p_continue) {
   namespace_array_it__nb_namespaceUris_i = namespace_array_it__p_nb_namespaceUris;
   namespace_array_it__currentnamespaceUriIdx_i = 0;
   *namespace_array_it__p_continue = ((0 < namespace_array_it__p_nb_namespaceUris) &&
      (namespace_array_it__p_nb_namespaceUris < constants__k_n_NamespaceIndex_max));
}

void namespace_array_it__continue_iter_namespaceUris(
   t_bool * const namespace_array_it__p_continue,
   t_entier4 * const namespace_array_it__p_namespaceUriIdx) {
   namespace_array_it__currentnamespaceUriIdx_i = namespace_array_it__currentnamespaceUriIdx_i +
      1;
   *namespace_array_it__p_namespaceUriIdx = namespace_array_it__currentnamespaceUriIdx_i;
   *namespace_array_it__p_continue = (namespace_array_it__currentnamespaceUriIdx_i < namespace_array_it__nb_namespaceUris_i);
}

