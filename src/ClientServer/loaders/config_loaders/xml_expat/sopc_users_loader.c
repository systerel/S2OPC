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

#include "sopc_users_loader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "sopc_assert.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_helper_expat.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

typedef enum
{
    PARSE_START,             // Beginning of file
    PARSE_S2OPC_USERS,       // In a S2OPC_Users
    PARSE_ANONYMOUS,         // ..In a Anonymous tag
    PARSE_USERPASSWORD,      // ..In a UserPassword tag
    PARSE_USERAUTHORIZATION, // ....In a UserAuthorization tag
} parse_state_t;

typedef struct user_rights
{
    bool read;
    bool write;
    bool exec;
} user_rights;

typedef struct user_password
{
    SOPC_String user;
    SOPC_ByteString password;
    user_rights rights; // mask of SOPC_UserAuthorization_OperationType

} user_password;

typedef struct _SOPC_UsersConfig
{
    SOPC_Dict* users;
    user_rights anonRights;
} SOPC_UsersConfig;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    SOPC_Dict* users;

    bool currentAnonymous;
    bool hasAnonymous;
    user_rights anonymousRights;

    user_password* currentUserPassword;

    parse_state_t state;
};

#define NS_SEPARATOR "|"
#define NS(ns, tag) ns NS_SEPARATOR tag

static SOPC_ReturnStatus parse(XML_Parser parser, FILE* fd)
{
    char buf[65365];

    while (!feof(fd))
    {
        size_t r = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fd);

        if ((r == 0) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            return SOPC_STATUS_NOK;
        }

        if (XML_STATUS_OK != XML_Parse(parser, buf, (int) r, 0))
        {
            enum XML_Error parser_error = XML_GetErrorCode(parser);

            if (XML_ERROR_NONE != parser_error)
            {
                fprintf(stderr, "XML parsing failed at line %lu, column %lu. Error code is %d.\n",
                        XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), XML_GetErrorCode(parser));
            }

            // else, the error comes from one of the callbacks, that log an error
            // themselves.

            return SOPC_STATUS_NOK;
        }
    }

    // Tell the parser that we are at the end of the file
    if (XML_STATUS_OK != XML_Parse(parser, "", 0, 1))
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

static const char* get_attr(struct parse_context_t* ctx, const char* attr_name, const XML_Char** attrs)
{
    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (0 == strcmp(attr_name, attr))
        {
            const char* attr_val = attrs[++i];

            if (NULL == attr_val)
            {
                LOG_XML_ERRORF(ctx->helper_ctx.parser, "Missing value for %s attribute", attr_name);
                return NULL;
            }
            return attr_val;
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }
    return NULL;
}

static bool end_anonymous(struct parse_context_t* ctx)
{
    ctx->currentAnonymous = false;
    return true;
}

static bool start_anonymous(struct parse_context_t* ctx, const XML_Char** attrs)
{
    SOPC_UNUSED_ARG(attrs);

    if (ctx->hasAnonymous)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Duplicated Anonymous tag found in XML");
        return false;
    }
    ctx->hasAnonymous = true;
    ctx->currentAnonymous = true;
    return true;
}

static bool end_userpassword(struct parse_context_t* ctx)
{
    bool found = false;
    SOPC_Dict_Get(ctx->users, &ctx->currentUserPassword->user, &found);
    if (found)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Duplicated user name %s found in XML",
                       SOPC_String_GetRawCString(&ctx->currentUserPassword->user));
        return false;
    }

    bool res = SOPC_Dict_Insert(ctx->users, &ctx->currentUserPassword->user, ctx->currentUserPassword);
    if (!res)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    // Reset current user
    ctx->currentUserPassword = NULL;

    return true;
}

static bool start_userpassword(struct parse_context_t* ctx, const XML_Char** attrs)
{
    ctx->currentUserPassword = SOPC_Calloc(1, sizeof(user_password));
    if (NULL == ctx->currentUserPassword)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    SOPC_String_Initialize(&ctx->currentUserPassword->user);
    SOPC_ByteString_Initialize(&ctx->currentUserPassword->password);

    const char* attr_val = get_attr(ctx, "user", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no username defined");
        return false;
    }
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&ctx->currentUserPassword->user, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    if (ctx->currentUserPassword->user.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "empty username is forbidden");
        return false;
    }

    attr_val = get_attr(ctx, "pwd", attrs);
    status = SOPC_String_CopyFromCString((SOPC_String*) &ctx->currentUserPassword->password, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool start_authorization(struct parse_context_t* ctx, const XML_Char** attrs)
{
    user_rights* rights = NULL;
    if (ctx->currentAnonymous)
    {
        rights = &ctx->anonymousRights;
    }
    else
    {
        assert(NULL != ctx->currentUserPassword);
        rights = &ctx->currentUserPassword->rights;
    }

    const char* attr_val = get_attr(ctx, "read", attrs);
    rights->read = attr_val != NULL && 0 == strcmp(attr_val, "true");

    attr_val = get_attr(ctx, "write", attrs);
    rights->write = attr_val != NULL && 0 == strcmp(attr_val, "true");

    attr_val = get_attr(ctx, "execute", attrs);
    rights->exec = attr_val != NULL && 0 == strcmp(attr_val, "true");

    return true;
}

static void start_element_handler(void* user_data, const XML_Char* name, const XML_Char** attrs)
{
    struct parse_context_t* ctx = user_data;
    SOPC_HelperExpatCtx* helperCtx = &ctx->helper_ctx;

    switch (ctx->state)
    {
    case PARSE_START:
        if (0 != strcmp(name, "S2OPC_Users"))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        ctx->state = PARSE_S2OPC_USERS;
        break;
    case PARSE_S2OPC_USERS:
        if (0 == strcmp(name, "UserPassword"))
        {
            if (!start_userpassword(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_USERPASSWORD;
        }
        else if (0 == strcmp(name, "Anonymous"))
        {
            if (!start_anonymous(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_ANONYMOUS;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    case PARSE_ANONYMOUS:
    case PARSE_USERPASSWORD:
    {
        if (0 != strcmp(name, "UserAuthorization"))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        if (!start_authorization(ctx, attrs))
        {
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        ctx->state = PARSE_USERAUTHORIZATION;

        break;
    }
    case PARSE_USERAUTHORIZATION:
        break;
    default:
        assert(false && "Unknown state.");
        break;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    SOPC_UNUSED_ARG(name);

    struct parse_context_t* ctx = user_data;

    switch (ctx->state)
    {
    case PARSE_USERAUTHORIZATION:
        if (ctx->currentAnonymous)
        {
            ctx->state = PARSE_ANONYMOUS;
        }
        else
        {
            assert(NULL != ctx->currentUserPassword);
            ctx->state = PARSE_USERPASSWORD;
        }
        break;
    case PARSE_ANONYMOUS:
        if (!end_anonymous(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_S2OPC_USERS;
        break;
    case PARSE_USERPASSWORD:
        if (!end_userpassword(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_S2OPC_USERS;
        break;
    case PARSE_S2OPC_USERS:
        break;
    case PARSE_START:
        assert(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        assert(false && "Unknown state.");
        break;
    }
}

static uint64_t username_hash(const void* u)
{
    const SOPC_String* user = u;
    return SOPC_DJBHash(user->Data, (size_t) user->Length);
}

static bool username_equal(const void* a, const void* b)
{
    return SOPC_String_Equal((const SOPC_String*) a, (const SOPC_String*) b);
}

static void userpassword_free(void* up)
{
    if (NULL != up)
    {
        user_password* userpassword = up;

        SOPC_String_Clear(&userpassword->user);
        SOPC_String_Clear(&userpassword->password);
        SOPC_Free(userpassword);
    }
}

/**
 * @brief compares two strings, using a constant number of iterations
 * based on \a sCmp length, whatever the result is
 * @param sRef The reference string to compare
 * @param sCmp The string to compare with.
 */
static bool secure_password_compare(const user_password* sRef, const SOPC_ByteString* sCmp)
{
    SOPC_ASSERT(NULL != sCmp);
    const SOPC_Byte* bCmp = sCmp->Data;
    const int32_t lCmp = sCmp->Length;
    const SOPC_Byte* bRef = (NULL != sRef ? sRef->password.Data : NULL);
    const int32_t lRef = (NULL != sRef ? sRef->password.Length : -1);

    // Using volatile aspect to avoid compiler optimizations and make iteration time
    // most constant as possible for every cases.

    // note : comparing length first would allow compiler to optimize and remove loop.
    // check is done at the end
    volatile bool result = true;

    for (int32_t i = 0; i < lCmp; i++)
    {
        SOPC_Byte b2 = bCmp[i];
        SOPC_Byte b1 = (i < lRef ? bRef[i] : 0);
        if (b1 != b2)
        {
            result = false;
        }
    }
    return result && (lRef == lCmp);
}

static SOPC_ReturnStatus authentication_fct(SOPC_UserAuthentication_Manager* authn,
                                            const SOPC_ExtensionObject* token,
                                            SOPC_UserAuthentication_Status* authenticated)
{
    assert(NULL != authn && NULL != authn->pData && NULL != token && NULL != authenticated);

    SOPC_UsersConfig* config = authn->pData;

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    assert(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        user_password* up = SOPC_Dict_Get(config->users, username, NULL);

        // Note: do not use SOPC_ByteString_Equal for PWD checking, because this may allow an attacker to
        // find expected PWD length, or beginning based on timed attacks.
        // Moreover, the comparison is also done if user does not match, to avoid possible detection of usernames.

        const bool pwd_match = secure_password_compare(up, &userToken->Password);

        // Check password
        if (pwd_match)
        {
            SOPC_ASSERT(NULL != up);
            // Check user access
            if (up->rights.read || up->rights.write || up->rights.exec)
            {
                // At least 1 type of access authorized
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
                // No user access authorized

                /* This value is described by OPC UA part 4 and tested by UACTT
                 * but access evaluation shall be enforced on other services calls
                 * (read, write, callmethod, etc.) */
                *authenticated = SOPC_USER_AUTHENTICATION_ACCESS_DENIED;
            }
        }
    }

    return SOPC_STATUS_OK;
}

/** \brief Authorize R/W/X operation callback */
static SOPC_ReturnStatus authorization_fct(SOPC_UserAuthorization_Manager* authorizationManager,
                                           SOPC_UserAuthorization_OperationType operationType,
                                           const SOPC_NodeId* nodeId,
                                           uint32_t attributeId,
                                           const SOPC_User* pUser,
                                           bool* pbOperationAuthorized)
{
    SOPC_UNUSED_ARG(nodeId);
    SOPC_UNUSED_ARG(attributeId);
    assert(NULL != authorizationManager && NULL != authorizationManager->pData);
    assert(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = false;
    SOPC_UsersConfig* config = authorizationManager->pData;

    if (SOPC_User_IsUsername(pUser))
    {
        // Authorize some users to write or execute methods
        const SOPC_String* username = SOPC_User_GetUsername(pUser);
        bool found = false;
        user_password* up = SOPC_Dict_Get(config->users, username, &found);
        if (found)
        {
            switch (operationType)
            {
            case SOPC_USER_AUTHORIZATION_OPERATION_READ:
                *pbOperationAuthorized = up->rights.read;
                break;
            case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
                *pbOperationAuthorized = up->rights.write;
                break;
            case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
                *pbOperationAuthorized = up->rights.exec;
                break;
            default:
                assert(false && "Unknown operation type.");
                break;
            }
        }
    }
    else if (SOPC_User_IsAnonymous(pUser))
    {
        switch (operationType)
        {
        case SOPC_USER_AUTHORIZATION_OPERATION_READ:
            *pbOperationAuthorized = config->anonRights.read;
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
            *pbOperationAuthorized = config->anonRights.write;
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
            *pbOperationAuthorized = config->anonRights.exec;
            break;
        default:
            assert(false && "Unknown operation type.");
            break;
        }
    }

    return SOPC_STATUS_OK;
}

static void UserAuthentication_Free(SOPC_UserAuthentication_Manager* authentication)
{
    if (NULL != authentication && NULL != authentication->pData)
    {
        SOPC_Dict_Delete(((SOPC_UsersConfig*) authentication->pData)->users);
        SOPC_Free(authentication->pData);
    }
    SOPC_Free(authentication);
}

static void UserAuthorization_Free(SOPC_UserAuthorization_Manager* authorization)
{
    SOPC_Free(authorization);
}

static const SOPC_UserAuthentication_Functions authentication_functions = {
    .pFuncFree = UserAuthentication_Free,
    .pFuncValidateUserIdentity = authentication_fct};

static const SOPC_UserAuthorization_Functions authorization_functions = {.pFuncFree = UserAuthorization_Free,
                                                                         .pFuncAuthorizeOperation = authorization_fct};

bool SOPC_UsersConfig_Parse(FILE* fd,
                            SOPC_UserAuthentication_Manager** authentication,
                            SOPC_UserAuthorization_Manager** authorization)
{
    assert(NULL != authentication);
    assert(NULL != authorization);

    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);

    SOPC_Dict* users = SOPC_Dict_Create(NULL, username_hash, username_equal, NULL, userpassword_free);

    if (NULL == users)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
        SOPC_Dict_Delete(users);
        return false;
    }

    struct parse_context_t ctx;
    memset(&ctx, 0, sizeof(struct parse_context_t));
    XML_SetUserData(parser, &ctx);

    ctx.state = PARSE_START;
    ctx.helper_ctx.parser = parser;
    ctx.users = users;
    ctx.currentAnonymous = false;
    ctx.hasAnonymous = false;
    ctx.anonymousRights = (user_rights){false, false, false};
    ctx.currentUserPassword = NULL;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);

    if (SOPC_STATUS_OK == res)
    {
        *authentication = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        *authorization = SOPC_Calloc(1, sizeof(SOPC_UserAuthorization_Manager));
        SOPC_UsersConfig* config = SOPC_Calloc(1, sizeof(SOPC_UsersConfig));
        if (NULL == *authentication || NULL == *authorization || NULL == config)
        {
            SOPC_Free(*authentication);
            *authentication = NULL;
            SOPC_Free(*authorization);
            *authorization = NULL;
            SOPC_Free(config);
            config = NULL;
            res = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            config->users = users;
            config->anonRights = ctx.anonymousRights;
            (*authentication)->pData = config;
            (*authentication)->pFunctions = &authentication_functions;
            (*authorization)->pData = config;
            (*authorization)->pFunctions = &authorization_functions;
        }
    }

    if (SOPC_STATUS_OK != res)
    {
        if (NULL != ctx.currentUserPassword)
        {
            SOPC_String_Delete(&ctx.currentUserPassword->user);
            SOPC_String_Delete(&ctx.currentUserPassword->password);
        }
        SOPC_Dict_Delete(users);
        return false;
    }

    return true;
}
