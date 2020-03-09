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

 File Name            : call_method_bs.h

 Date                 : 23/10/2019 09:09:27

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _call_method_bs_h
#define _call_method_bs_h

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
extern void call_method_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void call_method_bs__exec_callMethod(
   const constants__t_msg_i call_method_bs__p_req_msg,
   const constants__t_CallMethod_i call_method_bs__p_callMethod,
   const constants__t_endpoint_config_idx_i call_method_bs__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_bs__statusCode);
extern void call_method_bs__free_exec_result(void);
extern void call_method_bs__read_exec_result(
   const t_entier4 call_method_bs__index,
   constants__t_Variant_i * const call_method_bs__value);
extern void call_method_bs__read_nb_exec_result(
   t_entier4 * const call_method_bs__nb);

#endif
