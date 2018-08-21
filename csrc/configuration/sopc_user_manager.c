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

void SOPC_UserAuthorization_FreeManager(SOPC_UserAuthorization_Manager** ppAuthor)
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

SOPC_User* SOPC_User_Create(SOPC_ExtensionObject* pUserIdentity)
{
    assert(NULL != pUserIdentity);
    assert(SOPC_ExtObjBodyEncoding_Object == pUserIdentity->Encoding);
    assert(&OpcUa_AnonymousIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType ||
           &OpcUa_UserNameIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType);

    SOPC_User* pUser = (SOPC_User*) malloc(sizeof(SOPC_User));
    if (NULL == pUser)
    {
        return NULL;
    }

    if (&OpcUa_AnonymousIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType)
    {
        pUser->anonymous = true;
        SOPC_String_Initialize(&pUser->username);
    }
    else if (&OpcUa_UserNameIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType)
    {
        pUser->anonymous = false;
        OpcUa_UserNameIdentityToken* tok = (OpcUa_UserNameIdentityToken*) (pUserIdentity->Body.Object.Value);
        SOPC_ReturnStatus status = SOPC_String_Copy(&pUser->username, &tok->UserName);
        if (SOPC_STATUS_OK != status)
        {
            free(pUser);
            pUser = NULL;
        }
    }

    return pUser;
}

void SOPC_User_Free(SOPC_User** ppUser)
{
    if (NULL == ppUser || NULL == *ppUser)
    {
        return;
    }

    SOPC_User* pUser = *ppUser;
    SOPC_String_Clear(&pUser->username);
    free(pUser);
    *ppUser = NULL;
}

/** \brief A helper implementation of the validate UserIdentity callback, which always returns OK. */
static SOPC_ReturnStatus AlwayseValidate(SOPC_UserAuthentication_Manager* pAuthen,
                                         const SOPC_ExtensionObject* pUserIdentity,
                                         bool* pbUserAuthentified)
{
    (void) (pAuthen);
    (void) (pUserIdentity);
    assert(NULL != pbUserAuthentified);

    *pbUserAuthentified = true;
    return SOPC_STATUS_OK;
}

/** \brief A helper implementation of the authorize R/W operation callback, which always returns OK. */
static SOPC_ReturnStatus AlwaysAuthorize(SOPC_UserAuthorization_Manager* pAuthor,
                                         const bool bWriteOperation,
                                         const SOPC_NodeId* pNid,
                                         const uint32_t attributeId,
                                         const SOPC_User* pUser,
                                         bool* pbOperationAuthorized)
{
    (void) (pAuthor);
    (void) (bWriteOperation);
    (void) (pNid);
    (void) (attributeId);
    (void) (pUser);
    assert(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = true;
    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions AlwaysAuthenticateFunctions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) free,
    .pFuncValidateUserIdentity = AlwayseValidate};

static const SOPC_UserAuthorization_Functions AlwaysAuthorizeFunctions = {
    .pFuncFree = (SOPC_UserAuthorization_Free_Func) free,
    .pFuncAuthorizeOperation = AlwaysAuthorize};

SOPC_UserAuthentication_Manager* SOPC_UserAuthentication_CreateManager_UserAlwaysValid(void)
{
    SOPC_UserAuthentication_Manager* pAuthen =
        (SOPC_UserAuthentication_Manager*) calloc(1, sizeof(SOPC_UserAuthentication_Manager));

    if (NULL == pAuthen)
    {
        return NULL;
    }

    pAuthen->pFunctions = &AlwaysAuthenticateFunctions;
    return pAuthen;
}

SOPC_UserAuthorization_Manager* SOPC_UserAuthorization_CreateManager_OperationAlwaysValid(void)
{
    SOPC_UserAuthorization_Manager* pAuthor =
        (SOPC_UserAuthorization_Manager*) calloc(1, sizeof(SOPC_UserAuthorization_Manager));

    if (NULL == pAuthor)
    {
        return NULL;
    }

    pAuthor->pFunctions = &AlwaysAuthorizeFunctions;
    return pAuthor;
}
