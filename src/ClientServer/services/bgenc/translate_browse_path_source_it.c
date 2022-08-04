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

 File Name            : translate_browse_path_source_it.c

 Date                 : 04/08/2022 14:53:25

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_source_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 translate_browse_path_source_it__currentBrowsePathSourceIdx_i;
t_entier4 translate_browse_path_source_it__nb_browsePathSourceIdx_max_refs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_source_it__INITIALISATION(void) {
   translate_browse_path_source_it__currentBrowsePathSourceIdx_i = 0;
   translate_browse_path_source_it__nb_browsePathSourceIdx_max_refs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_source_it__init_iter_browsePathSourceIdx(
   const t_entier4 translate_browse_path_source_it__p_nb_browsePathSourceIdx_max_refs,
   t_bool * const translate_browse_path_source_it__p_continue) {
   translate_browse_path_source_it__nb_browsePathSourceIdx_max_refs_i = translate_browse_path_source_it__p_nb_browsePathSourceIdx_max_refs;
   translate_browse_path_source_it__currentBrowsePathSourceIdx_i = 0;
   *translate_browse_path_source_it__p_continue = (0 < translate_browse_path_source_it__p_nb_browsePathSourceIdx_max_refs);
}

void translate_browse_path_source_it__continue_iter_browsePathSourceIdx(
   t_bool * const translate_browse_path_source_it__p_continue,
   t_entier4 * const translate_browse_path_source_it__p_browsePathSourceIdx) {
   translate_browse_path_source_it__currentBrowsePathSourceIdx_i = translate_browse_path_source_it__currentBrowsePathSourceIdx_i +
      1;
   *translate_browse_path_source_it__p_browsePathSourceIdx = translate_browse_path_source_it__currentBrowsePathSourceIdx_i;
   *translate_browse_path_source_it__p_continue = (translate_browse_path_source_it__currentBrowsePathSourceIdx_i < translate_browse_path_source_it__nb_browsePathSourceIdx_max_refs_i);
}

