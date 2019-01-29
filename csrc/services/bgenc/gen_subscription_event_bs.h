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

 File Name            : gen_subscription_event_bs.h

 Date                 : 29/01/2019 09:56:53

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _gen_subscription_event_bs_h
#define _gen_subscription_event_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void gen_subscription_event_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void gen_subscription_event_bs__gen_data_changed_event(
   const constants__t_DataValue_i gen_subscription_event_bs__p_prev_dataValue,
   const constants__t_WriteValuePointer_i gen_subscription_event_bs__p_new_value_pointer);
extern void gen_subscription_event_bs__gen_data_changed_event_failed(
   const constants__t_DataValue_i gen_subscription_event_bs__p_prev_dataValue);

#endif
