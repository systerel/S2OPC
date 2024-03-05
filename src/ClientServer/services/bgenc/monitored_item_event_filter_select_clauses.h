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

 File Name            : monitored_item_event_filter_select_clauses.h

 Date                 : 05/04/2024 13:39:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_event_filter_select_clauses_h
#define _monitored_item_event_filter_select_clauses_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "monitored_item_event_filter_select_clauses_bs.h"
#include "monitored_item_event_filter_select_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "monitored_item_event_filter_treatment_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void monitored_item_event_filter_select_clauses__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_event_filter_select_clauses__apply_select_clauses_and_build_event_field_list(
   const constants__t_client_handle_i monitored_item_event_filter_select_clauses__p_clientHandle,
   const constants__t_LocaleIds_i monitored_item_event_filter_select_clauses__p_localeIds,
   const constants__t_TimestampsToReturn_i monitored_item_event_filter_select_clauses__p_timestampToReturn,
   const t_bool monitored_item_event_filter_select_clauses__p_userAccessGranted,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_select_clauses__p_filterCtx,
   const t_entier4 monitored_item_event_filter_select_clauses__p_nbSelectClauses,
   const constants__t_Event_i monitored_item_event_filter_select_clauses__p_event,
   t_bool * const monitored_item_event_filter_select_clauses__notifTriggered,
   constants__t_eventFieldList_i * const monitored_item_event_filter_select_clauses__eventNotif);
extern void monitored_item_event_filter_select_clauses__check_select_clauses_and_fill_ctx_and_result(
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_select_clauses__p_filterCtx,
   const constants__t_filterResult_i monitored_item_event_filter_select_clauses__p_filterResult,
   const t_entier4 monitored_item_event_filter_select_clauses__p_nbSelectClauses,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_select_clauses__scSelectClauses,
   t_bool * const monitored_item_event_filter_select_clauses__hasInvalidClause);

#endif
