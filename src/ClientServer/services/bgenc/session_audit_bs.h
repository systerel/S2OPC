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

 File Name            : session_audit_bs.h

 Date                 : 29/09/2025 13:30:26

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_audit_bs_h
#define _session_audit_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_audit_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_audit_bs__clear_audit_info(void);
extern void session_audit_bs__server_notify_session_activate(
   const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
   const constants__t_msg_i session_audit_bs__req_msg,
   const constants__t_session_i session_audit_bs__session,
   const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code);
extern void session_audit_bs__server_notify_session_closed(
   const constants__t_session_i session_audit_bs__session,
   const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code);
extern void session_audit_bs__server_notify_session_create(
   const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
   const constants__t_msg_i session_audit_bs__req_msg,
   const constants__t_msg_i session_audit_bs__resp_msg,
   const constants__t_session_i session_audit_bs__session,
   const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code);
extern void session_audit_bs__set_no_req_audit_info(
   const constants__t_channel_config_idx_i session_audit_bs__p_channel_config);
extern void session_audit_bs__set_request_audit_info(
   const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
   const constants__t_msg_header_i session_audit_bs__req_msg_header);

#endif
