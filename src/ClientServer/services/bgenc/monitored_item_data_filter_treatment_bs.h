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

 File Name            : monitored_item_data_filter_treatment_bs.h

 Date                 : 19/03/2024 16:03:55

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_data_filter_treatment_bs_h
#define _monitored_item_data_filter_treatment_bs_h

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
extern void monitored_item_data_filter_treatment_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_data_filter_treatment_bs__check_monitored_item_data_filter_valid(
   const constants__t_Node_i monitored_item_data_filter_treatment_bs__p_node,
   const constants__t_monitoringFilter_i monitored_item_data_filter_treatment_bs__p_filter,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_data_filter_treatment_bs__StatusCode,
   constants__t_monitoringFilterCtx_i * const monitored_item_data_filter_treatment_bs__filterCtx);

#endif
