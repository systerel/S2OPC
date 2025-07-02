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
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_pubsub_sks.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"
#include "sopc_sk_scheduler.h"
#include "sopc_sub_scheduler.h"
#include "sopc_toolkit_config.h"
#include "sopc_udp_sockets.h"
#include "sopc_xml_loader.h"

#include "device_config.h"
#include "device_pubsub.h"
#include "device_server.h"

// First MS Period of scheduler task. 500ms
#define PUBSUB_SCHEDULER_FIRST_MSPERIOD 500

typedef struct SOPC_SKS_Local_Configuration
{
    // array of security group id. Data should not be cleared.
    SOPC_Array* sksSecuGroupId;

} SOPC_SKS_Local_Configuration;

static int32_t pubsubOnline = 0;
static SOPC_PubSubConfiguration* g_pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* g_pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* g_pSourceConfig = NULL;
static bool g_isSecurity = false;
static bool g_isSks = false;

/* Initialise and start SK Scheduler and SK Manager if security is needed */
static bool PubSub_SKS_Start(void);

static void free_global_configurations(void);

static void pubSub_OnFatalError(void* userContext, const char* message)
{
    SOPC_UNUSED_ARG(userContext);
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "PubSub FatalError with message \"%s\"", message ? message : "");
}

static void Server_GapInDsmSnCb(SOPC_Conf_PublisherId pubId,
                                uint16_t groupId,
                                uint16_t writerId,
                                uint16_t prevSN,
                                uint16_t receivedSN)
{
    if (SOPC_UInteger_PublisherId == pubId.type)
    {
        printf("Gap detected in sequence numbers of DataSetMessage for PublisherId=%" PRIu64 " GroupId =%" PRIu16
               "  DataSetWriterId=%" PRIu16 ", missing SNs: [%" PRIu16 ", %" PRIu16 "]\n",
               pubId.data.uint, groupId, writerId, prevSN + 1, receivedSN - 1);
    }
    else
    {
        printf("Gap detected in sequence numbers of DataSetMessage for PublisherId=%s GroupId =%" PRIu16
               "  DataSetWriterId=%" PRIu16 ", missing SNs: [%" PRIu16 ", %" PRIu16 "]\n",
               SOPC_String_GetRawCString(&pubId.data.string), groupId, writerId, prevSN + 1, receivedSN - 1);
    }
}

static bool PubSub_SKS_Configure(SOPC_PubSubConfiguration* pPubSubConfig)
{
    if (!g_isSecurity)
    {
        return true;
    }
    else if (!g_isSks)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Create the SK managers for each security group from the configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_PubSubSKS_Init();
        bool res = SOPC_PubSubSKS_CreateManagersFromConfig(pPubSubConfig);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Security Keys Service: Configured\n");
    }
    else
    {
        // PubSub SKS cleared by PubSub_Stop
        printf("<Security Keys Service Error: Start failed\n");
    }

    return (SOPC_STATUS_OK == status);
}

SOPC_ReturnStatus PubSub_Configure(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* PubSub Configuration */
    FILE* fd = fopen(PUBSUB_CONFIG_PATH, "r");
    SOPC_PubSubConfiguration* pPubSubConfig = NULL;

    if (NULL == fd)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot open \"%s\": %s", PUBSUB_CONFIG_PATH, strerror(errno));
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        pPubSubConfig = SOPC_PubSubConfig_ParseXML(fd);
        if (NULL == pPubSubConfig)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot parse PubSub configuration file \"%s\"",
                                   PUBSUB_CONFIG_PATH);
            status = SOPC_STATUS_NOK;
        }
        else
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub XML configuration loaded");
        }
    }
    if (NULL != fd)
    {
        fclose(fd);
        fd = NULL;
    }

    /* Sub target configuration */
    SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pTargetConfig = SOPC_SubTargetVariableConfig_Create(&Server_SetTargetVariables);
        if (NULL == pTargetConfig)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot create Sub configuration");
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
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot create Pub configuration");
            status = SOPC_STATUS_NOK;
        }
    }

    /* at least one message with encrypt and/or sign security  */
    g_isSecurity = true;
    g_isSks = true;

    if (SOPC_STATUS_OK == status && g_isSecurity)
    {
        // Configure the SKS for both the PubSub applications of controller
        // and to push the security keys to devices
        bool sksOK = PubSub_SKS_Configure(pPubSubConfig);
        status = sksOK ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
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

    return status;
}

bool PubSub_IsRunning(void)
{
    return SOPC_Atomic_Int_Get(&pubsubOnline);
}

static bool PubSub_SKS_Start(void)
{
    if (!g_isSecurity)
    {
        return true;
    }
    else if (!g_isSks)
    {
        return false;
    }

    printf("<Security Keys Service: Started\n");

    return true;
}

bool PubSub_Start(void)
{
    bool subOK = false;
    bool pubOK = false;
    bool sksOK = false;
    uint32_t sub_nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(g_pPubSubConfig);
    uint32_t pub_nb_connections = SOPC_PubSubConfiguration_Nb_PubConnection(g_pPubSubConfig);

    sksOK = PubSub_SKS_Start();

    if (sub_nb_connections > 0)
    {
        subOK = SOPC_SubScheduler_Start(g_pPubSubConfig, g_pTargetConfig, NULL, Server_GapInDsmSnCb,
                                        pubSub_OnFatalError, 0);
    }
    if (pub_nb_connections > 0)
    {
        /* Note: the priority is 0 here because other values require higher execution privileges.
         *  However, this sample is currently used in tests, so it should not require admin privileges to be run */
        pubOK = SOPC_PubScheduler_Start(g_pPubSubConfig, g_pSourceConfig, 0);
    }
    if (sksOK && (subOK || pubOK))
    {
        SOPC_Atomic_Int_Set(&pubsubOnline, true);
    }

    return sksOK && (subOK || pubOK);
}

void PubSub_Stop(void)
{
    SOPC_Atomic_Int_Set(&pubsubOnline, false);
    SOPC_SubScheduler_Stop();
    SOPC_PubScheduler_Stop();

    SOPC_PubSubSKS_Clear();
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
