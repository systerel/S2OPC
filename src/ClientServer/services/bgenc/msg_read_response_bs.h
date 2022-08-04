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

 File Name            : msg_read_response_bs.h

 Date                 : 04/08/2022 14:53:37

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_read_response_bs_h
#define _msg_read_response_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_response_bs__alloc_read_response(
   const t_entier4 msg_read_response_bs__p_nb_resps,
   const constants__t_msg_i msg_read_response_bs__p_resp_msg,
   t_bool * const msg_read_response_bs__p_isvalid);
extern void msg_read_response_bs__set_read_response(
   const constants__t_msg_i msg_read_response_bs__p_resp_msg,
   const constants__t_ReadValue_i msg_read_response_bs__p_rvi,
   const constants__t_Variant_i msg_read_response_bs__p_value,
   const constants__t_RawStatusCode msg_read_response_bs__p_raw_sc,
   const constants__t_Timestamp msg_read_response_bs__p_ts_src,
   const constants__t_Timestamp msg_read_response_bs__p_ts_srv);

#endif
