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

 File Name            : msg_subscription_publish_ack_bs.h

 Date                 : 04/08/2022 14:53:40

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_publish_ack_bs_h
#define _msg_subscription_publish_ack_bs_h

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
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_subscription_publish_ack_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_publish_ack_bs__allocate_subscription_ack_results(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
   const t_entier4 msg_subscription_publish_ack_bs__p_nb_acks,
   t_bool * const msg_subscription_publish_ack_bs__bres);
extern void msg_subscription_publish_ack_bs__allocate_subscription_available_seq_nums(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
   const t_entier4 msg_subscription_publish_ack_bs__p_nb_seq_num,
   t_bool * const msg_subscription_publish_ack_bs__bres);
extern void msg_subscription_publish_ack_bs__get_msg_header_expiration_time(
   const constants__t_msg_header_i msg_subscription_publish_ack_bs__p_req_header,
   constants__t_timeref_i * const msg_subscription_publish_ack_bs__req_expiration_time);
extern void msg_subscription_publish_ack_bs__get_msg_publish_request_ack_params(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
   t_entier4 * const msg_subscription_publish_ack_bs__p_nb_acks);
extern void msg_subscription_publish_ack_bs__getall_msg_publish_request_ack(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
   const t_entier4 msg_subscription_publish_ack_bs__p_index,
   constants__t_subscription_i * const msg_subscription_publish_ack_bs__p_sub,
   constants__t_sub_seq_num_i * const msg_subscription_publish_ack_bs__p_seq_num);
extern void msg_subscription_publish_ack_bs__getall_msg_republish_request(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
   constants__t_subscription_i * const msg_subscription_publish_ack_bs__l_sub,
   constants__t_sub_seq_num_i * const msg_subscription_publish_ack_bs__l_seq_num);
extern void msg_subscription_publish_ack_bs__setall_msg_publish_resp_ack_result(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
   const t_entier4 msg_subscription_publish_ack_bs__p_index,
   const constants_statuscodes_bs__t_StatusCode_i msg_subscription_publish_ack_bs__p_sc);
extern void msg_subscription_publish_ack_bs__setall_msg_publish_resp_available_seq_num(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
   const t_entier4 msg_subscription_publish_ack_bs__p_index,
   const constants__t_sub_seq_num_i msg_subscription_publish_ack_bs__p_seq_num);
extern void msg_subscription_publish_ack_bs__setall_msg_republish_response(
   const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
   const constants__t_notif_msg_i msg_subscription_publish_ack_bs__p_notifMsg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_publish_ack_bs__sc);

#endif
