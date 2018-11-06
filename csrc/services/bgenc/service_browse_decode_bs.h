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

 File Name            : service_browse_decode_bs.h

 Date                 : 06/11/2018 10:49:40

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_decode_bs_h
#define _service_browse_decode_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_browse_decode_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse_decode_bs__decode_browse_request(
   const constants__t_msg_i service_browse_decode_bs__req_payload,
   constants__t_StatusCode_i * const service_browse_decode_bs__StatusCode_service);
extern void service_browse_decode_bs__free_browse_request(void);
extern void service_browse_decode_bs__get_BrowseView(
   constants__t_NodeId_i * const service_browse_decode_bs__p_nid_view);
extern void service_browse_decode_bs__get_nb_BrowseTargetMax(
   t_entier4 * const service_browse_decode_bs__p_nb_BrowseTargetMax);
extern void service_browse_decode_bs__get_nb_BrowseValue(
   t_entier4 * const service_browse_decode_bs__nb_req);
extern void service_browse_decode_bs__getall_BrowseValue(
   const constants__t_BrowseValue_i service_browse_decode_bs__p_bvi,
   constants__t_NodeId_i * const service_browse_decode_bs__p_NodeId,
   constants__t_BrowseDirection_i * const service_browse_decode_bs__p_dir,
   t_bool * const service_browse_decode_bs__p_isreftype,
   constants__t_NodeId_i * const service_browse_decode_bs__p_reftype,
   t_bool * const service_browse_decode_bs__p_inc_subtype);

#endif
