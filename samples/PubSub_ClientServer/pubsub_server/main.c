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

#include "client.h"
#include "config.h"
#include "pubsub.h"
#include "server.h"

static SOPC_ReturnStatus ClientServer_Initialize(char* logPath);
static char* ClientServer_GetLogPath(const char* binName, const char* port);

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

static char* ClientServer_GetLogPath(const char* binName, const char* port)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // './' + <name> + <port> + '_logs/'
    char* logPath = SOPC_Calloc(strlen(binName) + strlen(port) + 6 + 1, sizeof(char));
    if (NULL == logPath)
    {
        return NULL;
    }
    char* res = strcpy(logPath, binName);
    if (res == logPath)
    {
        res = strcpy(logPath + strlen(binName), port);
        if (res == logPath + strlen(binName))
        {
            res = strcpy(logPath + strlen(binName) + strlen(port), "_logs/");
            if (res != logPath + strlen(binName) + strlen(port))
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else
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
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    if (argc > 1)
    {
        SOPC_ASSERT(argc == 2);
        size_t tmp, tmp2, tmp3;
        bool res = SOPC_Helper_URI_ParseUri_WithPrefix("opc.tcp://", argv[1], &tmp, &tmp2, &tmp3);
        if (!res)
        {
            printf("# Error: invalid OPC UA server address\n");
            exit(1);
        }
        ENDPOINT_URL = argv[1];
    }
    else
    {
        ENDPOINT_URL = DEFAULT_ENDPOINT_URL;
    }

    size_t hostNameLength, portIdx, portLength;
    bool res = SOPC_Helper_URI_ParseUri_WithPrefix("opc.tcp://", ENDPOINT_URL, &hostNameLength, &portIdx, &portLength);
    if (!res)
    {
        printf("Invalid OPC UA server address: %s", ENDPOINT_URL);
        exit(1);
    }

    /* Initialize S2OPC Client / Server */
    char* logDirPath = ClientServer_GetLogPath(argv[0], &ENDPOINT_URL[portIdx]);
    SOPC_ReturnStatus status = ClientServer_Initialize(logDirPath);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Could not initialize the PubSub client/server");
    }

    /* Configure the Client application for SKS server access */
    if (SOPC_STATUS_OK == status)
    {
        status = Client_Initialize();
    }

    /* Configure the Server for PubSub data exposition */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_CreateServerConfig();
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Could not create the server configuration");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadAddressSpace();
    }

    /* Configuration of the PubSub module is done upon PubSub start through the local service */

    /* Start the Server */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_StartServer();
    }

    /* Write in PubSub nodes, which starts the PubSub */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_WritePubSubNodes();
    }

    int startWaitSec = 0;
    /* Wait for a signal */
    while (SOPC_STATUS_OK == status && Server_IsRunning() && stopSignal == 0)
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
        if (Server_PubSubStop_Requested())
        {
            PubSub_Stop();
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub stopped through Start/Stop Command");
            startWaitSec = WAIT_STOP_START / SLEEP_TIMEOUT;
        }
        if (startWaitSec > 0)
        {
            // This avoids to restart instantly after a STOP
            startWaitSec--;
        }
        else if (Server_PubSubStart_Requested())
        {
            status = PubSub_Configure();
            if (SOPC_STATUS_OK == status)
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub configured through Start/Stop Command");
            }
            else
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                         "Start/Stop Command failed to configure the PubSub module");
            }

            if (SOPC_STATUS_OK == status)
            {
                status = PubSub_Start() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                if (SOPC_STATUS_OK != status)
                {
                    PubSub_Stop(); // Ensure Pub & Sub are stopped in this case
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "PubSub started through Start/Stop Command");
            }
            else
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                         "Start/Stop Command failed to start the PubSub module");
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "waiting for a new configure + start command !");
            status = SOPC_STATUS_OK;
        }
        /* Writer Group Id cannot be equal to 0 if Acyclic publisher have been triggered to send something writer group
         * Id should be different than 0 */
        uint16_t writerGroupId = (uint16_t) Server_PubAcyclicSend_Requested();
        if (writerGroupId)
        {
            status = Server_Trigger_Publisher(writerGroupId) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            if (SOPC_STATUS_NOK == status)
            {
                printf("# Warning: Acyclic send request failed with writer group id %" PRIu16
                       ". \n Check that PubSub is started and if publisher is set "
                       "as acyclic publisher and writer group id correspond to configuration. \n",
                       writerGroupId);
            }
            status = SOPC_STATUS_OK;
        }
        /* Tuple [PublisherId, WriterGroupId, DataSetWriterId] cannot be equal to [0, 0, 0]. If FilteringDsmEmission
         * hasn't been trigger then publisherId type is NUll */
        struct publisherDsmIdentifier pubDsmId = Server_PubFilteringDataSetMessage_Requested();
        if (pubDsmId.pubId.type != SOPC_Null_PublisherId)
        {
            status = Server_Trigger_FilteringDsmEmission(pubDsmId) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            if (SOPC_STATUS_OK != status)
            {
                printf("# Warning: Filtering DataSetMessage request fail for tuple [ publisherId : %" PRIu64
                       ", WriterGroupId : %" PRIu16 ", dataSetWriterId : %" PRIu16
                       " ]. \n"
                       "Check that PubSub is started and configuration match the tuple furnish\n",
                       pubDsmId.pubId.data.uint, pubDsmId.writerGroupId, pubDsmId.dataSetWriterId);
            }
            status = SOPC_STATUS_OK;
        }
    }

    /* Clean and quit */
    PubSub_StopAndClear();
    Client_Clear();
    Server_StopAndClear();
    SOPC_CommonHelper_Clear();

    SOPC_Free(logDirPath);
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server closed");
}
