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

// First MS Period of ordonnancer task. 500ms
#define PUBSUB_ORDONNANCER_FIRST_MSPERIOD 500

typedef struct SOPC_SKS_Local_Configuration
{
    char* securityGroupId;

    // array of SOPC_SecurityKeyServices. Data should not be cleared.
    SOPC_Array* sksArray;

    // array of SOPC_ReaderGroup. Data should not be cleared.
    SOPC_Array* readerGroupArray;

    // array of SOPC_WriterGroup. Data should not be cleared.
    SOPC_Array* writerGroupArray;

} SOPC_SKS_Local_Configuration;

static int32_t pubsubOnline = 0;
static SOPC_PubSubConfiguration* g_pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* g_pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* g_pSourceConfig = NULL;
static SOPC_SKOrdonnancer* g_skOrdonnancer = NULL;
static SOPC_SKManager* g_skManager = NULL;
static SOPC_SKProvider* g_skProvider = NULL;
static bool g_isSecurity = false;
static bool g_isSks = false;

/* Initialise and start SK Ordonnancer and SK Manager if security is needed */
static bool PubSub_local_SKS_Start(void);

static void free_global_configurations(void);

static SOPC_SKS_Local_Configuration* SOPC_SKS_Local_Configuration_Create(void);
static void SOPC_SKS_Local_Configuration_Clear(SOPC_SKS_Local_Configuration* config);

static SOPC_ReturnStatus get_sks_config(SOPC_PubSubConfiguration* pPubSubConfig,
                                        bool* isSecurity,
                                        SOPC_SKS_Local_Configuration** sksConfigArray,
                                        uint32_t* length);

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
    g_isSks = false;
    /* List of SKS servers configurations. */
    SOPC_SKS_Local_Configuration* sksConfigArray;
    uint32_t sksConfigLength;
    if (SOPC_STATUS_OK == status)
    {
        status = get_sks_config(pPubSubConfig, &g_isSecurity, &sksConfigArray, &sksConfigLength);
        assert(2 > sksConfigLength); /* Only one SKS configuration is managed */
        // Check if configuration provides a SKS
        g_isSks = (NULL != sksConfigArray && 0 < sksConfigLength && 0 < SOPC_Array_Size(sksConfigArray->sksArray));
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Cannot retrieve PubSub Security Mode information.\n");
        }
        else if (g_isSecurity && !g_isSks)
        {
            printf("# Warning: PubSub Security is used but no SKS address provided.\n");
        }
    }

    if (SOPC_STATUS_OK == status && g_isSecurity && g_isSks)
    {
        assert(UINT32_MAX > SOPC_Array_Size(sksConfigArray->sksArray));
        uint32_t nbSks = (uint32_t) SOPC_Array_Size(sksConfigArray->sksArray);
        SOPC_SKProvider** providers = SOPC_Calloc(nbSks, sizeof(SOPC_SKProvider*));

        if (NULL == providers)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        /* 1. Create one instance of BySKS SKProvider for each Sks description */
        for (uint32_t i = 0; i < nbSks && SOPC_STATUS_OK == status; i++)
        {
            SOPC_SecurityKeyServices* sksElt = SOPC_Array_Get(sksConfigArray->sksArray, SOPC_SecurityKeyServices*, i);
            assert(NULL != sksElt);
            uint32_t SecureChannel_Id =
                Client_AddSecureChannelconfig(SOPC_SecurityKeyServices_Get_EndpointUrl(sksElt),
                                              SOPC_SecurityKeyServices_Get_ServerCertificate(sksElt));
            if (0 < SecureChannel_Id)
            {
                // Create a SK Provider which get Keys from a GetSecurityKeys request
                providers[i] = Client_Provider_BySKS_Create(SecureChannel_Id);
                if (NULL == providers[i])
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
                printf("# Error: PubSub Cannot configure Get Security Keys\n");
            }
        }

        /* 2. Then wrap these instances in at TryList SKProvider */
        if (SOPC_STATUS_OK == status)
        {
            g_skProvider = SOPC_SKProvider_TryList_Create(providers, nbSks);
        }

        if (NULL == g_skProvider && NULL != providers)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            for (uint32_t i = 0; i < nbSks; i++)
            {
                // Create a SK Provider which get Keys from a GetSecurityKeys request
                SOPC_SKProvider_Clear(providers[i]);
                SOPC_Free(providers[i]);
            }
            SOPC_Free(providers);
        }
    }

    // Clean sks config data
    if (g_isSks)
    {
        for (uint32_t i = 0; i < sksConfigLength; i++)
        {
            SOPC_SKS_Local_Configuration_Clear(&sksConfigArray[i]);
        }
        SOPC_Free(sksConfigArray);
        sksConfigArray = 0;
        sksConfigLength = 0;
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
    if (!g_isSecurity || !g_isSks)
    {
        return true;
    }

    SOPC_SKBuilder* builder = NULL;
    SOPC_SKProvider* provider = g_skProvider;
    g_skProvider = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Initialise SK Manager
    g_skManager = SOPC_SKManager_Create();
    bool sksOK = (NULL != g_skManager);
    sksOK = sksOK && (NULL != provider);
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
        // If it fails, builder and provider are deleted in Ordonnancer Clear function.
        Client_Start();
        status = SOPC_SKOrdonnancer_Start(g_skOrdonnancer);
        sksOK = (SOPC_STATUS_OK == status);
    }

    if (sksOK)
    {
        SOPC_LocalSKS_init();
        SOPC_LocalSKS_setSkManager(g_skManager);
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

    Client_Stop();
    SOPC_LocalSKS_setSkManager(NULL);
    SOPC_SKOrdonnancer_StopAndClear(g_skOrdonnancer);
    SOPC_Free(g_skOrdonnancer);
    g_skOrdonnancer = NULL;
    SOPC_SKManager_Clear(g_skManager);
    SOPC_Free(g_skManager);
    g_skManager = NULL;
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

static bool PubSub_SearchSKSInArray(SOPC_Array* array, SOPC_SecurityKeyServices* sks)
{
    if (NULL == array || NULL == sks)
    {
        return false;
    }
    size_t size = SOPC_Array_Size(array);
    for (size_t i = 0; i < size; i++)
    {
        SOPC_SecurityKeyServices* elt = SOPC_Array_Get(array, SOPC_SecurityKeyServices*, i);
        assert(NULL != elt);
        if (0 == strcmp(SOPC_SecurityKeyServices_Get_EndpointUrl(elt), SOPC_SecurityKeyServices_Get_EndpointUrl(sks)))
        {
            return true;
        }
    }
    return false;
}

static SOPC_ReturnStatus get_sks_config(SOPC_PubSubConfiguration* pPubSubConfig,
                                        bool* isSecurity,
                                        SOPC_SKS_Local_Configuration** sksConfigArray,
                                        uint32_t* length)
{
    assert(NULL != isSecurity);
    assert(NULL != sksConfigArray);
    assert(NULL != length);
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    *isSecurity = false;
    *sksConfigArray = SOPC_SKS_Local_Configuration_Create();

    if (NULL == *sksConfigArray)
    {
        *length = 0;
        return SOPC_STATUS_NOK;
    }
    *length = 1;

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

            uint32_t nbSKS = SOPC_WriterGroup_Nb_SecurityKeyServices(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbSKS)
            {
                printf("# Error: SKS address but security mode is none\n");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode)
            {
                // use to check that all sks list are the same
                size_t sksArraySize = SOPC_Array_Size((*sksConfigArray)->sksArray);

                // add all Sks
                for (uint16_t sksIndex = 0; sksIndex < nbSKS; sksIndex++)
                {
                    SOPC_SecurityKeyServices* desc = SOPC_WriterGroup_Get_SecurityKeyServices_At(group, sksIndex);
                    assert(NULL != desc);
                    if (!PubSub_SearchSKSInArray((*sksConfigArray)->sksArray, desc))
                    {
                        SOPC_Array_Append((*sksConfigArray)->sksArray, desc);
                        SOPC_Array_Append((*sksConfigArray)->writerGroupArray, group);
                        if (0 < sksArraySize)
                        {
                            // 2 groups have different SKS list. All list are merged
                            printf("# Warning: Different SKS addresses in PubSub configuration\n");
                        }
                    }
                }
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

            uint32_t nbSKS = SOPC_ReaderGroup_Nb_SecurityKeyServices(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbSKS)
            {
                printf("# Error: SKS address but security mode is none\n");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode)
            {
                // use to check that all sks list are the same
                size_t sksArraySize = SOPC_Array_Size((*sksConfigArray)->sksArray);

                // add all Sks
                for (uint16_t sksIndex = 0; sksIndex < nbSKS; sksIndex++)
                {
                    SOPC_SecurityKeyServices* desc = SOPC_ReaderGroup_Get_SecurityKeyServices_At(group, sksIndex);
                    assert(NULL != desc);
                    if (!PubSub_SearchSKSInArray((*sksConfigArray)->sksArray, desc))
                    {
                        SOPC_Array_Append((*sksConfigArray)->sksArray, desc);
                        SOPC_Array_Append((*sksConfigArray)->readerGroupArray, group);
                        if (0 < sksArraySize)
                        {
                            // 2 groups have different SKS list. All list are merged
                            printf("# Warning: Different SKS addresses in PubSub configuration\n");
                        }
                    }
                }
            }
        }
    }

    return result;
}

static SOPC_SKS_Local_Configuration* SOPC_SKS_Local_Configuration_Create()
{
    bool allocSuccess = true;
    SOPC_SKS_Local_Configuration* result = SOPC_Calloc(1, sizeof(SOPC_SKS_Local_Configuration));
    allocSuccess = (NULL != result);
    if (allocSuccess)
    {
        result->securityGroupId = NULL; /* Security group id are not managed */
        result->sksArray = SOPC_Array_Create(sizeof(SOPC_SecurityKeyServices*), 1, NULL);
        allocSuccess = (NULL != result->sksArray);
    }

    if (allocSuccess)
    {
        result->readerGroupArray = SOPC_Array_Create(sizeof(SOPC_ReaderGroup*), 10, NULL);
        allocSuccess = (NULL != result->readerGroupArray);
    }

    if (allocSuccess)
    {
        result->writerGroupArray = SOPC_Array_Create(sizeof(SOPC_WriterGroup*), 10, NULL);
        allocSuccess = (NULL != result->writerGroupArray);
    }
    if (!allocSuccess)
    {
        SOPC_SKS_Local_Configuration_Clear(result);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

static void SOPC_SKS_Local_Configuration_Clear(SOPC_SKS_Local_Configuration* config)
{
    if (NULL == config)
    {
        return;
    }

    SOPC_Free(config->securityGroupId);
    config->securityGroupId = NULL;
    SOPC_Array_Delete(config->sksArray);
    config->sksArray = NULL;
    SOPC_Array_Delete(config->readerGroupArray);
    config->readerGroupArray = NULL;
    SOPC_Array_Delete(config->writerGroupArray);
    config->writerGroupArray = NULL;
}
