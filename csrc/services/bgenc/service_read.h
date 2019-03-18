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

 File Name            : service_read.h

 Date                 : 05/02/2019 12:57:19

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_read_h
#define _service_read_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_request.h"
#include "msg_read_response_bs.h"
#include "service_read_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_read__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_read__alloc_read_response msg_read_response_bs__alloc_read_response
#define service_read__check_ReadRequest msg_read_request__check_ReadRequest

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_read__fill_read_response_1(
   const constants__t_TimestampsToReturn_i service_read__p_TimestampsToReturn,
   const constants__t_user_i service_read__p_user,
   const constants__t_msg_i service_read__p_resp_msg,
   const constants_statuscodes_bs__t_StatusCode_i service_read__p_sc,
   const constants__t_NodeId_i service_read__p_nid,
   const constants__t_AttributeId_i service_read__p_aid,
   const constants__t_IndexRange_i service_read__p_index_range,
   const constants__t_ReadValue_i service_read__p_rvi);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read__fill_read_response(
   const constants__t_TimestampsToReturn_i service_read__p_TimestampsToReturn,
   const constants__t_user_i service_read__p_user,
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg);

#endif
