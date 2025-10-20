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

 File Name            : app_overwrite_req_cb_bs.h

 Date                 : 19/11/2025 14:39:56

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _app_overwrite_req_cb_bs_h
#define _app_overwrite_req_cb_bs_h

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
extern void app_overwrite_req_cb_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void app_overwrite_req_cb_bs__has_server_overwrite_req_cb(
   const constants__t_endpoint_config_idx_i app_overwrite_req_cb_bs__p_endpoint_config_idx,
   t_bool * const app_overwrite_req_cb_bs__bres);
extern void app_overwrite_req_cb_bs__overwrite_service_request(
   const constants__t_msg_i app_overwrite_req_cb_bs__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const app_overwrite_req_cb_bs__p_sc,
   constants__t_msg_i * const app_overwrite_req_cb_bs__new_req_msg);

#endif
