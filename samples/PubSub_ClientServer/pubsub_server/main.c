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

#include "sopc_helper_uri.h"
#include "sopc_macros.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_time.h"

#include "config.h"
#include "pubsub.h"
#include "server.h"

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

int main(int argc, char* const argv[])
{
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    if (argc > 1)
    {
        assert(argc == 2);
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

    /* Initialize S2OPC Server */
    SOPC_ReturnStatus status = Server_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not initialize the server.\n");
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
    Server_StopAndClear(&s2opcConfig);
    printf("# Info: Server closed.\n");
}
