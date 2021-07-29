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
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_time.h"

#include "sopc_builtintypes.h"
#include "sopc_threads.h"
//
#include "config.h"
#include "interactive.h"
#include "safetyDemo.h"

#include "uas_logitf.h"

//
#include "uam.h"
#include "uam_ns.h"
#include "uam_ns_spduEncoders.h"
#include "uam_ns_impl.h"

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

typedef struct
{
    volatile sig_atomic_t stopSignal;
    UAM_SessionId sessionId;
    bool bBlock;
} ProdNS_Demo_interactive_Context;

/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static int prod_ns_help(const char* argv0);
static void prod_ns_init(void);
static void prod_ns_cycle(void);
static void prod_ns_stop(void);

static bool pfOnQuitEvent (const char* params, void* pUserParam);
static bool pfOnPubSubBlockUnblock (const char* params, void* pUserParam);

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static SOPC_ReturnStatus g_status = SOPC_STATUS_OK;
static ProdNS_Demo_interactive_Context g_context;
static Thread gCycleThread;

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

/*===========================================================================*/
static bool pfOnQuitEvent (const char* params, void* pUserParam)
{
    (void)params;
    (void)pUserParam;
    g_context.stopSignal = 1 ;
    return true;
}

/*===========================================================================*/
static bool pfOnPubSubBlockUnblock (const char* params, void* pUserParam)
{
    (void)params;
    (void)pUserParam;
    g_context.bBlock = !g_context.bBlock;
    printf("Current Blocked status: %d\n", g_context.bBlock);
    return true;
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
    const char* config_path = SAFETY_XML_PROVIDER_DEMO;

    if (SOPC_STATUS_OK == g_status)
    {
        g_status = UAM_NS_Impl_Initialize (config_path);
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
                                             .dwSessionId = SAMPLE1_SESSION_UAM_ID,
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
        Utils_Interactive_Initialize();
        Utils_Interactive_AddCallback ('q', "Quit", &pfOnQuitEvent, NULL);
        Utils_Interactive_AddCallback ('b', "Block/Unblock PubSub communication", &pfOnPubSubBlockUnblock, NULL);

        prod_ns_initialize_logs();

        prod_ns_initialize_sks();

        prod_ns_initialize_pubsub();

        prod_ns_initialize_uam();
    }
}

/*===========================================================================*/
static void* prod_ns_threadImpl (void* arg)
{
    assert (arg == NULL);
    LOG_Trace(LOG_DEBUG, "APP:Start Cycles");
    // Wait for a signal
    while (SOPC_STATUS_OK == g_status && 0 == g_context.stopSignal)
    {
        static const uint32_t msCycle = 50;
        if (!g_context.bBlock )
        {
            prod_ns_cycle();
        }

        // Wait for next cycle
        SOPC_Sleep(msCycle);
    }
    LOG_Trace(LOG_DEBUG, "APP:End Cycles");
    return NULL;
}

/*===========================================================================*/
static void prod_ns_stop(void)
{
    // TODO stop cleany everything
    // TODO : check memory leaks (LIBASAN?)
    Utils_Interactive_Clear();
    UAM_NS_Impl_Clear();
    UAM_NS_Clear();
    printf("# EXITING (code =%02X)\n", g_status);
}

/*===========================================================================*/
static void prod_ns_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        UAM_NS_CheckSpduReception(SAMPLE1_SESSION_UAM_ID);
    }
}



/*===========================================================================*/
static void prod_ns_interactive_cycle(void)
{
    Utils_Interactive_execute();
}


/*===========================================================================*/
int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        return prod_ns_help(argv[0]);
    }

    g_context.stopSignal = 0;
    g_context.bBlock = false;

    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    prod_ns_init();

    SOPC_Thread_Create(&gCycleThread, &prod_ns_threadImpl, NULL, "prod_ns_threadImpl");

    // Wait for a signal
    while (0 == g_context.stopSignal)
    {
        // Note : interactions with keyboard only work correctly in main thread.
        prod_ns_interactive_cycle ();
        SOPC_Sleep(10);
    }

    // Clean and quit
    prod_ns_stop();
    return 0;
}
