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
 * \brief Defines the user manager, the applicative interface used to authenticate users,
 *        and authorize read/write operations in the address space.
 *
 * These structures are only used on the server side.
 *
 * The developer shall follow these steps to use the user authentication and authorization:
 *  - create a static const instance of the _Functions structures,
 *  - define a free function, if needed, which frees private data of the instance,
 *  - define a creation function which mallocs a new _Manager structure and its private data if needed.
 */

#ifndef SOPC_USER_MANAGER_H_
#define SOPC_USER_MANAGER_H_

#include <stdbool.h>

#include "sopc_builtintypes.h"

typedef struct SOPC_UserAuthentication_Manager SOPC_UserAuthentication_Manager;
typedef struct SOPC_UserAuthorization_Manager SOPC_UserAuthorization_Manager;

/** \brief The operation type to authorize, see /p SOPC_UserAuthorization_IsAuthorizedOperation */
typedef enum
{
    SOPC_USER_AUTHORIZATION_OPERATION_READ,
    SOPC_USER_AUTHORIZATION_OPERATION_WRITE
} SOPC_UserAuthorization_OperationType;

/**
 * \brief Logged in (successfully) user structure.
 *
 * An authorization manager is embedded in the structure.
 * It is the manager associated with the endpoint on which the user is connected.
 * It is only a reference and should not be freed or cleared.
 */
typedef struct SOPC_User
{
    /**
     * Set when this is a request from a local service.
     * When \p local is true, the other members of the struct shall not be read.
     */
    bool local;
    /** Set when this is an anonymous user. When set, the \p username shall not be read. */
    bool anonymous;
    /** The username of the not local nor anonymous user. */
    SOPC_String username;

    /** A borrowed pointer to the authorization manager associated with the current endpoint. */
    SOPC_UserAuthorization_Manager* authorizationManager;
} SOPC_User;

typedef void (*SOPC_UserAuthentication_Free_Func)(SOPC_UserAuthentication_Manager* authenticationManager);
typedef SOPC_ReturnStatus (*SOPC_UserAuthentication_ValidateUserIdentity_Func)(
    SOPC_UserAuthentication_Manager* authenticationManager,
    const SOPC_ExtensionObject* pUser,
    bool* pbUserAuthenticated);

typedef void (*SOPC_UserAuthorization_Free_Func)(SOPC_UserAuthorization_Manager* authorizationManager);
typedef SOPC_ReturnStatus (*SOPC_UserAuthorization_AuthorizeOperation_Func)(
    SOPC_UserAuthorization_Manager* authorizationManager,
    SOPC_UserAuthorization_OperationType operationType,
    const SOPC_NodeId* pNid,
    uint32_t attributeId,
    const SOPC_User* pUser,
    bool* pbOperationAuthorized);

typedef struct SOPC_UserAuthentication_Functions
{
    /**
     * \brief Deallocation function, called upon SOPC_UserAuthentication_Manager destruction.
     *
     * This function can be the standard \p free function if nothing is stored in \p pData.
     */
    SOPC_UserAuthentication_Free_Func pFuncFree;

    /**
     * \brief Called to authorize a user connection, when receiving an ActivateSession request.
     *
     * \warning This callback should not block the thread that calls it, and shall return immediately.
     *          It also needs to be thread safe.
     *
     * \param authenticationManager  The SOPC_UserAuthentication_Manager instance.
     * \param pUser                  The user identity token which was received in the ActivateSession request.
     * \param pbUserAuthenticated    A valid pointer to the uninitialized result of the operation.
     *                               The callback shall set it to false when the connection is refused.
     *
     * \return SOPC_STATUS_OK when \p pbUserAuthenticated was set.
     */
    SOPC_UserAuthentication_ValidateUserIdentity_Func pFuncValidateUserIdentity;
} SOPC_UserAuthentication_Functions;

typedef struct SOPC_UserAuthorization_Functions
{
    /**
     * \brief Deallocation function, called upon SOPC_UserAuthorization_Manager destruction.
     *
     * This function can be the standard \p free function if nothing is stored in \p pData.
     */
    SOPC_UserAuthorization_Free_Func pFuncFree;

    /**
     * \brief Called to authorize a read or a write operation in the address space.
     *
     * \warning This callback should not block the thread that calls it, and shall return immediately.
     *          It also needs to be thread safe.
     *
     * \param authorizationManager   The SOPC_UserAuthorization_Manager instance.
     * \param operationType          Set to SOPC_USER_AUTHORIZATION_OPERATION_READ for a read operation,
                                     or SOPC_USER_AUTHORIZATION_OPERATION_WRITE for a write operation.
     * \param pNid                   The operation reads/write this NodeId.
     * \param attributeId            The operation reads/write this attribute.
     * \param pUser                  The connected SOPC_User which attempts the operation.
     * \param pbOperationAuthorized  A valid pointer to the uninitialized result of the operation.
     *                               The callback shall set it to false when the operation is refused.
     *
     * \return SOPC_STATUS_OK when \p pbUserAuthorized was set.
     */
    SOPC_UserAuthorization_AuthorizeOperation_Func pFuncAuthorizeOperation;
} SOPC_UserAuthorization_Functions;

struct SOPC_UserAuthentication_Manager
{
    /** It is recommended to have a pointer to a static const instance */
    const SOPC_UserAuthentication_Functions* pFunctions;

    /** This field may be used to store instance specific data. */
    void* pData;
};

struct SOPC_UserAuthorization_Manager
{
    /** It is recommended to have a pointer to a static const instance */
    const SOPC_UserAuthorization_Functions* pFunctions;

    /** This field may be used to store instance specific data. */
    void* pData;
};

/**
 * \brief Authenticate a user with the chosen authentication manager.
 *
 * \param authenticationManager  The SOPC_UserAuthentication_Manager instance.
 * \param pUser                  The user identity token which was received in the ActivateSession request.
 * \param pbUserAuthenticated    A valid pointer to the uninitialized result of the operation.
 *                               The callback shall set it to false when the connection is refused.
 *
 * \return SOPC_STATUS_OK when \p pbUserAuthenticated was set.
 */
SOPC_ReturnStatus SOPC_UserAuthentication_IsValidUserIdentity(SOPC_UserAuthentication_Manager* authenticationManager,
                                                              const SOPC_ExtensionObject* pUser,
                                                              bool* pbUserAuthenticated);

/**
 * \brief Authorize an operation with the chosen authorization manager.
 *
 * \param authorizationManager   The SOPC_UserAuthorization_Manager instance.
 * \param operationType          Set to SOPC_USER_AUTHORIZATION_OPERATION_READ for a read operation,
                                 or SOPC_USER_AUTHORIZATION_OPERATION_WRITE for a write operation.
 * \param pNid                   The operation reads/write this NodeId.
 * \param attributeId            The operation reads/write this attribute.
 * \param pUser                  The connected SOPC_User which attempts the operation.
 * \param pbOperationAuthorized  A valid pointer to the uninitialized result of the operation.
 *                               The callback shall set it to false when the operation is refused.
 *
 * \return SOPC_STATUS_OK when \p pbUserAuthorized was set.
 */
SOPC_ReturnStatus SOPC_UserAuthorization_IsAuthorizedOperation(SOPC_UserAuthorization_Manager* authorizationManager,
                                                               SOPC_UserAuthorization_OperationType operationType,
                                                               const SOPC_NodeId* pNid,
                                                               uint32_t attributeId,
                                                               const SOPC_User* pUser,
                                                               bool* pbOperationAuthorized);
/** \brief Deletes a SOPC_UserAuthentication_Manager using its pFuncFree. */
void SOPC_UserAuthentication_FreeManager(SOPC_UserAuthentication_Manager** ppAuthenticationManager);

/** \brief Deletes a SOPC_UserAuthorization_Manager using its pFuncFree. */
void SOPC_UserAuthorization_FreeManager(SOPC_UserAuthorization_Manager** ppAuthorizationManager);

/** \brief A helper implementation that always authentication positively a user. */
SOPC_UserAuthentication_Manager* SOPC_UserAuthentication_CreateManager_AllowAll(void);

/** \brief A helper implementation that always authorize an operation. */
SOPC_UserAuthorization_Manager* SOPC_UserAuthorization_CreateManager_AllowAll(void);

/** \brief User creation, \p authorizationManager may be NULL. */
SOPC_User* SOPC_User_Create(SOPC_ExtensionObject* pUserIdentity, SOPC_UserAuthorization_Manager* authorizationManager);

/** \brief Returns local user, which shall not be freed. */
const SOPC_User* SOPC_User_Get_Local(void);

/** \brief User deletion */
void SOPC_User_Free(SOPC_User** ppUser);

#endif /* SOPC_USER_MANAGER_H_ */
