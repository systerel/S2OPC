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

 File Name            : msg_history_read_response_bs.h

 Date                 : 11/08/2025 08:26:53

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_history_read_response_bs_h
#define _msg_history_read_response_bs_h

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
#include "msg_history_read_request.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_history_read_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_history_read_response_bs__alloc_msg_hist_read_resp_results(
   const t_entier4 msg_history_read_response_bs__p_nb_to_read,
   const constants__t_msg_i msg_history_read_response_bs__p_resp_msg,
   t_bool * const msg_history_read_response_bs__p_is_valid);
extern void msg_history_read_response_bs__set_msg_hist_read_response(
   const constants__t_msg_i msg_history_read_response_bs__p_resp_msg,
   const t_entier4 msg_history_read_response_bs__p_index,
   const constants_statuscodes_bs__t_StatusCode_i msg_history_read_response_bs__p_sc,
   const constants__t_Nonce_i msg_history_read_response_bs__p_contPoint,
   const t_entier4 msg_history_read_response_bs__p_nbDataValues,
   const constants__t_DataValue_array_i msg_history_read_response_bs__p_DataValues);

#endif
