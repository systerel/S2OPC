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

    if (strlen(attr_val) > 0)
    {
        char* path = SOPC_strdup(attr_val);

        if (path == NULL)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }

        *certificate = path;
    }

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

    if (strlen(attr_val) > 0)
    {
        char* path = SOPC_strdup(attr_val);

        if (path == NULL)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }

        *key = path;
    }

    attr_val = SOPC_HelperExpat_GetAttr(ctx, "encrypted", attrs);
    *encrypted = attr_val != NULL && 0 == strcmp(attr_val, "true");

    return true;
}

bool SOPC_ConfigLoaderInternal_start_pki(bool isServer, SOPC_HelperExpatCtx* ctx, char** pkiPath, const char** attrs)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != pkiPath);
    if (*pkiPath != NULL)
    {
        LOG_XML_ERRORF(ctx->parser, "%s PublicKeyInfrastructure defined several times", CLIENT_OR_SERVER(isServer));
        return false;
    }

    const char* attr_val = SOPC_HelperExpat_GetAttr(ctx, "path", attrs);

    if (strlen(attr_val) > 0)
    {
        char* path = SOPC_strdup(attr_val);
        if (path == NULL)
        {
            LOG_XML_ERRORF(ctx->parser, "%s PublicKeyInfrastructure: no path defined", CLIENT_OR_SERVER(isServer));
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }
        *pkiPath = path;
    }
    else
    {
        LOG_XML_ERRORF(ctx->parser, "%s PublicKeyInfrastructure: empty path is forbidden", CLIENT_OR_SERVER(isServer));
        return false;
    }

    return true;
}
