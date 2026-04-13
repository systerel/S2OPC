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

typedef enum
{
    PARSE_START,               // Beginning of file
    PARSE_S2OPC,               // In a S2OPC
    PARSE_SRVCONFIG,           // ..In a server config tag
    PARSE_LOCALES,             // ....In locales
    PARSE_LOCALE,              // ......In locale
    PARSE_APPLICATION_DESC,    // ....In application description
    PARSE_APPLICATION_URI,     // ......Application URI
    PARSE_PRODUCT_URI,         // ......Product URI
    PARSE_APPLICATION_NAME,    // ......Application Name
    PARSE_APPLICATION_TYPE,    // ......Application Type
    PARSE_APPLICATION_CERT,    // ....In application certificate
    PARSE_SERVER_CERT,         // ......Server certificate
    PARSE_SERVER_KEY,          // ......Server key
    PARSE_SERVER_PKI,          // ......Server PKI
    PARSE_ENDPOINTS,           // ....In an Endpoints tag
    PARSE_ENDPOINT,            // ......In an Endpoint tag
    PARSE_REVERSE_CONNECTIONS, // ........In reverse connections tag
    PARSE_REVERSE_CONNECTION,  // ..........In reverse connection tag
    PARSE_SECURITY_POLICIES,   // ........In security policies tag
    PARSE_SECURITY_POLICY,     // ..........In security policy tag
    PARSE_SECURITY_MODES,      // ............In security modes tag
    PARSE_SECURITY_MODE,       // ..............In security mode tag
    PARSE_USER_POLICIES,       // ............In user polcies tag
    PARSE_USER_POLICY,         // ..............In user policy tag
    PARSE_CLICONFIG            //..In a client config tag to skip
} server_parse_state_t;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    int32_t cli_skip_depth;

    bool localesSet;
    SOPC_Array* localeIds;

    bool appDescSet;
    OpcUa_ApplicationDescription appDesc;

    bool appCertSet;
    char* serverCertificate;
    char* serverKey;
    bool serverKeyEncrypted;
    char* serverPki;

    bool endpointsSet;
    SOPC_Array* endpoints;
    SOPC_Endpoint_Config* currentEpConfig;
    SOPC_Server_Config* serverConfigPtr;

    server_parse_state_t state;
};

#define NS_SEPARATOR "|"

static SOPC_ReturnStatus parse(XML_Parser parser, FILE* fd)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    const size_t BUF_SIZE = 65365;
    char* buf = SOPC_Malloc(BUF_SIZE);
    status = (NULL != buf) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    while (!feof(fd) && SOPC_STATUS_OK == status)
    {
        size_t r = fread(buf, sizeof(char), BUF_SIZE, fd);

        if ((0 == r) && (ferror(fd) != 0))
        {
            LOGF("Error while reading input file: %s", strerror(errno));
            status = SOPC_STATUS_WOULD_BLOCK;
        }

        if (SOPC_STATUS_OK == status && XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            const enum XML_Error parser_error = XML_GetErrorCode(parser);

            if (parser_error != XML_ERROR_NONE)
            {
                fprintf(stderr, "XML parsing failed at line %lu, column %lu. Error code is %d.\n",
                        XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), XML_GetErrorCode(parser));
            }

            // else, the error comes from one of the callbacks, that log an error
            // themselves.

            status = SOPC_STATUS_NOK;
        }
    }

    // Tell the parser that we are at the end of the file
    if (SOPC_STATUS_OK == status && XML_Parse(parser, "", 0, 1) != XML_STATUS_OK)
    {
        status = SOPC_STATUS_NOK;
    }

    SOPC_Free(buf);
    return status;
}

static bool end_server_config(struct parse_context_t* ctx)
{
    if (!ctx->localesSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no locales defined for the server");
        return false;
    }

    if (!ctx->appDescSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no application description defined for the server");
        return false;
    }

    if (!ctx->endpointsSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no endpoints defined for the server");
        return false;
    }

    return true;
}

static bool end_app_certs(struct parse_context_t* ctx)
{
    bool res = true;
    if (NULL == ctx->serverCertificate)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no server certificate defined for the server");
        res = false;
    }
    if (NULL == ctx->serverKey)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no server key defined for the server");
        res = false;
    }
    if (NULL == ctx->serverPki)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no PKI store defined for the server");
        res = false;
    }
    if (!res)
    {
        SOPC_Free(ctx->serverCertificate);
        SOPC_Free(ctx->serverKey);
        SOPC_Free(ctx->serverPki);
        ctx->serverCertificate = NULL;
        ctx->serverKey = NULL;
        ctx->serverPki = NULL;
    }
    return res;
}

static bool end_endpoints(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->endpoints))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no endpoint defined for the server");
        return false;
    }

    return true;
}

static bool start_endpoint(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "url", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "url attribute missing");
        return false;
    }

    char* url = SOPC_strdup(attr_val);

    if (url == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    SOPC_Endpoint_Config epConfig;
    memset(&epConfig, 0, sizeof(epConfig));

    epConfig.endpointURL = url;

    // Manage the hasDiscoveryEndpoint flag
    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "hasDiscoveryEndpoint", attrs);

    if (attr_val == NULL)
    {
        epConfig.hasDiscoveryEndpoint = true; // default value
    }
    else
    {
        epConfig.hasDiscoveryEndpoint = (strcmp(attr_val, "true") == 0);
    }

    // Manage the enableListening flag
    bool enableListening = true; // default value
    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "enableListening", attrs);
    if (attr_val != NULL)
    {
        enableListening = (strcmp(attr_val, "true") == 0);
    }
    epConfig.noListening = !enableListening;

    if (!SOPC_Array_Append(ctx->endpoints, epConfig) || SOPC_Array_Size(ctx->endpoints) > UINT8_MAX)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->currentEpConfig = SOPC_Array_Get_Ptr(ctx->endpoints, SOPC_Array_Size(ctx->endpoints) - 1);
    ctx->currentEpConfig->serverConfigPtr = ctx->serverConfigPtr;

    return true;
}

static bool end_endpoint(struct parse_context_t* ctx)
{
    if (0 == ctx->currentEpConfig->nbSecuConfigs)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no security policy defined for the endpoint");
        return false;
    }

    return true;
}

static bool start_reverse_connection(struct parse_context_t* ctx, const XML_Char** attrs)
{
    SOPC_Endpoint_Config* epConfig = ctx->currentEpConfig;

    if (epConfig->nbClientsToConnect >= SOPC_MAX_REVERSE_CLIENT_CONNECTIONS)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Maximum number of reverse connections %d reached",
                       SOPC_MAX_REVERSE_CLIENT_CONNECTIONS);
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "clientUrl", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "clientUrl attribute missing");
        return false;
    }

    char* clientUrl = SOPC_strdup(attr_val);

    if (clientUrl == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "clientAppUri", attrs);

    char* clientAppURI = NULL;
    if (attr_val != NULL)
    {
        clientAppURI = SOPC_strdup(attr_val);

        if (clientAppURI == NULL)
        {
            SOPC_Free(clientUrl);
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    epConfig->clientsToConnect[epConfig->nbClientsToConnect].clientApplicationURI = clientAppURI;
    epConfig->clientsToConnect[epConfig->nbClientsToConnect].clientEndpointURL = clientUrl;
    epConfig->nbClientsToConnect++;

    return true;
}

static bool start_policy(struct parse_context_t* ctx, const XML_Char** attrs)
{
    SOPC_Endpoint_Config* epConfig = ctx->currentEpConfig;

    if (epConfig->nbSecuConfigs >= SOPC_MAX_SECU_POLICIES_CFG)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Maximum number of policies %d reached", SOPC_MAX_SECU_POLICIES_CFG);
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "uri", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "uri attribute missing");
        return false;
    }

    SOPC_ReturnStatus status =
        SOPC_String_CopyFromCString(&epConfig->secuConfigurations[epConfig->nbSecuConfigs].securityPolicy, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

static bool end_policy(struct parse_context_t* ctx)
{
    SOPC_Endpoint_Config* epConfig = ctx->currentEpConfig;
    if (0 == (SOPC_SECURITY_MODE_ANY_MASK & epConfig->secuConfigurations[epConfig->nbSecuConfigs].securityModes))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "No security mode defined");
        return false;
    }
    if (0 == epConfig->secuConfigurations[epConfig->nbSecuConfigs].nbOfUserTokenPolicies)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "No user token policy defined");
        return false;
    }
    epConfig->nbSecuConfigs++;
    return true;
}

static bool start_mode(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "mode", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "mode attribute missing");
        return false;
    }

    SOPC_SecurityPolicy* secuPolicy = &ctx->currentEpConfig->secuConfigurations[ctx->currentEpConfig->nbSecuConfigs];

    if (strcmp(attr_val, "None") == 0)
    {
        secuPolicy->securityModes |= SOPC_SECURITY_MODE_NONE_MASK;
    }
    else if (strcmp(attr_val, "Sign") == 0)
    {
        secuPolicy->securityModes |= SOPC_SECURITY_MODE_SIGN_MASK;
    }
    else if (strcmp(attr_val, "SignAndEncrypt") == 0)
    {
        secuPolicy->securityModes |= SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
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
    SOPC_SecurityPolicy* secuPolicy = &ctx->currentEpConfig->secuConfigurations[ctx->currentEpConfig->nbSecuConfigs];

    if (secuPolicy->nbOfUserTokenPolicies >= SOPC_MAX_SECU_POLICIES_CFG)
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "Maximum number of user policies %d reached",
                       SOPC_MAX_SECU_POLICIES_CFG);
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "policyId", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "policyId attribute missing");
        return false;
    }

    OpcUa_UserTokenPolicy* userPolicy = &secuPolicy->userTokenPolicies[secuPolicy->nbOfUserTokenPolicies];
    OpcUa_UserTokenPolicy_Initialize(userPolicy);

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&userPolicy->PolicyId, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "tokenType", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "tokenType attribute missing");
        return false;
    }

    if (strcmp(attr_val, "anonymous") == 0)
    {
        userPolicy->TokenType = OpcUa_UserTokenType_Anonymous;
    }
    else if (strcmp(attr_val, "username") == 0)
    {
        userPolicy->TokenType = OpcUa_UserTokenType_UserName;
    }
    else if (strcmp(attr_val, "certificate") == 0)
    {
        userPolicy->TokenType = OpcUa_UserTokenType_Certificate;
    }
    else
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "tokenType attribute value %s not supported", attr_val);
        return false;
    }

    if (OpcUa_UserTokenType_UserName == userPolicy->TokenType ||
        OpcUa_UserTokenType_Certificate == userPolicy->TokenType)
    {
        attr_val = SOPC_HelperExpat_GetAttr(&ctx->helper_ctx, "securityUri", attrs);

        if (attr_val == NULL)
        {
            LOG_XML_ERROR(ctx->helper_ctx.parser, "securityUri attribute missing");
            return false;
        }

        status = SOPC_String_CopyFromCString(&userPolicy->SecurityPolicyUri, attr_val);

        if (SOPC_STATUS_OK != status)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
    }

    return true;
}

static void end_user_policy(struct parse_context_t* ctx)
{
    SOPC_SecurityPolicy* secuPolicy = &ctx->currentEpConfig->secuConfigurations[ctx->currentEpConfig->nbSecuConfigs];
    secuPolicy->nbOfUserTokenPolicies++;
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
        if (strcmp(name, "ServerConfiguration") == 0)
        {
            ctx->state = PARSE_SRVCONFIG;
        }
        else if (strcmp(name, "ClientConfiguration") == 0)
        {
            ctx->state = PARSE_CLICONFIG;
            ctx->cli_skip_depth = 1;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        return;
    case PARSE_CLICONFIG:
        ctx->cli_skip_depth++;

        return;
    case PARSE_SRVCONFIG:
    {
        if (strcmp(name, "Locales") == 0 && !ctx->localesSet)
        {
            ctx->localesSet = true;
            ctx->state = PARSE_LOCALES;
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
        else if (strcmp(name, "Endpoints") == 0 && !ctx->endpointsSet)
        {
            ctx->endpointsSet = true;
            ctx->state = PARSE_ENDPOINTS;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        break;
    }
    case PARSE_LOCALES:
        if (strcmp(name, "Locale") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_locale(&ctx->helper_ctx, ctx->localeIds, attrs))
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
                                                          ctx->serverConfigPtr->localeIds, attrs))
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
        if (strcmp(name, "ServerCertificate") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_cert(true, &ctx->helper_ctx, &ctx->serverCertificate, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }

            ctx->state = PARSE_SERVER_CERT;
        }
        else if (strcmp(name, "ServerKey") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_key(true, &ctx->helper_ctx, &ctx->serverKey, &ctx->serverKeyEncrypted,
                                                     attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_SERVER_KEY;
        }
        else if (strcmp(name, "ServerPublicKeyInfrastructure") == 0)
        {
            if (!SOPC_ConfigLoaderInternal_start_pki(true, &ctx->helper_ctx, &ctx->serverPki, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
            ctx->state = PARSE_SERVER_PKI;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_ENDPOINTS:
        if (strcmp(name, "Endpoint") == 0)
        {
            if (!start_endpoint(ctx, attrs))
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

        ctx->state = PARSE_ENDPOINT;
        break;
    case PARSE_ENDPOINT:
        if (strcmp(name, "SecurityPolicies") == 0)
        {
            ctx->state = PARSE_SECURITY_POLICIES;
        }
        else if (strcmp(name, "ReverseConnections") == 0)
        {
            ctx->state = PARSE_REVERSE_CONNECTIONS;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_REVERSE_CONNECTIONS:
        if (strcmp(name, "ReverseConnection") == 0)
        {
            if (!start_reverse_connection(ctx, attrs))
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

        ctx->state = PARSE_REVERSE_CONNECTION;
        break;
    case PARSE_SECURITY_POLICIES:
        if (strcmp(name, "SecurityPolicy") == 0)
        {
            if (!start_policy(ctx, attrs))
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
        ctx->state = PARSE_SECURITY_POLICY;
        break;
    case PARSE_SECURITY_POLICY:
        if (strcmp(name, "SecurityModes") == 0)
        {
            ctx->state = PARSE_SECURITY_MODES;
        }
        else if (strcmp(name, "UserPolicies") == 0)
        {
            ctx->state = PARSE_USER_POLICIES;
        }
        else
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }
        break;
    case PARSE_SECURITY_MODES:
        if (strcmp(name, "SecurityMode") == 0)
        {
            if (!start_mode(ctx, attrs))
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

        ctx->state = PARSE_SECURITY_MODE;
        break;
    case PARSE_USER_POLICIES:
        if (strcmp(name, "UserPolicy") == 0)
        {
            if (!start_user_policy(ctx, attrs))
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

        ctx->state = PARSE_USER_POLICY;
        break;
    default:
        return;
    }
}

static void end_element_handler(void* user_data, const XML_Char* name)
{
    SOPC_UNUSED_ARG(name);

    struct parse_context_t* ctx = user_data;

    switch (ctx->state)
    {
    case PARSE_USER_POLICY:
        end_user_policy(ctx);
        ctx->state = PARSE_USER_POLICIES;
        break;
    case PARSE_USER_POLICIES:
        ctx->state = PARSE_SECURITY_POLICY;
        break;
    case PARSE_SECURITY_MODE:
        ctx->state = PARSE_SECURITY_MODES;
        break;
    case PARSE_SECURITY_MODES:
        ctx->state = PARSE_SECURITY_POLICY;
        break;
    case PARSE_REVERSE_CONNECTION:
        ctx->state = PARSE_REVERSE_CONNECTIONS;
        break;
    case PARSE_REVERSE_CONNECTIONS:
        ctx->state = PARSE_ENDPOINT;
        break;
    case PARSE_SECURITY_POLICY:
        if (!end_policy(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_SECURITY_POLICIES;
        break;
    case PARSE_SECURITY_POLICIES:
        ctx->state = PARSE_ENDPOINT;
        break;
    case PARSE_ENDPOINT:
        if (!end_endpoint(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_ENDPOINTS;
        break;
    case PARSE_ENDPOINTS:
        if (!end_endpoints(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_SRVCONFIG;
        break;
    case PARSE_SERVER_KEY:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_SERVER_CERT:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_SERVER_PKI:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_APPLICATION_CERT:
        if (!end_app_certs(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_SRVCONFIG;
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
        ctx->state = PARSE_SRVCONFIG;
        break;
    case PARSE_LOCALE:
        ctx->state = PARSE_LOCALES;
        break;
    case PARSE_LOCALES:
        if (!SOPC_ConfigLoaderInternal_end_locales(true, &ctx->helper_ctx, ctx->localeIds,
                                                   &ctx->serverConfigPtr->localeIds))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->localeIds = NULL;
        ctx->state = PARSE_SRVCONFIG;
        break;
    case PARSE_SRVCONFIG:
        if (!end_server_config(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_S2OPC;
        break;
    case PARSE_CLICONFIG:
        ctx->cli_skip_depth--;
        if (ctx->cli_skip_depth == 0)
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

bool SOPC_ConfigServer_Parse(FILE* fd, SOPC_Server_Config* serverConfig)
{
    SOPC_ASSERT(NULL != serverConfig);
    SOPC_ServerConfig_Initialize(serverConfig);

    serverConfig->freeCstringsFlag = true; // C strings are allocated during parsing or NULL if undefined
    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);

    SOPC_Array* endpoints = SOPC_Array_Create(sizeof(SOPC_Endpoint_Config), 1, NULL);
    SOPC_Array* locales = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    if ((NULL == parser) || (NULL == endpoints) || (NULL == locales))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
        SOPC_Array_Delete(endpoints);
        SOPC_Array_Delete(locales);
        return false;
    }

    struct parse_context_t ctx;
    memset(&ctx, 0, sizeof(struct parse_context_t));
    XML_SetUserData(parser, &ctx);

    ctx.state = PARSE_START;
    ctx.helper_ctx.parser = parser;
    ctx.cli_skip_depth = 0;
    ctx.endpoints = endpoints;
    ctx.localeIds = locales;
    ctx.serverConfigPtr = serverConfig;
    ctx.helper_ctx.char_data_buffer = NULL;
    ctx.helper_ctx.char_data_cap = 0;
    OpcUa_ApplicationDescription_Initialize(&ctx.appDesc);
    ctx.appDesc.ApplicationType = OpcUa_ApplicationType_SizeOf;

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Array_Delete(ctx.localeIds);

    size_t nbEndpoints = SOPC_Array_Size(endpoints);

    if (res == SOPC_STATUS_OK && nbEndpoints <= UINT32_MAX)
    {
        serverConfig->endpoints = SOPC_Array_Into_Raw(ctx.endpoints);
        ctx.endpoints = NULL;
        SOPC_ASSERT(NULL != serverConfig->endpoints);
        serverConfig->nbEndpoints = (uint8_t) nbEndpoints;
        serverConfig->serverCertPath = ctx.serverCertificate;
        serverConfig->serverKeyPath = ctx.serverKey;
        serverConfig->serverPkiPath = ctx.serverPki;
        serverConfig->serverKeyEncrypted = ctx.serverKeyEncrypted;
        serverConfig->serverDescription = ctx.appDesc;
        return true;
    }
    else
    {
        SOPC_Array_Delete(endpoints);
        SOPC_Free(ctx.serverCertificate);
        SOPC_Free(ctx.serverKey);
        SOPC_Free(ctx.serverPki);
        OpcUa_ApplicationDescription_Clear(&ctx.appDesc);
        return false;
    }
}

bool SOPC_Config_Parse(FILE* fd, SOPC_S2OPC_Config* config)
{
    SOPC_S2OPC_Config_Initialize(config);
    bool res = SOPC_ConfigServer_Parse(fd, &config->serverConfig);
    if (res)
    {
        // Reset file position to beginning
        rewind(fd);
        res = SOPC_ConfigClient_Parse(fd, &config->clientConfig);
    }
    return res;
}
