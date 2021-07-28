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
//#include "interactive.h"
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
    SOPC_Dict* pCache;
    volatile sig_atomic_t stopSignal;
    UAM_SessionId sessionId;
} ProdNS_Demo_interactive_Context;

/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static int prod_ns_help(const char* argv0);
static void prod_ns_init(void);
static void prod_ns_cycle(void);
static void prod_ns_stop(void);

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
        prod_ns_cycle();

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

    UAM_NS_Impl_Clear();
    UAM_NS_Clear();
    printf("# EXITING (code =%02X)\n", g_status);
}

/*===========================================================================*/
static void prod_ns_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        // TODO : replace UAM_NS_CheckSpduReception by an event-based reading rather than periodic polling
        UAM_NS_CheckSpduReception(SAMPLE1_SESSION_UAM_ID);
        // TODO
    }
}


/*===========================================================================*/
/*===========================================================================*/
// TODO : move that "interactive" part in a new  file
#include <sys/ioctl.h>
#include "sopc_raw_sockets.h"
#include <unistd.h>
#define STDIN 0
#define USER_ENTRY_MAXSIZE (128u)

/*===========================================================================*/
static bool prod_ns_interactive_processCommand (const char* cmd)
{
    switch (cmd[0])
    {
        case 'q':
            g_context.stopSignal = 1;
            return true;
            break;
        default:
            break;
    }
    return false;
}

/*===========================================================================*/
static void prod_ns_interactive_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        SOPC_SocketSet fdSet;
        int maxfd = STDIN;
        int result = 0;
        ssize_t nbRead = 0;

        struct timeval* ptv = NULL;
    #define WAIT_MS 10 * 1000
    #if WAIT_MS > 0
        struct timeval tv;
        tv.tv_sec = WAIT_MS / (1000 * 1000);
        tv.tv_usec = WAIT_MS % (1000 * 1000);
        ptv = &tv;
    #endif
        char entry[USER_ENTRY_MAXSIZE];

        SOPC_SocketSet_Clear(&fdSet);
        SOPC_SocketSet_Add(STDIN, &fdSet);

        result = select(maxfd + 1, &fdSet.set, NULL, NULL, ptv);
        if (result < 0)
        {
            printf("SELECT failed: %d\n", result);
            g_context.stopSignal = 1;
        }
        else if (SOPC_SocketSet_IsPresent(STDIN, &fdSet))
        {
            nbRead = read(STDIN, entry, USER_ENTRY_MAXSIZE - 1);
            while (nbRead > 0 && entry[nbRead - 1] < ' ')
            {
                entry[nbRead - 1] = 0;
                nbRead--;
            }
            if (nbRead > 0)
            {
                prod_ns_interactive_processCommand(entry);
            }
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
