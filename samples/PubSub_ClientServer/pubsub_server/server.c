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
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_pub_scheduler.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "xml_expat/sopc_uanodeset_loader.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

#include "config.h"
#include "helpers.h"
#include "server.h"

/* These variables could be stored in a struct Server_Context, which is then passed to all functions.
 * This would mimic class instances and avoid global variables.
 */
static uint32_t epConfigIdx = 0;
static int32_t serverOnline = 0;
static int32_t pubSubStopRequested = false;
static int32_t pubSubStartRequested = false;

static SOPC_AddressSpace* address_space = NULL;
static uint8_t lastPubSubCommand = 0;
static char* lastPubSubConfigPath = NULL;

static char* default_trusted_certs[] = {CA_CERT_PATH, NULL};
static char* default_revoked_certs[] = {CA_CRL_PATH, NULL};
static char* empty_certs[] = {NULL};

static SOPC_ReturnStatus Server_SetAddressSpace(void);
static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      SOPC_App_AddSpace_Event event,
                                      void* opParam,
                                      SOPC_StatusCode opStatus);
static void Server_Event_Toolkit(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext);
static void Server_Event_Write(OpcUa_WriteValue* pwv);

SOPC_ReturnStatus Server_Initialize(void)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = LOG_PATH;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Server_Event_Toolkit);
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

    output_s2opcConfig->serverConfig.serverCertPath = SERVER_CERT_PATH;
    output_s2opcConfig->serverConfig.serverKeyPath = SERVER_KEY_PATH;
    output_s2opcConfig->serverConfig.trustedRootIssuersList = default_trusted_certs;
    output_s2opcConfig->serverConfig.trustedIntermediateIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.issuedCertificatesList = empty_certs;
    output_s2opcConfig->serverConfig.untrustedRootIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.untrustedIntermediateIssuersList = empty_certs;
    output_s2opcConfig->serverConfig.certificateRevocationPathList = default_revoked_certs;

#ifdef WITH_STATIC_SECURITY_DATA
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
#else

    status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(output_s2opcConfig->serverConfig.serverCertPath,
                                                                  &output_s2opcConfig->serverConfig.serverCertificate);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(output_s2opcConfig->serverConfig.serverKeyPath,
                                                                        &output_s2opcConfig->serverConfig.serverKey);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_CreateFromPaths(
            output_s2opcConfig->serverConfig.trustedRootIssuersList,
            output_s2opcConfig->serverConfig.trustedIntermediateIssuersList,
            output_s2opcConfig->serverConfig.untrustedRootIssuersList,
            output_s2opcConfig->serverConfig.untrustedIntermediateIssuersList,
            output_s2opcConfig->serverConfig.issuedCertificatesList,
            output_s2opcConfig->serverConfig.certificateRevocationPathList, &output_s2opcConfig->serverConfig.pki);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Failed loading certificates and key (check paths are valid).\n");
    }
#endif

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
    pEpConfig->endpointURL = ENDPOINT_URL;

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
        pEpConfig->secuConfigurations[1].userTokenPolicies[1] = SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy;
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
        pEpConfig->secuConfigurations[2].userTokenPolicies[1] = SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy;
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

#ifdef PUBSUB_STATIC_CONFIG

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

#else

// dynamic address space.
static SOPC_ReturnStatus Server_SetAddressSpace(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    FILE* fd = fopen(ADDRESS_SPACE_PATH, "r");

    /* Load address space from XML file */
    if (NULL == fd)
    {
        printf("# Error: Cannot open " xstr(ADDRESS_SPACE_PATH) ": %s.\n", strerror(errno));
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_UANodeSet_Parse(fd);

        if (NULL == address_space)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Loaded address space from " xstr(ADDRESS_SPACE_PATH) ".\n");
    }
    else
    {
        printf("# Error: Cannot parse XML address space " xstr(ADDRESS_SPACE_PATH) ".\n");
        SOPC_AddressSpace_Delete(address_space);
    }

    if (fd != NULL)
    {
        fclose(fd);
    }
    return status;
}
#endif

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

    /* TODO: Integrate runtime variables */
    /* if (SOPC_STATUS_OK == status)
     * {
     *     RuntimeVariables runtime_vars =
     *         build_runtime_variables(build_info, PRODUCT_URI, app_namespace_uris, "Systerel");
     *
     *     if (!set_runtime_variables(epConfigIdx, runtime_vars))
     *     {
     *         printf("<Test_Server_Toolkit: Failed to populate Server object");
     *         status = SOPC_STATUS_NOK;
     *     }
     * }
     */

    return status;
}

bool Server_IsRunning(void)
{
    return SOPC_Atomic_Int_Get(&serverOnline);
}

bool Server_PubSubStop_Requested(void)
{
    bool requested = SOPC_Atomic_Int_Get(&pubSubStopRequested);
    if (requested)
    {
        // Reset since request is transmitted on return
        SOPC_Atomic_Int_Set(&pubSubStopRequested, false);
    }
    return requested;
}

bool Server_PubSubStart_Requested(void)
{
    bool requested = SOPC_Atomic_Int_Get(&pubSubStartRequested);
    if (requested)
    {
        // Reset since request is transmitted on return
        SOPC_Atomic_Int_Set(&pubSubStartRequested, false);
    }
    return requested;
}

SOPC_ReturnStatus Server_WritePubSubNodes(void)
{
    SOPC_NodeId* nidConfig = SOPC_NodeId_FromCString(NODEID_PUBSUB_CONFIG, strlen(NODEID_PUBSUB_CONFIG));
    SOPC_NodeId* nidCommand = SOPC_NodeId_FromCString(NODEID_PUBSUB_COMMAND, strlen(NODEID_PUBSUB_COMMAND));
    SOPC_DataValue* dvConfig = SOPC_Calloc(1, sizeof(SOPC_DataValue));
    SOPC_DataValue* dvCommand = SOPC_Calloc(1, sizeof(SOPC_DataValue));
    if (NULL == nidConfig || NULL == nidCommand || NULL == dvConfig || NULL == dvCommand)
    {
        SOPC_Free(dvConfig);
        SOPC_Free(dvCommand);
        SOPC_NodeId_Clear(nidConfig);
        SOPC_Free(nidConfig);
        SOPC_NodeId_Clear(nidCommand);
        SOPC_Free(nidCommand);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Prepare values */
    SOPC_DataValue_Initialize(dvConfig);
    SOPC_DataValue_Initialize(dvCommand);
    SOPC_Variant* vConfig = &dvConfig->Value;
    SOPC_Variant* vCommand = &dvCommand->Value;

    vConfig->BuiltInTypeId = SOPC_String_Id;
    vConfig->ArrayType = SOPC_VariantArrayType_SingleValue;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
#ifndef PUBSUB_STATIC_CONFIG
    // Load XML file
    FILE* fd = fopen(PUBSUB_CONFIG_PATH, "r");
    if (NULL == fd)
    {
        printf("# Error: could not open configuration (file: %s).\n", PUBSUB_CONFIG_PATH);
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        fseek(fd, 0, SEEK_END);
        long fileSize = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        if (fileSize < 0)
        {
            printf("# Error: invalid configuration file size (file: %s)\n", PUBSUB_CONFIG_PATH);
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            char* Xml_config = SOPC_Calloc((size_t) fileSize + 1, sizeof(char));
            if (NULL == Xml_config)
            {
                printf("# Error: while allocating memory for xml configuration\n");
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == status)
            {
                size_t size = fread(Xml_config, sizeof(char), (size_t) fileSize, fd);
                if (size <= 0)
                {
                    printf("# Error: while reading xml configuration (file: %s)\n", PUBSUB_CONFIG_PATH);
                    status = SOPC_STATUS_INVALID_STATE;
                }
                else
                {
                    Xml_config[size] = '\0';
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_InitializeFromCString(&vConfig->Value.String, Xml_config);
            }
            SOPC_Free(Xml_config);
        }
    }

    if (NULL != fd)
    {
        int closed = fclose(fd);
        assert(closed == 0);
    }
#else
    /* configuration is static. No value for the XML file */
    SOPC_String_Initialize(&vConfig->Value.String);
    status = SOPC_STATUS_OK;
#endif

    vCommand->BuiltInTypeId = SOPC_Byte_Id;
    vCommand->ArrayType = SOPC_VariantArrayType_SingleValue;
    vCommand->Value.Byte = 1; /* Start */

    /* Build the WriteRequest and send it */
    if (SOPC_STATUS_OK == status)
    {
        /* Config must be set before the command is issued */
        SOPC_NodeId* lpNid[2] = {nidConfig, nidCommand};
        uint32_t lAttrId[2] = {13, 13};
        SOPC_DataValue* lpDv[2] = {dvConfig, dvCommand};

        status = Helpers_AsyncLocalWrite(epConfigIdx, lpNid, lAttrId, lpDv, 2, Server_Event_Write);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not write to local nodes.\n");
    }

    /* Clean */
    SOPC_NodeId_Clear(nidConfig);
    SOPC_Free(nidConfig);
    SOPC_NodeId_Clear(nidCommand);
    SOPC_Free(nidCommand);
    SOPC_DataValue_Clear(dvConfig);
    SOPC_Free(dvConfig);
    SOPC_DataValue_Clear(dvCommand);
    SOPC_Free(dvCommand);

    return status;
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
    SOPC_UNUSED_ARG(callCtxPtr);
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
    SOPC_UNUSED_ARG(idOrStatus);
    SOPC_EncodeableType* message_type = NULL;
    OpcUa_WriteResponse* writeResponse = NULL;
    OpcUa_ReadResponse* response = NULL;
    SOPC_ReturnStatus statusCopy = SOPC_STATUS_NOK;
    SOPC_PubSheduler_GetVariableRequestContext* ctx = NULL;

    switch (event)
    {
    case SE_CLOSED_ENDPOINT:
        printf("# Info: Closed endpoint event.\n");
        SOPC_Atomic_Int_Set(&serverOnline, 0);
        return;
    case SE_LOCAL_SERVICE_RESPONSE:
        message_type = *((SOPC_EncodeableType**) param);

        /* Listen for ReadResponses, used in GetSourceVariables */
        ctx = (SOPC_PubSheduler_GetVariableRequestContext*) appContext;
        if (message_type == &OpcUa_ReadResponse_EncodeableType && NULL != ctx)
        {
            Mutex_Lock(&ctx->mut);

            statusCopy = SOPC_STATUS_OK;
            response = (OpcUa_ReadResponse*) param;

            if (NULL != response) // Response if deleted by scheduler !!!
            {
                // Allocate data values
                ctx->ldv = SOPC_Calloc((size_t) ctx->NoOfNodesToRead, sizeof(SOPC_DataValue));

                // Copy to response
                if (NULL != ctx->ldv)
                {
                    for (size_t i = 0; i < (size_t) ctx->NoOfNodesToRead && SOPC_STATUS_OK == statusCopy; ++i)
                    {
                        statusCopy = SOPC_DataValue_Copy(&ctx->ldv[i], &response->Results[i]);
                    }

                    // Error, free allocated data values
                    if (SOPC_STATUS_OK != statusCopy)
                    {
                        for (size_t i = 0; i < (size_t) ctx->NoOfNodesToRead; ++i)
                        {
                            SOPC_DataValue_Clear(&ctx->ldv[i]);
                        }
                        SOPC_Free(ctx->ldv);
                        ctx->ldv = NULL;
                    }
                }
            }

            Condition_SignalAll(&ctx->cond);

            Mutex_Unlock(&ctx->mut);
        }
        else if (message_type == &OpcUa_WriteResponse_EncodeableType)
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

    /* It's easier to create the NodeIds once and for all than converting the event's NodeId to a string each time */
    static SOPC_NodeId* nidConfig = NULL;
    static SOPC_NodeId* nidCommand = NULL;
    if (NULL == nidConfig || NULL == nidCommand)
    {
        nidConfig = SOPC_NodeId_FromCString(NODEID_PUBSUB_CONFIG, strlen(NODEID_PUBSUB_CONFIG));
        nidCommand = SOPC_NodeId_FromCString(NODEID_PUBSUB_COMMAND, strlen(NODEID_PUBSUB_COMMAND));
    }
    assert(NULL != nidConfig && NULL != nidCommand);

    /* If config changes, store the new configuration path in global cache */
    int32_t cmpConfig = -1;
    SOPC_ReturnStatus status = SOPC_NodeId_Compare(nidConfig, &pwv->NodeId, &cmpConfig);
    assert(SOPC_STATUS_OK == status);
    SOPC_NodeId_Clear(nidConfig);
    SOPC_Free(nidConfig);
    nidConfig = NULL;

    /* If command changes, start, stop, or restart the PubSub module */
    int32_t cmpCommand = -1;
    status = SOPC_NodeId_Compare(nidCommand, &pwv->NodeId, &cmpCommand);
    assert(SOPC_STATUS_OK == status);
    SOPC_NodeId_Clear(nidCommand);
    SOPC_Free(nidCommand);
    nidCommand = NULL;

    if (0 == cmpConfig)
    {
        if (NULL != lastPubSubConfigPath)
        {
            SOPC_Free(lastPubSubConfigPath);
        }

        /* Its status code must be good, its type String, and be a single value */
        /* TODO: be compatible with multiple values */
        SOPC_DataValue* dv = &pwv->Value;
        if ((dv->Status & SOPC_GoodStatusOppositeMask) != 0)
        {
            printf("# Warning: Status Code not Good, ignoring Configuration path.\n");
            return;
        }

        SOPC_Variant* variant = &dv->Value;
        if (variant->BuiltInTypeId != SOPC_String_Id)
        {
            printf("# Warning: Configuration path value is of invalid type. Expected String, actual type id is %d.\n",
                   variant->BuiltInTypeId);
            return;
        }
        if (variant->ArrayType != SOPC_VariantArrayType_SingleValue)
        {
            printf("# Warning: Configuration path must be a single value, not an array nor a matrix.\n");
            return;
        }

        /* Actual command processing */
        lastPubSubConfigPath = SOPC_String_GetCString(&variant->Value.String);
    }

    if (0 == cmpCommand)
    {
        /* Its status code must be good, its type Byte, and be a single value */
        SOPC_DataValue* dv = &pwv->Value;
        if ((dv->Status & SOPC_GoodStatusOppositeMask) != 0)
        {
            printf("# Warning: Status Code not Good, ignoring Start/Stop Command.\n");
            return;
        }

        SOPC_Variant* variant = &dv->Value;
        if (variant->BuiltInTypeId != SOPC_Byte_Id)
        {
            printf("# Warning: Start/Stop Command value is of invalid type. Expected Byte, actual type id is %d.\n",
                   variant->BuiltInTypeId);
            return;
        }
        if (variant->ArrayType != SOPC_VariantArrayType_SingleValue)
        {
            printf("# Warning: Start/Stop Command must be a single value, not an array nor a matrix.\n");
            return;
        }

        /* Actual command processing */
        uint8_t command = variant->Value.Byte;
        if (lastPubSubCommand == 1)
        {
            SOPC_Atomic_Int_Set(&pubSubStopRequested, true);
            SOPC_Atomic_Int_Set(&pubSubStartRequested, false);
        }
        if (command == 1)
        {
            SOPC_Atomic_Int_Set(&pubSubStartRequested, true);
        }
        lastPubSubCommand = command;
    }
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

void Server_SetSubStatus(SOPC_PubSubState state)
{
    if (!Server_IsRunning())
    {
        return;
    }

    /* Create a WriteRequest with a single WriteValue */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    assert(SOPC_STATUS_OK == status);
    OpcUa_WriteValue* wv = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));

    /* Avoid the creation of the NodeId each call of the function */
    static SOPC_NodeId* nidStatus = NULL;
    if (NULL == nidStatus)
    {
        nidStatus = SOPC_NodeId_FromCString(NODEID_PUBSUB_STATUS, strlen(NODEID_PUBSUB_STATUS));
    }

    if (NULL == request || NULL == wv || NULL == nidStatus)
    {
        SOPC_Free(wv);
        SOPC_Free(nidStatus);
        return;
    }

    SOPC_DataValue* dv = &wv->Value;
    SOPC_Variant* val = &dv->Value;

    request->NoOfNodesToWrite = 1;
    request->NodesToWrite = wv;

    wv->AttributeId = 13;
    dv->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
    val->BuiltInTypeId = SOPC_Byte_Id;
    val->ArrayType = SOPC_VariantArrayType_SingleValue;
    val->Value.Byte = (SOPC_Byte) state;

    status = SOPC_NodeId_Copy(&wv->NodeId, nidStatus);
    SOPC_NodeId_Clear(nidStatus);
    SOPC_Free(nidStatus);
    nidStatus = NULL;

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, request, 0);
    }
    else
    {
        SOPC_Free(wv);
        wv = NULL;
        SOPC_Free(request);
        request = NULL;
    }
}

bool Server_SetTargetVariables(OpcUa_WriteValue* lwv, int32_t nbValues)
{
    if (!Server_IsRunning())
    {
        return true;
    }

    /* Encapsulate the WriteValues in a WriteRequest and send it as a local service,
     * acknowledge before the toolkit answers */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (NULL == request)
    {
        return false;
    }

    request->NoOfNodesToWrite = nbValues;
    request->NodesToWrite = lwv;
    SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, request, 0);

    return true;
}

SOPC_DataValue* Server_GetSourceVariables(OpcUa_ReadValueId* lrv, int32_t nbValues)
{
    if (!Server_IsRunning())
    {
        return NULL;
    }

    if (NULL == lrv || 0 >= nbValues)
    {
        return NULL;
    }

    OpcUa_ReadRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        SOPC_Calloc(1, sizeof(SOPC_PubSheduler_GetVariableRequestContext));

    if (NULL == request || NULL == requestContext)
    {
        SOPC_UNUSED_RESULT(SOPC_Encodeable_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &request));
        SOPC_Free(requestContext);

        return NULL;
    }

    requestContext->ldv = NULL;                 // Datavalue request result
    requestContext->NoOfNodesToRead = nbValues; // Use to alloc SOPC_DataValue by GetResponse
    Condition_Init(&requestContext->cond);
    Mutex_Initialization(&requestContext->mut);

    /* Encapsulate the ReadValues in a ReadRequest, awaits the Response */
    request->MaxAge = 0.;
    request->TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
    request->NoOfNodesToRead = nbValues;
    request->NodesToRead = lrv;

    Mutex_Lock(&requestContext->mut);

    SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, request, (uintptr_t) requestContext);

    Mutex_UnlockAndWaitCond(&requestContext->cond, &requestContext->mut);

    Mutex_Unlock(&requestContext->mut);

    if (NULL == requestContext->ldv)
    {
        Mutex_Clear(&requestContext->mut);
        Condition_Clear(&requestContext->cond);
        SOPC_Free(requestContext);
        return NULL;
    }
    SOPC_DataValue* ldv = NULL;

    ldv = requestContext->ldv;

    Mutex_Clear(&requestContext->mut);
    Condition_Clear(&requestContext->cond);
    SOPC_Free(requestContext);

    return ldv;
}
