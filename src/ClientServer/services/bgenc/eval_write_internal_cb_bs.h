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

 File Name            : eval_write_internal_cb_bs.h

 Date                 : 30/09/2026 12:44:10

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _eval_write_internal_cb_bs_h
#define _eval_write_internal_cb_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void eval_write_internal_cb_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void eval_write_internal_cb_bs__eval_write_internal_behavior(
   const constants__t_NodeId_i eval_write_internal_cb_bs__p_nid,
   const constants__t_AttributeId_i eval_write_internal_cb_bs__p_attribute,
   const constants__t_DataValue_i eval_write_internal_cb_bs__p_prev_dataValue,
   const constants__t_Variant_i eval_write_internal_cb_bs__p_new_val,
   const constants__t_RawStatusCode eval_write_internal_cb_bs__p_new_val_sc,
   const constants__t_Timestamp eval_write_internal_cb_bs__p_new_val_ts_src,
   const constants__t_Timestamp eval_write_internal_cb_bs__p_new_val_ts_srv);

#endif
