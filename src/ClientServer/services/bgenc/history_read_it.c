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

 File Name            : history_read_it.c

 Date                 : 31/07/2025 12:10:22

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "history_read_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 history_read_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void history_read_it__INITIALISATION(void) {
   history_read_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void history_read_it__init_iter_hist_read_request(
   const t_entier4 history_read_it__p_nb_req,
   t_bool * const history_read_it__p_continue) {
   history_read_it__rreqs_i = history_read_it__p_nb_req;
   *history_read_it__p_continue = (0 < history_read_it__p_nb_req);
}

void history_read_it__continue_iter_hist_read_request(
   t_bool * const history_read_it__p_continue,
   t_entier4 * const history_read_it__p_index) {
   *history_read_it__p_index = history_read_it__rreqs_i;
   history_read_it__rreqs_i = history_read_it__rreqs_i -
      1;
   *history_read_it__p_continue = (0 < history_read_it__rreqs_i);
}

