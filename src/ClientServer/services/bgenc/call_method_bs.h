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

 File Name            : call_method_bs.h

 Date                 : 16/02/2023 13:30:47

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _call_method_bs_h
#define _call_method_bs_h

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
extern void call_method_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void call_method_bs__exec_callMethod(
   const constants__t_endpoint_config_idx_i call_method_bs__p_endpoint_config_idx,
   const constants__t_CallMethodPointer_i call_method_bs__p_call_method_pointer,
   constants__t_RawStatusCode * const call_method_bs__p_rawStatusCode,
   t_entier4 * const call_method_bs__p_nb_out,
   constants__t_ArgumentsPointer_i * const call_method_bs__p_out_arguments);

#endif
