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

/** \file
 *
 * Implements the base machine for the status codes constants
 */

#include "constants_statuscodes_bs.h"

#include "util_b2c.h"

#include "sopc_builtintypes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void constants_statuscodes_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants_statuscodes_bs__get_const_RawStatusCode_BadInvalidState(
    constants__t_RawStatusCode* const constants_statuscodes_bs__p_raw_sc)
{
    *constants_statuscodes_bs__p_raw_sc = OpcUa_BadInvalidState;
}

void constants_statuscodes_bs__get_const_RawStatusCode_Good(
    constants__t_RawStatusCode* const constants_statuscodes_bs__p_raw_sc)
{
    *constants_statuscodes_bs__p_raw_sc = SOPC_GoodGenericStatus;
}

void constants_statuscodes_bs__getall_conv_RawStatusCode_To_StatusCode(
    const constants__t_RawStatusCode constants_statuscodes_bs__p_raw_sc,
    constants_statuscodes_bs__t_StatusCode_i* const constants_statuscodes_bs__p_sc)
{
    util_status_code__C_to_B(constants_statuscodes_bs__p_raw_sc, constants_statuscodes_bs__p_sc);
}

void constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(
    const constants_statuscodes_bs__t_StatusCode_i constants_statuscodes_bs__p_sc,
    constants__t_RawStatusCode* const constants_statuscodes_bs__p_raw_sc)
{
    util_status_code__B_to_C(constants_statuscodes_bs__p_sc, constants_statuscodes_bs__p_raw_sc);
}
