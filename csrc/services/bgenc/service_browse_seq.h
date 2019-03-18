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

 File Name            : service_browse_seq.h

 Date                 : 29/01/2019 09:56:41

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_seq_h
#define _service_browse_seq_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_browse.h"
#include "service_browse_decode_bs.h"
#include "service_browse_seq_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_browse_seq__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_browse_seq__decode_browse_request service_browse_decode_bs__decode_browse_request
#define service_browse_seq__free_browse_request service_browse_decode_bs__free_browse_request
#define service_browse_seq__free_browse_result service_browse__free_browse_result
#define service_browse_seq__write_BrowseResponse_msg_out service_browse__write_BrowseResponse_msg_out

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_browse_seq__fill_browse_response(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const t_entier4 service_browse_seq__p_nb_bri,
   const constants__t_Node_i service_browse_seq__p_src_node,
   const constants__t_BrowseDirection_i service_browse_seq__p_dir,
   const t_bool service_browse_seq__p_isreftype,
   const constants__t_NodeId_i service_browse_seq__p_reftype,
   const t_bool service_browse_seq__p_inc_subtype);
extern void service_browse_seq__fill_browse_response_ref(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const constants__t_Reference_i service_browse_seq__p_ref,
   const constants__t_BrowseDirection_i service_browse_seq__p_dir,
   const t_bool service_browse_seq__p_isreftype,
   const constants__t_NodeId_i service_browse_seq__p_ref_type,
   const t_bool service_browse_seq__p_inc_subtype,
   t_bool * const service_browse_seq__p_continue_bri);
extern void service_browse_seq__treat_browse_request_BrowseValue_1(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const t_entier4 service_browse_seq__p_nb_target_max);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse_seq__treat_browse_request_BrowseValues(
   constants__t_StatusCode_i * const service_browse_seq__StatusCode_service);

#endif
