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

#include "sopc_config_loader_internal.h"

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

#define CLIENT_OR_SERVER(isServer) (isServer ? "Server" : "Client")

bool SOPC_ConfigLoaderInternal_end_locales(bool isServer,
                                           SOPC_HelperExpatCtx* ctx,
                                           SOPC_Array* ctxLocaleIds,
                                           char*** configLocaleIds)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != configLocaleIds);
    if (0 == SOPC_Array_Size(ctxLocaleIds))
    {
        LOG_XML_ERRORF(ctx->parser, "no locales defined for the %s", CLIENT_OR_SERVER(isServer));
        return false;
    }
    if (!SOPC_Array_Append_Values(ctxLocaleIds, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *configLocaleIds = SOPC_Array_Into_Raw(ctxLocaleIds);
    if (NULL == *configLocaleIds)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    return true;
}

bool SOPC_ConfigLoaderInternal_start_locale(SOPC_HelperExpatCtx* ctx, SOPC_Array* ctxLocaleIds, const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "id", attrs);

    char* id = SOPC_strdup(attr_val);

    if (id == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (!SOPC_Array_Append(ctxLocaleIds, id))
    {
        SOPC_Free(id);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_end_app_desc(bool isServer,
                                            SOPC_HelperExpatCtx* ctx,
                                            OpcUa_ApplicationDescription* appDesc)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != appDesc);
    if (appDesc->ApplicationUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->parser, "ApplicationUri not defined");
        return false;
    }

    if (appDesc->ProductUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->parser, "ProductUri not defined");
        return false;
    }

    if (appDesc->ApplicationName.defaultText.Length <= 0)
    {
        LOG_XML_ERROR(ctx->parser, "ApplicationName not defined");
        return false;
    }

    if (OpcUa_ApplicationType_SizeOf == appDesc->ApplicationType)
    {
        if (isServer)
        {
            // Set default value "Server"
            appDesc->ApplicationType = OpcUa_ApplicationType_Server;
        }
        else
        {
            // Set default value "Client"
            appDesc->ApplicationType = OpcUa_ApplicationType_Client;
        }
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_start_app_uri(bool isServer,
                                             SOPC_HelperExpatCtx* ctx,
                                             OpcUa_ApplicationDescription* appDesc,
                                             const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != appDesc);
    if (appDesc->ApplicationUri.Length > 0)
    {
        LOG_XML_ERRORF(ctx->parser, "%s ApplicationUri defined several times", CLIENT_OR_SERVER(isServer));
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "uri", attrs);

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&appDesc->ApplicationUri, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    else if (appDesc->ApplicationUri.Length <= 0)
    {
        LOG_XML_ERRORF(ctx->parser, "%s Empty ApplicationUri uri", CLIENT_OR_SERVER(isServer));
        return false;
    }
    return true;
}

bool SOPC_ConfigLoaderInternal_start_prod_uri(SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != appDesc);
    if (appDesc->ProductUri.Length > 0)
    {
        LOG_XML_ERROR(ctx->parser, "ProductUri defined several times");
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "uri", attrs);

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&appDesc->ProductUri, attr_val);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    else if (appDesc->ProductUri.Length <= 0)
    {
        LOG_XML_ERROR(ctx->parser, "Empty ProductUri uri");
        return false;
    }
    return true;
}

static bool parse_app_type_text(bool isServer, OpcUa_ApplicationDescription* appDesc, const char* text)
{
    SOPC_ASSERT(NULL != appDesc);
    if (isServer && strcmp(text, "Server") == 0)
    {
        appDesc->ApplicationType = OpcUa_ApplicationType_Server;
    }
    else if (strcmp(text, "ClientAndServer") == 0)
    {
        appDesc->ApplicationType = OpcUa_ApplicationType_ClientAndServer;
    }
    else if (isServer && strcmp(text, "DiscoveryServer") == 0)
    {
        appDesc->ApplicationType = OpcUa_ApplicationType_DiscoveryServer;
    }
    else if (!isServer && strcmp(text, "Client") == 0)
    {
        appDesc->ApplicationType = OpcUa_ApplicationType_Client;
    }
    else
    {
        return false;
    }
    return true;
}

bool SOPC_ConfigLoaderInternal_start_app_type(bool isServer,
                                              SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != appDesc);
    if (appDesc->ApplicationType != OpcUa_ApplicationType_SizeOf)
    {
        LOG_XML_ERRORF(ctx->parser, "%s ApplicationType defined several times", CLIENT_OR_SERVER(isServer));
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "type", attrs);

    if (NULL == attr_val)
    {
        LOG_XML_ERRORF(ctx->parser, "%s ApplicationType 'type' attribute missing", CLIENT_OR_SERVER(isServer));
        return false;
    }
    else if (!parse_app_type_text(isServer, appDesc, attr_val))
    {
        LOG_XML_ERRORF(ctx->parser, "%s invalid application type: %s", CLIENT_OR_SERVER(isServer), attr_val);
        return false;
    }
    return true;
}

bool SOPC_ConfigLoaderInternal_start_app_name(bool isServer,
                                              SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              char** configLocaleIds,
                                              const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != appDesc);
    SOPC_ASSERT(NULL != configLocaleIds);

    const char* attr_text = SOPC_HelperExpat_GetAttr(ctx, "text", attrs);
    const char* attr_locale = SOPC_HelperExpat_GetAttr(ctx, "locale", attrs);

    if (NULL == attr_text || '\0' == attr_text[0])
    {
        LOG_XML_ERROR(ctx->parser, "Empty ApplicationName text");
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
        // Note: for client we use preferred locales which should also be supported locales but no guarantee
        status = SOPC_LocalizedText_AddOrSetLocale(&appDesc->ApplicationName, configLocaleIds, &tmp);

        if (SOPC_STATUS_NOT_SUPPORTED == status)
        {
            SOPC_LocalizedText_Clear(&tmp);
            LOG_XML_ERRORF(ctx->parser, "%s application name provided for an unsupported locale %s",
                           CLIENT_OR_SERVER(isServer), attr_locale);
            return false;
        }
    }
    SOPC_LocalizedText_Clear(&tmp);

    if (SOPC_STATUS_OK != status)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_start_cert(bool isServer,
                                          SOPC_HelperExpatCtx* ctx,
                                          char** certificate,
                                          const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != certificate);
    if (*certificate != NULL)
    {
        LOG_XML_ERRORF(ctx->parser, "%s certificate defined several times", CLIENT_OR_SERVER(isServer));
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    *certificate = path;

    return true;
}

bool SOPC_ConfigLoaderInternal_start_key(bool isServer,
                                         SOPC_HelperExpatCtx* ctx,
                                         char** key,
                                         bool* encrypted,
                                         const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != key);
    SOPC_ASSERT(NULL != encrypted);
    if (*key != NULL)
    {
        LOG_XML_ERRORF(ctx->parser, "%s key defined several times", CLIENT_OR_SERVER(isServer));
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    *key = path;

    attr_val = SOPC_HelperExpat_GetAttr(ctx, "encrypted", attrs);
    *encrypted = attr_val != NULL && 0 == strcmp(attr_val, "true");

    return true;
}

bool SOPC_ConfigLoaderInternal_end_trusted_issuers(bool isServer,
                                                   SOPC_HelperExpatCtx* ctx,
                                                   SOPC_Array* trustedRootIssuers)
{
    SOPC_ASSERT(NULL != ctx);
    if (0 == SOPC_Array_Size(trustedRootIssuers))
    {
        LOG_XML_ERRORF(ctx->parser, "%s: no trusted root CA defined", CLIENT_OR_SERVER(isServer));
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_start_issuer(bool isServer,
                                            SOPC_HelperExpatCtx* ctx,
                                            SOPC_Array* rootIssuers,
                                            SOPC_Array* IntermediateIssuers,
                                            SOPC_Array* crlCertificates,
                                            const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "root", attrs);

    if (attr_val == NULL)
    {
        LOG_XML_ERRORF(ctx->parser, "%s: root attribute missing in Issuer definition", CLIENT_OR_SERVER(isServer));
        return false;
    }

    bool isRoot = (strcmp(attr_val, "true") == 0);

    attr_val = SOPC_HelperExpat_GetAttr(ctx, "cert_path", attrs);

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

    attr_val = SOPC_HelperExpat_GetAttr(ctx, "revocation_list_path", attrs);

    char* pathCRL = SOPC_strdup(attr_val);

    if (pathCRL == NULL)
    {
        LOGF("%s Warning: CRL missing for the root certificate '%s'", CLIENT_OR_SERVER(isServer), pathCA);
    }
    else if (!SOPC_Array_Append(crlCertificates, pathCRL))
    {
        SOPC_Free(pathCRL);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_end_issued_certs(bool isServer, SOPC_HelperExpatCtx* ctx, SOPC_Array* issuedCertificates)
{
    SOPC_ASSERT(NULL != ctx);

    if (0 == SOPC_Array_Size(issuedCertificates))
    {
        LOG_XML_ERRORF(ctx->parser, "%s: no issued certificates defined", CLIENT_OR_SERVER(isServer));
        return false;
    }
    return true;
}

bool SOPC_ConfigLoaderInternal_start_issued_cert(SOPC_HelperExpatCtx* ctx,
                                                 SOPC_Array* issuedCertificates,
                                                 const XML_Char** attrs)
{
    SOPC_ASSERT(NULL != ctx);

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "path", attrs);

    char* path = SOPC_strdup(attr_val);

    if (path == NULL)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    if (!SOPC_Array_Append(issuedCertificates, path))
    {
        SOPC_Free(path);
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_end_untrusted_issuers(bool isServer,
                                                     SOPC_HelperExpatCtx* ctx,
                                                     SOPC_Array* untrustedRootIssuers,
                                                     SOPC_Array* untrustedIntermediateIssuers)
{
    if (0 == SOPC_Array_Size(untrustedRootIssuers) && 0 == SOPC_Array_Size(untrustedIntermediateIssuers))
    {
        LOG_XML_ERRORF(ctx->parser, "%s: no untrusted CA defined", CLIENT_OR_SERVER(isServer));
        return false;
    }

    return true;
}

bool SOPC_ConfigLoaderInternal_end_application_certificates(bool isServer,
                                                            SOPC_HelperExpatCtx* ctx,
                                                            SOPC_Array** trustedRootIssuers,
                                                            char*** trustedRootIssuersList,
                                                            SOPC_Array** trustedIntermediateIssuers,
                                                            char*** trustedIntermediateIssuersList,
                                                            SOPC_Array** issuedCertificates,
                                                            char*** issuedCertificatesList,
                                                            SOPC_Array** untrustedRootIssuers,
                                                            char*** untrustedRootIssuersList,
                                                            SOPC_Array** untrustedIntermediateIssuers,
                                                            char*** untrustedIntermediateIssuersList,
                                                            SOPC_Array** crlCertificates,
                                                            char*** crlCertificatesList,
                                                            bool issuedCertificatesSet,
                                                            bool trustedIssuersSet)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != trustedRootIssuers);
    SOPC_ASSERT(NULL != trustedRootIssuersList);
    SOPC_ASSERT(NULL != trustedIntermediateIssuers);
    SOPC_ASSERT(NULL != trustedIntermediateIssuersList);
    SOPC_ASSERT(NULL != issuedCertificates);
    SOPC_ASSERT(NULL != issuedCertificatesList);
    SOPC_ASSERT(NULL != untrustedRootIssuers);
    SOPC_ASSERT(NULL != untrustedRootIssuersList);
    SOPC_ASSERT(NULL != untrustedIntermediateIssuers);
    SOPC_ASSERT(NULL != untrustedIntermediateIssuersList);
    SOPC_ASSERT(NULL != crlCertificates);
    SOPC_ASSERT(NULL != crlCertificatesList);

    if (!SOPC_Array_Append_Values(*trustedRootIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *trustedRootIssuersList = SOPC_Array_Into_Raw(*trustedRootIssuers);
    if (NULL == *trustedRootIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *trustedRootIssuers = NULL;

    if (!SOPC_Array_Append_Values(*trustedIntermediateIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *trustedIntermediateIssuersList = SOPC_Array_Into_Raw(*trustedIntermediateIssuers);
    if (NULL == *trustedIntermediateIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *trustedIntermediateIssuers = NULL;

    if (!SOPC_Array_Append_Values(*issuedCertificates, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *issuedCertificatesList = SOPC_Array_Into_Raw(*issuedCertificates);
    if (NULL == *issuedCertificatesList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *issuedCertificates = NULL;

    if (!SOPC_Array_Append_Values(*untrustedRootIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *untrustedRootIssuersList = SOPC_Array_Into_Raw(*untrustedRootIssuers);
    if (NULL == *untrustedRootIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *untrustedRootIssuers = NULL;

    if (!SOPC_Array_Append_Values(*untrustedIntermediateIssuers, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *untrustedIntermediateIssuersList = SOPC_Array_Into_Raw(*untrustedIntermediateIssuers);
    if (NULL == *untrustedIntermediateIssuersList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *untrustedIntermediateIssuers = NULL;

    if (!SOPC_Array_Append_Values(*crlCertificates, NULL, 1))
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *crlCertificatesList = SOPC_Array_Into_Raw(*crlCertificates);
    if (NULL == *crlCertificatesList)
    {
        LOG_MEMORY_ALLOCATION_FAILURE;
        return false;
    }
    *crlCertificates = NULL;

    if (!issuedCertificatesSet && !trustedIssuersSet)
    {
        LOG_XML_ERRORF(ctx->parser,
                       "%s: if application certificates section is defined, at least one issued certificate or trusted "
                       "CA should be defined.",
                       CLIENT_OR_SERVER(isServer));
        return false;
    }

    return true;
}
