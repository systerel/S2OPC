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

#include "session_roles_granted_bs.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "sopc_assert.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/

static SOPC_SLinkedList* sessionRoles = NULL;

void session_roles_granted_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_roles_granted_bs__add_role_to_session(
    const constants__t_NodeId_i session_roles_granted_bs__p_role_nodeId)
{
    SOPC_NodeId* roleIdAdded =
        (SOPC_NodeId*) SOPC_SLinkedList_Append(sessionRoles, 0, (uintptr_t) session_roles_granted_bs__p_role_nodeId);
    if (NULL != roleIdAdded)
    {
        char* nodeId_string = SOPC_NodeId_ToCString(roleIdAdded);
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Role %s granted to session!", nodeId_string);
        SOPC_Free(nodeId_string);
    }
}

extern void session_roles_granted_bs__initialize_session_roles(void)
{
    sessionRoles = SOPC_SLinkedList_Create(0);
}

extern void session_roles_granted_bs__pop_session_roles(
    constants__t_sessionRoles_i* const session_roles_granted_bs__p_roles)
{
    SOPC_ASSERT(NULL != session_roles_granted_bs__p_roles);
    *session_roles_granted_bs__p_roles = sessionRoles;
}
