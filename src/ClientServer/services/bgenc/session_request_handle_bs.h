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

 File Name            : session_request_handle_bs.h

 Date                 : 04/08/2022 14:53:47

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_request_handle_bs_h
#define _session_request_handle_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "request_handle_bs.h"
#include "session_core.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_request_handle_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_request_handle_bs__client_add_session_request_handle(
   const constants__t_session_i session_request_handle_bs__session,
   const constants__t_client_request_handle_i session_request_handle_bs__req_handle);
extern void session_request_handle_bs__client_get_session_and_remove_request_handle(
   const constants__t_client_request_handle_i session_request_handle_bs__req_handle,
   constants__t_session_i * const session_request_handle_bs__session);
extern void session_request_handle_bs__client_remove_all_request_handles(
   const constants__t_session_i session_request_handle_bs__session);

#endif
