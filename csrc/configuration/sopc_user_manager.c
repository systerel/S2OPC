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
#include "sopc_macros.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

struct SOPC_UserWithAuthorization
{
    SOPC_User* user;
    SOPC_UserAuthorization_Manager* authorizationManager;
};

SOPC_ReturnStatus SOPC_UserAuthentication_IsValidUserIdentity(SOPC_UserAuthentication_Manager* authenticationManager,
                                                              const SOPC_ExtensionObject* pUser,
                                                              bool* pbUserAuthenticated)
{
    assert(NULL != authenticationManager && NULL != pUser && NULL != pbUserAuthenticated);
    assert(NULL != authenticationManager->pFunctions &&
           NULL != authenticationManager->pFunctions->pFuncValidateUserIdentity);

    return (authenticationManager->pFunctions->pFuncValidateUserIdentity)(authenticationManager, pUser,
                                                                          pbUserAuthenticated);
}

SOPC_ReturnStatus SOPC_UserAuthorization_IsAuthorizedOperation(SOPC_UserAuthorization_Manager* authorizationManager,
                                                               SOPC_UserAuthorization_OperationType operationType,
                                                               const SOPC_NodeId* pNid,
                                                               uint32_t attributeId,
                                                               const SOPC_User* pUser,
                                                               bool* pbOperationAuthorized)
{
    assert(NULL != authorizationManager && NULL != pNid && NULL != pUser && NULL != pbOperationAuthorized);
    assert(NULL != authorizationManager->pFunctions &&
           NULL != authorizationManager->pFunctions->pFuncAuthorizeOperation);

    return (authorizationManager->pFunctions->pFuncAuthorizeOperation)(authorizationManager, operationType, pNid,
                                                                       attributeId, pUser, pbOperationAuthorized);
}

void SOPC_UserAuthentication_FreeManager(SOPC_UserAuthentication_Manager** ppAuthenticationManager)
{
    if (NULL == ppAuthenticationManager || NULL == *ppAuthenticationManager)
    {
        return;
    }

    SOPC_UserAuthentication_Manager* authenticationManager = *ppAuthenticationManager;
    assert(NULL != authenticationManager->pFunctions && NULL != authenticationManager->pFunctions->pFuncFree);
    authenticationManager->pFunctions->pFuncFree(authenticationManager);
    *ppAuthenticationManager = NULL;
}

void SOPC_UserAuthorization_FreeManager(SOPC_UserAuthorization_Manager** ppAuthorizationManager)
{
    if (NULL == ppAuthorizationManager || NULL == *ppAuthorizationManager)
    {
        return;
    }

    SOPC_UserAuthorization_Manager* authorizationManager = *ppAuthorizationManager;
    assert(NULL != authorizationManager->pFunctions && NULL != authorizationManager->pFunctions->pFuncFree);
    authorizationManager->pFunctions->pFuncFree(authorizationManager);
    *ppAuthorizationManager = NULL;
}

/** \brief A helper implementation of the validate UserIdentity callback, which always returns OK. */
static SOPC_ReturnStatus AuthenticateAllowAll(SOPC_UserAuthentication_Manager* authenticationManager,
                                              const SOPC_ExtensionObject* pUserIdentity,
                                              bool* pbUserAuthenticated)
{
    (void) (authenticationManager);
    (void) (pUserIdentity);
    assert(NULL != pbUserAuthenticated);

    *pbUserAuthenticated = true;
    return SOPC_STATUS_OK;
}

/** \brief A helper implementation of the authorize R/W operation callback, which always returns OK. */
static SOPC_ReturnStatus AuthorizeAllowAll(SOPC_UserAuthorization_Manager* authorizationManager,
                                           SOPC_UserAuthorization_OperationType operationType,
                                           const SOPC_NodeId* pNid,
                                           uint32_t attributeId,
                                           const SOPC_User* pUser,
                                           bool* pbOperationAuthorized)
{
    (void) (authorizationManager);
    (void) (operationType);
    (void) (pNid);
    (void) (attributeId);
    (void) (pUser);
    assert(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = true;
    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions AlwaysAuthenticateFunctions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) free,
    .pFuncValidateUserIdentity = AuthenticateAllowAll};

static const SOPC_UserAuthorization_Functions AuthorizeAllowAllFunctions = {
    .pFuncFree = (SOPC_UserAuthorization_Free_Func) free,
    .pFuncAuthorizeOperation = AuthorizeAllowAll};

SOPC_UserAuthentication_Manager* SOPC_UserAuthentication_CreateManager_AllowAll(void)
{
    SOPC_UserAuthentication_Manager* authenticationManager = calloc(1, sizeof(SOPC_UserAuthentication_Manager));

    if (NULL == authenticationManager)
    {
        return NULL;
    }

    authenticationManager->pFunctions = &AlwaysAuthenticateFunctions;
    return authenticationManager;
}

SOPC_UserAuthorization_Manager* SOPC_UserAuthorization_CreateManager_AllowAll(void)
{
    SOPC_UserAuthorization_Manager* authorizationManager = calloc(1, sizeof(SOPC_UserAuthorization_Manager));

    if (NULL == authorizationManager)
    {
        return NULL;
    }

    authorizationManager->pFunctions = &AuthorizeAllowAllFunctions;
    return authorizationManager;
}

SOPC_UserWithAuthorization* SOPC_UserWithAuthorization_CreateFromIdentityToken(
    SOPC_ExtensionObject* pUserIdentity,
    SOPC_UserAuthorization_Manager* authorizationManager)
{
    assert(NULL != pUserIdentity);
    assert(SOPC_ExtObjBodyEncoding_Object == pUserIdentity->Encoding);
    assert(&OpcUa_AnonymousIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType ||
           &OpcUa_UserNameIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType);

    SOPC_UserWithAuthorization* userauthz = calloc(1, sizeof(SOPC_UserWithAuthorization));
    if (NULL == userauthz)
    {
        return NULL;
    }

    userauthz->authorizationManager = authorizationManager;
    if (&OpcUa_AnonymousIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        userauthz->user = (SOPC_User*) SOPC_User_GetAnonymous();
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
    else if (&OpcUa_UserNameIdentityToken_EncodeableType == pUserIdentity->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* tok = (OpcUa_UserNameIdentityToken*) (pUserIdentity->Body.Object.Value);
        userauthz->user = SOPC_User_CreateUsername(&tok->UserName);
    }

    if (NULL == userauthz->user)
    {
        free(userauthz);
        userauthz = NULL;
    }

    return userauthz;
}

SOPC_UserWithAuthorization* SOPC_UserWithAuthorization_CreateLocal(SOPC_UserAuthorization_Manager* authorizationManager)
{
    SOPC_UserWithAuthorization* userauthz = calloc(1, sizeof(SOPC_UserWithAuthorization));
    if (NULL == userauthz)
    {
        return NULL;
    }

    userauthz->authorizationManager = authorizationManager;
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    userauthz->user = (SOPC_User*) SOPC_User_GetLocal();
    SOPC_GCC_DIAGNOSTIC_RESTORE

    if (NULL == userauthz->user)
    {
        free(userauthz);
        userauthz = NULL;
    }

    return userauthz;
}

SOPC_UserAuthorization_Manager* SOPC_UserWithAuthorization_GetManager(SOPC_UserWithAuthorization* userWithAuthorization)
{
    assert(NULL != userWithAuthorization);
    return userWithAuthorization->authorizationManager;
}

const SOPC_User* SOPC_UserWithAuthorization_GetUser(SOPC_UserWithAuthorization* userWithAuthorization)
{
    assert(NULL != userWithAuthorization);
    return userWithAuthorization->user;
}

void SOPC_UserWithAuthorization_Free(SOPC_UserWithAuthorization** ppUserWithAutorization)
{
    if (NULL == ppUserWithAutorization || NULL == *ppUserWithAutorization)
    {
        return;
    }

    SOPC_UserWithAuthorization* userauthz = *ppUserWithAutorization;
    SOPC_User_Free(&userauthz->user);
    free(userauthz);
    *ppUserWithAutorization = NULL;
}
