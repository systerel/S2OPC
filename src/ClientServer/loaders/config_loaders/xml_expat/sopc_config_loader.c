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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "expat.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_array.h"
#include "sopc_encoder.h"
#include "sopc_helper_expat.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

typedef enum
{
    PARSE_START,               // Beginning of file
    PARSE_S2OPC,               // In a S2OPC
    PARSE_SRVCONFIG,           // ..In a server config tag
    PARSE_NAMESPACES,          // ....In namespaces
    PARSE_NAMESPACE,           // ......In namespace
    PARSE_LOCALES,             // ....In locales
    PARSE_LOCALE,              // ......In locale
    PARSE_APPLICATION_DESC,    // ....In application description
    PARSE_APPLICATION_URI,     // ......Application URI
    PARSE_PRODUCT_URI,         // ......Product URI
    PARSE_APPLICATION_NAME,    // ......Application Name
    PARSE_APPLICATION_CERT,    // ....In application certificate
    PARSE_SERVER_CERT,         // ......Server certificate
    PARSE_SERVER_KEY,          // ......Server key
    PARSE_TRUSTED_ISSUERS,     // ......Trusted issuer certificates
    PARSE_TRUSTED_ISSUER,      // ........Trusted issuer certificate + crl
    PARSE_ISSUED_CERTS,        // ......Trusted issued certificates
    PARSE_ISSUED_CERT,         // ........Trusted issued certificate
    PARSE_UNTRUSTED_ISSUERS,   // ......Untrusted issuer certificates
    PARSE_UNTRUSTED_ISSUER,    // ........Untrusted issuer certificate + crl
    PARSE_ENDPOINTS,           // ....In an Endpoints tag
    PARSE_ENDPOINT,            // ......In an Endpoint tag
    PARSE_REVERSE_CONNECTIONS, // ........In reverse connections tag
    PARSE_REVERSE_CONNECTION,  // ..........In reverse connection tag
    PARSE_SECURITY_POLICIES,   // ........In security policies tag
    PARSE_SECURITY_POLICY,     // ..........In security policy tag
    PARSE_SECURITY_MODES,      // ............In security modes tag
    PARSE_SECURITY_MODE,       // ..............In security mode tag
    PARSE_USER_POLICIES,       // ............In user polcies tag
    PARSE_USER_POLICY          // ..............In user policy tag
} parse_state_t;

struct parse_context_t
{
    SOPC_HelperExpatCtx helper_ctx;

    bool namespacesSet;
    SOPC_Array* namespaces;

    bool localesSet;
    SOPC_Array* localeIds;

    bool appDescSet;
    OpcUa_ApplicationDescription appDesc;

    bool appCertSet;
    char* serverCertificate;
    char* serverKey;
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

    bool endpointsSet;
    SOPC_Array* endpoints;
    SOPC_Endpoint_Config* currentEpConfig;
    SOPC_Server_Config* serverConfigPtr;

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

        if (XML_Parse(parser, buf, (int) r, 0) != XML_STATUS_OK)
        {
            enum XML_Error parser_error = XML_GetErrorCode(parser);

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

static const char* get_attr(struct parse_context_t* ctx, const char* attr_name, const XML_Char** attrs)
{
    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp(attr_name, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
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

static bool end_server_config(struct parse_context_t* ctx)
{
    if (!ctx->namespacesSet)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no namespaces defined for the server");
        return false;
    }

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

static bool end_namespaces(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->namespaces))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no namespace defined for the server");
        return false;
    }
    if (!SOPC_Array_Append_Values(ctx->namespaces, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->serverConfigPtr->namespaces = SOPC_Array_Into_Raw(ctx->namespaces);
    if (NULL == ctx->serverConfigPtr->namespaces)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->namespaces = NULL;
    return true;
}

static bool start_namespace(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = get_attr(ctx, "uri", attrs);

    char* uri = SOPC_strdup(attr_val);

    if (uri == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (!SOPC_Array_Append(ctx->namespaces, uri))
    {
        SOPC_Free(uri);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->state = PARSE_NAMESPACE;

    return true;
}

static bool end_locales(struct parse_context_t* ctx)
{
    if (0 == SOPC_Array_Size(ctx->localeIds))
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "no locales defined for the server");
        return false;
    }
    if (!SOPC_Array_Append_Values(ctx->localeIds, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->serverConfigPtr->localeIds = SOPC_Array_Into_Raw(ctx->localeIds);
    if (NULL == ctx->serverConfigPtr->localeIds)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->localeIds = NULL;
    return true;
}

static bool start_locale(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = get_attr(ctx, "id", attrs);

    char* id = SOPC_strdup(attr_val);

    if (id == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (!SOPC_Array_Append(ctx->localeIds, id))
    {
        SOPC_Free(id);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->state = PARSE_LOCALE;

    return true;
}

static bool end_app_desc(struct parse_context_t* ctx)
{
    if (ctx->appDesc.ApplicationUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ApplicationUri not defined");
        return false;
    }

    if (ctx->appDesc.ProductUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ProductUri not defined");
        return false;
    }

    if (ctx->appDesc.ApplicationName.defaultText.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ApplicationName not defined");
        return false;
    }

    return true;
}

static bool start_app_uri(struct parse_context_t* ctx, const XML_Char** attrs)
{
    if (ctx->appDesc.ApplicationUri.Length > 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ApplicationUri defined several times");
        return false;
    }

    const char* attr_val = get_attr(ctx, "uri", attrs);

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&ctx->appDesc.ApplicationUri, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    else if (ctx->appDesc.ApplicationUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Empty ApplicationUri uri");
        return false;
    }

    ctx->state = PARSE_APPLICATION_URI;

    return true;
}

static bool start_prod_uri(struct parse_context_t* ctx, const XML_Char** attrs)
{
    if (ctx->appDesc.ProductUri.Length > 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ProductUri defined several times");
        return false;
    }

    const char* attr_val = get_attr(ctx, "uri", attrs);

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&ctx->appDesc.ProductUri, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    else if (ctx->appDesc.ProductUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Empty ProductUri uri");
        return false;
    }

    ctx->state = PARSE_PRODUCT_URI;

    return true;
}

static bool start_app_name(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_text = get_attr(ctx, "text", attrs);
    const char* attr_locale = get_attr(ctx, "locale", attrs);

    if (NULL == attr_text || '\0' == attr_text[0])
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "Empty ApplicationName text");
        return false;
    }

    if (NULL == attr_locale)
    {
        attr_locale = "";
    }

    SOPC_LocalizedText tmp;
    SOPC_LocalizedText_Initialize(&tmp);
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&tmp.defaultLocale, attr_locale);
    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    status = SOPC_String_CopyFromCString(&tmp.defaultText, attr_text);

    if (SOPC_STATUS_OK == status)
    {
        // Additional definitions
        status =
            SOPC_LocalizedText_AddOrSetLocale(&ctx->appDesc.ApplicationName, ctx->serverConfigPtr->localeIds, &tmp);

        if (SOPC_STATUS_NOT_SUPPORTED == status)
        {
            SOPC_LocalizedText_Clear(&tmp);
            LOG_XML_ERRORF(ctx->helper_ctx.parser, "Application name provided for an unsupported locale %s",
                           attr_locale);
            return false;
        }
    }
    SOPC_LocalizedText_Clear(&tmp);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->state = PARSE_APPLICATION_NAME;

    return true;
}

static bool start_server_cert(struct parse_context_t* ctx, const XML_Char** attrs)
{
    if (ctx->serverCertificate != NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ServerCertificate defined several times");
        return false;
    }

    const char* attr_val = get_attr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->serverCertificate = path;

    ctx->state = PARSE_SERVER_CERT;

    return true;
}

static bool start_server_key(struct parse_context_t* ctx, const XML_Char** attrs)
{
    if (ctx->serverKey != NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "ServerKey defined several times");
        return false;
    }

    const char* attr_val = get_attr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->serverKey = path;

    ctx->state = PARSE_SERVER_KEY;

    return true;
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

static bool start_issuer(struct parse_context_t* ctx,
                         const XML_Char** attrs,
                         SOPC_Array* rootIssuers,
                         SOPC_Array* IntermediateIssuers)
{
    const char* attr_val = get_attr(ctx, "root", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "root attribute missing in Issuer definition");
        return false;
    }

    bool isRoot = (strcmp(attr_val, "true") == 0);

    attr_val = get_attr(ctx, "cert_path", attrs);

    char* pathCA = SOPC_strdup(attr_val);

    if (pathCA == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    SOPC_Array* issuers = isRoot ? rootIssuers : IntermediateIssuers;

    if (!SOPC_Array_Append(issuers, pathCA))
    {
        SOPC_Free(pathCA);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = get_attr(ctx, "revocation_list_path", attrs);

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

static bool start_issued_cert(struct parse_context_t* ctx, const XML_Char** attrs)
{
    const char* attr_val = get_attr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (!SOPC_Array_Append(ctx->issuedCertificates, path))
    {
        SOPC_Free(path);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    ctx->state = PARSE_ISSUED_CERT;

    return true;
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

static bool start_untrusted_issuer(struct parse_context_t* ctx, const XML_Char** attrs)
{
    bool result = start_issuer(ctx, attrs, ctx->untrustedRootIssuers, ctx->untrustedIntermediateIssuers);

    ctx->state = PARSE_UNTRUSTED_ISSUER;

    return result;
}

static bool end_application_certificates(struct parse_context_t* ctx)
{
    if (!SOPC_Array_Append_Values(ctx->trustedRootIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    ctx->serverConfigPtr->trustedRootIssuersList = SOPC_Array_Into_Raw(ctx->trustedRootIssuers);
    if (NULL == ctx->serverConfigPtr->trustedRootIssuersList)
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
    ctx->serverConfigPtr->trustedIntermediateIssuersList = SOPC_Array_Into_Raw(ctx->trustedIntermediateIssuers);
    if (NULL == ctx->serverConfigPtr->trustedIntermediateIssuersList)
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
    ctx->serverConfigPtr->issuedCertificatesList = SOPC_Array_Into_Raw(ctx->issuedCertificates);
    if (NULL == ctx->serverConfigPtr->issuedCertificatesList)
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
    ctx->serverConfigPtr->untrustedRootIssuersList = SOPC_Array_Into_Raw(ctx->untrustedRootIssuers);
    if (NULL == ctx->serverConfigPtr->untrustedRootIssuersList)
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
    ctx->serverConfigPtr->untrustedIntermediateIssuersList = SOPC_Array_Into_Raw(ctx->untrustedIntermediateIssuers);
    if (NULL == ctx->serverConfigPtr->untrustedIntermediateIssuersList)
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
    ctx->serverConfigPtr->certificateRevocationPathList = SOPC_Array_Into_Raw(ctx->crlCertificates);
    if (NULL == ctx->serverConfigPtr->certificateRevocationPathList)
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
    const char* attr_val = get_attr(ctx, "url", attrs);

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
    attr_val = get_attr(ctx, "hasDiscoveryEndpoint", attrs);

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
    attr_val = get_attr(ctx, "enableListening", attrs);
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

    ctx->state = PARSE_ENDPOINT;

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

    const char* attr_val = get_attr(ctx, "clientUrl", attrs);

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

    attr_val = get_attr(ctx, "clientAppUri", attrs);

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

    ctx->state = PARSE_REVERSE_CONNECTION;

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

    const char* attr_val = get_attr(ctx, "uri", attrs);

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

    ctx->state = PARSE_SECURITY_POLICY;

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
    const char* attr_val = get_attr(ctx, "mode", attrs);

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

    ctx->state = PARSE_SECURITY_MODE;

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

    const char* attr_val = get_attr(ctx, "policyId", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERROR(ctx->helper_ctx.parser, "policyId attribute missing");
        return false;
    }

    OpcUa_UserTokenPolicy* userPolicy = &secuPolicy->userTokenPolicies[secuPolicy->nbOfUserTokenPolicies];

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&userPolicy->PolicyId, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    attr_val = get_attr(ctx, "tokenType", attrs);

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
    else
    {
        LOG_XML_ERRORF(ctx->helper_ctx.parser, "tokenType attribute value %s not supported", attr_val);
        return false;
    }

    if (OpcUa_UserTokenType_UserName == userPolicy->TokenType)
    {
        attr_val = get_attr(ctx, "securityUri", attrs);

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

    ctx->state = PARSE_USER_POLICY;

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
        if (strcmp(name, "ServerConfiguration") != 0)
        {
            LOG_XML_ERRORF(helperCtx->parser, "Unexpected tag %s", name);
            XML_StopParser(helperCtx->parser, 0);
            return;
        }

        ctx->state = PARSE_SRVCONFIG;
        return;
    case PARSE_SRVCONFIG:
    {
        if (strcmp(name, "Namespaces") == 0 && !ctx->namespacesSet)
        {
            ctx->namespacesSet = true;
            ctx->state = PARSE_NAMESPACES;
        }
        else if (strcmp(name, "Locales") == 0 && !ctx->localesSet)
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
    case PARSE_NAMESPACES:
        if (strcmp(name, "Namespace") == 0)
        {
            if (!start_namespace(ctx, attrs))
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
    case PARSE_LOCALES:
        if (strcmp(name, "Locale") == 0)
        {
            if (!start_locale(ctx, attrs))
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
    case PARSE_APPLICATION_DESC:
        if (strcmp(name, "ApplicationURI") == 0)
        {
            if (!start_app_uri(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else if (strcmp(name, "ProductURI") == 0)
        {
            if (!start_prod_uri(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else if (strcmp(name, "ApplicationName") == 0)
        {
            if (!start_app_name(ctx, attrs))
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
    case PARSE_APPLICATION_CERT:
        if (strcmp(name, "ServerCertificate") == 0)
        {
            if (!start_server_cert(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
        }
        else if (strcmp(name, "ServerKey") == 0)
        {
            if (!start_server_key(ctx, attrs))
            {
                XML_StopParser(helperCtx->parser, 0);
                return;
            }
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
        if (strcmp(name, "IssuedCertificate") == 0)
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
        if (strcmp(name, "UntrustedIssuer") == 0)
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
    case PARSE_TRUSTED_ISSUER:
        ctx->state = PARSE_TRUSTED_ISSUERS;
        break;
    case PARSE_TRUSTED_ISSUERS:
        if (!end_trusted_issuers(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_ISSUED_CERT:
        ctx->state = PARSE_ISSUED_CERTS;
        break;
    case PARSE_ISSUED_CERTS:
        if (!end_issued_certs(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_APPLICATION_CERT;
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
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_SERVER_KEY:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_SERVER_CERT:
        ctx->state = PARSE_APPLICATION_CERT;
        break;
    case PARSE_APPLICATION_CERT:
        if (!end_application_certificates(ctx))
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
    case PARSE_APPLICATION_DESC:
        if (!end_app_desc(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
        ctx->state = PARSE_SRVCONFIG;
        break;
    case PARSE_NAMESPACE:
        ctx->state = PARSE_NAMESPACES;
        break;
    case PARSE_NAMESPACES:
        if (!end_namespaces(ctx))
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
        if (!end_locales(ctx))
        {
            XML_StopParser(ctx->helper_ctx.parser, 0);
            return;
        }
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
    case PARSE_S2OPC:
        break;
    case PARSE_START:
        assert(false && "Got end_element callback when in PARSE_START state.");
        break;
    default:
        assert(false && "Unknown state.");
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

bool SOPC_Config_Parse(FILE* fd, SOPC_S2OPC_Config* config)
{
    assert(NULL != config);
    SOPC_S2OPC_Config_Initialize(config);
    config->serverConfig.freeCstringsFlag = true; // C strings are allocated during parsing or NULL if undefined
    XML_Parser parser = XML_ParserCreateNS(NULL, NS_SEPARATOR[0]);

    SOPC_Array* endpoints = SOPC_Array_Create(sizeof(SOPC_Endpoint_Config), 1, NULL);
    SOPC_Array* ns = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* locales = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* trustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* trustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* issuedCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedRootIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);
    SOPC_Array* untrustedIntermediateIssuers = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    SOPC_Array* revokedListCerts = SOPC_Array_Create(sizeof(char*), 1, SOPC_Free_CstringFromPtr);

    if ((NULL == parser) || (NULL == endpoints) || (NULL == ns) || (NULL == locales) || (NULL == trustedRootIssuers) ||
        (NULL == trustedIntermediateIssuers) || (NULL == issuedCerts) || (NULL == untrustedRootIssuers) ||
        (NULL == untrustedIntermediateIssuers) || (NULL == revokedListCerts))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        XML_ParserFree(parser);
        SOPC_Array_Delete(endpoints);
        SOPC_Array_Delete(ns);
        SOPC_Array_Delete(locales);
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
    ctx.endpoints = endpoints;
    ctx.namespaces = ns;
    ctx.localeIds = locales;
    ctx.trustedRootIssuers = trustedRootIssuers;
    ctx.trustedIntermediateIssuers = trustedIntermediateIssuers;
    ctx.issuedCertificates = issuedCerts;
    ctx.untrustedRootIssuers = untrustedRootIssuers;
    ctx.untrustedIntermediateIssuers = untrustedIntermediateIssuers;
    ctx.crlCertificates = revokedListCerts;
    ctx.serverConfigPtr = &config->serverConfig;
    ctx.helper_ctx.char_data_buffer = NULL;
    ctx.helper_ctx.char_data_cap = 0;
    OpcUa_ApplicationDescription_Initialize(&ctx.appDesc);

    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    SOPC_ReturnStatus res = parse(parser, fd);
    XML_ParserFree(parser);
    SOPC_Array_Delete(ctx.namespaces);
    SOPC_Array_Delete(ctx.localeIds);
    SOPC_Array_Delete(ctx.trustedRootIssuers);
    SOPC_Array_Delete(ctx.trustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.issuedCertificates);
    SOPC_Array_Delete(ctx.untrustedRootIssuers);
    SOPC_Array_Delete(ctx.untrustedIntermediateIssuers);
    SOPC_Array_Delete(ctx.crlCertificates);

    size_t nbEndpoints = SOPC_Array_Size(endpoints);

    if (res == SOPC_STATUS_OK && nbEndpoints <= UINT32_MAX)
    {
        config->serverConfig.endpoints = SOPC_Array_Into_Raw(ctx.endpoints);
        ctx.endpoints = NULL;
        assert(NULL != config->serverConfig.endpoints);
        config->serverConfig.nbEndpoints = (uint8_t) nbEndpoints;
        config->serverConfig.serverCertPath = ctx.serverCertificate;
        config->serverConfig.serverKeyPath = ctx.serverKey;
        config->serverConfig.serverDescription = ctx.appDesc;
        return true;
    }
    else
    {
        SOPC_Array_Delete(ctx.endpoints);
        return false;
    }
}
