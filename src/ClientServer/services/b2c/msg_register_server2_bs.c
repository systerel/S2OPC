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

#include "msg_register_server2_bs.h"

#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_register_server2_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static const char* normativeServerCapabilities[] = {"NA", "DA",  "HD",  "AC",   "HE",  "GDS", "LDS",
                                                    "DI", "ADI", "FDI", "FDIC", "PLC", "S95", NULL};

void msg_register_server2_bs__check_mdns_server_capabilities(
    const constants__t_MdnsDiscoveryConfig_i msg_register_server2_bs__p_mdns_discovery_configuration,
    t_bool* const msg_register_server2_bs__p_valid_server_capabilities)
{
    const OpcUa_MdnsDiscoveryConfiguration* mdnsCfg = msg_register_server2_bs__p_mdns_discovery_configuration;
    *msg_register_server2_bs__p_valid_server_capabilities = true;
    if (mdnsCfg->NoOfServerCapabilities > 0)
    {
        // Validates provided capabilities regarding the normative ones
        for (int i = 0; i < mdnsCfg->NoOfServerCapabilities && *msg_register_server2_bs__p_valid_server_capabilities;
             i++)
        {
            const char* serverCapability = SOPC_String_GetRawCString(&mdnsCfg->ServerCapabilities[i]);
            const char* normServerCapability = normativeServerCapabilities[0];
            size_t serverCapibilityLen = 0;
            bool isValidServerCapability = false;
            // It shall be one of the normative ones
            for (int j = 1; NULL != normServerCapability && !isValidServerCapability; j++)
            {
                serverCapibilityLen = strlen(normServerCapability);
                // Comparison ignoring case
                isValidServerCapability = (serverCapibilityLen == strlen(serverCapability) && // same size
                                           0 == SOPC_strncmp_ignore_case(normServerCapability, serverCapability,
                                                                         serverCapibilityLen)); // same capability
                normServerCapability = normativeServerCapabilities[j];
            }
            *msg_register_server2_bs__p_valid_server_capabilities = isValidServerCapability;
        }
    }
}

void msg_register_server2_bs__check_mdns_server_name(
    const constants__t_MdnsDiscoveryConfig_i msg_register_server2_bs__p_mdns_discovery_configuration,
    t_bool* const msg_register_server2_bs__p_valid_mdns_server_name)
{
    const OpcUa_MdnsDiscoveryConfiguration* mdnsCfg = msg_register_server2_bs__p_mdns_discovery_configuration;
    // Name not empty and < 64 bytes (see Part4 v1.03, Table 103)
    *msg_register_server2_bs__p_valid_mdns_server_name =
        mdnsCfg->MdnsServerName.Length > 0 && mdnsCfg->MdnsServerName.Length < 64;
}

void msg_register_server2_bs__check_registered_discovery_url(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_discovery_url)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    // It shall not be empty
    *msg_register_server2_bs__p_valid_discovery_url =
        (regServ->NoOfDiscoveryUrls > 0 && regServ->DiscoveryUrls[0].Length > 0);
}

void msg_register_server2_bs__check_registered_product_uri(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_product_uri)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    // It shall not be empty
    *msg_register_server2_bs__p_valid_product_uri = regServ->ProductUri.Length > 0;
}

void msg_register_server2_bs__check_registered_semaphore_file(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_semaphore_file)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    // It shall be empty since it is not supported !
    *msg_register_server2_bs__p_valid_semaphore_file = regServ->SemaphoreFilePath.Length <= 0;
}

void msg_register_server2_bs__check_registered_server_names(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_server_names)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    // It shall not be empty
    *msg_register_server2_bs__p_valid_server_names =
        regServ->NoOfServerNames > 0 && regServ->ServerNames[0].defaultText.Length > 0;
}

void msg_register_server2_bs__check_registered_server_type(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_server_type)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    switch (regServ->ServerType)
    {
    case OpcUa_ApplicationType_Server:
    case OpcUa_ApplicationType_ClientAndServer:
    case OpcUa_ApplicationType_DiscoveryServer:
        *msg_register_server2_bs__p_valid_server_type = true;
        break;
    default:
        *msg_register_server2_bs__p_valid_server_type = false;
    }
}

void msg_register_server2_bs__check_registered_server_uri(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_valid_server_uri)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    // ServerURI is not empty
    *msg_register_server2_bs__p_valid_server_uri = regServ->ServerUri.Length > 0;
}

void msg_register_server2_bs__get_register_server2_req_registered_server(
    const constants__t_msg_i msg_register_server2_bs__p_req,
    constants__t_RegisteredServer_i* const msg_register_server2_bs__p_registered_server)
{
    *msg_register_server2_bs__p_registered_server =
        &(((OpcUa_RegisterServer2Request*) msg_register_server2_bs__p_req)->Server);
}

void msg_register_server2_bs__get_registered_server_is_online(
    const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
    t_bool* const msg_register_server2_bs__p_is_online)
{
    const OpcUa_RegisteredServer* regServ = msg_register_server2_bs__p_registered_server;
    *msg_register_server2_bs__p_is_online = regServ->IsOnline;
}

void msg_register_server2_bs__getall_register_server2_req_msdn_discovery_config(
    const constants__t_msg_i msg_register_server2_bs__p_req,
    t_bool* const msg_register_server2_bs__p_has_discovery_configuration,
    t_bool* const msg_register_server2_bs__p_has_one_and_only_one_mdns_config,
    constants__t_MdnsDiscoveryConfig_i* const msg_register_server2_bs__p_mdns_discovery_configuration,
    t_entier4* const msg_register_server2_bs__p_nb_discovery_config,
    t_entier4* const msg_register_server2_bs__p_mdns_discovery_config_index)
{
    const OpcUa_RegisterServer2Request* req = (const OpcUa_RegisterServer2Request*) msg_register_server2_bs__p_req;
    *msg_register_server2_bs__p_has_discovery_configuration = false;
    *msg_register_server2_bs__p_has_one_and_only_one_mdns_config = false;
    *msg_register_server2_bs__p_nb_discovery_config = 0;
    *msg_register_server2_bs__p_mdns_discovery_config_index = 0;
    *msg_register_server2_bs__p_mdns_discovery_configuration = constants__c_MdnsDiscoveryConfig_indet;
    if (req->NoOfDiscoveryConfiguration > 0)
    {
        *msg_register_server2_bs__p_has_discovery_configuration = true;
    }
    else
    {
        return;
    }
    bool hasMdnsConfig = false;
    int32_t nbMdnsConfigs = 0;
    int32_t mdnsIndex = 0;
    constants__t_MdnsDiscoveryConfig_i mdnsConfig = NULL;
    for (int32_t i = 0; i < req->NoOfDiscoveryConfiguration; i++)
    {
        SOPC_ExtensionObject* extObj = &req->DiscoveryConfiguration[i];
        if (SOPC_ExtObjBodyEncoding_Object == extObj->Encoding &&
            &OpcUa_MdnsDiscoveryConfiguration_EncodeableType == extObj->Body.Object.ObjType)
        {
            mdnsConfig = extObj->Body.Object.Value;
            mdnsIndex = i;
            nbMdnsConfigs++;
            hasMdnsConfig = true;
        }
    }
    if (hasMdnsConfig && 1 == nbMdnsConfigs)
    {
        *msg_register_server2_bs__p_has_one_and_only_one_mdns_config = true;
        *msg_register_server2_bs__p_nb_discovery_config = req->NoOfDiscoveryConfiguration;
        *msg_register_server2_bs__p_mdns_discovery_config_index = mdnsIndex;
        *msg_register_server2_bs__p_mdns_discovery_configuration = mdnsConfig;
    }
}

void msg_register_server2_bs__set_register_server2_resp_configuration_results(
    const constants__t_msg_i msg_register_server2_bs__p_resp,
    const t_entier4 msg_register_server2_bs__p_nb_discovery_config,
    const t_entier4 msg_register_server2_bs__p_mdns_discovery_config_index,
    constants_statuscodes_bs__t_StatusCode_i* const msg_register_server2_bs__p_sc)
{
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    OpcUa_RegisterServer2Response* resp = (OpcUa_RegisterServer2Response*) msg_register_server2_bs__p_resp;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    *msg_register_server2_bs__p_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    resp->ConfigurationResults =
        SOPC_Calloc((size_t) msg_register_server2_bs__p_nb_discovery_config, sizeof(*resp->ConfigurationResults));
    if (NULL != resp->ConfigurationResults)
    {
        *msg_register_server2_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
        resp->NoOfConfigurationResults = msg_register_server2_bs__p_nb_discovery_config;
        for (int32_t i = 0; i < resp->NoOfConfigurationResults; i++)
        {
            if (msg_register_server2_bs__p_mdns_discovery_config_index == i)
            {
                resp->ConfigurationResults[i] = SOPC_GoodGenericStatus;
            }
            else
            {
                resp->ConfigurationResults[i] = OpcUa_BadNotSupported;
            }
        }
    }
}
