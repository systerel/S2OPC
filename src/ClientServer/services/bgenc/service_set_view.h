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

 File Name            : service_set_view.h

 Date                 : 04/08/2022 14:53:14

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_set_view_h
#define _service_set_view_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_browse_bs.h"
#include "msg_browse_next_bs.h"
#include "service_browse_it.h"
#include "translate_browse_path.h"
#include "translate_browse_path_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_set_view__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_set_view__treat_browse_next_request_BrowseContinuationPoint_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_BrowseValue_i service_set_view__p_cpi);
extern void service_set_view__treat_browse_next_request_BrowseContinuationPoints(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const t_bool service_set_view__p_releaseCP,
   const t_entier4 service_set_view__p_nbCP,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service);
extern void service_set_view__treat_browse_next_request_ReleaseContinuationPoint_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_BrowseValue_i service_set_view__p_cpi);
extern void service_set_view__treat_browse_request_BrowseValue_1(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_NodeId_i service_set_view__p_nid_view,
   const t_entier4 service_set_view__p_nb_target_max,
   const constants__t_BrowseValue_i service_set_view__p_bvi,
   const t_bool service_set_view__p_autoReleaseCP);
extern void service_set_view__treat_browse_request_BrowseValues(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   const constants__t_NodeId_i service_set_view__p_nid_view,
   const t_entier4 service_set_view__p_nb_target_max,
   const t_entier4 service_set_view__p_nb_browse_value,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_set_view__service_set_view_UNINITIALISATION(void);
extern void service_set_view__service_set_view_service_node_management_used(void);
extern void service_set_view__service_set_view_set_session_closed(
   const constants__t_session_i service_set_view__p_session);
extern void service_set_view__treat_browse_next_request(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service);
extern void service_set_view__treat_browse_request(
   const constants__t_session_i service_set_view__p_session,
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service);
extern void service_set_view__treat_translate_browse_paths_request(
   const constants__t_msg_i service_set_view__p_req_msg,
   const constants__t_msg_i service_set_view__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_view__StatusCode_service);

#endif
