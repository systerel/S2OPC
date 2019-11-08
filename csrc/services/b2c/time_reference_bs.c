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

/*--------------
   SEES Clause
  --------------*/

#include "time_reference_bs.h"

#include "sopc_time.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void time_reference_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void time_reference_bs__get_current_TimerReference(constants__t_timeref_i* const time_reference_bs__p_timeref)
{
    *time_reference_bs__p_timeref = SOPC_TimeReference_GetCurrent();
}
void time_reference_bs__is_less_than_TimeReference(const constants__t_timeref_i time_reference_bs__p_left,
                                                   const constants__t_timeref_i time_reference_bs__p_right,
                                                   t_bool* const time_reference_bs__p_is_oldest)
{
    *time_reference_bs__p_is_oldest = time_reference_bs__p_left < time_reference_bs__p_right;
}
