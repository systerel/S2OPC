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
 * \brief Defines the logged-in (server-side) user.
 *
 * The user bears data specific to its type. The following user types are supported:
 * - the "local user" is used to verify local operations (see \p SOPC_ToolkitServer_AsyncLocalServiceRequest),
 *   and has no data,
 * - the "anonymous user" corresponds to an \p OpcUa_AnonymousIdentityToken and has no data,
 * - the "username user" corresponds to an \p OpcUa_UserNameIdentityToken and has a username
 *   (see \p SOPC_User_GetUsername).
 */

#ifndef SOPC_USER_H_
#define SOPC_USER_H_

#include <stdbool.h>

#include "sopc_builtintypes.h"

/** \brief Logged in (successfully) user structure. */
typedef struct SOPC_User SOPC_User;

/** \brief Returns a local user, which shall not be freed nor be modified. */
const SOPC_User* SOPC_User_GetLocal(void);
/** \brief Returns true if the user is a local user. */
bool SOPC_User_IsLocal(const SOPC_User* user);

/** \brief Returns an anonymous user, which shall not be freed nor be modified. */
const SOPC_User* SOPC_User_GetAnonymous(void);
/** \brief Returns true if the user is an anonymous user. */
bool SOPC_User_IsAnonymous(const SOPC_User* user);

/**
 * \brief Creates a \p SOPC_User which has a usename.
 *
 * \param username  A valid pointer to the username of the user to create.
 *
 * \return An instance of \p SOPC_User if successful, otherwise NULL.
 */
SOPC_User* SOPC_User_CreateUsername(SOPC_String* username);

/**
 * \brief Returns a reference to the internal storage of the username.
 *        The user must be a user with a username.
 */
const SOPC_String* SOPC_User_GetUsername(const SOPC_User* user);

/** \brief Returns true if the type of the user is username */
bool SOPC_User_IsUsername(const SOPC_User* user);

/** \brief Returns true if the users are the same type and content (if applicable) */
bool SOPC_User_Equal(const SOPC_User* left, const SOPC_User* right);

/** \brief User deletion, should not be called on local and anonymous users. */
void SOPC_User_Free(SOPC_User** ppUser);

/** \brief Copy the given user */
SOPC_User* SOPC_User_Copy(const SOPC_User* user);

/**
 * \brief Return the user as a const C string description:
 *        - No user (no session involved): 'NULL'
 *        - Local user (server local service context): '[local_user]'
 *        - Anonymous: '[anonymous]'
 *        - UserName: '&lt;user_name&gt;' (replaced by actual username)
 */
const char* SOPC_User_ToCString(const SOPC_User* user);

#endif /* SOPC_USER_H_ */
