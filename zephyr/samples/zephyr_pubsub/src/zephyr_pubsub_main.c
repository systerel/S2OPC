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

#include <stdio.h>
#include <kernel.h>
#include <limits.h>
#include <signal.h>

#include <stdlib.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_common.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"

#include "cache.h"
#include "network_init.h"
#include "pubsub_config_static.h"
#include "static_security_data.h"
#include "threading_alt.h"

volatile int stopSignal = 0;
static void signal_stop_server(int sig)
{
    (void) sig;

    if (stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        stopSignal = 1;
    }
}

static void cb_SetSubStatus(SOPC_PubSubState state)
{
    printk("New Pub/sub state: %d\n", (int) state);
}

/***************************************************/
// Callback in case of assert failure.
static void assert_UserCallback(const char* context)
{
    printk("ASSERT FAILED : <%p>\n", (void*) context);
    printk("Context: <%s>\n", context);
    stopSignal = 1;
    exit(1);
}

/***************************************************/
static void log_UserCallback(const char* context, const char* text)
{
    (void) context;
    if (NULL != text)
    {
        printk("%s\n", text);
    }
}

/***************************************************/
int main(int argc, char* const argv[])
{
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    (void) argc;
    (void) argv;

    SOPC_Assert_Set_UserCallback(&assert_UserCallback);

    bool netInit = Network_Initialize(NULL);
    SOPC_ASSERT(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    /* Initialize S2OPC Server */
    const SOPC_Log_Configuration logCfg = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                           .logSystem = SOPC_LOG_SYSTEM_USER,
                                           .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};

    SOPC_ReturnStatus status = SOPC_Common_Initialize(logCfg);

    if (SOPC_STATUS_OK != status)
    {
        printf("Error while initializing logs\n");
    }

    // CONFIGURE PUBSUB
    SOPC_PubSubConfiguration* pPubSubConfig = SOPC_PubSubConfig_GetStatic();
    if (NULL == pPubSubConfig)
    {
        return SOPC_STATUS_NOK;
    }

    /* Sub target configuration */
    SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pTargetConfig = SOPC_SubTargetVariableConfig_Create(&Cache_SetTargetVariables);
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
        pSourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
        if (NULL == pSourceConfig)
        {
            printf("# Error: Cannot create Pub configuration.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Configure SKS for PubSub
        SOPC_LocalSKS_init_static(pubSub_keySign, sizeof(pubSub_keySign),       //
                                  pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt), //
                                  pubSub_keyNonce, sizeof(pubSub_keyNonce));

        Cache_Initialize(pPubSubConfig);
    }


    // Start Pub sub
    bool bResult;
    bResult = SOPC_SubScheduler_Start(pPubSubConfig, pTargetConfig, cb_SetSubStatus, CONFIG_SOPC_SUBSCRIBER_PRIORITY);
    SOPC_ASSERT(bResult);
    bResult = SOPC_PubScheduler_Start(pPubSubConfig, pSourceConfig, CONFIG_SOPC_PUBLISHER_PRIORITY);
    SOPC_ASSERT(bResult);

    printk("\r\nDemo (PubSub + Server) started\r\n");

    while (!stopSignal)
    {
        k_sleep(K_MSEC(50));
    }

    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    Cache_Clear();
    SOPC_PubSourceVariableConfig_Delete(pSourceConfig);
    SOPC_SubTargetVariableConfig_Delete(pTargetConfig);
    SOPC_PubSubConfiguration_Delete(pPubSubConfig);

    printk("\r\nTEST ended\r\n");
    return 0;
}

