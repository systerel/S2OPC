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

 File Name            : msg_call_method_bs.h

 Date                 : 04/08/2022 14:53:35

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_call_method_bs_h
#define _msg_call_method_bs_h

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
extern void msg_call_method_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_call_method_bs__alloc_CallMethod_Res_InputArgumentResult(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod,
   const t_entier4 msg_call_method_bs__nb,
   constants_statuscodes_bs__t_StatusCode_i * const msg_call_method_bs__statusCode);
extern void msg_call_method_bs__alloc_CallMethod_Res_OutputArgument(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod,
   const t_entier4 msg_call_method_bs__nb,
   constants_statuscodes_bs__t_StatusCode_i * const msg_call_method_bs__statusCode);
extern void msg_call_method_bs__alloc_CallMethod_Result(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const t_entier4 msg_call_method_bs__nb,
   constants_statuscodes_bs__t_StatusCode_i * const msg_call_method_bs__statusCode);
extern void msg_call_method_bs__free_CallMethod_Res_InputArgument(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod);
extern void msg_call_method_bs__free_CallMethod_Res_OutputArgument(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod);
extern void msg_call_method_bs__read_CallMethod_InputArguments(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
   const t_entier4 msg_call_method_bs__p_index_arg,
   constants__t_Variant_i * const msg_call_method_bs__p_arg);
extern void msg_call_method_bs__read_CallMethod_MethodId(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
   constants__t_NodeId_i * const msg_call_method_bs__p_methodid);
extern void msg_call_method_bs__read_CallMethod_Nb_InputArguments(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
   t_entier4 * const msg_call_method_bs__p_nb);
extern void msg_call_method_bs__read_CallMethod_Objectid(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
   constants__t_NodeId_i * const msg_call_method_bs__p_objectid);
extern void msg_call_method_bs__read_call_method_request(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_call_method_bs__Status,
   t_entier4 * const msg_call_method_bs__p_nb);
extern void msg_call_method_bs__read_nb_CallMethods(
   const constants__t_msg_i msg_call_method_bs__p_req_msg,
   t_entier4 * const msg_call_method_bs__p_nb);
extern void msg_call_method_bs__write_CallMethod_Res_InputArgumentResult(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod,
   const t_entier4 msg_call_method_bs__index,
   const constants_statuscodes_bs__t_StatusCode_i msg_call_method_bs__statusCode);
extern void msg_call_method_bs__write_CallMethod_Res_OutputArgument(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod,
   const t_entier4 msg_call_method_bs__index,
   const constants__t_Variant_i msg_call_method_bs__value);
extern void msg_call_method_bs__write_CallMethod_Res_Status(
   const constants__t_msg_i msg_call_method_bs__p_res_msg,
   const constants__t_CallMethod_i msg_call_method_bs__callMethod,
   const constants__t_RawStatusCode msg_call_method_bs__rawStatusCode);

#endif
