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
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>
#include <unistd.h>

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

#include "fuzz_client.h"
#include "fuzz_main.h"
#include "fuzz_server.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

/* activate or deactivate log print */
bool debug = false;

/* set to 1 in case of failure */
int32_t sendFailures = 0;

/* not implemented yet: activate or deactivate security mode */
bool secuActive = false;

/* secure chanel config */
SOPC_SecureChannel_Config scConfig;

/* endpoint config */
SOPC_Endpoint_Config epConfig;
SOPC_S2OPC_Config output_s2opcConfig;

void Fuzz_Event_Fct(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;

    switch (event)
    {
    /* Client application events */
    case SE_SESSION_ACTIVATION_FAILURE:
        if (debug == true)
        {
            printf("SE_SESSION_ACTIVATION_FAILURE RECEIVED\n");
            printf("appContext: %lu\n", appContext);
            printf("sessionContext[appContext] = %lu\n", sessionContext[appContext]);
        }
        if (0 != appContext && appContext == sessionContext[appContext])
        {
            SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_FAILED);
        }
        else
        {
            assert(false && ">>Fuzz_Server_event: bad app context");
        }
        break;
    case SE_ACTIVATED_SESSION:
        SOPC_Atomic_Int_Set((int32_t*) &session, (int32_t) idOrStatus);
        if (debug == true)
        {
            printf("SE_ACTIVATED_SESSION RECEIVED\n");
        }
        if (true == debug)
        {
            printf(">>Fuzz_Server_event:: Session activated\n");
        }
        SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_CONNECTED);
        break;
    case SE_SESSION_REACTIVATING:
        if (debug == true)
        {
            printf("SE_SESSION_REACTIVATING RECEIVED\n");
        }
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (debug == true)
        {
            printf("SE_RCV_SESSION_RESPONSE RECEIVED\n");
        }
        if (NULL != param)
        {
            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;

            if (encType == &OpcUa_WriteResponse_EncodeableType)
            {
                if (true == debug)
                {
                    printf(">>FUZZ_Client: received ReadResponse \n");
                }
                SOPC_Atomic_Int_Set((SessionConnectedState*) &scState,
                                    (SessionConnectedState) SESSION_CONN_MSG_RECEIVED);
                OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;
                assert(*(writeResp)->Results == SOPC_GoodGenericStatus && "bad ReadResponse status");
            }
            else
            {
                if (debug == true)
                {
                    printf(">>FUZZ_Client: received ReadResponse Wrong context\n");
                }
            }
        }
        break;
    case SE_CLOSED_SESSION:
        if (debug == true)
        {
            printf("SE_CLOSED_SESSION RECEIVED\n");
        }
        break;

    case SE_RCV_DISCOVERY_RESPONSE:
        if (debug == true)
        {
            printf("SE_RCV_DISCOVERY_RESPONSE RECEIVED\n");
        }
        break;

    case SE_SND_REQUEST_FAILED:
        if (debug == true)
        {
            printf("SE_SND_REQUEST_FAILED RECEIVED\n");
        }
        SOPC_Atomic_Int_Add(&sendFailures, 1);
        break;

    /* Server application events */
    case SE_CLOSED_ENDPOINT:
        if (true == debug)
        {
            printf(">>FUZZ_server: closed endpoint event: OK\n");
        }
        SOPC_Atomic_Int_Set(&scState, SESSION_CONN_CLOSED);
        break;

    case SE_LOCAL_SERVICE_RESPONSE:
        if (true == debug)
        {
            printf("SE_LOCAL_SERVICE_RESPONSE RECEIVED\n");
        }
        break;
    default:
        if (true == debug)
        {
            printf(">>FUZZ_server: unexpected endpoint event %d : NOK\n", event);
        }
        break;
    }
}

int LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)
{
    /*  Install signal handler to close the server gracefully when
     * server needs to stop from a signal
     */
    //signal(SIGINT, StopSignal_serv);
    //signal(SIGTERM, StopSignal_serv);

    /* Used to initialize the server on the first iteration only */
    static bool init = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* one time initialization */
    if (!init)
    {
        status = Setup_serv();
        if (SOPC_STATUS_NOK == status)
        {
            assert(false && ">>FUZZ_main: Failed to setup the server\n");
        }
        if (SOPC_STATUS_OK == status)
        {
            status = Setup_client();
        }
        if (SOPC_STATUS_NOK == status)
        {
            assert(false && ">>FUZZ_main: Failed to setup the client\n");
        }
        if (SOPC_STATUS_OK == status)
        {
            status = AddSecureChannelconfig_client();
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Toolkit_Configured();
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EpConfig_serv();
        }
        if (SOPC_STATUS_OK == status)
        {
            init = true;
        }
    }

    /* create a client, send a request and free the client */
    if (true == init && SOPC_STATUS_OK == status)
    {
        char* buf_copy = SOPC_Calloc(1 + len, sizeof(char));
        assert(buf_copy != NULL);

        memcpy(buf_copy, buf, len);

        status = Run_client(buf_copy, len);

        if (SOPC_STATUS_OK != status)
        {
            if (true == debug)
            {
                printf(">>FUZZ_main: run_client Failed\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_main: run_client SUCCESS\n");
            }
        }

        Teardown_client();
        SOPC_Free(buf_copy);
    }
    else
    {
        if (true == debug)
        {
            printf("Configuration failed\n");
        }
        return (0);
    }
    return (1);
}
