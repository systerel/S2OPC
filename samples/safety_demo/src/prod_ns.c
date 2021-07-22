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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/**
 * \file This is the main program of the non-safe producer.
 *  The Non Safe program cyclically:
 *  - reads inputs (here it will be using STDIN)
 *  - provides inputs to SAFE partitions (SINGLE or DUAL)
 *  - waits for SAFE outputs
 *  - builds SPDU output
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/
#include <signal.h>
//#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
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

#include "sopc_builtintypes.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"
//
#include "config.h"
//#include "interactive.h"
#include "safetyDemo.h"

#include "uam_cache.h"
#include "uas_logitf.h"

//
#include "uam.h"
#include "uam_ns.h"
#include "uam_spduEncoders.h"

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

typedef struct
{
    SOPC_Dict* pCache;
    SOPC_PubSubConfiguration* pConfig;
    volatile sig_atomic_t stopSignal;
    SOPC_PubSourceVariableConfig* sourceConfig;
    SOPC_SubTargetVariableConfig* targetConfig;
    UAM_SessionHandle spduHandle;
} ProdNS_Demo_interactive_Context;

/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static int prod_ns_help(const char* argv0);
static void prod_ns_init(void);
static void prod_ns_cycle(void);
static void prod_ns_stop(void);
static void cache_Notify_CB(const SOPC_NodeId* const pNid, const SOPC_DataValue* const pDv);

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static SOPC_ReturnStatus g_status = SOPC_STATUS_OK;
static ProdNS_Demo_interactive_Context g_context;

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
static void prod_ns_initialize_logs(void)
{
    if (SOPC_STATUS_OK == g_status)
    {
        // Create two separate log folders
        const char* full_path = LOG_PATH "prov/";

        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = full_path;
        g_status = SOPC_Common_Initialize(logConfiguration);

        if (SOPC_STATUS_OK != g_status)
        {
            printf("Error while initializing logs\n");
        }
        else
        {
            printf("[OK] LOG initialized: %s\n", full_path);
        }

        LOG_SetFile("uas_prov.log");
        LOG_SetLevel(LOG_DEBUG);
    }
}

/*===========================================================================*/
static void prod_ns_initialize_pubsub(void)
{
    FILE* fd;
    const char* config_path = SAFETY_XML_PROVIDER_DEMO;

    if (SOPC_STATUS_OK == g_status)
    {
        fd = fopen(config_path, "r");
        if (NULL == fd)
        {
            printf("Cannot read configuration file %s\n", config_path);
            g_status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == g_status)
    {
        g_context.pConfig = SOPC_PubSubConfig_ParseXML(fd);
        int closed = fclose(fd);

        g_status = (0 == closed && NULL != g_context.pConfig) ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_STATUS_OK != g_status)
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
static void prod_ns_initialize_cache(void)
{
    bool res = false;
    if (SOPC_STATUS_OK == g_status)
    {
        assert(NULL != g_context.pConfig);

        res = UAM_Cache_Initialize(g_context.pConfig); // TODO should be moved in UAM NS?

        if (!res)
        {
            printf("Error while initializing the cache, refer to log files\n");
            g_status = SOPC_STATUS_NOK;
        }
        else
        {
            UAM_Cache_SetNotify(&cache_Notify_CB);
        }
    }
}

/*===========================================================================*/
static void prod_ns_initialize_publisher(void)
{
    SOPC_PubSubConnection* firstConnection = NULL;
    uint32_t nbPub = 0;
    if (SOPC_STATUS_OK == g_status)
    {
        assert(NULL != g_context.pConfig);

        g_context.sourceConfig = SOPC_PubSourceVariableConfig_Create(&UAM_Cache_GetSourceVariables);

        nbPub = SOPC_PubSubConfiguration_Nb_PubConnection(g_context.pConfig);
        if (0 == nbPub)
        {
            printf("# Info: No Publisher configured\n");
        }
        else
        {
            bool res = SOPC_PubScheduler_Start(g_context.pConfig, g_context.sourceConfig, 0);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_PubConnection_At(g_context.pConfig, 0);
                printf("# Info: Publisher started on %s\n", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                printf("# Error while starting the Publisher, do you have administrator privileges?\n");
                g_status = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void prod_ns_initialize_subscriber(void)
{
    SOPC_PubSubConnection* firstConnection = NULL;
    uint32_t nbSub = 0;
    if (SOPC_STATUS_OK == g_status)
    {
        assert(NULL != g_context.pConfig);

        g_context.targetConfig = SOPC_SubTargetVariableConfig_Create(&UAM_Cache_SetTargetVariables);

        nbSub = SOPC_PubSubConfiguration_Nb_SubConnection(g_context.pConfig);
        if (0 < nbSub)
        {
            bool res = SOPC_SubScheduler_Start(g_context.pConfig, g_context.targetConfig, NULL);
            if (res)
            {
                firstConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(g_context.pConfig, 0);
                printf("# Info: Subscriber started %s\n", SOPC_PubSubConnection_Get_Address(firstConnection));
            }
            else
            {
                printf("# Error while starting the Subscriber\n");
                g_status = SOPC_STATUS_NOK;
            }
        }
    }
}

/*===========================================================================*/
static void prod_ns_initialize_sks(void)
{
    if (SOPC_STATUS_OK == g_status)
    {
        SOPC_LocalSKS_init(SKS_SIGNING_KEY, SKS_ENCRYPTION_KEY, SKS_KEY_NONCE);
    }
}

/*===========================================================================*/
static void prod_ns_initialize_uam(void)
{
    bool result = false;
    if (SOPC_STATUS_OK == g_status)
    {
        g_status = UAM_SpduEncoder_Initialize();
    }

    if (SOPC_STATUS_OK == g_status)
    {
        // : note SPDU request is created on both PROV (Sub) & CONS (Pub)
        g_status = UAM_SpduEncoder_CreateSpduRequest(NODEID_SPDU_REQUEST_NUM);
        if (SOPC_STATUS_OK != g_status)
        {
            printf("[EE] Failed to create SPDU Response with NodeId=[ns=%d;i=%d]\n", UAM_NAMESPACE,
                   NODEID_SPDU_REQUEST_NUM);
            printf("     => This variable must be defined in configuration file with dataType=\"Structure\"\n");
        }
    }
    if (SOPC_STATUS_OK == g_status)
    {
        g_status = UAM_SpduEncoder_CreateSpduResponse(NODEID_SPDU_RESPONSE_NUM, SAMPLE1_SAFETY_DATA_LEN,
                                                      SAMPLE1_UNSAFE_DATA_LEN);
        if (SOPC_STATUS_OK != g_status)
        {
            printf("[EE] Failed to create SPDU Response with NodeId=[ns=%d;i=%d]\n", UAM_NAMESPACE,
                   NODEID_SPDU_RESPONSE_NUM);
            printf("     => This variable must be defined in configuration file with dataType=\"Structure\"\n");
        }
    }

    if (SOPC_STATUS_OK == g_status)
    {
        UAM_NS_Configuration_type zConfig = {.eRedundancyType = UAM_RDD_SINGLE_CHANNEL,
                                             .dwHandle = SESSION_UAM_ID,
                                             .bIsProvider = true,
                                             .uUserRequestId = NODEID_SPDU_REQUEST_NUM,
                                             .uUserResponseId = NODEID_SPDU_RESPONSE_NUM};
        UAM_NS_Initialize();

        result = UAM_NS_CreateSpdu(&zConfig);
        if (result == false)
        {
            printf("# UAM_NS_CreateSpdu failed\n");
            g_status = SOPC_STATUS_NOK;
        }
    }
}

/*===========================================================================*/
static int prod_ns_help(const char* argv0)
{
    printf("%s HELP (TODO)\n", argv0);
    return -1;
}

/*===========================================================================*/
static void prod_ns_init(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        prod_ns_initialize_logs();

        prod_ns_initialize_pubsub();

        prod_ns_initialize_cache();

        prod_ns_initialize_sks();

        prod_ns_initialize_publisher();
        prod_ns_initialize_subscriber();

        prod_ns_initialize_uam();

        // TODO : interactive!
    }
}

/*===========================================================================*/
static void prod_ns_stop(void)
{
    // TODO stop cleany everything

    UAM_NS_Clear();
    printf("# EXITING (code =%02X)\n", g_status);
}

/*===========================================================================*/
static void prod_ns_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        // TODO : replace UAM_NS_CheckSpduReception by an event-based reading rather than periodic polling
        UAM_NS_CheckSpduReception(SESSION_UAM_ID);
        // TODO
    }
}

/*===========================================================================*/
static void cache_Notify_CB(const SOPC_NodeId* const pNid, const SOPC_DataValue* const pDv)
{
    // Reminder: this function is called when the Cache is updated (in this case: on Pub-Sub new reception)
    if (pNid == NULL || pDv == NULL)
    {
        return;
    }
    if (pNid->Namespace == UAM_NAMESPACE && pNid->IdentifierType == SOPC_IdentifierType_Numeric &&
        pDv->Value.ArrayType == SOPC_VariantArrayType_SingleValue &&
        pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id && pDv->Value.Value.ExtObject->Length > 0)
    {
        // We received an extension object (not of array type) on the correct namespace.

        // Retrieve and copy the object
        // Check whether this is a SPDU received data
        // TODO : this could be made more generic!
        if (pNid->Data.Numeric == NODEID_SPDU_REQUEST_NUM)
        {
            UAM_NS_RequestMessageReceived(SESSION_UAM_ID);
        }
        if (pNid->Data.Numeric == NODEID_SPDU_RESPONSE_NUM)
        {
            UAM_NS_ResponseMessageReceived(SESSION_UAM_ID);
        }
    }
}

/*===========================================================================*/
int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        return prod_ns_help(argv[0]);
    }

    g_context.stopSignal = 0;

    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    prod_ns_init();

    printf("Start Cycles\n");
    // Wait for a signal
    while (SOPC_STATUS_OK == g_status && 0 == g_context.stopSignal)
    {
        static const uint32_t msCycle = 50;
        prod_ns_cycle();

        // Wait for next cycle
        SOPC_Sleep(msCycle);
    }
    printf("End Cycles\n");

    // Clean and quit
    prod_ns_stop();
    return 0;
}
