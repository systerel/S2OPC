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
#include "sopc_sub_scheduler.h"
#include "sopc_udp_sockets.h"
#include "sopc_xml_loader.h"

#include "config.h"
#include "pubsub.h"
#include "server.h"

#ifdef PUBSUB_STATIC_CONFIG
#include "pubsub_config_static.h"
#endif

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

static int32_t pubsubOnline = 0;
static SOPC_PubSubConfiguration* g_pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* g_pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* g_pSourceConfig = NULL;

static void free_global_configurations(void);

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

    if (SOPC_STATUS_OK == status)
    {
        free_global_configurations();
        g_pPubSubConfig = pPubSubConfig;
        g_pTargetConfig = pTargetConfig;
        g_pSourceConfig = pSourceConfig;
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

bool PubSub_Start(void)
{
    bool subOK = false;
    bool pubOK = false;
    uint32_t sub_nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(g_pPubSubConfig);
    uint32_t pub_nb_connections = SOPC_PubSubConfiguration_Nb_PubConnection(g_pPubSubConfig);
#ifdef WITH_STATIC_SECURITY_DATA
    printf("# Info: initialize local SKS with static security data\n");

    SOPC_LocalSKS_init_static(pubSub_keySign, sizeof(pubSub_keySign),       //
                              pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt), //
                              pubSub_keyNonce, sizeof(pubSub_keyNonce));    //
#else
    printf("# Info: initialize local SKS with dynamic security data\n");
    SOPC_LocalSKS_init(PUBSUB_SKS_SIGNING_KEY, PUBSUB_SKS_ENCRYPT_KEY, PUBSUB_SKS_KEY_NONCE);
#endif
    if (sub_nb_connections > 0)
    {
        subOK = SOPC_SubScheduler_Start(g_pPubSubConfig, g_pTargetConfig, Server_SetSubStatus, 0);
    }
    if (pub_nb_connections > 0)
    {
        /* Note: the priority is 0 here because other values require higher execution privileges.
         *  However, this sample is currently used in tests, so it should not require admin privileges to be run */
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

    return subOK || pubOK;
}

void PubSub_Stop(void)
{
    SOPC_Atomic_Int_Set(&pubsubOnline, 0);
    SOPC_SubScheduler_Stop();
    SOPC_PubScheduler_Stop();
    SOPC_PubSub_Protocol_ReleaseMqttManagerHandle();
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
