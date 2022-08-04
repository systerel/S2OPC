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

 File Name            : data_value_pointer_bs.h

 Date                 : 04/08/2022 14:53:31

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _data_value_pointer_bs_h
#define _data_value_pointer_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void data_value_pointer_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void data_value_pointer_bs__get_conv_DataValue_LocalDataType(
   const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
   constants__t_NodeId_i * const data_value_pointer_bs__p_dt);
extern void data_value_pointer_bs__get_conv_DataValue_SourceTimestamp(
   const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
   constants__t_Timestamp * const data_value_pointer_bs__p_st);
extern void data_value_pointer_bs__get_conv_DataValue_Status(
   const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
   constants__t_RawStatusCode * const data_value_pointer_bs__p_sc);
extern void data_value_pointer_bs__get_conv_DataValue_ValueRank(
   const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
   t_entier4 * const data_value_pointer_bs__p_vr);
extern void data_value_pointer_bs__get_conv_DataValue_Variant(
   const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
   constants__t_Variant_i * const data_value_pointer_bs__p_variant);

#endif
