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

#include "controller_client.h"
#include "controller_config.h"
#include "controller_pubsub.h"

#include "controller_server.h"
// First MS Period of scheduler task. 500ms
#define PUBSUB_SCHEDULER_FIRST_MSPERIOD 500

typedef struct SOPC_SKS_Local_Configuration
{
    // array of SOPC_SecurityKeyServices. Data should not be cleared.
    SOPC_Array* sksArray;

    // array of security group id. Data should not be cleared.
    SOPC_Array* sksSecuGroupId;

} SOPC_SKS_Local_Configuration;

static int32_t pubsubOnline = 0;
static SOPC_PubSubConfiguration* g_pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* g_pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* g_pSourceConfig = NULL;
static SOPC_SKscheduler* g_skScheduler = NULL;
static bool g_isSecurity = false;
static bool g_isSks = false;

/* Initialise and start SK Scheduler and SK Manager if security is needed */
static bool PubSub_SKS_Start(void);

static void free_global_configurations(void);

static SOPC_SKS_Local_Configuration* SOPC_SKS_Local_Configuration_Create(void);
static void SOPC_SKS_Local_Configuration_Clear(SOPC_SKS_Local_Configuration* config);

static SOPC_ReturnStatus get_sks_config(SOPC_PubSubConfiguration* pPubSubConfig,
                                        bool* isSecurity,
                                        SOPC_SKS_Local_Configuration** sksConfigArray,
                                        uint32_t* length);

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

static void SKBuilder_Update_Cb(SOPC_SKBuilder* skb, SOPC_SKManager* skm, uintptr_t userParam)
{
    SOPC_UNUSED_ARG(userParam);
    SOPC_UNUSED_ARG(skb);

    const char* securityGroupId = skm->securityGroupId;
    SOPC_SecureConnection_Config* secureConnCfg = (SOPC_SecureConnection_Config*) skm->userData;
    SOPC_String* secuPolicyUri = NULL;
    uint32_t firstToken = 0;
    SOPC_ByteString* keys = NULL;
    uint32_t nbKeys = 0;
    uint64_t timeToNextKey = 0;
    uint64_t keyLifetime = 0;
    SOPC_ReturnStatus status = SOPC_SKManager_GetKeys(skm, 0, UINT32_MAX, &secuPolicyUri, &firstToken, &keys, &nbKeys,
                                                      &timeToNextKey, &keyLifetime);
    if (SOPC_STATUS_OK == status)
    {
        status = (nbKeys > 0 ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Client_SetSecurityKeys(secureConnCfg, securityGroupId, secuPolicyUri, firstToken, keys, nbKeys - 1,
                                        &keys[1], timeToNextKey, keyLifetime);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                               "SKBuilder_Update_Cb: Cannot get keys from SK Manager for security group %s",
                               securityGroupId);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                               "SKBuilder_Update_Cb: Cannot set keys for security group %s with status %d",
                               securityGroupId, status);
    }
    SOPC_String_Delete(secuPolicyUri);
    int32_t iNbKeys = (int32_t) nbKeys;
    SOPC_Clear_Array(&iNbKeys, (void**) &keys, sizeof(*keys), SOPC_ByteString_ClearAux);
    SOPC_Free(keys);
}

static bool PubSub_SKS_Configure(SOPC_SKS_Local_Configuration* sksConfigArray)
{
    if (!g_isSecurity)
    {
        return true;
    }
    else if (!g_isSks || 0 == SOPC_Array_Size(sksConfigArray->sksArray))
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Init one SK Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        g_skScheduler = SOPC_SKscheduler_Create();
        if (NULL == g_skScheduler)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    SOPC_SKBuilder* skBuilder = NULL;
    SOPC_SKProvider* skProvider = NULL;

    /* Init SK Provider : Create Random Keys */
    skProvider = SOPC_SKProvider_RandomPubSub_Create(SKS_NB_GENERATED_KEYS);
    if (NULL == skProvider)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Init inner SK Builder Append + Truncate: adds Keys to Manager and removes obsolete Keys when maximum size is
     * reached */
    SOPC_SKBuilder* skbAppend = NULL;
    SOPC_SKBuilder* skbTruncate = NULL;
    if (SOPC_STATUS_OK == status)
    {
        skbAppend = SOPC_SKBuilder_Append_Create();
        if (NULL == skbAppend)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        skbTruncate = SOPC_SKBuilder_Truncate_Create(skbAppend, SKS_NB_MAX_KEYS);
        if (NULL == skbTruncate)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            // ownership of skbAppend is transferred to skbTruncate
            skbAppend = NULL;
        }
    }

    /* Init SK Builder callback: call inner SK builder and then the callback */
    if (SOPC_STATUS_OK == status)
    {
        skBuilder = SOPC_SKBuilder_Callback_Create(skbTruncate, SKBuilder_Update_Cb, (uintptr_t) NULL);
        if (NULL == skBuilder)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            // ownership of skbTruncate is transferred to skBuilder
            skbTruncate = NULL;
        }
    }

    static SOPC_SecureConnection_Config* secuConfig = NULL;
    // Create a connection configuration for each SKS address (limited to one)
    if (SOPC_STATUS_OK == status)
    {
        size_t nbSks = SOPC_Array_Size(sksConfigArray->sksArray);
        SOPC_ASSERT(1 == nbSks); // Currently only one SKS is supported

        /* 1. Create one instance of secure connection each Sks description */
        for (size_t i = 0; i < nbSks && SOPC_STATUS_OK == status; i++)
        {
            SOPC_SecurityKeyServices* sksElt = SOPC_Array_Get(sksConfigArray->sksArray, SOPC_SecurityKeyServices*, i);
            SOPC_ASSERT(NULL != sksElt);
            secuConfig = Client_AddSecureConnectionConfig(SOPC_SecurityKeyServices_Get_EndpointUrl(sksElt),
                                                          SOPC_SecurityKeyServices_Get_ServerCertificate(sksElt));
            if (NULL == secuConfig)
            {
                status = SOPC_STATUS_NOK;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "PubSub Cannot create secure connection for push SKS");
            }
        }
    }

    // Create the SK managers for each security group from the configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_PubSubSKS_Init();

        bool oneTaskAdded = false;

        size_t nbSecuGroups = SOPC_Array_Size(sksConfigArray->sksSecuGroupId);
        for (size_t i = 0; i < nbSecuGroups && SOPC_STATUS_OK == status; i++)
        {
            const char* secuGroupId = SOPC_Array_Get(sksConfigArray->sksSecuGroupId, const char*, i);
            SOPC_ASSERT(NULL != secuGroupId);

            SOPC_SKManager* skMgr = SOPC_SKManager_Create(
                secuGroupId,
                (uintptr_t) secuConfig); // a dedicated connection (or set of connections) might be provided instead
            status = (NULL == skMgr) ? SOPC_STATUS_OUT_OF_MEMORY : status;

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SKManager_SetKeyLifetime(skMgr, SKS_KEYLIFETIME);
                if (SOPC_STATUS_OK == status)
                {
                    SOPC_String policy;
                    SOPC_String_Initialize(&policy);
                    SOPC_String_CopyFromCString(&policy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
                    status = SOPC_SKManager_SetSecurityPolicyUri(skMgr, &policy);
                    SOPC_String_Clear(&policy);
                }
                if (SOPC_STATUS_OK == status)
                {
                    bool res = SOPC_PubSubSKS_AddSkManager(skMgr);
                    status = (res) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                    if (!res)
                    {
                        printf("<Test_SKS_Server: Failed to add SK Manager for security group %s\n",
                               skMgr->securityGroupId);
                    }
                }
                /* Create a new task */
                if (SOPC_STATUS_OK == status)
                {
                    /* Init the tasks with 1s */
                    // TODO: use SOPC_PubSubSKS_AddTasks instead after the loop ?
                    status = SOPC_SKscheduler_AddTask(g_skScheduler, skBuilder, skProvider, skMgr,
                                                      SKS_SCHEDULER_INIT_MSPERIOD);
                    if (SOPC_STATUS_OK == status)
                    {
                        oneTaskAdded = true;
                    }
                }
            }
        }
        if (oneTaskAdded)
        {
            // At least one task added: ownership transferred to scheduler
            skBuilder = NULL;
            skProvider = NULL;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Security Keys Service: Configured\n");
    }
    else
    {
        if (NULL != g_skScheduler)
        {
            SOPC_SKscheduler_StopAndClear(g_skScheduler);
            g_skScheduler = NULL;
        }
        printf("<Security Keys Service Error: Start failed\n");
    }

    // Clean unused SK modules if ownership was not transferred to scheduler
    if (NULL != skProvider)
    {
        SOPC_SKProvider_Clear(skProvider);
        SOPC_Free(skProvider);
        skProvider = NULL;
    }

    if (NULL != skbAppend)
    {
        SOPC_SKBuilder_Clear(skbAppend);
        SOPC_Free(skbAppend);
        skbAppend = NULL;
    }

    if (NULL != skBuilder)
    {
        SOPC_SKBuilder_Clear(skBuilder);
        SOPC_Free(skBuilder);
        skBuilder = NULL;
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
    g_isSecurity = false;
    g_isSks = false;
    /* List of SKS servers configurations. */
    SOPC_SKS_Local_Configuration* sksConfigArray = NULL;
    uint32_t sksConfigLength = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = get_sks_config(pPubSubConfig, &g_isSecurity, &sksConfigArray, &sksConfigLength);
        SOPC_ASSERT(2 > sksConfigLength); /* Only one SKS server configuration is managed */
        // Check if configuration provides a SKS
        g_isSks = (NULL != sksConfigArray && 0 < sksConfigLength && 0 < SOPC_Array_Size(sksConfigArray->sksArray));
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot retrieve PubSub Security Mode information");
        }
        else if (g_isSecurity && !g_isSks)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                     "PubSub Security is used but no SKS address provided, only the configured "
                                     "fallback keys will be used (never renewed) !");
        }
    }

    if (SOPC_STATUS_OK == status && g_isSecurity && g_isSks)
    {
        // Configure the SKS for both the PubSub applications of controller
        // and to push the security keys to devices
        bool sksOK = PubSub_SKS_Configure(sksConfigArray);
        status = sksOK ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Clean sks config data
    if (NULL != sksConfigArray)
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

    Client_Start();
    SOPC_ReturnStatus status = SOPC_SKscheduler_Start(g_skScheduler);

    if (SOPC_STATUS_OK == status)
    {
        printf("<Security Keys Service: Started\n");
    }
    else
    {
        Client_Stop();
        if (NULL != g_skScheduler)
        {
            SOPC_SKscheduler_StopAndClear(g_skScheduler);
            g_skScheduler = NULL;
        }
        printf("<Security Keys Service Error: Start failed\n");
    }

    return (status == SOPC_STATUS_OK);
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
        // Note: use SetSubStatusAsync function to avoid blocking treatment in sub scheduler
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

    Client_Stop();

    SOPC_SKscheduler_StopAndClear(g_skScheduler);
    SOPC_PubSubSKS_Clear();
    SOPC_Free(g_skScheduler);
    g_skScheduler = NULL;
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
        SOPC_ASSERT(NULL != elt);
        if (0 == strcmp(SOPC_SecurityKeyServices_Get_EndpointUrl(elt), SOPC_SecurityKeyServices_Get_EndpointUrl(sks)))
        {
            return true;
        }
    }
    return false;
}

static bool PubSub_SearchSecuGroupInArray(SOPC_Array* array, const char* secuGroupId)
{
    if (NULL == array || NULL == secuGroupId)
    {
        return false;
    }
    size_t size = SOPC_Array_Size(array);
    for (size_t i = 0; i < size; i++)
    {
        const char* elt = SOPC_Array_Get(array, const char*, i);
        SOPC_ASSERT(NULL != elt);
        if (0 == strcmp(elt, secuGroupId))
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
    SOPC_ASSERT(NULL != isSecurity);
    SOPC_ASSERT(NULL != sksConfigArray);
    SOPC_ASSERT(NULL != length);
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
            SOPC_ASSERT(NULL != group);
            securityMode = SOPC_WriterGroup_Get_SecurityMode(group);
            if (SOPC_SecurityMode_None != securityMode)
            {
                *isSecurity = true; /* at least one group with security */
            }

            uint32_t nbSKS = SOPC_WriterGroup_Nb_SecurityKeyServices(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbSKS)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SKS address but security mode is none");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode)
            {
                // used to check that all sks list are the same
                size_t sksArraySize = SOPC_Array_Size((*sksConfigArray)->sksArray);

                // add all Sks
                bool bres = true;
                for (uint16_t sksIndex = 0; bres && sksIndex < nbSKS; sksIndex++)
                {
                    SOPC_SecurityKeyServices* desc = SOPC_WriterGroup_Get_SecurityKeyServices_At(group, sksIndex);
                    SOPC_ASSERT(NULL != desc);
                    const char* secuGroupId = SOPC_WriterGroup_Get_SecurityGroupId(group);
                    SOPC_ASSERT(NULL != secuGroupId);

                    if (!PubSub_SearchSKSInArray((*sksConfigArray)->sksArray, desc))
                    {
                        bres &= SOPC_Array_Append((*sksConfigArray)->sksArray, desc);
                        if (0 < sksArraySize)
                        {
                            // 2 groups have different SKS list. All list are merged
                            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                     "Different SKS addresses in PubSub configuration");
                        }
                    }
                    if (!PubSub_SearchSecuGroupInArray((*sksConfigArray)->sksSecuGroupId, secuGroupId))
                    {
                        bres &= SOPC_Array_Append((*sksConfigArray)->sksSecuGroupId, secuGroupId);
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
            SOPC_ASSERT(NULL != group);
            securityMode = SOPC_ReaderGroup_Get_SecurityMode(group);
            if (SOPC_SecurityMode_None != securityMode)
            {
                *isSecurity = true; /* at least one group with security */
            }

            uint32_t nbSKS = SOPC_ReaderGroup_Nb_SecurityKeyServices(group);
            if (SOPC_SecurityMode_None == securityMode && 0 < nbSKS)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SKS address but security mode is none");
                result = SOPC_STATUS_NOK;
            }
            else if (SOPC_SecurityMode_None != securityMode)
            {
                // used to check that all sks list are the same
                size_t sksArraySize = SOPC_Array_Size((*sksConfigArray)->sksArray);

                // add all Sks
                bool bres = true;
                for (uint16_t sksIndex = 0; bres && sksIndex < nbSKS; sksIndex++)
                {
                    SOPC_SecurityKeyServices* desc = SOPC_ReaderGroup_Get_SecurityKeyServices_At(group, sksIndex);
                    SOPC_ASSERT(NULL != desc);
                    const char* secuGroupId = SOPC_ReaderGroup_Get_SecurityGroupId(group);
                    SOPC_ASSERT(NULL != secuGroupId);

                    if (!PubSub_SearchSKSInArray((*sksConfigArray)->sksArray, desc))
                    {
                        bres &= SOPC_Array_Append((*sksConfigArray)->sksArray, desc);
                        if (0 < sksArraySize)
                        {
                            // 2 groups have different SKS list. All list are merged
                            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                     "Different SKS addresses in PubSub configuration");
                        }
                    }

                    if (!PubSub_SearchSecuGroupInArray((*sksConfigArray)->sksSecuGroupId, secuGroupId))
                    {
                        bres &= SOPC_Array_Append((*sksConfigArray)->sksSecuGroupId, secuGroupId);
                    }
                }
            }
        }
    }

    return result;
}

static SOPC_SKS_Local_Configuration* SOPC_SKS_Local_Configuration_Create(void)
{
    bool allocSuccess = true;
    SOPC_SKS_Local_Configuration* result = SOPC_Calloc(1, sizeof(SOPC_SKS_Local_Configuration));
    allocSuccess = (NULL != result);
    if (allocSuccess)
    {
        result->sksArray = SOPC_Array_Create(sizeof(SOPC_SecurityKeyServices*), 1, NULL);
        allocSuccess = (NULL != result->sksArray);
    }

    if (allocSuccess)
    {
        result->sksSecuGroupId = SOPC_Array_Create(sizeof(char*), 10, NULL);
        allocSuccess = (NULL != result->sksSecuGroupId);
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

    SOPC_Array_Delete(config->sksArray);
    config->sksArray = NULL;
    SOPC_Array_Delete(config->sksSecuGroupId);
    config->sksSecuGroupId = NULL;
}
