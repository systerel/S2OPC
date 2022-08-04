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

 File Name            : msg_read_request_bs.h

 Date                 : 04/08/2022 14:53:37

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_read_request_bs_h
#define _msg_read_request_bs_h

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
extern void msg_read_request_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request_bs__getall_req_ReadValue_AttributeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   constants_statuscodes_bs__t_StatusCode_i * const msg_read_request_bs__p_sc,
   constants__t_AttributeId_i * const msg_read_request_bs__aid);
extern void msg_read_request_bs__getall_req_ReadValue_DataEncoding(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   t_bool * const msg_read_request_bs__is_known_encoding,
   constants__t_QualifiedName_i * const msg_read_request_bs__data_encoding);
extern void msg_read_request_bs__getall_req_ReadValue_IndexRange(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   constants__t_IndexRange_i * const msg_read_request_bs__index_range);
extern void msg_read_request_bs__getall_req_ReadValue_NodeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   constants__t_NodeId_i * const msg_read_request_bs__nid);
extern void msg_read_request_bs__read_req_MaxAge(
   const constants__t_msg_i msg_read_request_bs__p_msg,
   t_bool * const msg_read_request_bs__p_maxAge_valid);
extern void msg_read_request_bs__read_req_TimestampsToReturn(
   const constants__t_msg_i msg_read_request_bs__p_msg,
   constants__t_TimestampsToReturn_i * const msg_read_request_bs__p_tsToReturn);
extern void msg_read_request_bs__read_req_nb_ReadValue(
   const constants__t_msg_i msg_read_request_bs__msg,
   t_entier4 * const msg_read_request_bs__nb);

#endif
