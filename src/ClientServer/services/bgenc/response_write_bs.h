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

 File Name            : response_write_bs.h

 Date                 : 04/08/2022 14:53:44

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _response_write_bs_h
#define _response_write_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void response_write_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void response_write_bs__alloc_write_request_responses_malloc(
   const t_entier4 response_write_bs__nb_req,
   t_bool * const response_write_bs__ResponseWrite_allocated);
extern void response_write_bs__reset_ResponseWrite(void);
extern void response_write_bs__set_ResponseWrite_StatusCode(
   const constants__t_WriteValue_i response_write_bs__wvi,
   const constants_statuscodes_bs__t_StatusCode_i response_write_bs__sc);
extern void response_write_bs__write_WriteResponse_msg_out(
   const constants__t_msg_i response_write_bs__msg_out);

#endif
