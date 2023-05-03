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
#include <stdlib.h>

/* FreeRTOS include */
#include "FreeRTOS.h"
#include "task.h" // Used for disable task entering in user assert
#include "sys_evt.h" // Used to signal when MxNet has finished configuring Network interface

/* MIMXRT1064 includes */
//#include "fsl_debug_console.h"
#include "publisher.h"

/* S2OPC includes */
#include "assert.h"
#include "cache.h"
#include "config_publisher/pubsub_config_static.h"
#include "sopc_common.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_time.h"
#include "static_security_data.h"

#include "sopc_macros.h"

#define PUBLISHER_THREAD_PRIORITY 30
#define SLEEP_TIMEOUT 100

static void log_userCallback(const char* context, const char* text)
{
    SOPC_UNUSED_ARG(context);
    if (NULL != text)
    {
        LogInfo("%s\n", text);
    }
}

static void assert_userCallback(const char* context)
{
    LogInfo("ASSERT FAILED : <%p>\r\n", (void*) context);
    LogInfo("Context: <%s>", context);
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void cbToolkit_publisher(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Block until the network interface is connected */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVT_MASK_NET_CONNECTED,
                                  0x00,
                                  pdTRUE,
                                  portMAX_DELAY );

    vTaskDelay(5000);

    SOPC_PubSubConfiguration* config = NULL;
    bool res = false;

    // Set user assert
    SOPC_Assert_Set_UserCallback(&assert_userCallback);

    /* Initialize toolkit and configure logs */
    const SOPC_Log_Configuration logConfig = {.logLevel = SOPC_LOG_LEVEL_DEBUG,
                                              .logSystem = SOPC_LOG_SYSTEM_USER,
                                              .logSysConfig = {.userSystemLogConfig = {.doLog = &log_userCallback}}};
    status = SOPC_Common_Initialize(logConfig);

    if (SOPC_STATUS_OK == status)
    {
        config = SOPC_PubSubConfig_GetStatic();
        if (NULL == config)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Configure data source handler */
    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        sourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
        if (NULL == sourceConfig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Initialize key bunch for sign or sign and encrypt communication */
        SOPC_KeyBunch_init_static(pubSub_keySign, sizeof(pubSub_keySign), pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt),
                                  pubSub_keyNonce, sizeof(pubSub_keyNonce));

        res = Cache_Initialize(config);
        if (!res)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        res = SOPC_PubScheduler_Start(config, sourceConfig, PUBLISHER_THREAD_PRIORITY);
        if (!res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    /* Wait until program stop */
    while (SOPC_STATUS_OK == status)
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
    }

    /* Clean and quit */
    SOPC_PubScheduler_Stop();
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_Common_Clear();
    Cache_Clear();
}
