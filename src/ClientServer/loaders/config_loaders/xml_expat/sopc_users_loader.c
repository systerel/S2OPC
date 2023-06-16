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

#if !defined(SOPC_WITH_EXPAT) || SOPC_WITH_EXPAT

#include "sopc_users_loader.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_hash_based_crypto.h"
#include "sopc_helper_encode.h"
#include "sopc_helper_expat.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_types.h"

/* Default name for rejected token with wrong username (to avoid timing attack) */
#define REJECTED_USER "rejectedUser"
/* the value used to fill the memory for the salt and hash */
#define BYTE_REJECTED_USER 0xA5

typedef enum
{
    PARSE_START,                      // Beginning of file
    PARSE_S2OPC_USERS,                // In a S2OPC_Users
    PARSE_ANONYMOUS,                  // ..In a Anonymous tag
    PARSE_USERPASSWORD_CONFIGURATION, // ..In UserPasswordConfiguration tag
    PARSE_USERPASSWORD,               // ....In a UserPassword tag
    PARSE_USERAUTHORIZATION,          // ......In a UserAuthorization tag
    PARSE_USERCERTIFICATES,           // ..In userCertificates
    PARSE_TRUSTED_ISSUERS,            // ....In a TrustedIssuers tag
    PARSE_TRUSTED_ISSUER,             // ......In a TrustedIssuer tag
    PARSE_ISSUED_CERTS,               // ....In a IssuedCertificates tag
    PARSE_ISSUED_CERT,                // ......In a IssuedCertificate tag
    PARSE_UNTRUSTED_ISSUERS,          // ....In a UntrustedIssuers tag
    PARSE_UNTRUSTED_ISSUER,           // ......In a UntrustedIssuer tag
} parse_state_t;

typedef struct user_rights
{
    bool read;
    bool write;
    bool exec;
    bool addnode;
} user_rights;

typedef struct user_password
{
    SOPC_String user;
    SOPC_ByteString hash;
    SOPC_ByteString salt;
    size_t iteration_count;
    user_rights rights; // mask of SOPC_UserAuthorization_OperationType

} user_password;

typedef struct user_cert
{
    SOPC_String certThumb;
    user_rights rights; // mask of SOPC_UserAuthorization_OperationType
} user_cert;

typedef struct _SOPC_UsersConfig
{
    SOPC_PKIProvider* pX509_UserIdentity_PKI;
    SOPC_Dict* users;
    SOPC_Dict*
        certificates; // dict{key = hexadecimal certificate thumbprint as SOPC_String; value = user_cert structure}
    user_rights anonRights;
    user_password* rejectedUser;
    user_rights defaultCertRights; // rigth for accepted issued (not configured / not trusted)
} SOPC_UsersConfig;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    SOPC_Dict* users;
    SOPC_Dict*
        certificates; // dict{key = hexadecimal certificate thumbprint as SOPC_String; value = user_cert structure}

    bool currentAnonymous;
    bool hasAnonymous;
    user_rights anonymousRights;

    bool userCertSet;
    user_rights defaultCertRights;
    bool trustedIssuersSet;
    SOPC_Array* trustedRootIssuers;
    SOPC_Array* trustedIntermediateIssuers;
    bool issuedCertificatesSet;
    SOPC_Array* issuedCertificates;
    bool untrustedIssuersSet;
    SOPC_Array* untrustedRootIssuers;
    SOPC_Array* untrustedIntermediateIssuers;
    bool crlSet;
    SOPC_Array* crlCertificates;

    char** trustedRootIssuersList;
    char** trustedIntermediateIssuersList;
    char** issuedCertificatesList;
    char** untrustedRootIssuersList;
    char** untrustedIntermediateIssuersList;
    char** certificateRevocationList;

    user_password* currentUserPassword;
    bool usrPwdCfgSet;
    size_t hashIterationCount;
    size_t hashLength;
    size_t saltLength;

    user_cert* currentCert;
    parse_state_t state;
};

#define NS_SEPARATOR "|"
#define NS(ns, tag) ns NS_SEPARATOR tag

SOPC_ReturnStatus generate_fixed_hash_or_salt(SOPC_ByteString* toGen, size_t length);
static SOPC_ReturnStatus set_default_password_hash(user_password** up,
                                                   size_t hashLength,
                                                   size_t saltLength,
                                                   size_t iterationCount);

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
    SOPC_Dict_Get(ctx->users, (uintptr_t) &ctx->currentUserPassword->user, &found);
    if (found)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Duplicated user name %s found in XML",
                       SOPC_String_GetRawCString(&ctx->currentUserPassword->user));
        return false;
    }

    bool res =
        SOPC_Dict_Insert(ctx->users, (uintptr_t) &ctx->currentUserPassword->user, (uintptr_t) ctx->currentUserPassword);
    if (!res)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    // Reset current user
    ctx->currentUserPassword = NULL;

    return true;
}

static bool get_decode_buffer(const char* buffer,
                              bool base64,
                              size_t expected_length,
                              SOPC_ByteString* out,
                              XML_Parser parser)
{
    SOPC_ASSERT(NULL != buffer);
    size_t outLen = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Get and check the output length from the encoding input format
    if (!base64)
    {
        outLen = strlen(buffer);
        if (0 != outLen % 2)
        {
            LOG_XML_ERROR(parser, "Hash hex format must be a multiple of 2 bytes");
            return false;
        }
        outLen = outLen / 2;
    }
    else
    {
        outLen = strlen(buffer);
        if (0 != outLen % 4)
        {
            LOG_XML_ERROR(parser, "Hash base64 format must be a multiple of 4 bytes");
            return false;
        }
        size_t paddingLength = 0;
        status = SOPC_HelperDecode_Base64_GetPaddingLength(buffer, &paddingLength);
        if (SOPC_STATUS_OK != status)
        {
            LOG_XML_ERROR(parser, "Invalid padding for base64 encoding");
            return false;
        }
        outLen = (3 * (outLen / 4)) - paddingLength;
    }
    if (expected_length != outLen)
    {
        LOG_XML_ERROR(parser, "Hash length is not the same as the global configuration");
        return false;
    }
    // Decode
    SOPC_ByteString_Initialize(out);
    out->Data = SOPC_Malloc(sizeof(SOPC_Byte) * outLen);
    out->Length = (int32_t) outLen;
    if (NULL == out->Data)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    if (base64)
    {
        // TODO: Update SOPC_HelperDecode_Base64. This feature is disable for now (see ticket #1057)
        status = SOPC_HelperDecode_Base64(buffer, out->Data, &outLen);
    }
    else
    {
        status = SOPC_HelperDecode_Hex(buffer, out->Data, outLen);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(out->Data);
    }

    return SOPC_STATUS_OK == status;
}

static bool get_hash(struct parse_context_t* ctx, const XML_Char** attrs, bool base64)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "hash", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no password defined");
        return false;
    }
    // TODO: Add base64 encoding feature for hash (see ticket #1057)
    (void) base64;
    bool res =
        get_decode_buffer(attr_val, false, ctx->hashLength, &ctx->currentUserPassword->hash, ctx->helper_ctx.parser);
    return res;
}

static bool get_salt(struct parse_context_t* ctx, const XML_Char** attrs, bool base64)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "salt", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no salt defined");
        return false;
    }
    // TODO: Add base64 encoding feature for salt (see ticket #1057)
    (void) base64;
    bool res =
        get_decode_buffer(attr_val, false, ctx->saltLength, &ctx->currentUserPassword->salt, ctx->helper_ctx.parser);
    return res;
}

static bool start_user_password_configuration(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "hash_iteration_count", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no iteration count defined");
        return false;
    }

    SOPC_ReturnStatus status = SOPC_strtouint32_t(attr_val, (uint32_t*) &ctx->hashIterationCount, 10, '\0');
    if (SOPC_STATUS_OK != status)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "iteration count is not an integer");
        return false;
    }

    if (0 == ctx->hashIterationCount)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "iteration count is equal to zero");
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "hash_length", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no hash length defined");
        return false;
    }

    status = SOPC_strtouint32_t(attr_val, (uint32_t*) &ctx->hashLength, 10, '\0');
    if (SOPC_STATUS_OK != status)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "hash length is not an integer");
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "salt_length", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no salt length defined");
        return false;
    }

    status = SOPC_strtouint32_t(attr_val, (uint32_t*) &ctx->saltLength, 10, '\0');
    if (SOPC_STATUS_OK != status)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "salt length is not an integer");
        return false;
    }

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
    ctx->currentUserPassword->iteration_count = ctx->hashIterationCount;
    SOPC_String_Initialize(&ctx->currentUserPassword->user);
    SOPC_ByteString_Initialize(&ctx->currentUserPassword->hash);
    SOPC_ByteString_Initialize(&ctx->currentUserPassword->salt);

    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "user", attrs);
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

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "base64", attrs);
    bool base64 = attr_val != NULL && 0 == strcmp(attr_val, "true");

    bool res = get_hash(ctx, attrs, base64);
    if (res)
    {
        res = get_salt(ctx, attrs, base64);
    }
    return res;
}

static bool start_authorization(struct parse_context_t* ctx, const XML_Char** attrs, user_rights* rights)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "read", attrs);
    rights->read = attr_val != NULL && 0 == strcmp(attr_val, "true");

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "write", attrs);
    rights->write = attr_val != NULL && 0 == strcmp(attr_val, "true");

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "execute", attrs);
    rights->exec = attr_val != NULL && 0 == strcmp(attr_val, "true");

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "addnode", attrs);
    rights->addnode = attr_val != NULL && 0 == strcmp(attr_val, "true");

    return true;
}

static bool start_issuer(struct parse_context_t* ctx,
                         const XML_Char** attrs,
                         SOPC_Array* rootIssuers,
                         SOPC_Array* IntermediateIssuers)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "root", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "root attribute missing in Issuer definition");
        return false;
    }

    bool isRoot = (strcmp(attr_val, "true") == 0);

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "cert_path", attrs);

    char* pathCA = SOPC_strdup(attr_val);

    if (pathCA == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Issuer: no path defined");
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (0 == strlen(pathCA))
    {
        SOPC_Free(pathCA);
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Issuer: empty path is forbidden");
        return false;
    }

    SOPC_Array* issuers = isRoot ? rootIssuers : IntermediateIssuers;

    if (!SOPC_Array_Append(issuers, pathCA))
    {
        SOPC_Free(pathCA);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "revocation_list_path", attrs);

    char* pathCRL = SOPC_strdup(attr_val);

    if (pathCRL == NULL)
    {
        LOGF("Warning: CRL missing for the root certificate '%s'", pathCA);
    }
    else if (!SOPC_Array_Append(ctx->crlCertificates, pathCRL))
    {
        SOPC_Free(pathCRL);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool start_trusted_issuer(struct parse_context_t* ctx, const XML_Char** attrs)
{
    bool result = start_issuer(ctx, attrs, ctx->trustedRootIssuers, ctx->trustedIntermediateIssuers);

    ctx->state = PARSE_TRUSTED_ISSUER;

    return result;
}

static bool end_trusted_issuers(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->trustedRootIssuers))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no trusted root CA defined");
        return false;
    }
    ctx->trustedIssuersSet = true;

    return true;
}

static bool start_untrusted_issuer(struct parse_context_t* ctx, const XML_Char** attrs)
{
    bool result = start_issuer(ctx, attrs, ctx->untrustedRootIssuers, ctx->untrustedIntermediateIssuers);

    ctx->state = PARSE_UNTRUSTED_ISSUER;

    return result;
}

static bool end_untrusted_issuers(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->untrustedRootIssuers) && 0 == SOPC_Array_Size(ctx->untrustedIntermediateIssuers))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no untrusted CA defined");
        return false;
    }

    ctx->untrustedIssuersSet = true;

    return true;
}

static bool set_cert_authorization(struct parse_context_t* ctx, const XML_Char** attrs, const char* path)
{
    SOPC_CertificateList* pCert = NULL;
    char* thumbprint = NULL;
    ctx->currentCert = SOPC_Calloc(1, sizeof(user_cert));
    bool ret = false;
    if (NULL == ctx->currentCert)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return ret;
    }

    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(path, &pCert);
    if (SOPC_STATUS_OK != status)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "IssuedCertificate: Enable to load user certificate from path %s", path);
    }

    if (SOPC_STATUS_OK == status)
    {
        thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pCert);
        if (NULL != thumbprint)
        {
            status = SOPC_String_InitializeFromCString(&ctx->currentCert->certThumb, thumbprint);
        }

        if (NULL == thumbprint || SOPC_STATUS_OK != status)
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser,
                           "IssuedCertificate: Enable to get certificate thumbprint from path %s", path);
            SOPC_String_Clear(&ctx->currentCert->certThumb);
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        ret = true;
    }

    SOPC_Free(thumbprint);
    SOPC_KeyManager_Certificate_Free(pCert);

    start_authorization(ctx, attrs, &ctx->currentCert->rights);
    return ret;
}

static bool start_issued_cert(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "IssuedCertificate: no path defined");
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (0 == strlen(path))
    {
        SOPC_Free(path);
        LOG_XML_ERROR(ctx->helper_ctx.parser, "IssuedCertificate: empty path is forbidden");
        return false;
    }

    if (!SOPC_Array_Append(ctx->issuedCertificates, path))
    {
        SOPC_Free(path);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    bool res = set_cert_authorization(ctx, attrs, path);

    ctx->state = PARSE_ISSUED_CERT;

    return res;
}

static bool end_issued_cert(struct parse_context_t* ctx)
{
    if (ctx->currentCert)
    {
        bool found = false;
        SOPC_Dict_Get(ctx->certificates, (uintptr_t) &ctx->currentCert->certThumb, &found);
        if (found)
        {
            const char* thumb = SOPC_String_GetRawCString(&ctx->currentCert->certThumb);
            LOG_XML_ERRORF(ctx->helper_ctx.parser,
                           "Duplicated Issued certificate with SHA-1 thumbprint %s found in XML", thumb);
            return false;
        }

        bool res =
            SOPC_Dict_Insert(ctx->certificates, (uintptr_t) &ctx->currentCert->certThumb, (uintptr_t) ctx->currentCert);
        if (!res)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
        // Reset current user
        ctx->currentCert = NULL;
    }

    return true;
}

static bool end_issued_certs(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->issuedCertificates))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no issued certificates defined");
        return false;
    }

    ctx->issuedCertificatesSet = true;

    return true;
}

static bool end_user_certificates(struct parse_context_t* ctx)
{
    if (!SOPC_Array_Append_Values(ctx->trustedRootIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->trustedRootIssuersList = SOPC_Array_Into_Raw(ctx->trustedRootIssuers);
    if (NULL == ctx->trustedRootIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->trustedRootIssuers = NULL;

    if (!SOPC_Array_Append_Values(ctx->trustedIntermediateIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->trustedIntermediateIssuersList = SOPC_Array_Into_Raw(ctx->trustedIntermediateIssuers);
    if (NULL == ctx->trustedIntermediateIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->trustedIntermediateIssuers = NULL;

    if (!SOPC_Array_Append_Values(ctx->issuedCertificates, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->issuedCertificatesList = SOPC_Array_Into_Raw(ctx->issuedCertificates);
    if (NULL == ctx->issuedCertificatesList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->issuedCertificates = NULL;

    if (!SOPC_Array_Append_Values(ctx->untrustedRootIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->untrustedRootIssuersList = SOPC_Array_Into_Raw(ctx->untrustedRootIssuers);
    if (NULL == ctx->untrustedRootIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->untrustedRootIssuers = NULL;

    if (!SOPC_Array_Append_Values(ctx->untrustedIntermediateIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->untrustedIntermediateIssuersList = SOPC_Array_Into_Raw(ctx->untrustedIntermediateIssuers);
    if (NULL == ctx->untrustedIntermediateIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->untrustedIntermediateIssuers = NULL;

    if (!SOPC_Array_Append_Values(ctx->crlCertificates, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->certificateRevocationList = SOPC_Array_Into_Raw(ctx->crlCertificates);
    if (NULL == ctx->certificateRevocationList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->crlCertificates = NULL;

    if (!ctx->issuedCertificatesSet && !ctx->trustedIssuersSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser,
                      "If application certificates section is defined, at least one issued certificate or trusted CA "
                      "should be define.");
        return false;
    }

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
        if (0 == strcmp(name, "UserPasswordConfiguration") && !ctx->usrPwdCfgSet)
        {
            ctx->usrPwdCfgSet = true;
            if (!start_user_password_configuration(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_USERPASSWORD_CONFIGURATION;
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
        else if (0 == strcmp(name, "UserCertificates") && !ctx->userCertSet)
        {
            ctx->userCertSet = true;
            ctx->state = PARSE_USERCERTIFICATES;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    case PARSE_USERCERTIFICATES:
        if (0 == strcmp(name, "TrustedIssuers") && !ctx->trustedIssuersSet)
        {
            if (!start_authorization(ctx, attrs, &ctx->defaultCertRights))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_TRUSTED_ISSUERS;
        }
        else if (0 == strcmp(name, "IssuedCertificates") && !ctx->issuedCertificatesSet)
        {
            ctx->state = PARSE_ISSUED_CERTS;
        }
        else if (0 == strcmp(name, "UntrustedIssuers") && !ctx->untrustedIssuersSet)
        {
            ctx->state = PARSE_UNTRUSTED_ISSUERS;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_TRUSTED_ISSUERS:
        if (0 == strcmp(name, "TrustedIssuer"))
        {
            if (!start_trusted_issuer(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_ISSUED_CERTS:
        if (0 == strcmp(name, "IssuedCertificate"))
        {
            if (!start_issued_cert(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_UNTRUSTED_ISSUERS:
        if (0 == strcmp(name, "UntrustedIssuer"))
        {
            if (!start_untrusted_issuer(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_USERPASSWORD_CONFIGURATION:
        if (0 == strcmp(name, "UserPassword"))
        {
            if (!start_userpassword(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_USERPASSWORD;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    case PARSE_ANONYMOUS:
        if (0 != strcmp(name, "UserAuthorization"))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        if (!start_authorization(ctx, attrs, &ctx->anonymousRights))
        {
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        ctx->state = PARSE_USERAUTHORIZATION;
        break;
    case PARSE_USERPASSWORD:
        if (0 != strcmp(name, "UserAuthorization"))
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        SOPC_ASSERT(NULL != ctx->currentUserPassword);
        if (!start_authorization(ctx, attrs, &ctx->currentUserPassword->rights))
        {
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        ctx->state = PARSE_USERAUTHORIZATION;
        break;
    case PARSE_USERAUTHORIZATION:
        break;
    default:
        SOPC_ASSERT(false && "Unknown state.");
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
            SOPC_ASSERT(NULL != ctx->currentUserPassword);
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
        ctx->state = PARSE_USERPASSWORD_CONFIGURATION;
        break;
    case PARSE_USERPASSWORD_CONFIGURATION:
        ctx->state = PARSE_S2OPC_USERS;
        break;
    case PARSE_TRUSTED_ISSUER:
        ctx->state = PARSE_TRUSTED_ISSUERS;
        break;
    case PARSE_TRUSTED_ISSUERS:
        if (!end_trusted_issuers(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_USERCERTIFICATES;
        break;
    case PARSE_ISSUED_CERT:
        if (!end_issued_cert(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_ISSUED_CERTS;
        break;
    case PARSE_ISSUED_CERTS:
        if (!end_issued_certs(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_USERCERTIFICATES;
        break;
    case PARSE_UNTRUSTED_ISSUER:
        ctx->state = PARSE_UNTRUSTED_ISSUERS;
        break;
    case PARSE_UNTRUSTED_ISSUERS:
        if (!end_untrusted_issuers(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_USERCERTIFICATES;
        break;
    case PARSE_USERCERTIFICATES:
        if (!end_user_certificates(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_S2OPC_USERS;
        break;
    case PARSE_S2OPC_USERS:
        break;
    case PARSE_START:
        SOPC_ASSERT(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        SOPC_ASSERT(false && "Unknown state.");
        break;
    }
}

static uint64_t string_hash(const uintptr_t s)
{
    const SOPC_String* str = (SOPC_String*) s;
    return SOPC_DJBHash(str->Data, (size_t) str->Length);
}

static bool string_equal(const uintptr_t a, const uintptr_t b)
{
    return SOPC_String_Equal((const SOPC_String*) a, (const SOPC_String*) b);
}

static void userpassword_free(uintptr_t up)
{
    if (NULL != (void*) up)
    {
        user_password* userpassword = (user_password*) up;

        SOPC_String_Clear(&userpassword->user);
        SOPC_ByteString_Clear(&userpassword->hash);
        SOPC_ByteString_Clear(&userpassword->salt);
        SOPC_Free(userpassword);
    }
}

static void user_cert_free(uintptr_t uc)
{
    if (NULL != (void*) uc)
    {
        user_cert* userCert = (user_cert*) uc;

        SOPC_String_Clear(&userCert->certThumb);
        SOPC_Free(userCert);
    }
}

/**
 * @brief compares two strings, using a constant number of iterations
 * based on \a sCmp length, whatever the result is
 * @param sRef The reference string to compare
 * @param sCmp The string to compare with.
 */
static bool secure_hash_compare(const user_password* sRef, const SOPC_ByteString* sCmp)
{
    SOPC_ASSERT(NULL != sCmp);
    const SOPC_Byte* bCmp = sCmp->Data;
    const int32_t lCmp = sCmp->Length;
    const SOPC_Byte* bRef = (NULL != sRef ? sRef->hash.Data : NULL);
    const int32_t lRef = (NULL != sRef ? sRef->hash.Length : -1);

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

SOPC_ReturnStatus generate_fixed_hash_or_salt(SOPC_ByteString* toGen, size_t length)
{
    if (NULL == toGen || 0 == length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_ByteString_InitializeFixedSize(toGen, (uint32_t) length);
    if (SOPC_STATUS_OK == status)
    {
        memset(toGen->Data, BYTE_REJECTED_USER, length);
    }

    return status;
}

static SOPC_ReturnStatus set_default_password_hash(user_password** up,
                                                   size_t hashLength,
                                                   size_t saltLength,
                                                   size_t iterationCount)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    if (NULL == up)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Allocate the user password structure
    user_password* pwd = SOPC_Malloc(sizeof(user_password) * 1);
    if (NULL == pwd)
    {
        return status;
    }
    // Allocate the default hash
    status = generate_fixed_hash_or_salt(&pwd->hash, hashLength);
    // Allocate the default salt
    if (SOPC_STATUS_OK == status)
    {
        status = generate_fixed_hash_or_salt(&pwd->salt, saltLength);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Allocate the default user
        SOPC_String_Initialize(&pwd->user);
        status = SOPC_String_CopyFromCString((SOPC_String*) &pwd->user, REJECTED_USER);
    }

    // Clear the structure in case of error
    if (SOPC_STATUS_OK != status)
    {
        userpassword_free((uintptr_t) pwd);
    }
    else
    {
        // Set the default iteration count
        pwd->iteration_count = iterationCount;
        *up = pwd;
    }
    return status;
}

static SOPC_ReturnStatus authentication_fct(SOPC_UserAuthentication_Manager* authn,
                                            const SOPC_ExtensionObject* token,
                                            SOPC_UserAuthentication_Status* authenticated)
{
    SOPC_ASSERT(NULL != authn && NULL != authn->pData && NULL != token && NULL != authenticated);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_UsersConfig* config = authn->pData;

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        user_password* up = (user_password*) SOPC_Dict_Get(config->users, (uintptr_t) username, NULL);
        // if rejected Token with wrong username: use default password hash configuration to avoid timing attack (to
        // have a constant time during authentication process)
        if (NULL == up)
        {
            up = config->rejectedUser;
        }

        SOPC_HashBasedCrypto_Config* configHash = NULL;
        SOPC_ReturnStatus status_crypto = SOPC_HashBasedCrypto_Config_Create(&configHash);
        SOPC_ByteString* UserPasswordHash = NULL;

        SOPC_ASSERT(0 < up->hash.Length);
        // Configure the salt and the counter from the XML
        status_crypto =
            SOPC_HashBasedCrypto_Config_PBKDF2(configHash, &up->salt, up->iteration_count, (size_t) up->hash.Length);
        if (SOPC_STATUS_OK == status_crypto)
        {
            // Hash the password issued form the userToken
            status_crypto = SOPC_HashBasedCrypto_Run(configHash, &userToken->Password, &UserPasswordHash);
        }

        // Compare the result
        if (SOPC_STATUS_OK == status_crypto)
        {
            // Note: do not use SOPC_ByteString_Equal for PWD checking, because this may allow an attacker to
            // find expected PWD length, or beginning based on timed attacks.
            // Moreover, the comparison is also done if user does not match, to avoid possible detection of usernames.

            const bool pwd_match = secure_hash_compare(up, UserPasswordHash);

            // Check password
            if (pwd_match)
            {
                SOPC_ASSERT(NULL != up);
                // Check user access
                if (up->rights.read || up->rights.write || up->rights.exec || up->rights.addnode)
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

        status = status_crypto;
        SOPC_ByteString_Delete(UserPasswordHash);
        SOPC_HashBasedCrypto_Config_Free(configHash);
    }

    if (&OpcUa_X509IdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        const SOPC_PKIProvider* pkiProvider = config->pX509_UserIdentity_PKI;
        OpcUa_X509IdentityToken* x509Token = token->Body.Object.Value;
        SOPC_ByteString* rawCert = &x509Token->CertificateData;
        SOPC_CertificateList* pUserCert = NULL;
        SOPC_StatusCode errorStatus;

        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(rawCert->Data, (uint32_t) rawCert->Length, &pUserCert);

        if (SOPC_STATUS_OK == status)
        {
            status = pkiProvider->pFnValidateCertificate(pkiProvider, pUserCert, &errorStatus);
            if (SOPC_STATUS_OK == status)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
                /* UACTT expected BadIdentityTokenRejected */
                *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
                char* tpr = SOPC_KeyManager_Certificate_GetCstring_SHA1(pUserCert);
                if (NULL == tpr)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "authentication: Validation of User Certificate failed with error: %X",
                                           errorStatus);
                }
                else
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "authentication: Validation of User Certificate with SHA-1 thumbprint %s failed with error: %X",
                        tpr, errorStatus);
                    SOPC_Free(tpr);
                }
            }
            /* The certificate validation failed but not the authentication function itself*/
            status = SOPC_STATUS_OK;
        }

        /* Clear */
        SOPC_KeyManager_Certificate_Free(pUserCert);
    }
    return status;
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
    SOPC_ASSERT(NULL != authorizationManager && NULL != authorizationManager->pData);
    SOPC_ASSERT(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = false;
    SOPC_UsersConfig* config = authorizationManager->pData;

    if (SOPC_User_IsUsername(pUser))
    {
        // Authorize some users to write or execute methods
        const SOPC_String* username = SOPC_User_GetUsername(pUser);
        bool found = false;
        user_password* up = (user_password*) SOPC_Dict_Get(config->users, (uintptr_t) username, &found);
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
            case SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE:
                *pbOperationAuthorized = up->rights.addnode;
                break;
            default:
                SOPC_ASSERT(false && "Unknown operation type.");
                break;
            }
        }
    }
    else if (SOPC_User_IsCertificate(pUser))
    {
        const SOPC_String* certThumb = SOPC_User_GetCertificate_Thumbprint(pUser);
        user_rights* userRights = NULL;
        bool found = false;
        user_cert* pUserCert = (user_cert*) SOPC_Dict_Get(config->certificates, (uintptr_t) certThumb, &found);
        if (found)
        {
            userRights = &pUserCert->rights;
        }
        else
        {
            /* Default certificate authorization for accepted issued (not configured) evaluated as trustworthy according
             * to trust chain */
            userRights = &config->defaultCertRights;
        }

        switch (operationType)
        {
        case SOPC_USER_AUTHORIZATION_OPERATION_READ:
            *pbOperationAuthorized = userRights->read;
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
            *pbOperationAuthorized = userRights->write;
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
            *pbOperationAuthorized = userRights->exec;
            break;
        case SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE:
            *pbOperationAuthorized = userRights->addnode;
            break;
        default:
            SOPC_ASSERT(false && "Unknown operation type.");
            break;
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
        case SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE:
            *pbOperationAuthorized = config->anonRights.addnode;
            break;
        default:
            SOPC_ASSERT(false && "Unknown operation type.");
            break;
        }
    }

    return SOPC_STATUS_OK;
}

static void UserAuthentication_Free(SOPC_UserAuthentication_Manager* authentication)
{
    if (NULL != authentication)
    {
        if (NULL != authentication->pData)
        {
            SOPC_UsersConfig* config = (SOPC_UsersConfig*) authentication->pData;
            SOPC_Dict_Delete(config->users);
            userpassword_free((uintptr_t) config->rejectedUser);
            SOPC_Dict_Delete(config->certificates);
            SOPC_PKIProvider_Free(&config->pX509_UserIdentity_PKI);
            SOPC_Free(authentication->pData);
        }
        SOPC_Free(authentication);
    }
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

static void SOPC_Free_CstringFromPtr(void* data)
{
    if (NULL != data)
    {
        SOPC_Free(*(char**) data);
    }
}

static void sopc_free_certificate_lists(struct parse_context_t* ctx)
{
    for (int i = 0; NULL != ctx->trustedRootIssuersList && NULL != ctx->trustedRootIssuersList[i]; i++)
    {
        SOPC_Free(ctx->trustedRootIssuersList[i]);
    }
    SOPC_Free(ctx->trustedRootIssuersList);

    for (int i = 0; NULL != ctx->trustedIntermediateIssuersList && NULL != ctx->trustedIntermediateIssuersList[i]; i++)
    {
        SOPC_Free(ctx->trustedIntermediateIssuersList[i]);
    }
    SOPC_Free(ctx->trustedIntermediateIssuersList);

    for (int i = 0; NULL != ctx->issuedCertificatesList && NULL != ctx->issuedCertificatesList[i]; i++)
    {
        SOPC_Free(ctx->issuedCertificatesList[i]);
    }
    SOPC_Free(ctx->issuedCertificatesList);

    for (int i = 0; NULL != ctx->untrustedRootIssuersList && NULL != ctx->untrustedRootIssuersList[i]; i++)
    {
        SOPC_Free(ctx->untrustedRootIssuersList[i]);
    }
    SOPC_Free(ctx->untrustedRootIssuersList);

    for (int i = 0; NULL != ctx->untrustedIntermediateIssuersList && NULL != ctx->untrustedIntermediateIssuersList[i];
         i++)
    {
        SOPC_Free(ctx->untrustedIntermediateIssuersList[i]);
    }
    SOPC_Free(ctx->untrustedIntermediateIssuersList);

    for (int i = 0; NULL != ctx->certificateRevocationList && NULL != ctx->certificateRevocationList[i]; i++)
    {
        SOPC_Free(ctx->certificateRevocationList[i]);
    }
    SOPC_Free(ctx->certificateRevocationList);
}

bool SOPC_UsersConfig_Parse(FILE* fd,
                            SOPC_UserAuthentication_Manager** authentication,
                            SOPC_UserAuthorization_Manager** authorization)
{
    SOPC_ASSERT(NULL != authentication);
    SOPC_ASSERT(NULL != authorization);

    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);
    SOPC_ReturnStatus pki_status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pX509_UserIdentity_PKI = NULL;

    SOPC_Dict* users = SOPC_Dict_Create((uintptr_t) NULL, string_hash, string_equal, NULL, userpassword_free);
    SOPC_Dict* certificates = SOPC_Dict_Create((uintptr_t) NULL, string_hash, string_equal, NULL, user_cert_free);

    SOPC_Array* trustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* trustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* issuedCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* revokedListCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    if ((NULL == users) || (NULL == certificates) || (NULL == trustedRootIssuers) ||
        (NULL == trustedIntermediateIssuers) || (NULL == issuedCerts) || (NULL == untrustedRootIssuers) ||
        (NULL == untrustedIntermediateIssuers) || (NULL == revokedListCerts))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
        SOPC_Dict_Delete(users);
        SOPC_Dict_Delete(certificates);
        SOPC_Array_Delete(trustedRootIssuers);
        SOPC_Array_Delete(trustedIntermediateIssuers);
        SOPC_Array_Delete(issuedCerts);
        SOPC_Array_Delete(untrustedRootIssuers);
        SOPC_Array_Delete(untrustedIntermediateIssuers);
        SOPC_Array_Delete(revokedListCerts);
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
    ctx.anonymousRights = (user_rights){false, false, false, false};
    ctx.currentUserPassword = NULL;
    ctx.usrPwdCfgSet = false;

    ctx.certificates = certificates;
    ctx.defaultCertRights = (user_rights){false, false, false, false};
    ctx.currentCert = NULL;
    ctx.userCertSet = false;

    ctx.trustedIssuersSet = false;
    ctx.trustedRootIssuers = trustedRootIssuers;
    ctx.trustedIntermediateIssuers = trustedIntermediateIssuers;
    ctx.issuedCertificatesSet = false;
    ctx.issuedCertificates = issuedCerts;
    ctx.untrustedIssuersSet = false;
    ctx.untrustedRootIssuers = untrustedRootIssuers;
    ctx.untrustedIntermediateIssuers = untrustedIntermediateIssuers;
    ctx.crlCertificates = revokedListCerts;

    ctx.trustedRootIssuersList = NULL;
    ctx.trustedIntermediateIssuersList = NULL;
    ctx.issuedCertificatesList = NULL;
    ctx.untrustedRootIssuersList = NULL;
    ctx.untrustedIntermediateIssuersList = NULL;
    ctx.certificateRevocationList = NULL;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Array_Delete(ctx.trustedRootIssuers);
    SOPC_Array_Delete(ctx.trustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.issuedCertificates);
    SOPC_Array_Delete(ctx.untrustedRootIssuers);
    SOPC_Array_Delete(ctx.untrustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.crlCertificates);

    if (SOPC_STATUS_OK == res)
    {
        *authentication = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        *authorization = SOPC_Calloc(1, sizeof(SOPC_UserAuthorization_Manager));
        SOPC_UsersConfig* config = SOPC_Calloc(1, sizeof(SOPC_UsersConfig));
        user_password* rejectedUser = NULL;
        res = set_default_password_hash(&rejectedUser, ctx.hashLength, ctx.saltLength, ctx.hashIterationCount);
        if (ctx.userCertSet)
        {
            pki_status = SOPC_PKIProviderStack_CreateFromPaths(
                ctx.trustedRootIssuersList, ctx.trustedIntermediateIssuersList, ctx.untrustedRootIssuersList,
                ctx.untrustedIntermediateIssuersList, ctx.issuedCertificatesList, ctx.certificateRevocationList,
                &pX509_UserIdentity_PKI);
        }
        if (NULL == *authentication || NULL == *authorization || NULL == config || SOPC_STATUS_OK != pki_status ||
            SOPC_STATUS_OK != res)
        {
            SOPC_Free(*authentication);
            *authentication = NULL;
            SOPC_Free(*authorization);
            *authorization = NULL;
            SOPC_PKIProvider_Free(&pX509_UserIdentity_PKI);
            pX509_UserIdentity_PKI = NULL;
            SOPC_Free(config);
            config = NULL;
            userpassword_free((uintptr_t) rejectedUser);
            res = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            /* Set PKI for users */
            SOPC_PKIProviderStack_SetUserCert(pX509_UserIdentity_PKI, true);
            config->pX509_UserIdentity_PKI = pX509_UserIdentity_PKI;
            config->users = users;
            config->rejectedUser = rejectedUser;
            config->certificates = certificates;
            config->defaultCertRights = ctx.defaultCertRights;
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
            SOPC_String_Clear(&ctx.currentUserPassword->user);
            SOPC_ByteString_Clear(&ctx.currentUserPassword->hash);
            SOPC_ByteString_Clear(&ctx.currentUserPassword->salt);
        }
        if (NULL != ctx.currentCert)
        {
            SOPC_String_Delete(&ctx.currentCert->certThumb);
        }
        SOPC_Dict_Delete(users);
        SOPC_Dict_Delete(certificates);
        return false;
    }

    /* Delete pki paths loading from xml */
    sopc_free_certificate_lists(&ctx);
    return true;
}
#endif
