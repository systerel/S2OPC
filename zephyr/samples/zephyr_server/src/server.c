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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "embedded/sopc_addspace_loader.h"
#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_dict.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "static_security_data.h"
#include "xml_expat/sopc_uanodeset_loader.h"

#include "helpers.h"
#include "server.h"

#ifndef WITH_STATIC_SECURITY_DATA
#error "This demo requires WITH_STATIC_SECURITY_DATA build flag!"
#endif

/* These variables could be stored in a struct Server_Context, which is then passed to all functions.
 * This would mimic class instances and avoid global variables.
 */
static uint32_t epConfigIdx = 0;
static int32_t serverOnline = 0;

static SOPC_AddressSpace* address_space = NULL;
static char* lastPubSubConfigPath = NULL;

static char* default_trusted_certs[] = {NULL, NULL};
static char* default_revoked_certs[] = {NULL, NULL};
static char* empty_certs[] = {NULL};

static SOPC_ReturnStatus Server_SetAddressSpace(void);
static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      SOPC_App_AddSpace_Event event,
                                      void* opParam,
                                      SOPC_StatusCode opStatus);
static void Server_Event_Toolkit(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext);
static void Server_Event_Write(OpcUa_WriteValue* pwv);

/**********************************************************************/
SOPC_ReturnStatus Server_Initialize(const SOPC_Log_Configuration* pLogCfg)
{
    assert(NULL != pLogCfg);
    SOPC_ReturnStatus status = SOPC_Common_Initialize(*pLogCfg);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Server_Event_Toolkit);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: SOPC_Toolkit_Initialize failed : %d\n", status);
        }
    }
    else
    {
        printf("SOPC_Common_Initialize failed : %d\n", status);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Server initialized.\n");
    }
    else
    {
        printf("# Error: Server initialization failed.\n");
    }

    return status;
}

/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */

SOPC_ReturnStatus Server_CreateServerConfig(SOPC_S2OPC_Config* output_s2opcConfig)
{
    if (NULL == output_s2opcConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;

    /* Application description configuration */
    OpcUa_ApplicationDescription* serverDescription = &output_s2opcConfig->serverConfig.serverDescription;
    OpcUa_ApplicationDescription_Initialize(serverDescription);
    SOPC_String_AttachFromCstring(&serverDescription->ApplicationUri, APPLICATION_URI);
    SOPC_String_AttachFromCstring(&serverDescription->ProductUri, PRODUCT_URI);
    serverDescription->ApplicationType = OpcUa_ApplicationType_Server;
    SOPC_String_AttachFromCstring(&serverDescription->ApplicationName.defaultText, SERVER_DESCRIPTION);

    /* Cryptographic configuration */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    output_s2opcConfig->serverConfig.serverCertPath = NULL;
    output_s2opcConfig->serverConfig.serverKeyPath = NULL;
    output_s2opcConfig->serverConfig.trustedRootIssuersList = default_trusted_certs;
    output_s2opcConfig->serverConfig.trustedIntermediateIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.issuedCertificatesList = empty_certs;
    output_s2opcConfig->serverConfig.untrustedRootIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.untrustedIntermediateIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.certificateRevocationPathList = default_revoked_certs;

    SOPC_SerializedCertificate* static_cacert = NULL;
    SOPC_CRLList* static_cacrl = NULL;

    status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(server_2k_cert, sizeof(server_2k_cert),
                                                                 &output_s2opcConfig->serverConfig.serverCertificate);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(server_2k_key),
                                                                        &output_s2opcConfig->serverConfig.serverKey);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &static_cacert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_cacrl);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_Create(static_cacert, static_cacrl, &output_s2opcConfig->serverConfig.pki);
    }

    /* Clean in all cases */
    SOPC_KeyManager_SerializedCertificate_Delete(static_cacert);

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Failed loading certificates and key (check paths are valid).\n");
    }

    /* Configuration of the endpoint descriptions */
    output_s2opcConfig->serverConfig.nbEndpoints = 1;

    output_s2opcConfig->serverConfig.endpoints = SOPC_Calloc(sizeof(SOPC_Endpoint_Config), 1);

    if (NULL == output_s2opcConfig->serverConfig.endpoints)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_Endpoint_Config* pEpConfig = &output_s2opcConfig->serverConfig.endpoints[0];
    pEpConfig->nbSecuConfigs = 3;

    /* Server's listening endpoint */
    pEpConfig->serverConfigPtr = &output_s2opcConfig->serverConfig;
    pEpConfig->endpointURL = CONFIG_SOPC_ENDPOINT_ADDRESS;

    /* 1st Security policy is None without user (users on unsecure channel shall be forbidden) */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_None_URI);
        pEpConfig->secuConfigurations[0].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        pEpConfig->secuConfigurations[0].nbOfUserTokenPolicies = 1;
        pEpConfig->secuConfigurations[0].userTokenPolicies[0] = SOPC_UserTokenPolicy_Anonymous;
    }

    /* 2nd Security policy is Basic256 with anonymous or username authentication allowed
     * (without password encryption) */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[1].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[1].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256_URI);
        pEpConfig->secuConfigurations[1].securityModes =
            SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        pEpConfig->secuConfigurations[1].nbOfUserTokenPolicies = 2;
        pEpConfig->secuConfigurations[1].userTokenPolicies[0] = SOPC_UserTokenPolicy_Anonymous;
        pEpConfig->secuConfigurations[1].userTokenPolicies[1] =
            SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy;
    }

    /* 3rd Security policy is Basic256Sha256 with anonymous or username authentication allowed
     * (without password encryption) */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[2].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[2].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        pEpConfig->secuConfigurations[2].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        pEpConfig->secuConfigurations[2].nbOfUserTokenPolicies = 2;
        pEpConfig->secuConfigurations[2].userTokenPolicies[0] = SOPC_UserTokenPolicy_Anonymous;
        pEpConfig->secuConfigurations[2].userTokenPolicies[1] =
            SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy;
    }

    /* User authentication and authorization */
    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
        authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == authenticationManager || NULL == authorizationManager)
        {
            printf("# Error: Failed to create user authentication and authorization managers.\n");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pEpConfig->authenticationManager = authenticationManager;
        pEpConfig->authorizationManager = authorizationManager;
    }
    else
    {
        SOPC_UserAuthentication_FreeManager(&authenticationManager);
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
    }

    return status;
}

// static address space.
static SOPC_ReturnStatus Server_SetAddressSpace(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    address_space = SOPC_Embedded_AddressSpace_Load();
    if (NULL == address_space)
    {
        printf("# Error: Cannot load static address space.\n");
        status = SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_ReturnStatus Server_LoadAddressSpace(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = Server_SetAddressSpace();

    /* Set address space and set its notification callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(address_space);

        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to set the address space configuration.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(&Server_Event_AddressSpace);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to configure the address space notification callback.\n");
        }
    }

    return status;
}

SOPC_ReturnStatus Server_ConfigureStartServer(SOPC_Endpoint_Config* pEpConfig)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    /* Add endpoint description configuration and set the Toolkit as configured */
    epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(pEpConfig);
    if (epConfigIdx != 0)
    {
        status = SOPC_ToolkitServer_Configured();
    }
    else
    {
        printf("# Error: Failed to configure the endpoint.\n");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Endpoint and Toolkit configured.\n");
    }

    /* Starts the server */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
        SOPC_Atomic_Int_Set(&serverOnline, 1);
        printf("# Info: Server started.\n");
    }

    return status;
}

void Server_Interrupt(void)
{
    SOPC_Atomic_Int_Set(&serverOnline, 0);
}

bool Server_IsRunning(void)
{
    return SOPC_Atomic_Int_Get(&serverOnline);
}

bool Server_LocalWriteSingleNode(SOPC_NodeId* pNid, SOPC_DataValue* pDv)
{
    SOPC_NodeId* lpNid[1] = {pNid};
    uint32_t lAttrId[1] = {13};
    SOPC_DataValue* lpDv[1] = {pDv};

    SOPC_ReturnStatus status = Helpers_AsyncLocalWrite(epConfigIdx, lpNid, lAttrId, lpDv, 1, Server_Event_Write);
    return status == SOPC_STATUS_OK;
}

void Server_StopAndClear(SOPC_S2OPC_Config* pConfig)
{
    /* SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx); */
    SOPC_Toolkit_Clear();

    if (Server_IsRunning())
    {
        SOPC_Atomic_Int_Set(&serverOnline, 0);
    }

    SOPC_AddressSpace_Delete(address_space);

    if (NULL != pConfig)
    {
        SOPC_S2OPC_Config_Clear(pConfig);
        pConfig = NULL;
    }

    if (NULL != lastPubSubConfigPath)
    {
        SOPC_Free(lastPubSubConfigPath);
        lastPubSubConfigPath = NULL;
    }
}

static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      SOPC_App_AddSpace_Event event,
                                      void* opParam,
                                      SOPC_StatusCode opStatus)
{
    (void) callCtxPtr;
    /* Watch modifications of configuration paths and the start/stop command */
    if (AS_WRITE_EVENT != event)
    {
        printf("# Warning: Received unexpected event in address space notification handler: %d.\n", event);
        return;
    }

    if ((opStatus & SOPC_GoodStatusOppositeMask) != 0)
    {
        printf("# Warning: Received address space event which status code is not good: 0x%08X. Ignored\n",
               (unsigned int) opStatus);
        return;
    }

    Server_Event_Write((OpcUa_WriteValue*) opParam);
}

static void Server_Event_Toolkit(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    (void) idOrStatus;
    SOPC_EncodeableType* message_type = NULL;

    OpcUa_WriteResponse* writeResponse = NULL;

    switch (event)
    {
    case SE_CLOSED_ENDPOINT:
        printf("# Info: Closed endpoint event.\n");
        SOPC_Atomic_Int_Set(&serverOnline, 0);
        return;
    case SE_LOCAL_SERVICE_RESPONSE:
        message_type = *((SOPC_EncodeableType**) param);
        /* Listen for WriteResponses, which only contain status codes */
        /*if (message_type == &OpcUa_WriteResponse_EncodeableType)
        {
            OpcUa_WriteResponse* write_response = param;
            bool ok = (write_response->ResponseHeader.ServiceResult == SOPC_GoodGenericStatus);
        }*/
        /* Listen for ReadResponses, used in GetSourceVariables
         * This can be used for example when PubSub is defined and uses address space */

        /*if (message_type == &OpcUa_ReadResponse_EncodeableType && NULL != ctx)
        {
            ctx = (SOPC_PubSheduler_GetVariableRequestContext*) appContext;
            // Then copy content of response to ctx...
        } */
        if (message_type == &OpcUa_WriteResponse_EncodeableType)
        {
            writeResponse = param;
            // Service should have succeeded
            assert(0 == (SOPC_GoodStatusOppositeMask & writeResponse->ResponseHeader.ServiceResult));
        }
        else
        {
            assert(false);
        }
        return;
    default:
        printf("# Warning: Unexpected endpoint event: %d.\n", event);
        return;
    }
}

static void Server_Event_Write(OpcUa_WriteValue* pwv)
{
    if (NULL == pwv)
    {
        printf("# Warning: NULL pointer passed instead of WriteValue to address space notification callback.\n");
        return;
    }

    /* IndexRange should be empty (entire Value) */
    if (0 < pwv->IndexRange.Length)
    {
        printf("# Warning: Address space notification with non empty IndexRange: %s.\n",
               SOPC_String_GetRawCString(&pwv->IndexRange));
        /* return; */
    }

    /* We only care about modifications of the Value attribute */
    if (13 != pwv->AttributeId)
    {
        return;
    }

    char* nidStr = SOPC_NodeId_ToCString(&pwv->NodeId);
    SOPC_ASSERT(NULL != nidStr);
    printf("# Info: Received write event on nodeId : %s\n", nidStr);
    SOPC_Free(nidStr);
}

SOPC_Array* Server_GetConfigurationPaths(void)
{
    SOPC_Array* array = SOPC_Array_Create(sizeof(char*), 1, NULL);

    bool ok = false;
    if (NULL != array)
    {
        ok = SOPC_Array_Append(array, lastPubSubConfigPath);
    }

    if (!ok)
    {
        SOPC_Array_Delete(array);
        array = NULL;
    }

    return array;
}
