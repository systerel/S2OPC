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

 File Name            : request_handle_bs.h

 Date                 : 04/08/2022 14:53:44

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _request_handle_bs_h
#define _request_handle_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void request_handle_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void request_handle_bs__client_fresh_req_handle(
   const constants__t_msg_type_i request_handle_bs__req_typ,
   const constants__t_msg_type_i request_handle_bs__resp_typ,
   const t_bool request_handle_bs__is_applicative,
   const constants__t_application_context_i request_handle_bs__app_context,
   constants__t_client_request_handle_i * const request_handle_bs__request_handle);
extern void request_handle_bs__client_remove_req_handle(
   const constants__t_client_request_handle_i request_handle_bs__req_handle);
extern void request_handle_bs__client_req_handle_to_request_id(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   constants__t_request_context_i * const request_handle_bs__request_id);
extern void request_handle_bs__client_request_id_to_req_handle(
   const constants__t_request_context_i request_handle_bs__request_id,
   constants__t_client_request_handle_i * const request_handle_bs__request_handle);
extern void request_handle_bs__client_validate_response_request_handle(
   const constants__t_channel_i request_handle_bs__channel,
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   const constants__t_msg_type_i request_handle_bs__resp_typ,
   t_bool * const request_handle_bs__ret);
extern void request_handle_bs__get_req_handle_app_context(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   t_bool * const request_handle_bs__is_applicative,
   constants__t_application_context_i * const request_handle_bs__app_context);
extern void request_handle_bs__get_req_handle_channel(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   constants__t_channel_i * const request_handle_bs__channel);
extern void request_handle_bs__get_req_handle_req_typ(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   constants__t_msg_type_i * const request_handle_bs__req_typ);
extern void request_handle_bs__get_req_handle_resp_typ(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   constants__t_msg_type_i * const request_handle_bs__resp_typ);
extern void request_handle_bs__is_valid_req_handle(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   t_bool * const request_handle_bs__ret);
extern void request_handle_bs__set_req_handle_channel(
   const constants__t_client_request_handle_i request_handle_bs__req_handle,
   const constants__t_channel_i request_handle_bs__channel);

#endif
