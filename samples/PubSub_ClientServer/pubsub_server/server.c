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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "embedded/sopc_addspace_loader.h"
#include "sopc_address_space.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
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
#include "server_static_security_data.h"
#endif

#include "client.h"
#include "config.h"
#include "helpers.h"
#include "server.h"

/* These variables could be stored in a struct Server_Context, which is then passed to all functions.
 * This would mimic class instances and avoid global variables.
 */
static uint32_t epConfigIdx = 0;
int32_t serverOnline = 0;
static int32_t pubSubStopRequested = false;
static int32_t pubSubStartRequested = false;
static int32_t pubAcyclicSendRequest = false;
static int32_t pubAcyclicSendWriterId = 0;

static SOPC_AddressSpace* address_space = NULL;
static uint8_t lastPubSubCommand = 0;
static char* lastPubSubConfigPath = NULL;

static char* default_trusted_certs[] = {CA_CERT_PATH, NULL};
static char* default_revoked_certs[] = {CA_CRL_PATH, NULL};
static char* empty_certs[] = {NULL};

typedef enum PublisherSendStatus
{
    PUBLISHER_ACYCLIC_NOT_TRIGGERED = 0,
    PUBLISHER_ACYCLIC_IN_PROGRESS = 1,
    PUBLISHER_ACYCLIC_SENT = 2,
    PUBLISHER_ACYCLIC_ERROR = 3,
} PublisherSendStatus;

static SOPC_ReturnStatus Server_SetAddressSpace(void);

static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      SOPC_App_AddSpace_Event event,
                                      void* opParam,
                                      SOPC_StatusCode opStatus);
static void Server_Event_Write(OpcUa_WriteValue* pwv);
static void Server_request_change_sendAcyclicStatus(PublisherSendStatus state);

/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */

#ifndef WITH_STATIC_SECURITY_DATA
static bool SOPC_TestHelper_AskPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environement or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
    */
    char* _outPassword = getenv(PASSWORD_ENV_NAME);
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    if (NULL == *outPassword)
    {
        printf("INFO: %s environment variable not set or empty, use terminal interactive input:\n", PASSWORD_ENV_NAME);
        return SOPC_AskPass_CustomPromptFromTerminal("Server private key password:\n", outPassword);
    }
    return true;
}
#endif

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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed loading certificates and key (check paths are valid)");
    }
#else

    status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(output_s2opcConfig->serverConfig.serverCertPath,
                                                                  &output_s2opcConfig->serverConfig.serverCertificate);

    // Retrieve the password
    char* password = NULL;
    size_t lenPassword = 0;
    bool res = false;

    if (SOPC_STATUS_OK == status && ENCRYPTED_SERVER_KEY)
    {
        res = SOPC_TestHelper_AskPass_FromEnv(&password);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        lenPassword = strlen(password);
        if (UINT32_MAX < lenPassword)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(
            output_s2opcConfig->serverConfig.serverKeyPath, &output_s2opcConfig->serverConfig.serverKey, password,
            (uint32_t) lenPassword);
    }

    if (NULL != password)
    {
        SOPC_Free(password);
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed loading certificates and key (check paths are valid)");
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
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Failed to create user authentication and authorization managers");
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot load static address space");
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot open " xstr(ADDRESS_SPACE_PATH) ": %s", strerror(errno));
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
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Loaded address space from " xstr(ADDRESS_SPACE_PATH));
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot parse XML address space " xstr(ADDRESS_SPACE_PATH));
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
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to set the address space configuration");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(&Server_Event_AddressSpace);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Failed to configure the address space notification callback");
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the endpoint");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Endpoint and Toolkit configured");
    }

    /* Starts the server */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
        SOPC_Atomic_Int_Set(&serverOnline, true);
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server started");
    }

    /* TODO: Integrate runtime variables */

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

int32_t Server_PubAcyclicSend_Requested(void)
{
    int32_t acyclicSendReq = SOPC_Atomic_Int_Get(&pubAcyclicSendRequest);
    int32_t writerGroupId = 0;
    if (acyclicSendReq)
    {
        writerGroupId = SOPC_Atomic_Int_Get(&pubAcyclicSendWriterId);
        if (0 == writerGroupId)
        {
            Server_request_change_sendAcyclicStatus(PUBLISHER_ACYCLIC_ERROR);
            printf("# Warning : writerGroupId cannot be equal to 0\n");
        }
        /* Reset since request is transmitted on return */
        SOPC_Atomic_Int_Set(&pubAcyclicSendRequest, 0);
    }
    return writerGroupId;
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "could not open configuration (file: %s)", PUBSUB_CONFIG_PATH);
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        long fileSize = -1L;
        int res = fseek(fd, 0, SEEK_END);
        if (0 != res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "invalid configuration file size (file: %s)",
                                   PUBSUB_CONFIG_PATH);
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            fileSize = ftell(fd);
            if (fileSize < 0)
            {
                printf("# Error: invalid configuration file size (file: %s)\n", PUBSUB_CONFIG_PATH);
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            res = fseek(fd, 0, SEEK_SET);
            if (0 != res)
            {
                printf("# Error: could not seek from beginning of file (file: %s)\n", PUBSUB_CONFIG_PATH);
                status = SOPC_STATUS_INVALID_STATE;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            char* Xml_config = SOPC_Calloc((size_t) fileSize + 1, sizeof(char));
            if (NULL == Xml_config)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "while allocating memory for xml configuration");
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == status)
            {
                size_t size = fread(Xml_config, sizeof(char), (size_t) fileSize, fd);
                if (size <= 0)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "while reading xml configuration (file: %s)",
                                           PUBSUB_CONFIG_PATH);
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
        SOPC_ASSERT(closed == 0);
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Could not write to local nodes");
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

bool Server_Trigger_Publisher(uint16_t writerGroupId)
{
    if (!Server_IsRunning())
    {
        return false;
    }

    bool res = SOPC_PubScheduler_AcyclicSend(writerGroupId);
    if (res)
    {
        Server_request_change_sendAcyclicStatus(PUBLISHER_ACYCLIC_SENT);
    }
    else
    {
        Server_request_change_sendAcyclicStatus(PUBLISHER_ACYCLIC_ERROR);
    }
    return res;
}

void Server_StopAndClear(SOPC_S2OPC_Config* pConfig)
{
    /* SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx); */
    SOPC_Toolkit_Clear();

    if (Server_IsRunning())
    {
        SOPC_Atomic_Int_Set(&serverOnline, false);
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
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                 "Received unexpected event in address space notification handler: %d", event);
        return;
    }

    if ((opStatus & SOPC_GoodStatusOppositeMask) != 0)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                 "Received address space event which status code is not good: 0x%08X. Ignored",
                                 (unsigned int) opStatus);
        return;
    }

    Server_Event_Write((OpcUa_WriteValue*) opParam);
}

static void Server_request_change_sendAcyclicStatus(PublisherSendStatus state)
{
    /* Create a WriteRequest with a single WriteValue */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    OpcUa_WriteValue* wv = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));

    /* Avoid the creation of the NodeId each call of the function */
    static SOPC_NodeId* nidSendStatus = NULL;
    if (NULL == nidSendStatus)
    {
        nidSendStatus = SOPC_NodeId_FromCString(NODEID_ACYCLICPUB_SEND_STATUS, strlen(NODEID_ACYCLICPUB_SEND_STATUS));
    }

    if (NULL == request || NULL == wv || NULL == nidSendStatus)
    {
        SOPC_Free(wv);
        SOPC_Free(nidSendStatus);
    }
    else
    {
        SOPC_DataValue* dv = &wv->Value;
        SOPC_Variant* val = &dv->Value;

        request->NoOfNodesToWrite = 1;
        request->NodesToWrite = wv;

        wv->AttributeId = 13;
        dv->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        val->BuiltInTypeId = SOPC_Byte_Id;
        val->ArrayType = SOPC_VariantArrayType_SingleValue;
        val->Value.Byte = (SOPC_Byte) state;

        status = SOPC_NodeId_Copy(&wv->NodeId, nidSendStatus);
        SOPC_NodeId_Clear(nidSendStatus);
        SOPC_Free(nidSendStatus);
        nidSendStatus = NULL;

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
}

static void Server_Event_Write(OpcUa_WriteValue* pwv)
{
    if (NULL == pwv)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                 "NULL pointer passed instead of WriteValue to address space notification callback");
        return;
    }

    /* IndexRange should be empty (entire Value) */
    if (0 < pwv->IndexRange.Length)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "Address space notification with non empty IndexRange: %s",
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
    static SOPC_NodeId* nidSend = NULL;
    if (NULL == nidConfig || NULL == nidCommand || NULL == nidSend)
    {
        nidConfig = SOPC_NodeId_FromCString(NODEID_PUBSUB_CONFIG, strlen(NODEID_PUBSUB_CONFIG));
        nidCommand = SOPC_NodeId_FromCString(NODEID_PUBSUB_COMMAND, strlen(NODEID_PUBSUB_COMMAND));
        nidSend = SOPC_NodeId_FromCString(NODEID_ACYCLICPUB_SEND, strlen(NODEID_ACYCLICPUB_SEND));
    }
    SOPC_ASSERT(NULL != nidConfig && NULL != nidCommand && NULL != nidSend);

    /* If config changes, store the new configuration path in global cache */
    int32_t cmpConfig = -1;
    SOPC_ReturnStatus status = SOPC_NodeId_Compare(nidConfig, &pwv->NodeId, &cmpConfig);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_NodeId_Clear(nidConfig);
    SOPC_Free(nidConfig);
    nidConfig = NULL;

    /* If command changes, start, stop, or restart the PubSub module */
    int32_t cmpCommand = -1;
    status = SOPC_NodeId_Compare(nidCommand, &pwv->NodeId, &cmpCommand);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_NodeId_Clear(nidCommand);
    SOPC_Free(nidCommand);
    nidCommand = NULL;

    /* If send changes, Send every writer group with new values stored in address space */
    int32_t cmpSend = -1;
    status = SOPC_NodeId_Compare(nidSend, &pwv->NodeId, &cmpSend);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_NodeId_Clear(nidSend);
    SOPC_Free(nidSend);
    nidSend = NULL;

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
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "Status Code not Good, ignoring Configuration path");
            return;
        }

        SOPC_Variant* variant = &dv->Value;
        if (variant->BuiltInTypeId != SOPC_String_Id)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_PUBSUB,
                "Configuration path value is of invalid type. Expected String, actual type id is %d",
                variant->BuiltInTypeId);
            return;
        }
        if (variant->ArrayType != SOPC_VariantArrayType_SingleValue)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                     "Configuration path must be a single value, not an array nor a matrix");
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
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "Status Code not Good, ignoring Start/Stop Command");
            return;
        }

        SOPC_Variant* variant = &dv->Value;
        if (variant->BuiltInTypeId != SOPC_Byte_Id)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                     "Start/Stop Command value is of invalid type. Expected Byte, actual type id is %d",
                                     variant->BuiltInTypeId);
            return;
        }
        if (variant->ArrayType != SOPC_VariantArrayType_SingleValue)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                     "Start/Stop Command must be a single value, not an array nor a matrix");
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

    if (0 == cmpSend)
    {
        /* Its status code must be good, its type uint16, and be a single value */
        SOPC_DataValue* dv = &pwv->Value;
        if ((dv->Status & SOPC_GoodStatusOppositeMask) != 0)
        {
            printf("# Warning: Status Code not Good, ignoring Send command.\n");
            return;
        }

        SOPC_Variant* variant = &dv->Value;
        if (variant->BuiltInTypeId != SOPC_UInt16_Id)
        {
            printf("# Warning: Send Command value is of invalid type. Expected UInt16 type, actual type id is %d.\n",
                   variant->BuiltInTypeId);
            return;
        }
        if (variant->ArrayType != SOPC_VariantArrayType_SingleValue)
        {
            printf("# Warning: Send Command must be a single value, not an array nor a matrix.\n");
            return;
        }

        /* Command processing */
        uint16_t command = variant->Value.Uint16;
        SOPC_Atomic_Int_Set(&pubAcyclicSendRequest, (int32_t) true);
        SOPC_Atomic_Int_Set(&pubAcyclicSendWriterId, (int32_t) command);
        Server_request_change_sendAcyclicStatus(PUBLISHER_ACYCLIC_IN_PROGRESS);
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
    SOPC_ASSERT(SOPC_STATUS_OK == status);
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
    SOPC_Condition_Init(&requestContext->cond);
    SOPC_Mutex_Initialization(&requestContext->mut);

    /* Encapsulate the ReadValues in a ReadRequest, awaits the Response */
    request->MaxAge = 0.;
    request->TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
    request->NoOfNodesToRead = nbValues;
    request->NodesToRead = lrv;

    SOPC_Mutex_Lock(&requestContext->mut);

    SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, request, (uintptr_t) requestContext);

    SOPC_Mutex_UnlockAndWaitCond(&requestContext->cond, &requestContext->mut);

    SOPC_Mutex_Unlock(&requestContext->mut);

    if (NULL == requestContext->ldv)
    {
        SOPC_Mutex_Clear(&requestContext->mut);
        SOPC_Condition_Clear(&requestContext->cond);
        SOPC_Free(requestContext);
        return NULL;
    }
    SOPC_DataValue* ldv = NULL;

    ldv = requestContext->ldv;

    SOPC_Mutex_Clear(&requestContext->mut);
    SOPC_Condition_Clear(&requestContext->cond);
    SOPC_Free(requestContext);

    return ldv;
}

void Server_Treat_Local_Service_Response(void* param, uintptr_t appContext)
{
    SOPC_EncodeableType* message_type = *((SOPC_EncodeableType**) param);
    OpcUa_WriteResponse* writeResponse = NULL;
    OpcUa_ReadResponse* response = NULL;
    SOPC_ReturnStatus statusCopy = SOPC_STATUS_NOK;

    /* Listen for WriteResponses, which only contain status codes */

    SOPC_PubSheduler_GetVariableRequestContext* ctx = (SOPC_PubSheduler_GetVariableRequestContext*) appContext;
    if (message_type == &OpcUa_ReadResponse_EncodeableType && NULL != ctx)
    {
        SOPC_Mutex_Lock(&ctx->mut);

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

        SOPC_Condition_SignalAll(&ctx->cond);

        SOPC_Mutex_Unlock(&ctx->mut);
    }
    else if (message_type == &OpcUa_WriteResponse_EncodeableType)
    {
        writeResponse = param;
        // Service should have succeeded
        SOPC_ASSERT(0 == (SOPC_GoodStatusOppositeMask & writeResponse->ResponseHeader.ServiceResult));
    }
    else
    {
        SOPC_ASSERT(false);
    }
}
