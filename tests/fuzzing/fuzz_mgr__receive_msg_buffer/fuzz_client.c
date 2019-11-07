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
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"

#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"
#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "sopc_mem_alloc.h"

#include "constants.h"

#include "embedded/sopc_addspace_loader.h"

#include "fuzz_main.h"
#include "fuzz_server.h"
#include "fuzz_client.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

OpcUa_WriteRequest *pWriteReq = NULL;

t_CerKey ck_cli;
uint32_t session = 0;
uintptr_t sessionContext[2] = {0, 1};
SessionConnectedState scState = SESSION_CONN_NEW;
uint32_t channel_config_idx = 0;

static SOPC_ReturnStatus CerAndKeyLoader_client();

// Configure the 2 secure channel connections to use and retrieve channel configuration index
static SOPC_ReturnStatus CerAndKeyLoader_client()
{
	SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
		#ifdef WITH_STATIC_SECURITY_DATA
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(client_2k_cert, sizeof(client_2k_cert),
                                                                     &(ck_cli).Certificate);
        scConfig->crt_cli = (ck_cli).Certificate;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(client_2k_key),
                                                                            &&(ck_cli).Key);
            scConfig->key_priv_cli = (ck_cli).Key;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &(ck_cli).authCertificate);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(&(ck_cli).authCertificate, NULL, &(ck_cli).pkiProvider);
            scConfig->pki = (ck_cli).pkiProvider;
        }
        #endif
        if (SOPC_STATUS_OK != status)
        {
            if (true == debug)
            {
                printf("<Test_Server_Toolkit: Failed loading certificates and key (check paths are valid)\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf("<Test_Server_Toolkit: Certificates and key loaded\n");
            }
        }
    }
    else
    {
    	scConfig.crt_cli = NULL;
    	scConfig.key_priv_cli = NULL;
    	scConfig.pki = NULL;
    }
    return (status);
}

static void setScConfig_client(bool onSecu)
{
	memset(&scConfig, 0, sizeof(struct SOPC_SecureChannel_Config));

    if (true == onSecu)
    {
        scConfig.isClientSc = true;
        scConfig.url = ENDPOINT_URL;
        scConfig.crt_srv = NULL;
        scConfig.reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;
        scConfig.requestedLifetime = 20000;
        scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
    }
    else
    {
        scConfig.isClientSc = true;
        scConfig.url = ENDPOINT_URL;
        scConfig.crt_srv = NULL;
        scConfig.reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI;
        scConfig.requestedLifetime = 20000;
        scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_None;
    }
}

SOPC_ReturnStatus Setup_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Certificate and key serialization
    status = CerAndKeyLoader_client();
    if (SOPC_STATUS_OK == status)
    {
        setScConfig_client(false);
        if (true == debug)
        {
            printf(">>FUZZ_Client: Certificate, key and Sc configured successfully\n");
        }
    }
    else
    {
        if (true == debug)
    	{
        	printf(">>FUZZ_Client: FAILED on configuring Certificate, key and Sc\n");
    	}
    }

    return (status);
}

SOPC_ReturnStatus AddSecureChannelconfig_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

	channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
	if (channel_config_idx != 0)
	{
		if (true == debug)
	    {
			printf(">>FUZZ_Client: Client configured\n");
	    }
	}
	else
	{
		if (true == debug)
	    {
			printf(">>FUZZ_Client: Failed to configure the secure channel connections\n");
	    }
	status = SOPC_STATUS_NOK;
	}
	return (status);
}

SOPC_ReturnStatus Wait_response_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 1;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&scState) != SESSION_CONN_MSG_RECEIVED
           && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    return (status);
}

static SOPC_ReturnStatus ActivateSessionWait_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 1;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    /* Wait until session is activated or timeout */
    while (SOPC_Atomic_Int_Get(&scState) != SESSION_CONN_CONNECTED && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_Atomic_Int_Get(&scState) == SESSION_CONN_CONNECTED && SOPC_Atomic_Int_Get(&sendFailures) == 0)
    {
        if (true == debug)
        {
            printf(">>FUZZ_Client: Sessions activated: OK'\n");
        }
    }
    else
    {
        if (true == debug)
        {
            printf(">>FUZZ_Client:: Sessions activated: NOK'\n");
        }
        status = SOPC_STATUS_NOK;
    }
    return (status);
}

OpcUa_WriteRequest *newWriteRequest_client(const char *buff, size_t len)
{
    OpcUa_WriteValue *lwv = NULL;
    SOPC_ByteString buf;
    SOPC_ByteString_Initialize(&buf);

    buf.Length = (int32_t) len;
    buf.Data = SOPC_Malloc(len + 1);
    lwv = SOPC_Malloc(sizeof (OpcUa_WriteValue));
    if (NULL == buf.Data)
    {
    	exit(1);
    }

    memcpy((void*) (buf.Data), buff, len + 1);

    lwv[0] = (OpcUa_WriteValue){.encodeableType = &OpcUa_WriteValue_EncodeableType,
                               .NodeId = {
                               	   		  .IdentifierType = SOPC_IdentifierType_String,
                                          .Data.String.Data = (uint8_t*)"ByteString_042",
										  .Data.String.Length = strlen("ByteString_042"),
										  .Data.String.DoNotClear = true,
                                          .Namespace = 1
    									 },
                               .AttributeId = constants__e_aid_Value,
                               .IndexRange = {.Length = 0},
                               .Value = {
                            		   .Value = {
                            				     .BuiltInTypeId = SOPC_ByteString_Id,
											     .ArrayType = SOPC_VariantArrayType_SingleValue,
											     .Value.String = buf
                            		   	   	    },
									   .Status = SOPC_GoodGenericStatus
    									}
    						};

    OpcUa_WriteRequest* pReq = DESIGNATE_NEW(OpcUa_WriteRequest, .encodeableType = &OpcUa_WriteRequest_EncodeableType,
                                             .NoOfNodesToWrite = (int32_t) 1, .NodesToWrite = lwv);
    if (NULL == pReq)
    {
    	exit(1);
    }
    return (pReq);
}

SOPC_ReturnStatus Run_client(char *buff, size_t len)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (true == debug)
	{
    	printf(">>FUZZ_Client: Channel_config_idx :%d\n", channel_config_idx);
	}
    status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx, 1, "anonymous");
    if (SOPC_STATUS_OK == status)
    {
        if (true == debug)
    	{
        	printf(">>FUZZ_Client: SOPC_ToolkitClient_AsyncActivateSession_Anonymous request sent\n");
    	}
        status = ActivateSessionWait_client();
        if (true == debug)
        {
            printf(">>FUZZ_Client: Creating/Activating 1 sessions on 1 SC: OK\n");
        }
    }
    else
    {
        if (true == debug)
        {
            printf(">>FUZZ_Client: Failed to create/Activate session\n");
        }
    }
    if (SOPC_STATUS_OK == status)
    {

        // Create WriteRequest to be sent (deallocated by toolkit)
        pWriteReq = newWriteRequest_client(buff, len);

        if (true == debug)
        {
            printf(">>FUZZ_Client: write request sending\n");
        }
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session), pWriteReq,
                                                     1);

    }
    if (SOPC_STATUS_OK == status)
    {
    	status = Wait_response_client();
    }
    return (status);
}

SOPC_ReturnStatus Teardown_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t session1_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session);

    sendFailures = 0;

    /* Close the session */
	SessionConnectedState scStateCompare = SOPC_Atomic_Int_Get(&scState);
    if (0 != session1_idx && (scStateCompare == SESSION_CONN_MSG_RECEIVED || scStateCompare == SESSION_CONN_CONNECTED))
    {
        SOPC_ToolkitClient_AsyncCloseSession(session1_idx);
    }
    SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_NEW);
    return (status);
}
