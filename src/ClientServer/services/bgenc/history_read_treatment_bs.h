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

 File Name            : history_read_treatment_bs.h

 Date                 : 06/08/2025 12:36:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _history_read_treatment_bs_h
#define _history_read_treatment_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "msg_history_read_request.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void history_read_treatment_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void history_read_treatment_bs__external_history_raw_read(
   const constants__t_readRawModifiedDetails_i history_read_treatment_bs__p_readRawDetails,
   const t_bool history_read_treatment_bs__p_TSrequired,
   const t_bool history_read_treatment_bs__p_ContinuationPoint,
   const constants__t_historyReadValueId_i history_read_treatment_bs__p_singleValueId,
   constants_statuscodes_bs__t_StatusCode_i * const history_read_treatment_bs__p_StatusCode,
   constants__t_Nonce_i * const history_read_treatment_bs__p_contPoint,
   t_entier4 * const history_read_treatment_bs__p_nbDataValues,
   constants__t_DataValue_array_i * const history_read_treatment_bs__p_DataValues);
extern void history_read_treatment_bs__set_ts_srv_dataValues(
   const t_entier4 history_read_treatment_bs__p_nbDataValues,
   const constants__t_DataValue_array_i history_read_treatment_bs__p_DataValues,
   const constants__t_Timestamp history_read_treatment_bs__p_currentTs);

#endif
