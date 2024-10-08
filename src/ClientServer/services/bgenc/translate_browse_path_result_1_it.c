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

 File Name            : translate_browse_path_result_1_it.c

 Date                 : 24/07/2023 14:29:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_result_1_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 translate_browse_path_result_1_it__currentBrowsePathIdx_i;
t_entier4 translate_browse_path_result_1_it__nb_browsePathIdx_max_refs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_result_1_it__INITIALISATION(void) {
   translate_browse_path_result_1_it__currentBrowsePathIdx_i = 0;
   translate_browse_path_result_1_it__nb_browsePathIdx_max_refs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_result_1_it__init_iter_browsePathIdx(
   const t_entier4 translate_browse_path_result_1_it__p_nb_browsePathIdx_max_refs,
   t_bool * const translate_browse_path_result_1_it__p_continue) {
   translate_browse_path_result_1_it__nb_browsePathIdx_max_refs_i = translate_browse_path_result_1_it__p_nb_browsePathIdx_max_refs;
   translate_browse_path_result_1_it__currentBrowsePathIdx_i = 0;
   *translate_browse_path_result_1_it__p_continue = (0 < translate_browse_path_result_1_it__p_nb_browsePathIdx_max_refs);
}

void translate_browse_path_result_1_it__continue_iter_browsePathIdx(
   t_bool * const translate_browse_path_result_1_it__p_continue,
   t_entier4 * const translate_browse_path_result_1_it__p_browsePathIdx) {
   translate_browse_path_result_1_it__currentBrowsePathIdx_i = translate_browse_path_result_1_it__currentBrowsePathIdx_i +
      1;
   *translate_browse_path_result_1_it__p_browsePathIdx = translate_browse_path_result_1_it__currentBrowsePathIdx_i;
   *translate_browse_path_result_1_it__p_continue = (translate_browse_path_result_1_it__currentBrowsePathIdx_i < translate_browse_path_result_1_it__nb_browsePathIdx_max_refs_i);
}

