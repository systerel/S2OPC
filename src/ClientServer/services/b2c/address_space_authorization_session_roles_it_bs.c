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

#include "address_space_authorization_session_roles_it_bs.h"

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_singly_linked_list.h"

static SOPC_SLinkedList* sessionRoles = NULL;
static SOPC_SLinkedListIterator sessionRolesIt = NULL;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_authorization_session_roles_it_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_authorization_session_roles_it_bs__continue_iter_roles(
    t_bool* const address_space_authorization_session_roles_it_bs__p_continue,
    constants__t_NodeId_i* const address_space_authorization_session_roles_it_bs__p_role)
{
    SOPC_ASSERT(NULL != sessionRolesIt);
    *address_space_authorization_session_roles_it_bs__p_role =
        (constants__t_NodeId_i) SOPC_SLinkedList_Next(&sessionRolesIt);
    *address_space_authorization_session_roles_it_bs__p_continue = SOPC_SLinkedList_HasNext(&sessionRolesIt);
}

void address_space_authorization_session_roles_it_bs__init_iter_roles(
    const constants__t_user_i address_space_authorization_session_roles_it_bs__p_user,
    const constants__t_sessionRoles_i address_space_authorization_session_roles_it_bs__p_roles,
    t_bool* const address_space_authorization_session_roles_it_bs__p_continue)
{
    SOPC_UNUSED_ARG(address_space_authorization_session_roles_it_bs__p_user);
    SOPC_ASSERT(NULL != address_space_authorization_session_roles_it_bs__p_roles);
    sessionRoles = address_space_authorization_session_roles_it_bs__p_roles;
    sessionRolesIt = SOPC_SLinkedList_GetIterator(sessionRoles);
    if (NULL != sessionRolesIt)
    {
        *address_space_authorization_session_roles_it_bs__p_continue = SOPC_SLinkedList_HasNext(&sessionRolesIt);
    }
    else
    {
        *address_space_authorization_session_roles_it_bs__p_continue = false;
    }
}

void address_space_authorization_session_roles_it_bs__clear_iter_roles(void)
{
    sessionRoles = NULL;
    sessionRolesIt = NULL;
}
