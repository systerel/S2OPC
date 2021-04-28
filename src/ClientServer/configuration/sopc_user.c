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

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_user.h"

typedef enum
{
    USER_LOCAL,
    USER_ANONYMOUS,
    USER_USERNAME
} user_type_t;

struct SOPC_User
{
    user_type_t type;
    union {
        /** The \p username is only valid for the \p USER_USERNAME type. */
        SOPC_String username;
    } data;
};

static const SOPC_User user_local = {.type = USER_LOCAL};
static const SOPC_User user_anonymous = {.type = USER_ANONYMOUS};

const SOPC_User* SOPC_User_GetLocal(void)
{
    return &user_local;
}

bool SOPC_User_IsLocal(const SOPC_User* user)
{
    return &user_local == user;
}

const SOPC_User* SOPC_User_GetAnonymous(void)
{
    return &user_anonymous;
}

bool SOPC_User_IsAnonymous(const SOPC_User* user)
{
    return &user_anonymous == user;
}

SOPC_User* SOPC_User_CreateUsername(SOPC_String* username)
{
    SOPC_User* user = SOPC_Calloc(1, sizeof(SOPC_User));
    if (NULL == user)
    {
        return NULL;
    }

    user->type = USER_USERNAME;
    SOPC_String_Initialize(&user->data.username);
    SOPC_ReturnStatus status = SOPC_String_Copy(&user->data.username, username);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_String_Clear(&user->data.username);
        SOPC_Free(user);
        user = NULL;
    }

    return user;
}

const SOPC_String* SOPC_User_GetUsername(const SOPC_User* user)
{
    assert(SOPC_User_IsUsername(user));
    return &user->data.username;
}

bool SOPC_User_IsUsername(const SOPC_User* user)
{
    assert(NULL != user);
    return USER_USERNAME == user->type;
}

bool SOPC_User_Equal(const SOPC_User* left, const SOPC_User* right)
{
    assert(NULL != left);
    assert(NULL != right);

    if (left->type == right->type)
    {
        switch (left->type)
        {
        case USER_LOCAL:
        case USER_ANONYMOUS:
            return true;
        case USER_USERNAME:
            return SOPC_String_Equal(&left->data.username, &right->data.username);
        default:
            assert(false && "Unknown Type");
            break;
        }
    }
    return false;
}

void SOPC_User_Free(SOPC_User** ppUser)
{
    if (NULL == ppUser || NULL == *ppUser)
    {
        return;
    }

    SOPC_User* user = *ppUser;
    if (!SOPC_User_IsLocal(user) && !SOPC_User_IsAnonymous(user))
    {
        assert(SOPC_User_IsUsername(user));
        SOPC_String_Clear(&user->data.username);
        SOPC_Free(user);
    }
    *ppUser = NULL;
}

SOPC_User* SOPC_User_Copy(const SOPC_User* user)
{
    SOPC_User* userCopy = NULL;
    if (NULL == user)
    {
        return NULL;
    }
    if (SOPC_User_IsLocal(user))
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        userCopy = (SOPC_User*) SOPC_User_GetLocal();
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
    else if (SOPC_User_IsAnonymous(user))
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        userCopy = (SOPC_User*) SOPC_User_GetAnonymous();
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
    else
    {
        SOPC_ReturnStatus status = SOPC_STATUS_NOK;
        userCopy = SOPC_Calloc(1, sizeof(*userCopy));
        if (NULL != userCopy)
        {
            userCopy->type = user->type;
            status = SOPC_String_Copy(&userCopy->data.username, &user->data.username);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(userCopy);
            userCopy = NULL;
        }
    }
    return userCopy;
}

const char* SOPC_User_ToCString(const SOPC_User* user)
{
    if (NULL == user)
    {
        return "NULL";
    }

    switch (user->type)
    {
    case USER_LOCAL:
        return "[local_user]";
    case USER_ANONYMOUS:
        return "[anonymous]";
    case USER_USERNAME:
        return SOPC_String_GetRawCString(SOPC_User_GetUsername(user));
    default:
        assert(false && "Unknown user type");
    }

    return NULL;
}
