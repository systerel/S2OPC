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

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>


#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#include <fcntl.h>
#include <kernel.h>
#include <net/net_ip.h>
#include <net/socket.h>
#ifndef NULL
#define NULL ((void*) 0)
#endif

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"
#include "sopc_common.h"
#include "sopc_mutexes.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"
#include "sopc_helper_uri.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_time.h"

#include "embedded/sopc_addspace_loader.h"
#include "runtime_variables.h"
#include "network_init.h"
#include "threading_alt.h"

#include "static_security_data.h"

#include "config.h"
#include "pubsub.h"
#include "server.h"

#define LIBC_STDOUT_BUFFER_HOOK 0

/****************************************/

#if LIBC_STDOUT_BUFFER_HOOK == 1

extern void __stdout_buffer_hook_install(void (*fn)(const char*, int));
extern void __stdout_hook_install(int (*fn)(int));

K_MUTEX_DEFINE(my_mutex);

char gBuffer[256];

static void _zephyr_callback_log_buffer_hook(const char* buffer, int size)
{
    //SOPC_LogServer_Print(0, buffer, size, false);
    k_mutex_lock(&my_mutex,K_FOREVER);
    size = size > 256 ? 256 : size;
    memset(gBuffer,0,256);
    memcpy(gBuffer,buffer,size);
    gBuffer[size-1]=0;
    (void)printk("%s\r\n",(char*)gBuffer);
    k_mutex_unlock(&my_mutex);
    return;
}

static int _zephyr_callback_log_hook(int character)
{
    if (character == '\r')
    {
        return character;
    }
    return character;
}

static int _zephyr_add_log_hook(struct device* d)
{
    ARG_UNUSED(d);
    __stdout_hook_install(_zephyr_callback_log_hook);
    __stdout_buffer_hook_install(_zephyr_callback_log_buffer_hook);
    return 0;
}

#endif



#ifndef SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ
void Pub_BeatHeart(void)
{
    SOPC_Sleep(50);
}
#endif

volatile uint32_t stopSignal = 0;

static void* callbackTest(void* pCtx)
{
    /* Signal handling: close the server gracefully when interrupted */
#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 1
    uint32_t tickValue = 0;
#endif

    int argc = 1;
    char** argv = (char**) SOPC_Calloc(1, sizeof(char*));
    assert(argv != NULL);
    argv[0] = (char*) SOPC_Calloc(1, 16);
    assert(argv[0] != NULL);
    snprintf(argv[0], 16, "%s", "toolkit_test");

    /* Parse command line arguments ? */
    if (argc > 1)
    {
        assert(argc == 2);
        int32_t tmp, tmp2, tmp3;
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
        //  Beat heart. Simple pause during SOPC_TIMER_RESOLUTION_MS if
        //  SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ #define is set to 0
        //  in the sopc_pub_scheduler header file.
        //  Else, SOPC_PubScheduler_BeatHeartFromIRQ is defined and called
        //  with a monotonic uint32_t counter, followed by a pause of
        //  SOPC_TIMER_RESOLTION_MS

        Pub_BeatHeart();

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
    return NULL;
}

static void TEST_LAUNCH(void)
{
    SOPC_ReturnStatus result;
#if LIBC_STDOUT_BUFFER_HOOK == 1
    _zephyr_add_log_hook(NULL);
#endif

    bool netInit = Network_Initialize();
    assert(netInit == true);

    Thread sopcThreadHandle1 = NULL;

    printk("\r\nReady to launch application :)\r\n");

    result = SOPC_Thread_Create(&sopcThreadHandle1, callbackTest, NULL, NULL);
    assert(SOPC_STATUS_OK == result);

    SOPC_Thread_Join(sopcThreadHandle1);

    while (true)
    {
        printf("\r\nThread quit, error and go to idle...\r\n");
        SOPC_Sleep(1000);
    }

    return;
}

void main(void)
{
    TEST_LAUNCH();
}
