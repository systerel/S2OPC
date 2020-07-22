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

#include <assert.h>

#include "constants_bs.h"
#include "user_authorization_bs.h"
#include "util_b2c.h"

#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_user_manager.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void user_authorization_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void user_authorization_bs__get_user_authorization(
    const constants__t_operation_type_i user_authorization_bs__p_operation_type,
    const constants__t_NodeId_i user_authorization_bs__p_node_id,
    const constants__t_AttributeId_i user_authorization_bs__p_attribute_id,
    const constants__t_user_i user_authorization_bs__p_user,
    t_bool* const user_authorization_bs__p_authorized)
{
    /* Authorization */
    SOPC_UserAuthorization_OperationType operationType = SOPC_USER_AUTHORIZATION_OPERATION_READ;
    util_operation_type__B_to_C(user_authorization_bs__p_operation_type, &operationType);

    SOPC_ReturnStatus status = SOPC_UserAuthorization_IsAuthorizedOperation(
        user_authorization_bs__p_user, operationType, user_authorization_bs__p_node_id,
        user_authorization_bs__p_attribute_id, user_authorization_bs__p_authorized);

    /* Log failures */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SOPC_UserAuthorization_IsAuthorizedOperation failed with status %i\n", (int) status);
        *user_authorization_bs__p_authorized = false;
    }
    else if (!*user_authorization_bs__p_authorized)
    {
        char* s_node_id = SOPC_NodeId_ToCString(user_authorization_bs__p_node_id);
        const char* operation = NULL;
        switch (operationType)
        {
        case SOPC_USER_AUTHORIZATION_OPERATION_READ:
            operation = "read";
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
            operation = "write";
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
            operation = "executable";
            break;
        default:
            operation = "unknown";
            break;
        }
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "SOPC_UserAuthorization_IsAuthorizedOperation did not authorize %s operation on value \"%s\"\n", operation,
            s_node_id);
        SOPC_Free(s_node_id);
    }
}
