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

 File Name            : service_response_cb_bs.h

 Date                 : 04/08/2022 14:53:46

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_response_cb_bs_h
#define _service_response_cb_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

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
extern void service_response_cb_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_response_cb_bs__cli_service_response(
   const constants__t_session_i service_response_cb_bs__session,
   const constants__t_msg_i service_response_cb_bs__resp_msg,
   const constants__t_application_context_i service_response_cb_bs__app_context);
extern void service_response_cb_bs__cli_snd_failure(
   const constants__t_msg_type_i service_response_cb_bs__req_typ,
   const constants__t_application_context_i service_response_cb_bs__app_context,
   const constants_statuscodes_bs__t_StatusCode_i service_response_cb_bs__error_status);
extern void service_response_cb_bs__srv_service_response(
   const constants__t_endpoint_config_idx_i service_response_cb_bs__endpoint_config_idx,
   const constants__t_msg_i service_response_cb_bs__resp_msg,
   const constants__t_application_context_i service_response_cb_bs__app_context);
extern void service_response_cb_bs__srv_write_notification(
   const constants__t_WriteValuePointer_i service_response_cb_bs__write_value_pointer,
   const constants_statuscodes_bs__t_StatusCode_i service_response_cb_bs__write_status);

#endif
