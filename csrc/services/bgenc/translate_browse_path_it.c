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

 File Name            : translate_browse_path_it.c

 Date                 : 12/11/2019 08:49:15

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 translate_browse_path_it__current_browsePath_idx;
t_entier4 translate_browse_path_it__nb_browsePaths_to_iterate;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_it__INITIALISATION(void) {
   translate_browse_path_it__current_browsePath_idx = 0;
   translate_browse_path_it__nb_browsePaths_to_iterate = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_it__init_iter_browsePaths(
   t_bool * const translate_browse_path_it__p_continue) {
   translate_browse_path__read_nb_BrowsePaths(&translate_browse_path_it__nb_browsePaths_to_iterate);
   translate_browse_path_it__current_browsePath_idx = translate_browse_path_it__nb_browsePaths_to_iterate;
   *translate_browse_path_it__p_continue = (0 < translate_browse_path_it__current_browsePath_idx);
}

void translate_browse_path_it__continue_iter_browsePath(
   t_bool * const translate_browse_path_it__p_continue,
   constants__t_BrowsePath_i * const translate_browse_path_it__p_browsePath) {
   constants__get_cast_t_BrowsePath(translate_browse_path_it__current_browsePath_idx,
      translate_browse_path_it__p_browsePath);
   translate_browse_path_it__current_browsePath_idx = translate_browse_path_it__current_browsePath_idx -
      1;
   *translate_browse_path_it__p_continue = (0 < translate_browse_path_it__current_browsePath_idx);
}

