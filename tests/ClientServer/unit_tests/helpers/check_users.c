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

#include "check_helpers.h"
#include "opcua_identifiers.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

#define USERNAME "S2OPC user"
#define PASSWORD "}XaD2+-fK_~ )cGKUK=\\z`|+)Y$Rv#\0n"
#define POLICY_USERNAME "UserName"
#define USERNAME_INVALID "S2OPC_user"
#define PASSWORD_INVALID "}XaD2+-fK_~_)cGKUK=\\z`|+)Y$Rv#\0n"

#define ATTRIBUTEID_BROWSENAME 3
#define ATTRIBUTEID_VALUE 13
#define ATTRIBUTEID_ACCESSLEVEL 17
#define ATTRIBUTEID_USERACCESSLEVEL 18
#define ATTRIBUTEID_EXECUTABLE 21

/* Fixtures global variables */
static SOPC_User* gUser = NULL;
static SOPC_UserAuthentication_Manager* gAuthenticationManager = NULL;
static SOPC_UserAuthentication_Manager* gAuthenticationManagerSelective = NULL;
static SOPC_UserAuthorization_Manager* gAuthorizationManager = NULL;
static SOPC_UserAuthorization_Manager* gAuthorizationManagerSelective = NULL;

/* Global complex objects */
static const SOPC_ExtensionObject anonymousIdentityToken = {
    .Encoding = SOPC_ExtObjBodyEncoding_Object,
    .TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric,
    .TypeId.NodeId.Data.Numeric = OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary,
    .Body.Object.ObjType = &OpcUa_AnonymousIdentityToken_EncodeableType,
    .Body.Object.Value = NULL};
static OpcUa_UserNameIdentityToken opcuaUsernameToken = {
    .PolicyId = {.Length = sizeof(POLICY_USERNAME) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) POLICY_USERNAME},
    .UserName = {.Length = sizeof(USERNAME) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) USERNAME},
    .Password = {.Length = sizeof(PASSWORD) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) PASSWORD}};
static const SOPC_ExtensionObject usernameIdentityToken = {
    .Encoding = SOPC_ExtObjBodyEncoding_Object,
    .TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric,
    .TypeId.NodeId.Data.Numeric = OpcUaId_UserNameIdentityToken_Encoding_DefaultBinary,
    .Body.Object.ObjType = &OpcUa_UserNameIdentityToken_EncodeableType,
    .Body.Object.Value = &opcuaUsernameToken};
static OpcUa_UserNameIdentityToken opcuaUsernameToken_invalid = {
    .PolicyId = {.Length = sizeof(POLICY_USERNAME) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) POLICY_USERNAME},
    .UserName = {.Length = sizeof(USERNAME) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) USERNAME},
    .Password = {.Length = sizeof(PASSWORD) - 1, .DoNotClear = true, .Data = (SOPC_Byte*) PASSWORD_INVALID}};
static const SOPC_ExtensionObject usernameIdentityToken_invalid = {
    .Encoding = SOPC_ExtObjBodyEncoding_Object,
    .TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric,
    .TypeId.NodeId.Data.Numeric = OpcUaId_UserNameIdentityToken_Encoding_DefaultBinary,
    .Body.Object.ObjType = &OpcUa_UserNameIdentityToken_EncodeableType,
    .Body.Object.Value = &opcuaUsernameToken_invalid};
static const SOPC_NodeId authorizedNodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                             .Namespace = 42,
                                             .Data.Numeric = 36263};
static const SOPC_NodeId unauthorizedNodeId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                               .Namespace = 42,
                                               .Data.Numeric = 36264};

/* Custom authentication and authorization functions */
static SOPC_ReturnStatus selectiveAuthenticationValidate(SOPC_UserAuthentication_Manager* authn,
                                                         const SOPC_ExtensionObject* token,
                                                         SOPC_UserAuthentication_Status* authenticated)
{
    SOPC_UNUSED_ARG(authn);
    assert(NULL != token && NULL != authenticated);

    /* Only validates the username with the correct password */
    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    assert(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        if (strcmp(SOPC_String_GetRawCString(username), USERNAME) == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == sizeof(PASSWORD) - 1 && memcmp(pwd->Data, PASSWORD, sizeof(PASSWORD) - 1) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions selectiveAuthenticationFunctions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func*) &SOPC_Free,
    .pFuncValidateUserIdentity = selectiveAuthenticationValidate};

static SOPC_ReturnStatus selectiveAuthorizationAllow(SOPC_UserAuthorization_Manager* authz,
                                                     SOPC_UserAuthorization_OperationType operationType,
                                                     const SOPC_NodeId* nodeId,
                                                     uint32_t attributeId,
                                                     const SOPC_User* user,
                                                     bool* authorized)
{
    SOPC_UNUSED_ARG(authz);

    assert(NULL != nodeId && NULL != user && NULL != authorized && 1 <= attributeId && attributeId <= 22);

    /* Only the USERNAME is authorized */
    *authorized = false;
    if (!SOPC_User_IsUsername(user))
    {
        return SOPC_STATUS_OK;
    }
    if (strcmp(SOPC_String_GetRawCString(SOPC_User_GetUsername(user)), USERNAME) != 0)
    {
        return SOPC_STATUS_OK;
    }

    /* Allow read on all attributes, but on valid node id only */
    /* Restrict write to valid node id and value attribute */
    int cmpNid = -2;
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(nodeId, &authorizedNodeId, &cmpNid));
    switch (operationType)
    {
    case SOPC_USER_AUTHORIZATION_OPERATION_READ:
        *authorized = 0 == cmpNid;
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
        if (13 == attributeId)
        {
            *authorized = 0 == cmpNid;
        }
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
        if (ATTRIBUTEID_EXECUTABLE == attributeId)
        {
            *authorized = 0 == cmpNid;
        }
        break;
    default:
        assert(false);
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthorization_Functions selectiveAuthorizationFunctions = {
    .pFuncFree = (SOPC_UserAuthorization_Free_Func*) &SOPC_Free,
    .pFuncAuthorizeOperation = selectiveAuthorizationAllow};

/* Fixture setup and teardown functions */
static inline void setup_user(void)
{
    SOPC_String username;
    ck_assert(SOPC_STATUS_OK == SOPC_String_InitializeFromCString(&username, USERNAME));

    gUser = SOPC_User_CreateUsername(&username);
    ck_assert_ptr_nonnull(gUser);

    SOPC_String_Clear(&username);
}

static inline void teardown_user(void)
{
    SOPC_User_Free(&gUser);
}

static inline void setup_authentication(void)
{
    gAuthenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
    ck_assert_ptr_nonnull(gAuthenticationManager);
    gAuthenticationManagerSelective = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    ck_assert_ptr_nonnull(gAuthenticationManagerSelective);
    gAuthenticationManagerSelective->pFunctions = &selectiveAuthenticationFunctions;
}

static inline void teardown_authentication(void)
{
    SOPC_UserAuthentication_FreeManager(&gAuthenticationManager);
    ck_assert_ptr_null(gAuthenticationManager);
    SOPC_UserAuthentication_FreeManager(&gAuthenticationManagerSelective);
    ck_assert_ptr_null(gAuthenticationManagerSelective);
}

static inline void setup_authorization(void)
{
    gAuthorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    ck_assert_ptr_nonnull(gAuthorizationManager);
    gAuthorizationManagerSelective = SOPC_Calloc(1, sizeof(SOPC_UserAuthorization_Manager));
    ck_assert_ptr_nonnull(gAuthorizationManagerSelective);
    gAuthorizationManagerSelective->pFunctions = &selectiveAuthorizationFunctions;
}

static inline void teardown_authorization(void)
{
    SOPC_UserAuthorization_FreeManager(&gAuthorizationManager);
    ck_assert_ptr_null(gAuthorizationManager);
    SOPC_UserAuthorization_FreeManager(&gAuthorizationManagerSelective);
    ck_assert_ptr_null(gAuthorizationManagerSelective);
}

static inline void create_users_with_authorization(SOPC_UserAuthorization_Manager* authorizationManager,
                                                   SOPC_UserWithAuthorization** userLocalWithAuthorization,
                                                   SOPC_UserWithAuthorization** userAnonymousWithAuthorization,
                                                   SOPC_UserWithAuthorization** userUsernameWithAuthorization)
{
    ck_assert_ptr_nonnull(userLocalWithAuthorization);
    ck_assert_ptr_nonnull(userAnonymousWithAuthorization);
    ck_assert_ptr_nonnull(userUsernameWithAuthorization);
    *userLocalWithAuthorization = SOPC_UserWithAuthorization_CreateLocal(authorizationManager);
    *userAnonymousWithAuthorization =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(&anonymousIdentityToken, authorizationManager);
    *userUsernameWithAuthorization =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(&usernameIdentityToken, authorizationManager);
    ck_assert_ptr_nonnull(*userLocalWithAuthorization);
    ck_assert_ptr_nonnull(*userAnonymousWithAuthorization);
    ck_assert_ptr_nonnull(*userUsernameWithAuthorization);
}

START_TEST(test_user)
{
    const SOPC_User* user_local = SOPC_User_GetLocal();
    const SOPC_User* user_anonymous = SOPC_User_GetAnonymous();

    ck_assert_ptr_nonnull(user_local);
    ck_assert(SOPC_User_IsLocal(user_local));
    ck_assert(!SOPC_User_IsAnonymous(user_local));
    ck_assert(!SOPC_User_IsUsername(user_local));

    ck_assert_ptr_nonnull(user_anonymous);
    ck_assert(!SOPC_User_IsLocal(user_anonymous));
    ck_assert(SOPC_User_IsAnonymous(user_anonymous));
    ck_assert(!SOPC_User_IsUsername(user_anonymous));

    ck_assert(!SOPC_User_IsLocal(gUser));
    ck_assert(!SOPC_User_IsAnonymous(gUser));
    ck_assert(SOPC_User_IsUsername(gUser));

    SOPC_String username;
    ck_assert(SOPC_STATUS_OK == SOPC_String_InitializeFromCString(&username, USERNAME));
    const SOPC_String* new_username = SOPC_User_GetUsername(gUser);
    ck_assert_ptr_nonnull(new_username);
    ck_assert(SOPC_String_Equal(&username, new_username));
    SOPC_String_Clear(&username);
}
END_TEST

START_TEST(test_user_with_authorization)
{
    SOPC_UserWithAuthorization* userLocalWithAuthorization = NULL;
    SOPC_UserWithAuthorization* userAnonymousWithAuthorization = NULL;
    SOPC_UserWithAuthorization* userUsernameWithAuthorization = NULL;
    create_users_with_authorization(gAuthorizationManager, &userLocalWithAuthorization, &userAnonymousWithAuthorization,
                                    &userUsernameWithAuthorization);

    /* Local user with authorization */
    ck_assert_ptr_eq(gAuthorizationManager, SOPC_UserWithAuthorization_GetManager(userLocalWithAuthorization));
    const SOPC_User* user = SOPC_UserWithAuthorization_GetUser(userLocalWithAuthorization);
    ck_assert(SOPC_User_IsLocal(user));
    ck_assert(!SOPC_User_IsAnonymous(user));
    ck_assert(!SOPC_User_IsUsername(user));

    /* User with authorization from anonymous identity token */
    user = SOPC_UserWithAuthorization_GetUser(userAnonymousWithAuthorization);
    ck_assert(!SOPC_User_IsLocal(user));
    ck_assert(SOPC_User_IsAnonymous(user));
    ck_assert(!SOPC_User_IsUsername(user));

    /* User with authorization from username identity token */
    user = SOPC_UserWithAuthorization_GetUser(userUsernameWithAuthorization);
    ck_assert(!SOPC_User_IsLocal(user));
    ck_assert(!SOPC_User_IsAnonymous(user));
    ck_assert(SOPC_User_IsUsername(user));

    SOPC_String username;
    ck_assert(SOPC_STATUS_OK == SOPC_String_InitializeFromCString(&username, USERNAME));
    const SOPC_String* new_username = SOPC_User_GetUsername(user);
    ck_assert_ptr_nonnull(new_username);
    ck_assert(SOPC_String_Equal(&username, new_username));
    SOPC_String_Clear(&username);

    SOPC_UserWithAuthorization_Free(&userLocalWithAuthorization);
    SOPC_UserWithAuthorization_Free(&userAnonymousWithAuthorization);
    SOPC_UserWithAuthorization_Free(&userUsernameWithAuthorization);
}
END_TEST

START_TEST(test_authentication_allow_all)
{
    SOPC_UserAuthentication_Status authenticated = SOPC_USER_AUTHENTICATION_ACCESS_DENIED;

#define TEST_AUTHN(token, expected)                                                                        \
    authenticated = SOPC_USER_AUTHENTICATION_INVALID_TOKEN;                                                \
    ck_assert(SOPC_STATUS_OK ==                                                                            \
              SOPC_UserAuthentication_IsValidUserIdentity(gAuthenticationManager, token, &authenticated)); \
    ck_assert(authenticated == expected);

    TEST_AUTHN(&anonymousIdentityToken, SOPC_USER_AUTHENTICATION_OK)
    TEST_AUTHN(&usernameIdentityToken, SOPC_USER_AUTHENTICATION_OK)
    TEST_AUTHN(&usernameIdentityToken_invalid, SOPC_USER_AUTHENTICATION_OK)

#undef TEST_AUTHN
}
END_TEST

START_TEST(test_authentication_selective)
{
    SOPC_UserAuthentication_Status authenticated = SOPC_USER_AUTHENTICATION_ACCESS_DENIED;

#define TEST_AUTHN(token, expected)                                                                                 \
    authenticated = SOPC_USER_AUTHENTICATION_INVALID_TOKEN;                                                         \
    ck_assert(SOPC_STATUS_OK ==                                                                                     \
              SOPC_UserAuthentication_IsValidUserIdentity(gAuthenticationManagerSelective, token, &authenticated)); \
    ck_assert(authenticated == expected);

    TEST_AUTHN(&anonymousIdentityToken, SOPC_USER_AUTHENTICATION_REJECTED_TOKEN)
    TEST_AUTHN(&usernameIdentityToken, SOPC_USER_AUTHENTICATION_OK)
    TEST_AUTHN(&usernameIdentityToken_invalid, SOPC_USER_AUTHENTICATION_REJECTED_TOKEN)

#undef TEST_AUTHN
}
END_TEST

START_TEST(test_authorization_allow_all)
{
    bool authorized = false;
    SOPC_UserWithAuthorization* userLocal = NULL;
    SOPC_UserWithAuthorization* userAnonymous = NULL;
    SOPC_UserWithAuthorization* userUsername = NULL;
    create_users_with_authorization(gAuthorizationManager, &userLocal, &userAnonymous, &userUsername);

#define TEST_AUTHZ(user, operation, nid, attribute, expected)                                              \
    authorized = !expected;                                                                                \
    ck_assert(SOPC_STATUS_OK ==                                                                            \
              SOPC_UserAuthorization_IsAuthorizedOperation(user, operation, nid, attribute, &authorized)); \
    ck_assert(authorized == expected);

    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               true)

    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               true)

    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               true)

#undef TEST_AUTHZ

    SOPC_UserWithAuthorization_Free(&userLocal);
    SOPC_UserWithAuthorization_Free(&userAnonymous);
    SOPC_UserWithAuthorization_Free(&userUsername);
}
END_TEST

START_TEST(test_authorization_selective)
{
    bool authorized = false;
    SOPC_UserWithAuthorization* userLocal = NULL;
    SOPC_UserWithAuthorization* userAnonymous = NULL;
    SOPC_UserWithAuthorization* userUsername = NULL;
    create_users_with_authorization(gAuthorizationManagerSelective, &userLocal, &userAnonymous, &userUsername);

#define TEST_AUTHZ(user, operation, nid, attribute, expected)                                              \
    authorized = !expected;                                                                                \
    ck_assert(SOPC_STATUS_OK ==                                                                            \
              SOPC_UserAuthorization_IsAuthorizedOperation(user, operation, nid, attribute, &authorized)); \
    ck_assert(authorized == expected);

    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               false)
    TEST_AUTHZ(userLocal, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)

    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               false)
    TEST_AUTHZ(userAnonymous, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)

    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_READ, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_BROWSENAME, false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &authorizedNodeId, ATTRIBUTEID_VALUE, true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &unauthorizedNodeId, ATTRIBUTEID_ACCESSLEVEL,
               false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               true)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &authorizedNodeId, ATTRIBUTEID_VALUE, false)
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_EXECUTABLE,
               false);
    TEST_AUTHZ(userUsername, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &unauthorizedNodeId, ATTRIBUTEID_BROWSENAME,
               false)

#undef TEST_AUTHZ

    SOPC_UserWithAuthorization_Free(&userLocal);
    SOPC_UserWithAuthorization_Free(&userAnonymous);
    SOPC_UserWithAuthorization_Free(&userUsername);
}
END_TEST

Suite* tests_make_suite_users(void)
{
    Suite* s = NULL;
    TCase* tc_user = NULL;
    TCase* tc_authentication = NULL;
    TCase* tc_authorization = NULL;

    s = suite_create("User, authentication, and authorization");
    tc_user = tcase_create("User");
    tc_authentication = tcase_create("Authentication");
    tc_authorization = tcase_create("Authorization");

    suite_add_tcase(s, tc_user);
    tcase_add_checked_fixture(tc_user, setup_user, teardown_user);
    tcase_add_checked_fixture(tc_user, setup_authorization, teardown_authorization);
    tcase_add_test(tc_user, test_user);
    tcase_add_test(tc_user, test_user_with_authorization);

    tcase_add_checked_fixture(tc_authentication, setup_authentication, teardown_authentication);
    tcase_add_test(tc_authentication, test_authentication_allow_all);
    tcase_add_test(tc_authentication, test_authentication_selective);
    suite_add_tcase(s, tc_authentication);

    tcase_add_checked_fixture(tc_authorization, setup_user, teardown_user);
    tcase_add_checked_fixture(tc_authorization, setup_authorization, teardown_authorization);
    tcase_add_test(tc_authorization, test_authorization_allow_all);
    tcase_add_test(tc_authorization, test_authorization_selective);
    /* TODO: UserAccessLevel and UserWriteMask */
    suite_add_tcase(s, tc_authorization);

    return s;
}
