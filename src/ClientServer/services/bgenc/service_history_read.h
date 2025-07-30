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

 File Name            : service_history_read.h

 Date                 : 29/08/2025 13:21:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_history_read_h
#define _service_history_read_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "history_read_it.h"
#include "history_read_treatment_bs.h"
#include "msg_history_read_request.h"
#include "msg_history_read_response_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_history_read__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_history_read__l_fill_response_hist_read(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_user_i service_history_read__p_user,
   const constants__t_sessionRoles_i service_history_read__p_roles,
   const t_entier4 service_history_read__p_nb_nodes,
   const constants__t_readRawModifiedDetails_i service_history_read__p_readRawDetails,
   const t_bool service_history_read__p_TsSrcRequired,
   const t_bool service_history_read__p_TsSrvRequired,
   const t_bool service_history_read__p_ContinuationPoint);
extern void service_history_read__l_fill_response_hist_read_1(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_Timestamp service_history_read__p_currentTs,
   const t_entier4 service_history_read__p_index,
   const constants__t_user_i service_history_read__p_user,
   const constants__t_sessionRoles_i service_history_read__p_roles,
   const constants__t_readRawModifiedDetails_i service_history_read__p_readRawDetails,
   const t_bool service_history_read__p_TsSrvRequired,
   const t_bool service_history_read__p_TsSrcRequired,
   const t_bool service_history_read__p_ContinuationPoint);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_history_read__treat_history_read_request(
   const constants__t_msg_i service_history_read__p_req_msg,
   const constants__t_msg_i service_history_read__p_resp_msg,
   const constants__t_user_i service_history_read__p_user,
   constants_statuscodes_bs__t_StatusCode_i * const service_history_read__p_StatusCode);

#endif
