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

 File Name            : msg_history_read_request_bs.h

 Date                 : 21/10/2025 08:49:09

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_history_read_request_bs_h
#define _msg_history_read_request_bs_h

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

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_history_read_request_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_history_read_request_bs__get_msg_hist_read_req_TSToReturn(
   const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
   constants__t_TimestampsToReturn_i * const msg_history_read_request_bs__p_tsToReturn);
extern void msg_history_read_request_bs__get_msg_hist_read_req_nb_nodes_to_read(
   const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
   t_entier4 * const msg_history_read_request_bs__p_nb_nodes_to_read);
extern void msg_history_read_request_bs__get_msg_hist_read_req_release_CP(
   const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
   t_bool * const msg_history_read_request_bs__p_continuation_point);
extern void msg_history_read_request_bs__getall_msg_hist_read_req_read_details(
   const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_history_read_request_bs__p_sc,
   constants__t_readRawModifiedDetails_i * const msg_history_read_request_bs__p_hist_read_details);
extern void msg_history_read_request_bs__getall_msg_hist_read_req_singleValueId(
   const constants__t_msg_i msg_history_read_request_bs__p_req_msg,
   const t_entier4 msg_history_read_request_bs__p_index,
   constants_statuscodes_bs__t_StatusCode_i * const msg_history_read_request_bs__p_sc,
   constants__t_historyReadValueId_i * const msg_history_read_request_bs__p_singleValueId,
   constants__t_NodeId_i * const msg_history_read_request_bs__p_nodeId);

#endif
