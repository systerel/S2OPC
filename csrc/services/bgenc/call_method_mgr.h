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

 File Name            : call_method_mgr.h

 Date                 : 28/10/2019 11:36:46

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _call_method_mgr_h
#define _call_method_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "call_method_bs.h"
#include "call_method_it.h"
#include "call_method_result_it.h"
#include "msg_call_method_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "session_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void call_method_mgr__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void call_method_mgr__check_exec_result(
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__check_method_call_inputs(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__check_nodeId(
   const constants__t_NodeId_i call_method_mgr__nodeid,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__statusCode,
   constants__t_Node_i * const call_method_mgr__node);
extern void call_method_mgr__copy_exec_result(
   const constants__t_msg_i call_method_mgr__p_res_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);
extern void call_method_mgr__session_get_endpoint_config(
   const constants__t_session_i call_method_mgr__p_session,
   constants__t_endpoint_config_idx_i * const call_method_mgr__endpoint_config_idx);
extern void call_method_mgr__treat_one_method_call(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_endpoint_config_idx_i call_method_mgr__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void call_method_mgr__treat_method_call_request(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode_service);

#endif
