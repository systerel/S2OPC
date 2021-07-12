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

/** \file Provides a cache for data values.
 *
 * GetSource and SetTarget callback will get and set their values in the cache.
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "sopc_common.h"
#include "sopc_common_build_info.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"
#include "sopc_xml_loader.h"

#include <stdbool.h>
#include "sopc_builtintypes.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"

#include "config.h"
#include "interactive.h"
#include "safetyTypes.h"
#include "uam_cache.h"

#include "uam.h"
#include "uam_spduEncoders.h"

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/
/**
    Basic description (TODO:  TBC)

    This program

    The SafetyProducer demo is:
    - a Publisher on  TBD1 (SPDUresponse)
    - a Subscriber on TBD2 (SPDUrequest)
    The SafetyConsumer demo is:
    - a Publisher on TBD2 (SPDUrequest)
    - a Subscriber on TBD1 (SPDUresponse)

    Start the Provider (as root, using config file ./safety_demo_prov.xml):
    - ./safety_demo prod
    Start the Consumer (as root, using config file ./safety_demo_cons.xml):
    - ./safety_demo cons



*/

/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static void SafetyDemo_Stop(SafetyDemo_interactive_Context* pContext);
static const char* getenv_default(const char* name, const char* default_value);
static void SafetyDemo_initialize_logs(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_pubsub(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_publisher(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_subscriber(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_sks(SafetyDemo_interactive_SKS_Params* pSksParams, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_cache(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_startPublisher(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_initialize_startSubscriber(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_Initialize_UAM(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus);
static void SafetyDemo_Stop(SafetyDemo_interactive_Context* pContext);
static int help(const char* argv0);

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
SafetyDemo_interactive_Context g_context;

/*============================================================================
 * LOCAL FUNCTIONS
 *===========================================================================*/
static void signal_stop_server(int sig)
{
    (void) sig;

    if (g_context.stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        g_context.stopSignal = 1;
    }
}

/**
 * ENVIRONNEMENT CONFIGURATION AND SETUP
 */

/*===========================================================================*/
static const char* getenv_default(const char* name, const char* default_value)
{
    const char* val = getenv(name);
    const char* result = (val != NULL) ? val : default_value;

    printf("Using%s value [%s = %s]\n", (val != NULL) ? "" : " default", name, result);
    return result;
}

/*===========================================================================*/
static void SafetyDemo_initialize_logs(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert(NULL != pStatus && NULL != pContext);
    if (SOPC_STATUS_OK == *pStatus)
    {
        // Create two separate log folders
        const char* log_path = getenv_default("LOG_PATH", LOG_PATH);
        const char* subpath = pContext->isProvider ? "prov/" : "cons/";
        const size_t path_len = strlen(log_path) + strlen(subpath) + 1;
        char* full_path = SOPC_Malloc(path_len);
        assert(full_path != NULL);
        sprintf(full_path, "%s%s", log_path, subpath);

        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = full_path;
        *pStatus = SOPC_Common_Initialize(logConfiguration);

        if (SOPC_STATUS_OK != *pStatus)
        {
            printf("Error while initializing logs\n");
        }
        else
        {
            printf("[OK] LOG initialized: %s\n", full_path);
        }

        SOPC_Free(full_path);

        const char* uaslog = pContext->isProvider ? "uas_prov.log" : "uas_cons.log";
        LOG_SetFile(uaslog);
        LOG_SetLevel(LOG_DEBUG);
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_pubsub(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert((NULL != pContext) && (NULL != pStatus));
    FILE* fd;
    const char* config_path = pContext->isProvider ? SAFETY_XML_PROVIDER_DEMO : SAFETY_XML_CONSUMER_DEMO;

    if (SOPC_STATUS_OK == *pStatus)
    {
        fd = fopen(config_path, "r");
        if (NULL == fd)
        {
            printf("Cannot read configuration file %s\n", config_path);
            *pStatus = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == *pStatus)
    {
        pContext->pConfig = SOPC_PubSubConfig_ParseXML(fd);
        int closed = fclose(fd);

        *pStatus = (0 == closed && NULL != pContext->pConfig) ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_STATUS_OK != *pStatus)
        {
            printf("Error while loading PubSub configuration from %s\n", config_path);
        }
        else
        {
            printf("[OK] PUBSUB initialized: (%s)\n", config_path);
            // Add SPDUs to
        }
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_publisher(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert((NULL != pContext) && (NULL != pStatus));
    if (SOPC_STATUS_OK == *pStatus)
    {
        pContext->sourceConfig = SOPC_PubSourceVariableConfig_Create(&UAM_Cache_GetSourceVariables);
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_subscriber(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert((NULL != pContext) && (NULL != pStatus));
    if (SOPC_STATUS_OK == *pStatus)
    {
        pContext->targetConfig = SOPC_SubTargetVariableConfig_Create(&UAM_Cache_SetTargetVariables);
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_sks(SafetyDemo_interactive_SKS_Params* pSksParams, SOPC_ReturnStatus* pStatus)
{
    assert((NULL != pSksParams) && (NULL != pStatus));
    if (SOPC_STATUS_OK == *pStatus)
    {
        pSksParams->signing_key = getenv_default("SKS_SIGNING_KEY", SKS_SIGNING_KEY);
        pSksParams->encryption_key = getenv_default("SKS_ENCRYPTION_KEY", SKS_ENCRYPTION_KEY);
        pSksParams->nonce = getenv_default("SKS_KEY_NONCE", SKS_KEY_NONCE);

        SOPC_LocalSKS_init(pSksParams->signing_key, pSksParams->encryption_key, pSksParams->nonce);
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_cache(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert(NULL != pStatus);
    if (SOPC_STATUS_OK == *pStatus)
    {
        assert((NULL != pContext) && (NULL != pContext->pConfig));
        SafetyDemo_Interactive_Initialize(pContext);

        bool res = false;
        res = UAM_Cache_Initialize(pContext->pConfig);

        if (!res)
        {
            printf("Error while initializing the cache, refer to log files\n");
            *pStatus = SOPC_STATUS_NOK;
        }
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_startPublisher(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    SOPC_PubSubConnection* firstConnection = NULL;
    assert(NULL != pStatus);
    if (SOPC_STATUS_OK == *pStatus)
    {
        assert((NULL != pContext) && (NULL != pContext->pConfig));
        const uint32_t nbPub = SOPC_PubSubConfiguration_Nb_PubConnection(pContext->pConfig);
        if (0 == nbPub)
        {
            printf("# Info: No Publisher configured\n");
        }
        else
        {
            bool res = SOPC_PubScheduler_Start(pContext->pConfig, pContext->sourceConfig, 0);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_PubConnection_At(pContext->pConfig, 0);
                printf("# Info: Publisher started on %s\n", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                printf("# Error while starting the Publisher, do you have administrator privileges?\n");
                *pStatus = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void SafetyDemo_initialize_startSubscriber(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    SOPC_PubSubConnection* firstConnection = NULL;
    assert(NULL != pStatus);
    if (SOPC_STATUS_OK == *pStatus)
    {
        assert((NULL != pContext) && (NULL != pContext->pConfig));
        const uint32_t nbSub = SOPC_PubSubConfiguration_Nb_SubConnection(pContext->pConfig);
        if (0 < nbSub)
        {
            bool res = SOPC_SubScheduler_Start(pContext->pConfig, pContext->targetConfig, NULL);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(pContext->pConfig, 0);
                printf("# Info: Subscriber started %s\n", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                printf("# Error while starting the Subscriber\n");
                *pStatus = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void SafetyDemo_Initialize_UAM(SafetyDemo_interactive_Context* pContext, SOPC_ReturnStatus* pStatus)
{
    assert(NULL != pStatus);

    if (SOPC_STATUS_OK == *pStatus)
    {
        *pStatus = UAM_SpduEncoder_Initialize();
    }
    if (SOPC_STATUS_OK == *pStatus)
    {
        // : note SPDU request is created on both PROV (Sub) & CONS (Pub)
        pContext->spduRequestId = NODEID_SPDU_REQUEST_NUM;
        *pStatus = UAM_SpduEncoder_CreateSpduRequest(pContext->spduRequestId);
        if (SOPC_STATUS_OK != *pStatus)
        {
            printf("[EE] Failed to create SPDU Response with NodeId=[ns=%d;i=%d]\n", UAM_NAMESPACE,
                   NODEID_SPDU_REQUEST_NUM);
            printf("     => This variable must be defined in configuration file with dataType=\"Structure\"\n");
        }
    }
    if (SOPC_STATUS_OK == *pStatus)
    {
        pContext->spduResponseId = NODEID_SPDU_RESPONSE_NUM;
        *pStatus = UAM_SpduEncoder_CreateSpduResponse(pContext->spduResponseId, SAMPLE1_SAFETY_DATA_LEN,
                                                      SAMPLE1_UNSAFE_DATA_LEN);
        if (SOPC_STATUS_OK != *pStatus)
        {
            printf("[EE] Failed to create SPDU Response with NodeId=[ns=%d;i=%d]\n", UAM_NAMESPACE,
                   NODEID_SPDU_RESPONSE_NUM);
            printf("     => This variable must be defined in configuration file with dataType=\"Structure\"\n");
        }
    }

    if (SOPC_STATUS_OK == *pStatus)
    {
        assert((NULL != pContext) && (NULL != pContext->pConfig));
        if (pContext->isProvider)
        {
            *pStatus = SafetyTypes_Create_ProviderSample();

            if (SOPC_STATUS_OK != *pStatus)
            {
                printf("# Error while starting Create_ProviderSample : code= %02X\n", *pStatus);
            }
        }
        else
        {
            *pStatus = SafetyTypes_Create_ConsumerSample();

            if (SOPC_STATUS_OK != *pStatus)
            {
                printf("# Error while starting Create_ConsumerSample : code= %02X\n", *pStatus);
            }
        }
    }
    if (SOPC_STATUS_OK == *pStatus)
    {
        *pStatus = UAM_StartSafety();
    }
}

/*===========================================================================*/
static void SafetyDemo_Stop(SafetyDemo_interactive_Context* pContext)
{
    SafetyDemo_Interactive_Clear();
    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    SOPC_PubSourceVariableConfig_Delete(pContext->sourceConfig);
    SOPC_SubTargetVariableConfig_Delete(pContext->targetConfig);
    SOPC_PubSubConfiguration_Delete(pContext->pConfig);

    UAM_SpduEncoder_Clear();
    UAM_Cache_Clear();
    UAM_Clear();
}

/*===========================================================================*/
static int help(const char* argv0)
{
    printf("Missing parameter\n");
    printf("   %s prov      : start provider APP\n", argv0);
    printf("   %s cons      : start consumer APP\n", argv0);
    return -1;
}

/*===========================================================================*/
int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (argc < 2)
    {
        return help(argv[0]);
    }
    bool isCons = true;
    g_context.isProvider = (strcmp(argv[1], "prov") == 0);
    isCons = (strcmp(argv[1], "cons") == 0);
    if (!g_context.isProvider && !isCons)
    {
        return help(argv[0]);
    }

    g_context.stopSignal = 0;

    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* General initializations */
    SafetyDemo_initialize_logs(&g_context, &status);

    SafetyDemo_initialize_pubsub(&g_context, &status);

    SafetyDemo_initialize_cache(&g_context, &status);

    SafetyDemo_initialize_publisher(&g_context, &status);

    SafetyDemo_initialize_subscriber(&g_context, &status);

    SafetyDemo_initialize_sks(&g_context.sks, &status);

    SafetyDemo_initialize_startPublisher(&g_context, &status);
    SafetyDemo_initialize_startSubscriber(&g_context, &status);

    SafetyDemo_Initialize_UAM(&g_context, &status);

    // Wait for a signal
    while (SOPC_STATUS_OK == status && 0 == g_context.stopSignal)
    {
        static const uint32_t msCycle = 100;
        SafetyDemo_Interactive_execute(&g_context);

        status = UAM_Cycle();

        // Wait for next cycle
        SOPC_Sleep(msCycle);
    }

    // Clean and quit
    SafetyDemo_Stop(&g_context);
    printf("# Info: PubSub stopped\n");
    return 0;
}
