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

 File Name            : msg_browse_bs.h

 Date                 : 04/08/2022 14:53:34

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_browse_bs_h
#define _msg_browse_bs_h

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

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_browse_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_browse_bs__alloc_browse_response(
   const constants__t_msg_i msg_browse_bs__p_resp_msg,
   const t_entier4 msg_browse_bs__p_nb_bvi,
   t_bool * const msg_browse_bs__p_isallocated);
extern void msg_browse_bs__get_browse_request_params(
   const constants__t_msg_i msg_browse_bs__p_req_msg,
   constants__t_NodeId_i * const msg_browse_bs__p_nid_view,
   t_entier4 * const msg_browse_bs__p_nb_BrowseTargetMax,
   t_entier4 * const msg_browse_bs__p_nb_browse_value);
extern void msg_browse_bs__getall_BrowseValue(
   const constants__t_msg_i msg_browse_bs__p_req_msg,
   const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
   constants__t_NodeId_i * const msg_browse_bs__p_NodeId,
   constants__t_BrowseDirection_i * const msg_browse_bs__p_dir,
   constants__t_NodeId_i * const msg_browse_bs__p_reftype,
   t_bool * const msg_browse_bs__p_inc_subtype,
   constants__t_BrowseNodeClassMask_i * const msg_browse_bs__p_class_mask,
   constants__t_BrowseResultMask_i * const msg_browse_bs__p_result_mask);
extern void msg_browse_bs__set_ResponseBrowse_BrowseResult(
   const constants__t_msg_i msg_browse_bs__p_resp_msg,
   const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
   const t_entier4 msg_browse_bs__p_nb_targets,
   const constants__t_BrowseResultReferences_i msg_browse_bs__p_browseResultReferences);
extern void msg_browse_bs__set_ResponseBrowse_BrowseStatus(
   const constants__t_msg_i msg_browse_bs__p_resp_msg,
   const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
   const constants_statuscodes_bs__t_StatusCode_i msg_browse_bs__p_sc);
extern void msg_browse_bs__set_ResponseBrowse_ContinuationPoint(
   const constants__t_msg_i msg_browse_bs__p_resp_msg,
   const constants__t_BrowseValue_i msg_browse_bs__p_bvi,
   const constants__t_ContinuationPointId_i msg_browse_bs__p_continuationPointId);

#endif
