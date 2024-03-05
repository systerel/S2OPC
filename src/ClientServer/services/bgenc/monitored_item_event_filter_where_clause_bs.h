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

 File Name            : monitored_item_event_filter_where_clause_bs.h

 Date                 : 10/04/2024 15:55:55

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_event_filter_where_clause_bs_h
#define _monitored_item_event_filter_where_clause_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "monitored_item_event_filter_treatment_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void monitored_item_event_filter_where_clause_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_event_filter_where_clause_bs__init_where_elt_results(
   const constants__t_filterResult_i monitored_item_event_filter_where_clause_bs__p_filterResult);
extern void monitored_item_event_filter_where_clause_bs__set_where_element_result(
   const constants__t_filterResult_i monitored_item_event_filter_where_clause_bs__p_filterResult,
   const t_entier4 monitored_item_event_filter_where_clause_bs__p_whereEltIdx,
   const constants__t_RawStatusCode monitored_item_event_filter_where_clause_bs__p_rawOperatorSc,
   const constants__t_RawStatusCode monitored_item_event_filter_where_clause_bs__p_rawOperandSc);

#endif
