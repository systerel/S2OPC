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

 File Name            : monitored_item_event_filter_select_it.c

 Date                 : 09/04/2024 12:58:30

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "monitored_item_event_filter_select_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i;
t_entier4 monitored_item_event_filter_select_it__nb_eventFilterSelectClauses_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_select_it__INITIALISATION(void) {
   monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i = 0;
   monitored_item_event_filter_select_it__nb_eventFilterSelectClauses_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_select_it__init_iter_eventFilterSelectClause(
   const t_entier4 monitored_item_event_filter_select_it__p_nb_eventFilterSelectClauses,
   t_bool * const monitored_item_event_filter_select_it__p_continue) {
   monitored_item_event_filter_select_it__nb_eventFilterSelectClauses_i = monitored_item_event_filter_select_it__p_nb_eventFilterSelectClauses;
   monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i = 0;
   *monitored_item_event_filter_select_it__p_continue = (0 < monitored_item_event_filter_select_it__p_nb_eventFilterSelectClauses);
}

void monitored_item_event_filter_select_it__continue_iter_eventFilterSelectClauseIdx(
   t_bool * const monitored_item_event_filter_select_it__p_continue,
   t_entier4 * const monitored_item_event_filter_select_it__p_eventFilterSelectClauseIdx) {
   monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i = monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i +
      1;
   *monitored_item_event_filter_select_it__p_eventFilterSelectClauseIdx = monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i;
   *monitored_item_event_filter_select_it__p_continue = (monitored_item_event_filter_select_it__currentEventFilterSelectClauseIdx_i < monitored_item_event_filter_select_it__nb_eventFilterSelectClauses_i);
}

