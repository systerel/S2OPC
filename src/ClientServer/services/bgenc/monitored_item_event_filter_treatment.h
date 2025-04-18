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

 File Name            : monitored_item_event_filter_treatment.h

 Date                 : 04/02/2025 14:21:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_event_filter_treatment_h
#define _monitored_item_event_filter_treatment_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "monitored_item_event_filter_select_clauses.h"
#include "monitored_item_event_filter_treatment_bs.h"
#include "monitored_item_event_filter_where_clause.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "monitored_item_pointer_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void monitored_item_event_filter_treatment__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define monitored_item_event_filter_treatment__delete_event_filter_context monitored_item_event_filter_treatment_bs__delete_event_filter_context

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_event_filter_treatment__check_monitored_item_event_filter_valid(
   const constants__t_endpoint_config_idx_i monitored_item_event_filter_treatment__p_endpoint_idx,
   const constants__t_AttributeId_i monitored_item_event_filter_treatment__p_aid,
   const constants__t_monitoringFilter_i monitored_item_event_filter_treatment__p_filter,
   const constants__t_Variant_i monitored_item_event_filter_treatment__p_value,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_event_filter_treatment__statusCode,
   constants__t_monitoringFilterCtx_i * const monitored_item_event_filter_treatment__filterCtx,
   constants__t_filterResult_i * const monitored_item_event_filter_treatment__filterResult);
extern void monitored_item_event_filter_treatment__get_event_user_authorization(
   const constants__t_Event_i monitored_item_event_filter_treatment__p_event,
   const constants__t_user_i monitored_item_event_filter_treatment__p_user,
   const constants__t_sessionRoles_i monitored_item_event_filter_treatment__p_roles,
   t_bool * const monitored_item_event_filter_treatment__p_bres);
extern void monitored_item_event_filter_treatment__server_subscription_get_notification_on_event(
   const constants__t_client_handle_i monitored_item_event_filter_treatment__p_clientHandle,
   const constants__t_LocaleIds_i monitored_item_event_filter_treatment__p_localeIds,
   const constants__t_TimestampsToReturn_i monitored_item_event_filter_treatment__p_timestampToReturn,
   const t_bool monitored_item_event_filter_treatment__p_userAccessGranted,
   const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment__p_filterCtx,
   const constants__t_Event_i monitored_item_event_filter_treatment__p_event,
   t_bool * const monitored_item_event_filter_treatment__notifTriggered,
   constants__t_eventFieldList_i * const monitored_item_event_filter_treatment__eventNotif);

#endif
