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

 File Name            : msg_read_request.h

 Date                 : 04/08/2022 14:53:07

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_read_request_h
#define _msg_read_request_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_request_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request__check_ReadRequest(
   const constants__t_msg_i msg_read_request__p_msg,
   t_bool * const msg_read_request__p_read_ok,
   t_entier4 * const msg_read_request__p_nb_ReadValue,
   constants__t_TimestampsToReturn_i * const msg_read_request__p_tsToReturn,
   constants_statuscodes_bs__t_StatusCode_i * const msg_read_request__p_statusCode);
extern void msg_read_request__get_nb_ReadValue(
   t_entier4 * const msg_read_request__p_nb_ReadValue);
extern void msg_read_request__getall_ReadValue_NodeId_AttributeId(
   const constants__t_msg_i msg_read_request__p_msg,
   const constants__t_ReadValue_i msg_read_request__p_rvi,
   constants_statuscodes_bs__t_StatusCode_i * const msg_read_request__p_sc,
   constants__t_NodeId_i * const msg_read_request__p_nid,
   constants__t_AttributeId_i * const msg_read_request__p_aid,
   constants__t_IndexRange_i * const msg_read_request__p_index_range);

#endif
