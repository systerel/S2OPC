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

 File Name            : msg_history_read_request.h

 Date                 : 22/08/2025 08:55:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_history_read_request_h
#define _msg_history_read_request_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_history_read_request_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_history_read_request__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define msg_history_read_request__getall_msg_hist_read_req_singleValueId msg_history_read_request_bs__getall_msg_hist_read_req_singleValueId

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_history_read_request__check_history_read_request(
   const constants__t_msg_i msg_history_read_request__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_history_read_request__p_StatusCode,
   constants__t_readRawModifiedDetails_i * const msg_history_read_request__p_readRawDetails,
   t_bool * const msg_history_read_request__p_TsSrcRequired,
   t_bool * const msg_history_read_request__p_TsSrvRequired,
   t_bool * const msg_history_read_request__p_ContinuationPoint,
   t_entier4 * const msg_history_read_request__p_nb_nodes_to_read);

#endif
