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

 File Name            : service_write_decode_bs.h

 Date                 : 04/08/2022 14:53:46

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_write_decode_bs_h
#define _service_write_decode_bs_h

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
extern void service_write_decode_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_decode_bs__decode_write_request(
   const constants__t_msg_i service_write_decode_bs__write_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_write_decode_bs__StatusCode_service);
extern void service_write_decode_bs__free_write_request(void);
extern void service_write_decode_bs__get_nb_WriteValue(
   t_entier4 * const service_write_decode_bs__nb_req);
extern void service_write_decode_bs__getall_WriteValue(
   const constants__t_WriteValue_i service_write_decode_bs__wvi,
   t_bool * const service_write_decode_bs__isvalid,
   constants_statuscodes_bs__t_StatusCode_i * const service_write_decode_bs__status,
   constants__t_NodeId_i * const service_write_decode_bs__nid,
   constants__t_AttributeId_i * const service_write_decode_bs__aid,
   constants__t_DataValue_i * const service_write_decode_bs__dataValue,
   constants__t_IndexRange_i * const service_write_decode_bs__index_range);
extern void service_write_decode_bs__getall_WriteValuePointer(
   const constants__t_WriteValue_i service_write_decode_bs__wvi,
   constants__t_WriteValuePointer_i * const service_write_decode_bs__wvPointer);

#endif
