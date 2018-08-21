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
#include <stdbool.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

SOPC_ReturnStatus SOPC_UserAuthentication_IsValidUserIdentity(SOPC_UserAuthentication_Manager* pAuthen,
                                                              const SOPC_ExtensionObject* pUser,
                                                              bool* pbUserAuthentified)
{
    assert(NULL != pAuthen && NULL != pUser && NULL != pbUserAuthentified);
    assert(NULL != pAuthen->pFunctions && NULL != pAuthen->pFunctions->pFuncValidateUserIdentity);

    return (pAuthen->pFunctions->pFuncValidateUserIdentity)(pAuthen, pUser, pbUserAuthentified);
}

SOPC_ReturnStatus SOPC_UserAuthorization_IsAuthorizedOperation(SOPC_UserAuthorization_Manager* pAuthor,
                                                               const bool bWriteOperation,
                                                               const SOPC_NodeId* pNid,
                                                               const uint32_t attributeId,
                                                               const SOPC_User* pUser,
                                                               bool* pbOperationAuthorized)
{
    assert(NULL != pAuthor && NULL != pNid && NULL != pUser && NULL != pbOperationAuthorized);
    assert(NULL != pAuthor->pFunctions && NULL != pAuthor->pFunctions->pFuncAuthorizeOperation);

    return (pAuthor->pFunctions->pFuncAuthorizeOperation)(pAuthor, bWriteOperation, pNid, attributeId, pUser,
                                                          pbOperationAuthorized);
}

void SOPC_UserAuthentication_FreeManager(SOPC_UserAuthentication_Manager** ppAuthen)
{
    if (NULL == ppAuthen || NULL == *ppAuthen)
    {
        return;
    }

    SOPC_UserAuthentication_Manager* pAuthen = *ppAuthen;
    assert(NULL != pAuthen->pFunctions && NULL != pAuthen->pFunctions->pFuncFree);
    (pAuthen->pFunctions->pFuncFree)(pAuthen);
    *ppAuthen = NULL;
}

void SOPC_UserAuthorization_Manager_Free(SOPC_UserAuthorization_Manager** ppAuthor);
{
    if (NULL == ppAuthor || NULL == *ppAuthor)
    {
        return;
    }

    SOPC_UserAuthorization_Manager* pAuthor = *ppAuthor;
    assert(NULL != pAuthor->pFunctions && NULL != pAuthor->pFunctions->pFuncFree);
    (pAuthor->pFunctions->pFuncFree)(pAuthor);
    *ppAuthor = NULL;
}
