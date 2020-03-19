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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "sopc_array.h"
#include "sopc_atomic.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_ordonnancer.h"
#include "sopc_sk_provider.h"
#include "sopc_sub_scheduler.h"
#include "sopc_udp_sockets.h"
#include "sopc_xml_loader.h"

#include "client.h"
#include "config.h"
#include "pubsub.h"
#include "server.h"

#ifdef PUBSUB_STATIC_CONFIG
#include "pubsub_config_static.h"
#endif

// First MS Period of ordonnancer task. 3s
#define PUBSUB_ORDONNANCER_FIRST_MSPERIOD 3000

static int32_t pubsubOnline = 0;
static SOPC_PubSubConfiguration* g_pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* g_pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* g_pSourceConfig = NULL;
static SOPC_SKOrdonnancer* g_skOrdonnancer = NULL;
static SOPC_SKManager* g_skManager = NULL;
static bool g_isSecurity = false;

/* Initialise and start SK Ordonnancer and SK Manager if security is needed */
static bool PubSub_local_SKS_Start(void);

static void free_global_configurations(void);

static SOPC_ReturnStatus get_sks_address(SOPC_PubSubConfiguration* pPubSubConfig,
                                         bool* isSecurity,
                                         const char** sksAddress);

#ifndef PUBSUB_STATIC_CONFIG
static void PubSub_SaveConfiguration(char* configBuffer);

static void PubSub_SaveConfiguration(char* configBuffer)
{
    FILE* fd = fopen(PUBSUB_CONFIG_PATH, "w");
    if (NULL != fd)
    {
        const int ret = fputs(configBuffer, fd);
        if (EOF == ret)
        {
            printf("# Error: Cannot write %s\n", configBuffer);
        }
        fclose(fd);
    }
    else
    {
        printf("# Error: Cannot open \"%s\": %s\n", PUBSUB_CONFIG_PATH, strerror(errno));
    }
}
#endif

SOPC_ReturnStatus PubSub_Configure(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
#ifdef PUBSUB_STATIC_CONFIG
    /* static configuration cannot be changed */
    if (NULL != g_pPubSubConfig)
    {
        return SOPC_STATUS_OK;
    }
    SOPC_PubSubConfiguration* pPubSubConfig = SOPC_PubSubConfig_GetStatic();
    if (NULL == pPubSubConfig)
    {
        return SOPC_STATUS_NOK;
    }
    printf("# Info: PubSub static configuration loaded.\n");
#else
    /* PubSub Configuration */
    SOPC_Array* configBuffers = Server_GetConfigurationPaths();
    if (NULL == configBuffers || SOPC_Array_Size(configBuffers) != 1)
    {
        printf("# Error: Multiple configuration paths.\n");
        SOPC_Array_Delete(configBuffers);
        return SOPC_STATUS_NOK;
    }

    char* configBuffer = SOPC_Array_Get(configBuffers, char*, 0);
    FILE* fd = SOPC_FileSystem_fmemopen((void*) configBuffer, strlen(configBuffer), "r");
    SOPC_PubSubConfiguration* pPubSubConfig = NULL;

    if (NULL == fd)
    {
        printf("# Error: Cannot open \"%s\": %s.\n", configBuffer, strerror(errno));
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pPubSubConfig = SOPC_PubSubConfig_ParseXML(fd);
        if (NULL == pPubSubConfig)
        {
            printf("# Error: Cannot parse PubSub configuration file \"%s\".\n", configBuffer);
            status = SOPC_STATUS_NOK;
        }
        else
        {
            printf("# Info: PubSub XML configuration loaded.\n");
        }
    }
    if (NULL != fd)
    {
        fclose(fd);
        fd = NULL;
    }
#endif

    /* Sub target configuration */
    SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pTargetConfig = SOPC_SubTargetVariableConfig_Create(&Server_SetTargetVariables);
        if (NULL == pTargetConfig)
        {
            printf("# Error: Cannot create Sub configuration.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    /* Pub target configuration */
    SOPC_PubSourceVariableConfig* pSourceConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pSourceConfig = SOPC_PubSourceVariableConfig_Create(&Server_GetSourceVariables);
        if (NULL == pSourceConfig)
        {
            printf("# Error: Cannot create Pub configuration.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    /* at least one message with encrypt and/or sign security  */
    g_isSecurity = false;
    /* first SKS address found */
    const char* sksAddress = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = get_sks_address(pPubSubConfig, &g_isSecurity, &sksAddress);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Cannot retrieve PubSub Security Mode information.\n");
        }
        else if (g_isSecurity && NULL == sksAddress)
        {
            printf("# Warning: PubSub Security is used but no SKS address provided.\n");
        }
    }

    /* TODO multi sks. Keep SC config */
    if (SOPC_STATUS_OK == status && g_isSecurity && NULL != sksAddress)
    {
        if (SOPC_STATUS_OK == status)
        {
            status = Client_AddSecureChannelconfig(sksAddress, SKS_SERVER_CERT_PATH);
        }

        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: PubSub Cannot configure Get Security Keys\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        free_global_configurations();
        g_pPubSubConfig = pPubSubConfig;
        g_pTargetConfig = pTargetConfig;
        g_pSourceConfig = pSourceConfig;
    }
    else
    {
        SOPC_SubTargetVariableConfig_Delete(pTargetConfig);
        SOPC_PubSourceVariableConfig_Delete(pSourceConfig);
        SOPC_PubSubConfiguration_Delete(pPubSubConfig);
    }

#ifndef PUBSUB_STATIC_CONFIG
    /* Save XML configuration */
    if (SOPC_STATUS_OK == status)
    {
        PubSub_SaveConfiguration(configBuffer);
    }
    SOPC_Array_Delete(configBuffers);
#endif

    return status;
}

bool PubSub_IsRunning(void)
{
    return SOPC_Atomic_Int_Get(&pubsubOnline);
}

bool PubSub_local_SKS_Start()
{
    if (!g_isSecurity)
    {
        return true;
    }

    SOPC_SKBuilder* builder = NULL;
    SOPC_SKProvider* provider = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Initialise SK Manager
    g_skManager = SOPC_SKManager_Create();
    bool sksOK = (NULL != g_skManager);
    if (sksOK)
    {
        g_skOrdonnancer = SOPC_SKOrdonnancer_Create();
        sksOK = (NULL != g_skOrdonnancer);
    }

    // Create a SK Builder which replace all Keys of a SK Manager
    if (sksOK)
    {
        builder = SOPC_SKBuilder_Setter_Create();
        sksOK = (NULL != builder);
    }

    // Create a SK Provider which get Keys from a GetSecurityKeys request
    if (sksOK)
    {
        provider = Client_Provider_BySKS_Create();
        sksOK = (NULL != provider);
    }

    // Create a SK Ordonnancer with a single task : call GetSecurityKeys and replace all keys
    if (sksOK)
    {
        status = SOPC_SKOrdonnancer_AddTask(g_skOrdonnancer, builder, provider, g_skManager,
                                            PUBSUB_ORDONNANCER_FIRST_MSPERIOD); // Start with 3s
        sksOK = (SOPC_STATUS_OK == status);
    }

    if (!sksOK)
    {
        // Delete Builder and Provider. Manager and Ordonnancer are delete in Global Context Clear
        SOPC_SKBuilder_Clear(builder);
        SOPC_Free(builder);
        SOPC_SKProvider_Clear(provider);
        SOPC_Free(provider);
        printf("# Error: Local SKS cannot be init with Security Keys\n");
    }

    if (sksOK)
    {
        // If it fails, builder and provider are deleted in Ordonnacer Clear function.
        status = SOPC_SKOrdonnancer_Start(g_skOrdonnancer);
        sksOK = (SOPC_STATUS_OK == status);
    }

    if (sksOK)
    {
        SOPC_LocalSKS_init(g_skManager);
    }

    return sksOK;
}

bool PubSub_Start(void)
{
    bool subOK = false;
    bool pubOK = false;
    bool sksOK = false;
    uint32_t sub_nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(g_pPubSubConfig);
    uint32_t pub_nb_connections = SOPC_PubSubConfiguration_Nb_PubConnection(g_pPubSubConfig);

    sksOK = PubSub_local_SKS_Start();

    if (sub_nb_connections > 0)
    {
        subOK = SOPC_SubScheduler_Start(g_pPubSubConfig, g_pTargetConfig, Server_SetSubStatus);
    }
    if (pub_nb_connections > 0)
    {
        /* TODO: left 0 because this sample is currently used in tests */
        pubOK = SOPC_PubScheduler_Start(g_pPubSubConfig, g_pSourceConfig, 0);
    }
    if (subOK || pubOK)
    {
        SOPC_Atomic_Int_Set(&pubsubOnline, 1);

        if (!subOK)
        {
            // PubSubStatus NOT managed by Sub scheduler: set operational manually
            Server_SetSubStatus(SOPC_PubSubState_Operational);
        }
    }

    return sksOK && (subOK || pubOK);
}

void PubSub_Stop(void)
{
    SOPC_Atomic_Int_Set(&pubsubOnline, 0);
    SOPC_SubScheduler_Stop();
    SOPC_PubScheduler_Stop();
    SOPC_PubSub_Protocol_ReleaseMqttManagerHandle();

    SOPC_SKOrdonnancer_StopAndClear(g_skOrdonnancer);
    SOPC_Free(g_skOrdonnancer);
    SOPC_SKManager_Clear(g_skManager);
    SOPC_Free(g_skManager);

    // Force Disabled after stop in case Sub scheduler was not start (no management of the status)
    Server_SetSubStatus(SOPC_PubSubState_Disabled);
}

void PubSub_StopAndClear(void)
{
    PubSub_Stop();
    free_global_configurations();
}

static void free_global_configurations(void)
{
    SOPC_SubTargetVariableConfig_Delete(g_pTargetConfig);
    g_pTargetConfig = NULL;
    SOPC_PubSourceVariableConfig_Delete(g_pSourceConfig);
    g_pSourceConfig = NULL;
    SOPC_PubSubConfiguration_Delete(g_pPubSubConfig);
    g_pPubSubConfig = NULL;
}

static SOPC_ReturnStatus get_sks_address(SOPC_PubSubConfiguration* pPubSubConfig,
                                         bool* isSecurity,
                                         const char** sksAddress)
{
    assert(NULL != isSecurity);
    assert(NULL != sksAddress);
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    *isSecurity = false;
    *sksAddress = NULL;
    uint32_t pub_nb_connections = SOPC_PubSubConfiguration_Nb_PubConnection(pPubSubConfig);
    for (uint32_t i = 0; i < pub_nb_connections && SOPC_STATUS_OK == result; i++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(pPubSubConfig, i);
        uint16_t nbGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
        for (uint16_t j = 0; j < nbGroup && SOPC_STATUS_OK == result; j++)
        {
            SOPC_SecurityMode_Type securityMode;
            SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);
            assert(NULL != group);
            securityMode = SOPC_WriterGroup_Get_SecurityMode(group);
            if (SOPC_SecurityMode_None != securityMode)
            {
                *isSecurity = true; /* at least one group with security */
            }

            uint32_t nbEndpoint = SOPC_WriterGroup_Nb_EndPointDescription(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbEndpoint)
            {
                printf("# Error: SKS address but security mode is none\n");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode && 1 == nbEndpoint)
            {
                OpcUa_EndpointDescription* desc = SOPC_WriterGroup_Get_EndPointDescription(group, 0);
                assert(NULL != desc);
                if (NULL == *sksAddress)
                {
                    *sksAddress = SOPC_String_GetRawCString(&desc->EndpointUrl);
                }
                else if (0 != strcmp(*sksAddress, SOPC_String_GetRawCString(&desc->EndpointUrl)))
                {
                    printf("# Warning: Different SKS address in PubSub configuration\n");
                }
            }
            else if (1 < nbEndpoint)
            {
                /* Filtered by XML loader : should not happen */
                printf("# Error: Only one SKS address is managed per Group\n");
                result = SOPC_STATUS_NOK;
            }
        }
    }

    uint32_t sub_nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(pPubSubConfig);
    for (uint32_t i = 0; i < sub_nb_connections && SOPC_STATUS_OK == result; i++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(pPubSubConfig, i);
        uint16_t nbGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
        for (uint16_t j = 0; j < nbGroup && SOPC_STATUS_OK == result; j++)
        {
            SOPC_SecurityMode_Type securityMode;
            SOPC_ReaderGroup* group = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, j);
            assert(NULL != group);
            securityMode = SOPC_ReaderGroup_Get_SecurityMode(group);
            if (SOPC_SecurityMode_None != securityMode)
            {
                *isSecurity = true; /* at least one group with security */
            }

            uint32_t nbEndpoint = SOPC_ReaderGroup_Nb_EndPointDescription(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbEndpoint)
            {
                printf("# Error: SKS address but security mode is none\n");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode && 1 == nbEndpoint)
            {
                OpcUa_EndpointDescription* desc = SOPC_ReaderGroup_Get_EndPointDescription(group, 0);
                assert(NULL != desc);
                if (NULL == *sksAddress)
                {
                    *sksAddress = SOPC_String_GetRawCString(&desc->EndpointUrl);
                }
                else if (0 != strcmp(*sksAddress, SOPC_String_GetRawCString(&desc->EndpointUrl)))
                {
                    printf("# Warning: Different SKS address in PubSub configuration\n");
                }
            }
            else if (1 < nbEndpoint)
            {
                /* Filtered by XML loader : should not happen */
                printf("# Error: Only one SKS address is managed per Group\n");
                result = SOPC_STATUS_NOK;
            }
        }
    }

    return result;
}
