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

#include "sopc_config_loader.h"
#include "sopc_config_loader_internal.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_helper_expat.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define SOPC_DEFAULT_REQ_LIFETIME_MS 3600000

typedef enum
{
    PARSE_START,             // Beginning of file
    PARSE_S2OPC,             // In a S2OPC
    PARSE_CLICONFIG,         // ..In a client config tag
    PARSE_PREFERRED_LOCALES, // ....In preferred locales
    PARSE_LOCALE,            // ......In locale
    PARSE_APPLICATION_CERT,  // ....In application certificate
    PARSE_CLIENT_CERT,       // ......Client certificate
    PARSE_CLIENT_KEY,        // ......Client key
    PARSE_TRUSTED_ISSUERS,   // ......Trusted issuer certificates
    PARSE_TRUSTED_ISSUER,    // ........Trusted issuer certificate + crl
    PARSE_ISSUED_CERTS,      // ......Trusted issued certificates
    PARSE_ISSUED_CERT,       // ........Trusted issued certificate
    PARSE_UNTRUSTED_ISSUERS, // ......Untrusted issuer certificates
    PARSE_UNTRUSTED_ISSUER,  // ........Untrusted issuer certificate + crl
    PARSE_APPLICATION_DESC,  // ....In application description
    PARSE_APPLICATION_URI,   // ......Application URI
    PARSE_PRODUCT_URI,       // ......Product URI
    PARSE_APPLICATION_NAME,  // ......Application Name
    PARSE_APPLICATION_TYPE,  // ......Application Type
    PARSE_CONNECTIONS,       // ....In an Connections tag
    PARSE_CONNECTION,        // ......In an Connection tag
    PARSE_SERVER_CERT,       // ........Server certificate
    PARSE_SECURITY_POLICY,   // ........Security policy
    PARSE_SECURITY_MODE,     // ........Security mode
    PARSE_USER_POLICY,       // ........In user policy tag
    PARSE_USER_CERT,         // ........UserX509 configuration
    PARSE_SRVCONFIG          //..In a server config tag to skip
} client_parse_state_t;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    int32_t srv_skip_depth;

    bool localesSet;
    SOPC_Array* preferredLocaleIds;

    bool appDescSet;
    OpcUa_ApplicationDescription appDesc;

    bool appCertSet;
    char* clientCertificate;
    char* clientKey;
    bool clientKeyEncrypted;
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

    bool connectionsSet;
    SOPC_SecureConnection_Config* currentSecConnConfig;
    uint16_t nbConnections;
    uint16_t nbReverseUrls;

    SOPC_Client_Config* clientConfigPtr;

    client_parse_state_t state;
};

#define NS_SEPARATOR "|"
#define NS(ns, tag) ns NS_SEPARATOR tag

static SOPC_ReturnStatus parse(XML_Parser parser, FILE* fd)
{
    char buf[65365];

    while (!feof(fd))
    {
        size_t r = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fd);

        if ((0 == r) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            return SOPC_STATUS_NOK;
        }

        if (XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            const enum XML_Error parser_error = XML_GetErrorCode(parser);

            if (parser_error != XML_ERROR_NONE)
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
    if (XML_Parse(parser, "", 0, 1) != XML_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

static bool end_client_config(struct parse_context_t* ctx)
{
    SOPC_ASSERT(ctx != NULL);
    if (!ctx->localesSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no preferred locales defined for the client");
        return false;
    }

    if (!ctx->appDescSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no application description defined for the client");
        return false;
    }

    if (!ctx->connectionsSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no connections defined for the client");
        return false;
    }

    if (NULL == ctx->clientCertificate || NULL == ctx->clientKey)
    {
        if (NULL == ctx->clientCertificate && NULL == ctx->clientKey)
        {
            for (size_t i = 0; i < ctx->nbConnections; i++)
            {
                if (OpcUa_MessageSecurityMode_None !=
                    ctx->clientConfigPtr->secureConnections[i]->scConfig.msgSecurityMode)
                {
                    LOG_XML_ERROR(
                        ctx->helper_ctx.parser,
                        "client certificate and key are not defined whereas security mode is not always None");
                }
            }
        }
        else
        {
            LOG_XML_ERROR(ctx->helper_ctx.parser, "client certificate and key are not both NULL");
            return false;
        }
    }

    return true;
}

static bool end_connections(struct parse_context_t* ctx)
{
    if (0 == ctx->nbConnections)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no connection defined for the client");
        return false;
    }

    return true;
}

static bool start_connection(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "serverURL", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "serverURL attribute missing");
        return false;
    }

    char* url = SOPC_strdup(attr_val);

    if (NULL == url)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    SOPC_SecureConnection_Config** ppSecConnConfig = &ctx->clientConfigPtr->secureConnections[ctx->nbConnections];
    *ppSecConnConfig = SOPC_Calloc(1, sizeof(**ppSecConnConfig));
    if (NULL == *ppSecConnConfig)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        SOPC_Free(url);
        return false;
    }
    SOPC_SecureConnection_Config* pSecConnConfig = *ppSecConnConfig;

    pSecConnConfig->scConfig.url = url;

    // Manage reverseEndpointURL
    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "reverseEndpointURL", attrs);

    if (attr_val != NULL)
    {
        bool found = false;
        for (uint16_t i = 0; !found && i < ctx->nbReverseUrls; i++)
        {
            found = (0 == strcmp(attr_val, ctx->clientConfigPtr->reverseEndpointURLs[i]));
        }
        if (!found)
        {
            ctx->clientConfigPtr->reverseEndpointURLs[ctx->nbReverseUrls] = SOPC_strdup(attr_val);
            if (NULL == ctx->clientConfigPtr->reverseEndpointURLs[ctx->nbReverseUrls])
            {
                LOG_MEMORY_ALLOCATION_FAILURE;
                return false;
            }
            ctx->nbReverseUrls++;
        }
        pSecConnConfig->reverseURL = SOPC_strdup(attr_val);
        if (NULL == pSecConnConfig->reverseURL)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    // Manage user defined id
    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "id", attrs);

    if (attr_val != NULL)
    {
        pSecConnConfig->userDefinedId = SOPC_strdup(attr_val);
        if (NULL == pSecConnConfig->userDefinedId)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    // Manage serverURI
    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "serverURI", attrs);

    if (attr_val != NULL)
    {
        pSecConnConfig->scConfig.serverUri = SOPC_strdup(attr_val);
        if (NULL == pSecConnConfig->scConfig.serverUri)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "reqLifetimeMs", attrs);

    if (attr_val != NULL)
    {
        bool res = SOPC_strtouint(attr_val, strlen(attr_val), 32, &pSecConnConfig->scConfig.requestedLifetime);
        if (!res)
        {
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "client requested secure channel lifetime value invalid %s",
                           attr_val);
            return false;
        }
    }
    else
    {
        pSecConnConfig->scConfig.requestedLifetime = SOPC_DEFAULT_REQ_LIFETIME_MS;
    }

    pSecConnConfig->sessionConfig.userPolicyId =
        SOPC_strdup(""); // Empty user policy id per default (anonymous without policy)
    if (NULL == pSecConnConfig->sessionConfig.userPolicyId)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->currentSecConnConfig = pSecConnConfig;

    return true;
}

static bool end_connection(struct parse_context_t* ctx)
{
    if (NULL == ctx->currentSecConnConfig->scConfig.reqSecuPolicyUri)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no security policy defined for the connection");
        return false;
    }
    if (OpcUa_MessageSecurityMode_Invalid == ctx->currentSecConnConfig->scConfig.msgSecurityMode)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no security mode defined for the connection");
        return false;
    }
    if (NULL == ctx->currentSecConnConfig->serverCertPath &&
        (OpcUa_MessageSecurityMode_None != ctx->currentSecConnConfig->scConfig.msgSecurityMode ||
         OpcUa_UserTokenType_UserName == ctx->currentSecConnConfig->sessionConfig.userTokenType))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser,
                      "no server certificate path defined for the connection when security mode is not None or user "
                      "policy is UserName (certificate needed for password encryption)");
        return false;
    }

    ctx->currentSecConnConfig->scConfig.isClientSc = true;
    ctx->currentSecConnConfig->scConfig.clientConfigPtr = ctx->clientConfigPtr;
    ctx->currentSecConnConfig->secureConnectionIdx = ctx->nbConnections;

    ctx->nbConnections++;

    return true;
}

static bool SOPC_end_app_certs(struct parse_context_t* ctx)
{
    // Prepare struct to store the certificates paths
    ctx->clientConfigPtr->configFromPaths = SOPC_Calloc(sizeof(*ctx->clientConfigPtr->configFromPaths), 1);
    if (NULL == ctx->clientConfigPtr->configFromPaths)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->clientConfigPtr->isConfigFromPathsNeeded = true;

    return true;
}

static bool start_policy(struct parse_context_t* ctx, const XML_Char** attrs)
{
    SOPC_SecureConnection_Config* secConnConfig = ctx->currentSecConnConfig;

    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "uri", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "uri attribute missing");
        return false;
    }

    secConnConfig->scConfig.reqSecuPolicyUri = SOPC_strdup(attr_val);

    if (NULL == secConnConfig->scConfig.reqSecuPolicyUri)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool start_mode(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "mode", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "mode attribute missing");
        return false;
    }

    if (strcmp(attr_val, "None") == 0)
    {
        ctx->currentSecConnConfig->scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_None;
    }
    else if (strcmp(attr_val, "Sign") == 0)
    {
        ctx->currentSecConnConfig->scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_Sign;
    }
    else if (strcmp(attr_val, "SignAndEncrypt") == 0)
    {
        ctx->currentSecConnConfig->scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
    }
    else
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "invalid mode attribute value %s", attr_val);
        return false;
    }

    return true;
}

static bool start_user_policy(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "policyId", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "policyId attribute missing");
        return false;
    }
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Free((void*) ctx->currentSecConnConfig->sessionConfig.userPolicyId); // free default empty C string
    SOPC_GCC_DIAGNOSTIC_RESTORE
    ctx->currentSecConnConfig->sessionConfig.userPolicyId = SOPC_strdup(attr_val);
    if (NULL == ctx->currentSecConnConfig->sessionConfig.userPolicyId)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "tokenType", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "tokenType attribute missing");
        return false;
    }

    if (strcmp(attr_val, "anonymous") == 0)
    {
        ctx->currentSecConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_Anonymous;
    }
    else if (strcmp(attr_val, "username") == 0)
    {
        ctx->currentSecConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_UserName;
    }
    else if (strcmp(attr_val, "certificate") == 0)
    {
        ctx->currentSecConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_Certificate;
    }
    else
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "tokenType attribute value %s not supported", attr_val);
        return false;
    }

    return true;
}

static bool end_user_policy(struct parse_context_t* ctx)
{
    if (NULL == ctx->currentSecConnConfig->sessionConfig.userPolicyId &&
        OpcUa_UserTokenType_Anonymous != ctx->currentSecConnConfig->sessionConfig.userTokenType)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no user policy id defined for the connection");
        return false;
    }

    if (OpcUa_UserTokenType_Certificate == ctx->currentSecConnConfig->sessionConfig.userTokenType)
    {
        SOPC_Session_UserX509* userX509 = &ctx->currentSecConnConfig->sessionConfig.userToken.userX509;
        if (NULL == userX509->configFromPaths->userCertPath || 0 == strlen(userX509->configFromPaths->userCertPath))
        {
            LOG_XML_ERROR(ctx->helper_ctx.parser, "no user certificate path defined for the connection");
            return false;
        }
        if (NULL == userX509->configFromPaths->userKeyPath || 0 == strlen(userX509->configFromPaths->userKeyPath))
        {
            LOG_XML_ERROR(ctx->helper_ctx.parser, "no user key path defined for the connection");
            return false;
        }
    }

    return true;
}

static bool start_user_cert(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "cert_path", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "user cert_path attribute missing");
        return false;
    }

    SOPC_Session_UserX509* userX509 = &ctx->currentSecConnConfig->sessionConfig.userToken.userX509;
    userX509->configFromPaths = SOPC_Calloc(sizeof(*userX509->configFromPaths), 1);
    if (NULL == userX509->configFromPaths)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    userX509->configFromPaths->userCertPath = SOPC_strdup(attr_val);
    if (NULL == userX509->configFromPaths->userCertPath)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "key_path", attrs);
    if (NULL == attr_val)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "user key_path attribute missing");
        return false;
    }
    userX509->configFromPaths->userKeyPath = SOPC_strdup(attr_val);
    if (NULL == userX509->configFromPaths->userKeyPath)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "encrypted", attrs);
    userX509->configFromPaths->userKeyEncrypted = attr_val != NULL && 0 == strcmp(attr_val, "true");

    userX509->isConfigFromPathNeeded = true;

    return true;
}

static void start_element_handler(void* user_data, const XML_Char* name, const XML_Char** attrs)
{
    struct parse_context_t* ctx = user_data;
    SOPC_HelperExpatCtx* helperCtx = &ctx->helper_ctx;

    switch (ctx->state)
    {
    case PARSE_START:
        if (strcmp(name, "S2OPC") != 0)
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        ctx->state = PARSE_S2OPC;
        return;
    case PARSE_S2OPC:
        if (strcmp(name, "ClientConfiguration") == 0)
        {
            ctx->state = PARSE_CLICONFIG;
        }
        else if (strcmp(name, "ServerConfiguration") == 0)
        {
            ctx->state = PARSE_SRVCONFIG;
            ctx->srv_skip_depth = 1;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        return;
    case PARSE_SRVCONFIG:
        ctx->srv_skip_depth++;

        return;
    case PARSE_CLICONFIG:
    {
        if (strcmp(name, "PreferredLocales") == 0 && !ctx->localesSet)
        {
            ctx->localesSet = true;
            ctx->state = PARSE_PREFERRED_LOCALES;
        }
        else if (strcmp(name, "ApplicationDescription") == 0 && !ctx->appDescSet)
        {
            ctx->appDescSet = true;
            ctx->state = PARSE_APPLICATION_DESC;
        }
        else if (strcmp(name, "ApplicationCertificates") == 0 && !ctx->appCertSet)
        {
            ctx->appCertSet = true;
            ctx->state = PARSE_APPLICATION_CERT;
        }
        else if (strcmp(name, "Connections") == 0 && !ctx->connectionsSet)
        {
            ctx->connectionsSet = true;
            ctx->state = PARSE_CONNECTIONS;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    }
    case PARSE_PREFERRED_LOCALES:
        if (strcmp(name, "Locale") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_locale(&ctx->helper_ctx, ctx->preferredLocaleIds, attrs))
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

        ctx->state = PARSE_LOCALE;
        break;
    case PARSE_APPLICATION_DESC:
        if (strcmp(name, "ApplicationURI") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_app_uri(true, &ctx->helper_ctx, &ctx->appDesc, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_APPLICATION_URI;
        }
        else if (strcmp(name, "ProductURI") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_prod_uri(&ctx->helper_ctx, &ctx->appDesc, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_PRODUCT_URI;
        }
        else if (strcmp(name, "ApplicationName") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_app_name(true, &ctx->helper_ctx, &ctx->appDesc,
                                                          ctx->clientConfigPtr->clientLocaleIds, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_APPLICATION_NAME;
        }
        else if (strcmp(name, "ApplicationType") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_app_type(true, &ctx->helper_ctx, &ctx->appDesc, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_APPLICATION_TYPE;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_APPLICATION_CERT:
        if (strcmp(name, "ClientCertificate") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_cert(true, &ctx->helper_ctx, &ctx->clientCertificate, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_CLIENT_CERT;
        }
        else if (strcmp(name, "ClientKey") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_key(true, &ctx->helper_ctx, &ctx->clientKey, &ctx->clientKeyEncrypted,
                                                     attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_CLIENT_KEY;
        }
        else if (strcmp(name, "TrustedIssuers") == 0 && !ctx->trustedIssuersSet)
        {
            ctx->state = PARSE_TRUSTED_ISSUERS;
        }
        else if (strcmp(name, "IssuedCertificates") == 0 && !ctx->issuedCertificatesSet)
        {
            ctx->state = PARSE_ISSUED_CERTS;
        }
        else if (strcmp(name, "UntrustedIssuers") == 0 && !ctx->untrustedIssuersSet)
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
        if (strcmp(name, "TrustedIssuer") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_issuer(true, &ctx->helper_ctx, ctx->trustedRootIssuers,
                                                        ctx->trustedIntermediateIssuers, ctx->crlCertificates, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_TRUSTED_ISSUER;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_ISSUED_CERTS:
        if (strcmp(name, "IssuedCertificate") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_issued_cert(&ctx->helper_ctx, ctx->issuedCertificates, attrs))
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

        ctx->state = PARSE_ISSUED_CERT;
        break;
    case PARSE_UNTRUSTED_ISSUERS:
        if (strcmp(name, "UntrustedIssuer") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_issuer(true, &ctx->helper_ctx, ctx->untrustedRootIssuers,
                                                        ctx->untrustedIntermediateIssuers, ctx->crlCertificates, attrs))
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

        ctx->state = PARSE_UNTRUSTED_ISSUER;
        break;
    case PARSE_CONNECTIONS:
        if (strcmp(name, "Connection") == 0)
        {
            if (!start_connection(ctx, attrs))
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

        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_CONNECTION:
        if (strcmp(name, "ServerCertificate") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_cert(true, &ctx->helper_ctx,
                                                      &ctx->currentSecConnConfig->serverCertPath, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            else
            {
                ctx->currentSecConnConfig->isServerCertFromPathNeeded = true;
            }

            ctx->state = PARSE_SERVER_CERT;
        }
        else if (strcmp(name, "SecurityPolicy") == 0)
        {
            if (!start_policy(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_SECURITY_POLICY;
        }
        else if (strcmp(name, "SecurityMode") == 0)
        {
            if (!start_mode(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_SECURITY_MODE;
        }
        else if (strcmp(name, "UserPolicy") == 0)
        {
            if (!start_user_policy(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_USER_POLICY;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_USER_POLICY:
        if (strcmp(name, "UserX509") == 0)
        {
            if (!start_user_cert(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_USER_CERT;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    default:
        return;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    SOPC_UNUSED_ARG(name);

    struct parse_context_t* ctx = user_data;
    SOPC_ASSERT(ctx != NULL);

    switch (ctx->state)
    {
    case PARSE_USER_CERT:
        ctx->state = PARSE_USER_POLICY;
        break;
    case PARSE_SERVER_CERT:
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_SECURITY_POLICY:
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_USER_POLICY:
        if (!end_user_policy(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_SECURITY_MODE:
        ctx->state = PARSE_CONNECTION;
        break;
    case PARSE_CONNECTION:
        if (!end_connection(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_CONNECTIONS;
        break;
    case PARSE_CONNECTIONS:
        if (!end_connections(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_CLICONFIG;
        break;
    case PARSE_TRUSTED_ISSUER:
        ctx->state = PARSE_TRUSTED_ISSUERS;
        break;
    case PARSE_TRUSTED_ISSUERS:
        if (!SOPC_ConfigLoaderInternal_end_trusted_issuers(true, &ctx->helper_ctx, ctx->trustedRootIssuers))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->trustedIssuersSet = true;
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_ISSUED_CERT:
        ctx->state = PARSE_ISSUED_CERTS;
        break;
    case PARSE_ISSUED_CERTS:
        if (!SOPC_ConfigLoaderInternal_end_issued_certs(true, &ctx->helper_ctx, ctx->issuedCertificates))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->issuedCertificatesSet = true;
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_UNTRUSTED_ISSUER:
        ctx->state = PARSE_UNTRUSTED_ISSUERS;
        break;
    case PARSE_UNTRUSTED_ISSUERS:
        if (!SOPC_ConfigLoaderInternal_end_untrusted_issuers(true, &ctx->helper_ctx, ctx->untrustedRootIssuers,
                                                             ctx->untrustedIntermediateIssuers))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->untrustedIssuersSet = true;
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_CLIENT_KEY:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_CLIENT_CERT:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_APPLICATION_CERT:
        if (!SOPC_end_app_certs(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        if (!SOPC_ConfigLoaderInternal_end_application_certificates(
                true, &ctx->helper_ctx, &ctx->trustedRootIssuers,
                &ctx->clientConfigPtr->configFromPaths->trustedRootIssuersList, &ctx->trustedIntermediateIssuers,
                &ctx->clientConfigPtr->configFromPaths->trustedIntermediateIssuersList, &ctx->issuedCertificates,
                &ctx->clientConfigPtr->configFromPaths->issuedCertificatesList, &ctx->untrustedRootIssuers,
                &ctx->clientConfigPtr->configFromPaths->untrustedRootIssuersList, &ctx->untrustedIntermediateIssuers,
                &ctx->clientConfigPtr->configFromPaths->untrustedIntermediateIssuersList, &ctx->crlCertificates,
                &ctx->clientConfigPtr->configFromPaths->certificateRevocationPathList, ctx->issuedCertificatesSet,
                ctx->trustedIssuersSet))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_CLICONFIG;
        break;
    case PARSE_APPLICATION_NAME:
        ctx->state = PARSE_APPLICATION_DESC;
        break;
    case PARSE_PRODUCT_URI:
        ctx->state = PARSE_APPLICATION_DESC;
        break;
    case PARSE_APPLICATION_URI:
        ctx->state = PARSE_APPLICATION_DESC;
        break;
    case PARSE_APPLICATION_TYPE:
        ctx->state = PARSE_APPLICATION_DESC;
        break;
    case PARSE_APPLICATION_DESC:
        if (!SOPC_ConfigLoaderInternal_end_app_desc(true, &ctx->helper_ctx, &ctx->appDesc))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_CLICONFIG;
        break;
    case PARSE_LOCALE:
        ctx->state = PARSE_PREFERRED_LOCALES;
        break;
    case PARSE_PREFERRED_LOCALES:
        if (!SOPC_ConfigLoaderInternal_end_locales(true, &ctx->helper_ctx, ctx->preferredLocaleIds,
                                                   &ctx->clientConfigPtr->clientLocaleIds))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->preferredLocaleIds = NULL;
        ctx->state = PARSE_CLICONFIG;
        break;
    case PARSE_CLICONFIG:
        if (!end_client_config(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_S2OPC;
        break;
    case PARSE_SRVCONFIG:
        ctx->srv_skip_depth--;
        if (ctx->srv_skip_depth == 0)
        {
            ctx->state = PARSE_S2OPC;
        }
        break;
    case PARSE_S2OPC:
        break;
    case PARSE_START:
        SOPC_ASSERT(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        SOPC_ASSERT(false && "Unknown state.");
        break;
    }
}

static void SOPC_Free_CstringFromPtr(void* data)
{
    if (NULL != data)
    {
        SOPC_Free(*(char**) data);
    }
}

bool SOPC_ConfigClient_Parse(FILE* fd, SOPC_Client_Config* clientConfig)
{
    SOPC_ASSERT(NULL != clientConfig);
    SOPC_ClientConfig_Initialize(clientConfig);
    clientConfig->freeCstringsFlag = true; // C strings are allocated during parsing or NULL if undefined
    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);

    SOPC_Array* preferredLocales = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* trustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* trustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* issuedCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    SOPC_Array* revokedListCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    if ((NULL == parser) || (NULL == preferredLocales) || (NULL == trustedRootIssuers) ||
        (NULL == trustedIntermediateIssuers) || (NULL == issuedCerts) || (NULL == untrustedRootIssuers) ||
        (NULL == untrustedIntermediateIssuers) || (NULL == revokedListCerts))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
        SOPC_Array_Delete(preferredLocales);
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
    ctx.srv_skip_depth = 0;
    ctx.nbConnections = 0;
    ctx.preferredLocaleIds = preferredLocales;
    ctx.trustedRootIssuers = trustedRootIssuers;
    ctx.trustedIntermediateIssuers = trustedIntermediateIssuers;
    ctx.issuedCertificates = issuedCerts;
    ctx.untrustedRootIssuers = untrustedRootIssuers;
    ctx.untrustedIntermediateIssuers = untrustedIntermediateIssuers;
    ctx.crlCertificates = revokedListCerts;
    ctx.clientConfigPtr = clientConfig;
    ctx.helper_ctx.char_data_buffer = NULL;
    ctx.helper_ctx.char_data_cap = 0;
    OpcUa_ApplicationDescription_Initialize(&ctx.appDesc);
    ctx.appDesc.ApplicationType = OpcUa_ApplicationType_SizeOf;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Array_Delete(ctx.preferredLocaleIds);
    SOPC_Array_Delete(ctx.trustedRootIssuers);
    SOPC_Array_Delete(ctx.trustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.issuedCertificates);
    SOPC_Array_Delete(ctx.untrustedRootIssuers);
    SOPC_Array_Delete(ctx.untrustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.crlCertificates);

    if (res == SOPC_STATUS_OK)
    {
        clientConfig->nbSecureConnections = ctx.nbConnections;
        clientConfig->nbReverseEndpointURLs = ctx.nbReverseUrls;
        if (clientConfig->isConfigFromPathsNeeded && NULL != clientConfig->configFromPaths)
        {
            clientConfig->configFromPaths->clientCertPath = ctx.clientCertificate;
            clientConfig->configFromPaths->clientKeyPath = ctx.clientKey;
            clientConfig->configFromPaths->clientKeyEncrypted = ctx.clientKeyEncrypted;
        }
        else
        {
            SOPC_ASSERT(NULL == ctx.clientCertificate);
            SOPC_ASSERT(NULL == ctx.clientKey);
        }
        clientConfig->clientDescription = ctx.appDesc;
        return true;
    }
    else
    {
        return false;
    }
}
