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

#include "sopc_user.h"
#include "sopc_assert.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

typedef enum
{
    USER_LOCAL,
    USER_ANONYMOUS,
    USER_USERNAME,
    USER_CERTIFICATE
} user_type_t;

typedef struct user_certificate_t
{
    SOPC_String thumbprint;
    SOPC_ByteString der;
} user_certificate_t;

struct SOPC_User
{
    user_type_t type;
    union
    {
        /** The \p username is only valid for the \p USER_USERNAME type. */
        SOPC_String username;
        user_certificate_t certificate;
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

SOPC_User* SOPC_User_CreateCertificate(SOPC_ByteString* certificateData)
{
    SOPC_CertificateList* pCert = NULL;
    char* thumbprint = NULL;
    SOPC_User* user = SOPC_Calloc(1, sizeof(SOPC_User));
    if (NULL == user)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(
        certificateData->Data, (uint32_t) certificateData->Length, &pCert);
    if (SOPC_STATUS_OK == status)
    {
        thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pCert);
        if (NULL == thumbprint)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        user->type = USER_CERTIFICATE;
        status = SOPC_String_InitializeFromCString(&user->data.certificate.thumbprint, thumbprint);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ByteString_Initialize(&user->data.certificate.der);
        status = SOPC_ByteString_Copy(&user->data.certificate.der, certificateData);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_String_Clear(&user->data.certificate.thumbprint);
        SOPC_ByteString_Clear(&user->data.certificate.der);
        SOPC_Free(user);
        user = NULL;
    }

    SOPC_KeyManager_Certificate_Free(pCert);
    SOPC_Free(thumbprint);
    return user;
}

const SOPC_String* SOPC_User_GetUsername(const SOPC_User* user)
{
    SOPC_ASSERT(SOPC_User_IsUsername(user));
    return &user->data.username;
}

bool SOPC_User_IsUsername(const SOPC_User* user)
{
    SOPC_ASSERT(NULL != user);
    return USER_USERNAME == user->type;
}

const SOPC_ByteString* SOPC_User_GetCertificate(const SOPC_User* user)
{
    SOPC_ASSERT(SOPC_User_IsCertificate(user));
    return &user->data.certificate.der;
}

const SOPC_String* SOPC_User_GetCertificate_Thumbprint(const SOPC_User* user)
{
    SOPC_ASSERT(SOPC_User_IsCertificate(user));
    return &user->data.certificate.thumbprint;
}

bool SOPC_User_IsCertificate(const SOPC_User* user)
{
    SOPC_ASSERT(NULL != user);
    return USER_CERTIFICATE == user->type;
}

bool SOPC_User_Equal(const SOPC_User* left, const SOPC_User* right)
{
    SOPC_ASSERT(NULL != left);
    SOPC_ASSERT(NULL != right);

    bool sameCert = false;

    if (left->type == right->type)
    {
        switch (left->type)
        {
        case USER_LOCAL:
        case USER_ANONYMOUS:
            return true;
        case USER_USERNAME:
            return SOPC_String_Equal(&left->data.username, &right->data.username);
        case USER_CERTIFICATE:
            sameCert = SOPC_String_Equal(&left->data.certificate.thumbprint, &right->data.certificate.thumbprint);
            if (sameCert)
            {
                sameCert = SOPC_ByteString_Equal(&left->data.certificate.der, &right->data.certificate.der);
            }
            return sameCert;
        default:
            SOPC_ASSERT(false && "Unknown Type");
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
        SOPC_Boolean is_username = SOPC_User_IsUsername(user);
        SOPC_Boolean is_certificate = SOPC_User_IsCertificate(user);
        SOPC_ASSERT(is_username || is_certificate);
        if (is_username)
        {
            SOPC_String_Clear(&user->data.username);
        }
        if (is_certificate)
        {
            SOPC_String_Clear(&user->data.certificate.thumbprint);
            SOPC_ByteString_Clear(&user->data.certificate.der);
        }
        SOPC_Free(user);
    }
    *ppUser = NULL;
}

SOPC_User* SOPC_User_Copy(const SOPC_User* user)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
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
    else if (SOPC_User_IsUsername(user))
    {
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
    else
    {
        userCopy = SOPC_Calloc(1, sizeof(*userCopy));
        if (NULL != userCopy)
        {
            userCopy->type = user->type;
            status = SOPC_String_Copy(&userCopy->data.certificate.thumbprint, &user->data.certificate.thumbprint);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ByteString_Copy(&userCopy->data.certificate.der, &user->data.certificate.der);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_String_Clear(&userCopy->data.certificate.thumbprint);
            SOPC_ByteString_Clear(&userCopy->data.certificate.der);
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
    case USER_CERTIFICATE:
        return SOPC_String_GetRawCString(SOPC_User_GetCertificate_Thumbprint(user));
    default:
        SOPC_ASSERT(false && "Unknown user type");
    }

    return NULL;
}
