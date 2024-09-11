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

#include "session_role_identity_bs.h"
#include "sopc_assert.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_identity_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_role_identity_bs__read_identity_criteria(
    const constants__t_Identity_i session_role_identity_bs__p_identity,
    constants__t_Criteria_i* const session_role_identity_bs__p_criteria)
{
    SOPC_ASSERT(NULL != session_role_identity_bs__p_criteria);
    SOPC_String* criteria = &session_role_identity_bs__p_identity->Criteria;
    *session_role_identity_bs__p_criteria = criteria;
}

extern void session_role_identity_bs__read_identity_criteriaType(
    const constants__t_Identity_i session_role_identity_bs__p_identity,
    constants__t_CriteriaType_i* const session_role_identity_bs__p_criteriaType)
{
    SOPC_ASSERT(NULL != session_role_identity_bs__p_criteriaType);
    *session_role_identity_bs__p_criteriaType =
        (constants__t_CriteriaType_i) session_role_identity_bs__p_identity->CriteriaType;
}
