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

/**
 * This file is an excerpt from sopc_user_manager.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

typedef struct SOPC_UserAuthentication_Manager SOPC_UserAuthentication_Manager;
typedef struct SOPC_UserAuthorization_Manager SOPC_UserAuthorization_Manager;

typedef enum
{
    SOPC_USER_AUTHORIZATION_OPERATION_READ,
    SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
    SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE
} SOPC_UserAuthorization_OperationType;

typedef struct SOPC_UserWithAuthorization SOPC_UserWithAuthorization;

typedef enum
{
    SOPC_USER_AUTHENTICATION_INVALID_TOKEN,
    SOPC_USER_AUTHENTICATION_REJECTED_TOKEN,
    /** It is strongly discouraged to use this value, prefer \p SOPC_USER_AUTHENTICATION_REJECTED_TOKEN. */
    SOPC_USER_AUTHENTICATION_ACCESS_DENIED,
    SOPC_USER_AUTHENTICATION_OK
} SOPC_UserAuthentication_Status;

typedef void SOPC_UserAuthentication_Free_Func(SOPC_UserAuthentication_Manager* authenticationManager);
typedef SOPC_ReturnStatus SOPC_UserAuthentication_ValidateUserIdentity_Func(
    SOPC_UserAuthentication_Manager* authenticationManager,
    const SOPC_ExtensionObject* pUser,
    SOPC_UserAuthentication_Status* pUserAuthenticated);

typedef void SOPC_UserAuthorization_Free_Func(SOPC_UserAuthorization_Manager* authorizationManager);
typedef SOPC_ReturnStatus SOPC_UserAuthorization_AuthorizeOperation_Func(
    SOPC_UserAuthorization_Manager* authorizationManager,
    SOPC_UserAuthorization_OperationType operationType,
    const SOPC_NodeId* nodeId,
    uint32_t attributeId,
    const SOPC_User* pUser,
    bool* pbOperationAuthorized);

void SOPC_UserAuthentication_FreeManager(SOPC_UserAuthentication_Manager** ppAuthenticationManager);
void SOPC_UserAuthorization_FreeManager(SOPC_UserAuthorization_Manager** ppAuthorizationManager);

SOPC_UserAuthentication_Manager* SOPC_UserAuthentication_CreateManager_AllowAll(void);
SOPC_UserAuthorization_Manager* SOPC_UserAuthorization_CreateManager_AllowAll(void);

/* These structures are required to create one's own managers */
typedef struct SOPC_UserAuthentication_Functions
{
    SOPC_UserAuthentication_Free_Func* pFuncFree;
    SOPC_UserAuthentication_ValidateUserIdentity_Func* pFuncValidateUserIdentity;
} SOPC_UserAuthentication_Functions;

typedef struct SOPC_UserAuthorization_Functions
{
    SOPC_UserAuthorization_Free_Func* pFuncFree;
    SOPC_UserAuthorization_AuthorizeOperation_Func* pFuncAuthorizeOperation;
} SOPC_UserAuthorization_Functions;

struct SOPC_UserAuthentication_Manager
{
    const SOPC_UserAuthentication_Functions* pFunctions;
    void* pData;
};

struct SOPC_UserAuthorization_Manager
{
    const SOPC_UserAuthorization_Functions* pFunctions;
    void* pData;
};
