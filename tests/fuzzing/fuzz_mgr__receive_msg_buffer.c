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

#include "embedded/loader.h"
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

#include "fuzz_mgr__receive_msg_buffer.h"
#include "fuzz_mgr__receive_msg_buffer_client.h"
#include "fuzz_mgr__receive_msg_buffer_server.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

bool secuActive = false;

int32_t sendFailures = 0;
int32_t getEndpointsReceived = 0;

uint32_t context2session[3] = {0, 0, 0};
uint32_t cptReadResps = 0;

uintptr_t sessionContext[2] = {1, 2};

void Fuzz_Event_Fct(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;
    (void) appContext;

    switch (event)
    {
    case SE_SESSION_ACTIVATION_FAILURE:
        if (0 != appContext && appContext == sessionContext[appContext])
        {
            SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_FAILED);
            assert(appContext != sessionContext[appContext] || idOrStatus == sessionContext[appContext]);
        }
        else
        {
            assert(false);
        }
        break;
    case SE_ACTIVATED_SESSION:
        // must check context
        SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_CONNECTED);
        break;
    case SE_SESSION_REACTIVATING:
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (NULL != param)
        {
            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;
            if (encType == &OpcUa_ReadResponse_EncodeableType)
            {
                if (true == debug)
                {
                    printf(">>FUZZ_Client: received ReadResponse \n");
                }
                OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) param;
                cptReadResps++;
                // Check context value is same as those provided with request
                assert(cptReadResps == appContext);
                if (cptReadResps <= 1)
                {
                    test_results_set_service_result(
                        test_read_request_response(readResp, readResp->ResponseHeader.ServiceResult, 0) ? true : false);
                }
                else
                {
                    // Second read response is to test write effect (through read result)
                    test_results_set_service_result(
                        tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
                }
            }
            else if (encType == &OpcUa_WriteResponse_EncodeableType)
            {
                // Check context value is same as one provided with request
                assert(1 == appContext);
                if (true == debug)
                {
                    printf(">>FUZZ_Client: received WriteResponse \n");
                }
                OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;
                test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
            }
        }
        break;
    case SE_CLOSED_SESSION:
        break;
    case SE_RCV_DISCOVERY_RESPONSE:
        break;
    case SE_SND_REQUEST_FAILED:
        SOPC_Atomic_Int_Add(&sendFailures, 1);
        break;
    case SE_CLOSED_ENDPOINT:
        if (true == debug)
        {
            printf(">>FUZZ_server: closed endpoint event: OK\n");
        }
        SOPC_Atomic_Int_Set(&endpointClosed, 1);
        break;
    case SE_LOCAL_SERVICE_RESPONSE:
        SOPC_EncodeableType* message_type = *((SOPC_EncodeableType**) param);

        if (message_type != &OpcUa_WriteResponse_EncodeableType)
        {
            return;
        }

        OpcUa_WriteResponse* write_response = param;
        bool ok = (write_response->ResponseHeader.ServiceResult == SOPC_GoodGenericStatus);

        for (int32_t i = 0; i < write_response->NoOfResults; ++i)
        {
            ok &= (write_response->Results[i] == SOPC_GoodGenericStatus);
        }

        if (!ok)
        {
            if (true == debug)
            {
                printf(">>FUZZ_server: Error while updating address space\n");
            }
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
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, StopSignal_serv);
    signal(SIGTERM, StopSignal_serv);
    static bool init = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!init)
    {
        // must be activated and deactivated depending
        //        secuActive = false;
        //        secuActive = true;
        status = Setup_serv();
        if (SOPC_STATUS_OK == status)
        {
            init = true;
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_main: Failed to setup the server\n");
            }
            assert(true);
        }
    }

    if (true == init)
    {
        char* buf_copy = SOPC_Calloc(1 + len, sizeof(char));
        assert(buf_copy != NULL);

        memcpy(buf_copy, buf, len);

        Setup_client();
        // secuActive
        Run_client(buf_copy, len);
        Teardown_client();

        SOPC_Free(buf_copy);

        return (0);
    }
    return (1);
}
