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

 File Name            : publish_request_queue_bs.h

 Date                 : 04/08/2022 14:53:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _publish_request_queue_bs_h
#define _publish_request_queue_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void publish_request_queue_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void publish_request_queue_bs__allocate_new_publish_queue(
   t_bool * const publish_request_queue_bs__bres,
   constants__t_publishReqQueue_i * const publish_request_queue_bs__queue);
extern void publish_request_queue_bs__append_publish_request_to_queue(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   const constants__t_session_i publish_request_queue_bs__p_session,
   const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
   const constants__t_server_request_handle_i publish_request_queue_bs__p_req_handle,
   const constants__t_request_context_i publish_request_queue_bs__p_req_ctx,
   const constants__t_msg_i publish_request_queue_bs__p_resp_msg,
   t_bool * const publish_request_queue_bs__bres);
extern void publish_request_queue_bs__clear_and_deallocate_publish_queue(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue);
extern void publish_request_queue_bs__clear_publish_queue(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue);
extern void publish_request_queue_bs__continue_pop_head_iter_publish_request(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   t_bool * const publish_request_queue_bs__p_continue,
   constants__t_session_i * const publish_request_queue_bs__p_session,
   constants__t_timeref_i * const publish_request_queue_bs__p_req_exp_time,
   constants__t_server_request_handle_i * const publish_request_queue_bs__p_req_handle,
   constants__t_request_context_i * const publish_request_queue_bs__p_req_ctx,
   constants__t_msg_i * const publish_request_queue_bs__p_resp_msg);
extern void publish_request_queue_bs__discard_oldest_publish_request(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   constants__t_session_i * const publish_request_queue_bs__old_session,
   constants__t_msg_i * const publish_request_queue_bs__old_resp_msg,
   constants__t_server_request_handle_i * const publish_request_queue_bs__old_req_handle,
   constants__t_request_context_i * const publish_request_queue_bs__old_req_ctx);
extern void publish_request_queue_bs__get_nb_publish_requests(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   t_entier4 * const publish_request_queue_bs__nb_pub_reqs);
extern void publish_request_queue_bs__init_iter_publish_request(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   t_bool * const publish_request_queue_bs__continue);
extern void publish_request_queue_bs__is_request_expired(
   const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
   t_bool * const publish_request_queue_bs__bres);
extern void publish_request_queue_bs__pop_valid_publish_request_queue(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   constants__t_session_i * const publish_request_queue_bs__p_session,
   constants__t_timeref_i * const publish_request_queue_bs__p_req_exp_time,
   constants__t_server_request_handle_i * const publish_request_queue_bs__p_req_handle,
   constants__t_request_context_i * const publish_request_queue_bs__p_req_ctx,
   constants__t_msg_i * const publish_request_queue_bs__p_resp_msg);
extern void publish_request_queue_bs__prepend_publish_request_to_queue(
   const constants__t_publishReqQueue_i publish_request_queue_bs__p_queue,
   const constants__t_session_i publish_request_queue_bs__p_session,
   const constants__t_timeref_i publish_request_queue_bs__p_req_exp_time,
   const constants__t_server_request_handle_i publish_request_queue_bs__p_req_handle,
   const constants__t_request_context_i publish_request_queue_bs__p_req_ctx,
   const constants__t_msg_i publish_request_queue_bs__p_resp_msg,
   t_bool * const publish_request_queue_bs__bres);

#endif
