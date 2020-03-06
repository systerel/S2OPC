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

#include "msg_find_servers_bs.h"

#include <assert.h>

#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_find_servers_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_find_servers_bs__alloc_find_servers(const constants__t_msg_i msg_find_servers_bs__p_resp,
                                             const t_entier4 msg_find_servers_bs__p_nb_servers,
                                             t_bool* const msg_find_servers_bs__p_allocSuccess)
{
    *msg_find_servers_bs__p_allocSuccess = false;
    assert(msg_find_servers_bs__p_nb_servers > 0);
    OpcUa_FindServersResponse* resp = msg_find_servers_bs__p_resp;
    resp->Servers = SOPC_Malloc(sizeof(*resp->Servers) * (size_t) msg_find_servers_bs__p_nb_servers);
    if (NULL != resp->Servers)
    {
        resp->NoOfServers = msg_find_servers_bs__p_nb_servers;
        for (int32_t i = 0; i < resp->NoOfServers; i++)
        {
            OpcUa_ApplicationDescription_Initialize(&resp->Servers[i]);
        }
        *msg_find_servers_bs__p_allocSuccess = true;
    }
}

void msg_find_servers_bs__get_find_servers_req_params(const constants__t_msg_i msg_find_servers_bs__p_req,
                                                      constants__t_LocaleIds_i* const msg_find_servers_bs__p_LocaleIds,
                                                      t_entier4* const msg_find_servers_bs__p_nbServerUri,
                                                      constants__t_ServerUris* const msg_find_servers_bs__p_ServerUris)
{
    OpcUa_FindServersRequest* req = msg_find_servers_bs__p_req;
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *msg_find_servers_bs__p_LocaleIds = SOPC_String_GetCStringArray(req->NoOfLocaleIds, req->LocaleIds);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    assert(NULL != *msg_find_servers_bs__p_LocaleIds); // TODO: change LocaleIds format or return alloc boolean
    *msg_find_servers_bs__p_nbServerUri = req->NoOfServerUris;
    *msg_find_servers_bs__p_ServerUris = req->ServerUris;
}

static bool registerServerToApplicationDescription(const OpcUa_RegisteredServer* regServ,
                                                   char** localeIds,
                                                   OpcUa_ApplicationDescription* appDesc)
{
    SOPC_ReturnStatus status = SOPC_LocalizedTextArray_GetPreferredLocale(
        &appDesc->ApplicationName, localeIds, regServ->NoOfServerNames, regServ->ServerNames);
    if (SOPC_STATUS_OK != status)
    {
        return false;
    }
    if (regServ->NoOfDiscoveryUrls > 0)
    {
        appDesc->DiscoveryUrls = SOPC_Malloc(sizeof(SOPC_String) * (size_t) regServ->NoOfDiscoveryUrls);
        if (NULL != appDesc->DiscoveryUrls)
        {
            appDesc->NoOfDiscoveryUrls = regServ->NoOfDiscoveryUrls;
            for (int32_t i = 0; i < appDesc->NoOfDiscoveryUrls; i++)
            {
                status = SOPC_String_AttachFrom(&appDesc->DiscoveryUrls[i], &regServ->DiscoveryUrls[i]);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "Failed to set DiscoveryUrl in registered server of response");
                }
            }
        }
        else
        {
            SOPC_LocalizedText_Clear(&appDesc->ApplicationName);
            return false;
        }
    }

    appDesc->ApplicationType = regServ->ServerType;
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    status = SOPC_String_AttachFrom(&appDesc->ApplicationUri, (SOPC_String*) &regServ->ServerUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Application URI in application description of response");
    }
    // not managed: appDesc->DiscoveryProfileUri
    if (regServ->GatewayServerUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&appDesc->GatewayServerUri, (SOPC_String*) &regServ->GatewayServerUri);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set GatewayServerUri in application description of response");
        }
    }
    status = SOPC_String_AttachFrom(&appDesc->ProductUri, (SOPC_String*) &regServ->ProductUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Product URI in application description of response");
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE
    return true;
}

void msg_find_servers_bs__set_find_servers_server(
    const constants__t_msg_i msg_find_servers_bs__p_resp,
    const t_entier4 msg_find_servers_bs__p_srv_index,
    const constants__t_LocaleIds_i msg_find_servers_bs__p_localeIds,
    const constants__t_RegisteredServer_i msg_find_servers_bs__p_registered_server,
    constants_statuscodes_bs__t_StatusCode_i* const msg_find_servers_bs__ret)
{
    *msg_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    OpcUa_FindServersResponse* resp = msg_find_servers_bs__p_resp;
    assert(resp->NoOfServers > msg_find_servers_bs__p_srv_index);
    bool success = registerServerToApplicationDescription(msg_find_servers_bs__p_registered_server,
                                                          msg_find_servers_bs__p_localeIds,
                                                          &resp->Servers[msg_find_servers_bs__p_srv_index]);
    if (success)
    {
        *msg_find_servers_bs__ret = constants_statuscodes_bs__e_sc_ok;
    }
}

static bool has_none_security_mode(SOPC_Endpoint_Config* epConfig)
{
    for (int i = 0; i < epConfig->nbSecuConfigs; i++)
    {
        if ((epConfig->secuConfigurations[i].securityModes & SOPC_SECURITY_MODE_NONE_MASK) != 0)
        {
            return true;
        }
    }
    return false;
}

static bool util_ApplicationDescription_addImplicitDiscoveryEndpoint(OpcUa_ApplicationDescription* dst,
                                                                     SOPC_Endpoint_Config* endpoint_config)
{
    assert(NULL != dst);
    assert(NULL != endpoint_config);
    assert(dst->NoOfDiscoveryUrls <= 0);

    /* If no discovery URL already defined for the current endpoint, try to find one.
     * It has one, if either the current endpoint has an implicit discovery endpoint
     * or the current endpoint accepts None security mode.
     * Otherwise we are not able to define automatically a discovery endpoint.
     *
     */
    if (endpoint_config->hasDiscoveryEndpoint || has_none_security_mode(endpoint_config))
    {
        // There is an endpoint with SecurityMode None allowed or an implicit discovery endpoint is added
        dst->DiscoveryUrls = SOPC_Calloc(1, sizeof(SOPC_String));
        SOPC_String_Initialize(dst->DiscoveryUrls);

        if (dst->DiscoveryUrls == NULL)
        {
            return false;
        }

        SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&dst->DiscoveryUrls[0], endpoint_config->endpointURL);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(dst->DiscoveryUrls);
            dst->DiscoveryUrls = NULL;
            return false;
        }

        dst->NoOfDiscoveryUrls = 1;
    }

    return true;
}

void msg_find_servers_bs__set_find_servers_server_ApplicationDescription(
    const constants__t_msg_i msg_find_servers_bs__p_resp,
    const t_entier4 msg_find_servers_bs__p_srv_index,
    const constants__t_LocaleIds_i msg_find_servers_bs__p_localeIds,
    const constants__t_endpoint_config_idx_i msg_find_servers_bs__p_endpoint_config_idx,
    const constants__t_ApplicationDescription_i msg_find_servers_bs__p_app_desc,
    constants_statuscodes_bs__t_StatusCode_i* const msg_find_servers_bs__ret)
{
    *msg_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    OpcUa_FindServersResponse* resp = msg_find_servers_bs__p_resp;
    assert(resp->NoOfServers > msg_find_servers_bs__p_srv_index);
    const OpcUa_ApplicationDescription* src = msg_find_servers_bs__p_app_desc;
    OpcUa_ApplicationDescription* dest = &resp->Servers[msg_find_servers_bs__p_srv_index];
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_ReturnStatus status = SOPC_LocalizedText_GetPreferredLocale(
        &dest->ApplicationName, (char**) msg_find_servers_bs__p_localeIds, &src->ApplicationName);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    if (SOPC_STATUS_OK != status)
    {
        return;
    }
    if (src->NoOfDiscoveryUrls > 0)
    {
        dest->DiscoveryUrls = SOPC_Calloc((size_t) src->NoOfDiscoveryUrls, sizeof(SOPC_String));
        if (NULL != dest->DiscoveryUrls)
        {
            dest->NoOfDiscoveryUrls = src->NoOfDiscoveryUrls;
            for (int32_t i = 0; i < dest->NoOfDiscoveryUrls; i++)
            {
                status = SOPC_String_AttachFrom(&dest->DiscoveryUrls[i], &src->DiscoveryUrls[i]);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "Failed to set DiscoveryUrls in application description of response");
                }
            }
        }
        else
        {
            SOPC_LocalizedText_Clear(&dest->ApplicationName);
            return;
        }
    }
    else
    {
        // Attempt to add implicit discovery URL of current endpoint to the application description
        SOPC_Endpoint_Config* endpoint_config =
            SOPC_ToolkitServer_GetEndpointConfig(msg_find_servers_bs__p_endpoint_config_idx);
        if (endpoint_config == NULL)
        {
            return;
        }

        if (!util_ApplicationDescription_addImplicitDiscoveryEndpoint(dest, endpoint_config))
        {
            return;
        }
    }

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    dest->ApplicationType = src->ApplicationType;
    status = SOPC_String_AttachFrom(&dest->ApplicationUri, (SOPC_String*) &src->ApplicationUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Application URI in application description of response");
    }
    if (src->DiscoveryProfileUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&dest->DiscoveryProfileUri, (SOPC_String*) &src->DiscoveryProfileUri);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set DiscoveryProfileURI in application description of response");
        }
    }
    if (src->GatewayServerUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&dest->GatewayServerUri, (SOPC_String*) &src->GatewayServerUri);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set GatewayServerUri in application description of response");
        }
    }
    status = SOPC_String_AttachFrom(&dest->ProductUri, (SOPC_String*) &src->ProductUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Product URI in application description of response");
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE
    *msg_find_servers_bs__ret = constants_statuscodes_bs__e_sc_ok;
}
