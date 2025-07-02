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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_date_time.h"
#include "sopc_helper_uri.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"

#include "device_config.h"
#include "device_pubsub.h"
#include "device_server.h"

static SOPC_ReturnStatus ClientServer_Initialize(char* logPath);
static char* ClientServer_GetLogPath(const char* binName);

volatile sig_atomic_t stopSignal = 0;
static void signal_stop_server(int sig)
{
    SOPC_UNUSED_ARG(sig);

    if (stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        stopSignal = 1;
    }
}

static SOPC_ReturnStatus ClientServer_Initialize(char* logPath)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = logPath;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;

    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server initialized");
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Server initialization failed");
    }
    return status;
}

static char* ClientServer_GetLogPath(const char* binName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // './' + <name> + '_logs/'
    char* logPath = SOPC_Calloc(strlen(binName) + 6 + 1, sizeof(char));
    if (NULL == logPath)
    {
        return NULL;
    }
    char* res = strcpy(logPath, binName);
    if (res == logPath)
    {
        res = strcpy(logPath + strlen(binName), "_logs/");
        if (res != logPath + strlen(binName))
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server log path initialized to '%s'", logPath);
    }
    else
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server log initialized with default configuration './logs'");
        res = strcpy(logPath, "./logs");
    }
    return logPath;
}

int main(int argc, char* const argv[])
{
    SOPC_UNUSED_ARG(argc);

    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Initialize S2OPC Client / Server */
    char* logDirPath = ClientServer_GetLogPath(argv[0]);
    SOPC_ReturnStatus status = ClientServer_Initialize(logDirPath);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Could not initialize the PubSub client/server");
    }

    /* Configure the Server */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_CreateServerConfig();
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Could not create the server configuration");
        }
    }

    /* Start the Server */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_StartServer();
    }

    /* Start the PubSub. */
    if (SOPC_STATUS_OK == status)
    {
        status = PubSub_Configure();
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub configured");
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Fail to configure the PubSub module");
        }
        if (SOPC_STATUS_OK == status)
        {
            status = PubSub_Start() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to start the PubSub module");
                PubSub_Stop(); // Ensure Pub & Sub are stopped in this case
            }
            else
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub started");
            }
        }
    }

    /* Wait for a signal */
    if (SOPC_STATUS_OK == status)
    {
        while (Server_IsRunning() && stopSignal == 0)
        {
            SOPC_Sleep(SLEEP_TIMEOUT);
        }
    }

    /* Clean and quit */
    PubSub_StopAndClear();
    Server_StopAndClear();
    SOPC_CommonHelper_Clear();

    SOPC_Free(logDirPath);
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server closed");
}
