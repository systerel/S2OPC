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

#include "user_bs.h"
#include "sopc_assert.h"
#include "sopc_macros.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void user_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void user_bs__are_username_equal(const constants__t_user_i user_bs__p_user,
                                        const constants__t_Criteria_i user_bs__p_username,
                                        t_bool* const user_bs__b_res)
{
    SOPC_ASSERT(NULL != user_bs__b_res);
    SOPC_ASSERT(true == SOPC_User_IsUsername(SOPC_UserWithAuthorization_GetUser(user_bs__p_user)));
    const SOPC_User* user = SOPC_UserWithAuthorization_GetUser(user_bs__p_user);
    const SOPC_String* username = SOPC_User_GetUsername(user);
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_String_Compare(username, user_bs__p_username, false, &comparison);
    if (0 == comparison && SOPC_STATUS_OK == status)
    {
        *user_bs__b_res = true;
    }
    else
    {
        *user_bs__b_res = false;
    }
}

extern void user_bs__is_anonymous(const constants__t_user_i user_bs__p_user, t_bool* const user_bs__b_res)
{
    SOPC_ASSERT(NULL != user_bs__b_res);
    const SOPC_User* user = SOPC_UserWithAuthorization_GetUser(user_bs__p_user);
    *user_bs__b_res = SOPC_User_IsAnonymous(user);
}

extern void user_bs__is_username(const constants__t_user_i user_bs__p_user, t_bool* const user_bs__b_res)
{
    SOPC_ASSERT(NULL != user_bs__b_res);
    const SOPC_User* user = SOPC_UserWithAuthorization_GetUser(user_bs__p_user);
    *user_bs__b_res = SOPC_User_IsUsername(user);
}
