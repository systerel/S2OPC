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

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_helper_uri.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"

#include "client.h"
#include "config.h"
#include "pubsub.h"
#include "server.h"

static void ClientServer_Event_Toolkit(SOPC_App_Com_Event event,
                                       uint32_t idOrStatus,
                                       void* param,
                                       uintptr_t appContext);
static SOPC_ReturnStatus ClientServer_Initialize(char* logPath);
static char* ClientServer_GetLogPath(const char* binName, const char* port);

volatile sig_atomic_t stopSignal = 0;
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

static void ClientServer_Event_Toolkit(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    (void) idOrStatus;
    bool debug = false;
    switch (event)
    {
    /* Client application events */
    case SE_SESSION_ACTIVATION_FAILURE:
        if (debug)
        {
            printf(">>Client debug : SE_SESSION_ACTIVATION_FAILURE RECEIVED\n");
            printf(">>Client debug : appContext: %lu\n", appContext);
        }
        if (0 != appContext && appContext == Client_SessionContext)
        {
            SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_FAILED);
        }
        else
        {
            assert(false && ">>Client : bad app context");
        }
        break;
    case SE_ACTIVATED_SESSION:
        SOPC_Atomic_Int_Set((int32_t*) &session, (int32_t) idOrStatus);
        if (debug)
        {
            printf(">>Client debug : SE_ACTIVATED_SESSION RECEIVED\n");
        }
        SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_CONNECTED);
        break;
    case SE_SESSION_REACTIVATING:
        if (debug)
        {
            printf(">>Client debug : SE_SESSION_REACTIVATING RECEIVED\n");
        }
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (debug)
        {
            printf(">>Client debug : SE_RCV_SESSION_RESPONSE RECEIVED\n");
        }
        Client_Treat_Session_Response(param, appContext);
        break;
    case SE_CLOSED_SESSION:
        if (debug == true)
        {
            printf(">>Client debug : SE_CLOSED_SESSION RECEIVED\n");
        }
        break;

    case SE_RCV_DISCOVERY_RESPONSE:
        if (debug == true)
        {
            printf(">>Client debug : SE_RCV_DISCOVERY_RESPONSE RECEIVED\n");
        }
        break;

    case SE_SND_REQUEST_FAILED:
        if (debug == true)
        {
            printf(">>Client debug : SE_SND_REQUEST_FAILED RECEIVED\n");
        }
        SOPC_Atomic_Int_Add(&sendFailures, 1);
        break;

        /* SERVER EVENT */
    case SE_CLOSED_ENDPOINT:
        printf("# Info: Closed endpoint event.\n");
        SOPC_Atomic_Int_Set(&serverOnline, 0);
        return;
    case SE_LOCAL_SERVICE_RESPONSE:
        Server_Treat_Local_Service_Response(param, appContext);
        return;
    default:
        printf("# Warning: Unexpected endpoint event: %d.\n", event);
        return;
    }
}

SOPC_ReturnStatus ClientServer_Initialize(char* logPath)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = logPath;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(ClientServer_Event_Toolkit);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Server initialized.\n");
    }
    else
    {
        printf("# Error: Server initialization failed.\n");
        return status;
    }
    return SOPC_STATUS_OK;
}

char* ClientServer_GetLogPath(const char* binName, const char* port)
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
        printf("# Info: Server log path initialized to '%s'.\n", logPath);
    }
    else
    {
        printf("# Info: Server log initialized with default configuration './logs'.\n");
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
        assert(argc == 2);
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
        printf("# Error: invalid OPC UA server address\n");
        exit(1);
    }

    /* Initialize S2OPC Server */
    char* logDirPath = ClientServer_GetLogPath(argv[0], &ENDPOINT_URL[portIdx]);
    SOPC_ReturnStatus status = ClientServer_Initialize(logDirPath);
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not initialize the PubSub client/server.\n");
    }

    /* Configure the Server */
    SOPC_S2OPC_Config s2opcConfig;
    SOPC_S2OPC_Config_Initialize(&s2opcConfig);

    if (SOPC_STATUS_OK == status)
    {
        status = Server_CreateServerConfig(&s2opcConfig);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Could not create the server configuration.\n");
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
        status = Server_ConfigureStartServer(&s2opcConfig.serverConfig.endpoints[0]);
    }

    /* Write in PubSub nodes, which starts the PubSub */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_WritePubSubNodes();
    }

    /* Wait for a signal */
    while (SOPC_STATUS_OK == status && Server_IsRunning() && stopSignal == 0)
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
        if (Server_PubSubStop_Requested())
        {
            PubSub_Stop();
            printf("# Info: PubSub stopped through Start/Stop Command.\n");
        }
        if (Server_PubSubStart_Requested())
        {
            status = PubSub_Configure();
            if (SOPC_STATUS_OK == status)
            {
                printf("# Info: PubSub configured through Start/Stop Command.\n");
            }
            else
            {
                printf("# Warning: Start/Stop Command failed to configure the PubSub module.\n");
            }

            if (SOPC_STATUS_OK == status)
            {
                status = PubSub_Start() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                if (SOPC_STATUS_NOK == status)
                {
                    PubSub_Stop(); // Ensure Pub & Sub are stopped in this case
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                printf("# Info: PubSub started through Start/Stop Command.\n");
            }
            else
            {
                printf("# Warning: Start/Stop Command failed to start the PubSub module.\n");
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("# Warning: waiting for a new configure + start command !\n");
            status = SOPC_STATUS_OK;
        }
    }

    /* Clean and quit */
    PubSub_StopAndClear();
    Client_Teardown();
    Server_StopAndClear(&s2opcConfig);
    SOPC_Free(logDirPath);
    printf("# Info: Server closed.\n");
}
