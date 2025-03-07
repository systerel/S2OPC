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
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_pub_scheduler.h"
#include "sopc_threads.h"
#include "xml_expat/sopc_uanodeset_loader.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

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
int32_t serverOnline = 0;
static int32_t pubSubStopRequested = false;
static int32_t pubSubStartRequested = false;
static int32_t pubAcyclicSendRequest = false;
static int32_t pubFilteringDsmEmissionRequest = false;

static struct networkMessageIdentifier pubAcyclicSendNetworkMessageId = {
    .pubId = {.type = SOPC_UInteger_PublisherId, .data.uint = 0},
    .writerGroupId = 0};

static struct publisherDsmIdentifier pubFilteringDsmId = {.pubId = {.type = SOPC_UInteger_PublisherId, .data.uint = 0},
                                                          .writerGroupId = 0,
                                                          .dataSetWriterId = 0,
                                                          .enableEmission = false};

static SOPC_AddressSpace* address_space = NULL;
static uint8_t lastPubSubCommand = 0;
static char* lastPubSubConfigPath = NULL;

typedef enum PublisherMethodStatus
{
    PUBLISHER_METHOD_NOT_TRIGGERED = 0,
    PUBLISHER_METHOD_IN_PROGRESS = 1,
    PUBLISHER_METHOD_SUCCESS = 2,
    PUBLISHER_METHOD_ERROR = 3,
} PublisherMethodStatus;

static SOPC_ReturnStatus Server_SetAddressSpace(void);

static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      OpcUa_WriteValue* writeValue,
                                      SOPC_StatusCode opStatus);
static void Server_Event_Write(OpcUa_WriteValue* pwv);
static void Server_request_change_sendAcyclicStatus(PublisherMethodStatus state);
static void Server_request_change_DsmFilteringStatus(PublisherMethodStatus state);

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

static SOPC_StatusCode Server_Method_Func_AcyclicSend(const SOPC_CallContext* callContextPtr,
                                                      const SOPC_NodeId* objectId,
                                                      uint32_t nbInputArgs,
                                                      const SOPC_Variant* inputArgs,
                                                      uint32_t* nbOutputArgs,
                                                      SOPC_Variant** outputArgs,
                                                      void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(objectId);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != inputArgs);

    // We expect 2 parameters publisher Id and writer group id
    if (2 != nbInputArgs || SOPC_UInt64_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType || SOPC_UInt16_Id != inputArgs[1].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    static SOPC_NodeId* nidSendStatus = NULL;
    if (NULL == nidSendStatus)
    {
        nidSendStatus = SOPC_NodeId_FromCString(NODEID_ACYCLICPUB_SEND_STATUS);
    }

    SOPC_ASSERT(NULL != nidSendStatus);

    // Check that acyclic send status is not in progress
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode code = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, nidSendStatus, NULL, &dv);
    if (!SOPC_IsGoodStatus(code) || SOPC_Int32_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType ||
        dv->Value.Value.Int32 == PUBLISHER_METHOD_IN_PROGRESS)
    {
        code = OpcUa_BadInvalidState;
    }
    else
    {
        // Update acyclic send status
        dv->Value.Value.Int32 = PUBLISHER_METHOD_IN_PROGRESS;
        SOPC_DateTime ts = 0;
        code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, nidSendStatus, NULL, &dv->Value, NULL, &ts, NULL);

        // If Status is different to IN PROGRESS request flags shall false
        int32_t acyclicSendRequested = SOPC_Atomic_Int_Get(&pubAcyclicSendRequest);
        // If a request is already in progress don't access shared memory
        if (SOPC_IsGoodStatus(code) && !acyclicSendRequested)
        {
            /* Command processing */
            uint64_t publisherId = inputArgs[0].Value.Uint64;
            uint16_t writerGroupId = inputArgs[1].Value.Uint16;
            pubAcyclicSendNetworkMessageId.pubId.data.uint = publisherId;
            pubAcyclicSendNetworkMessageId.writerGroupId = writerGroupId;
            SOPC_Atomic_Int_Set(&pubAcyclicSendRequest, (int32_t) true);
        }
    }
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    SOPC_NodeId_Clear(nidSendStatus);
    SOPC_Free(nidSendStatus);
    nidSendStatus = NULL;
    return code;
}

static SOPC_StatusCode Server_Method_Func_DataSetMessageFiltering(const SOPC_CallContext* callContextPtr,
                                                                  const SOPC_NodeId* objectId,
                                                                  uint32_t nbInputArgs,
                                                                  const SOPC_Variant* inputArgs,
                                                                  uint32_t* nbOutputArgs,
                                                                  SOPC_Variant** outputArgs,
                                                                  void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(objectId);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != inputArgs);

    // We expect 4 parameters Publisher Id, WriterGroup id and DataSetMessage Id, enableEmission
    if (4 != nbInputArgs || SOPC_UInt64_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType || SOPC_UInt16_Id != inputArgs[1].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType || SOPC_UInt16_Id != inputArgs[2].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[2].ArrayType || SOPC_Boolean_Id != inputArgs[3].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[3].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    static SOPC_NodeId* nidDataSetMessageFilteringStatus = NULL;
    if (NULL == nidDataSetMessageFilteringStatus)
    {
        nidDataSetMessageFilteringStatus = SOPC_NodeId_FromCString(NODEID_DSM_FILTERING_STATUS);
    }

    SOPC_ASSERT(NULL != nidDataSetMessageFilteringStatus);

    // Check that DatasetMessage filtering status is not in progress
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode code = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, nidDataSetMessageFilteringStatus, NULL, &dv);
    if (!SOPC_IsGoodStatus(code) || SOPC_Int32_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType ||
        dv->Value.Value.Int32 == PUBLISHER_METHOD_IN_PROGRESS)
    {
        code = OpcUa_BadInvalidState;
    }
    else
    {
        // Update acyclic send status
        dv->Value.Value.Int32 = PUBLISHER_METHOD_IN_PROGRESS;
        SOPC_DateTime ts = 0;
        code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, nidDataSetMessageFilteringStatus, NULL, &dv->Value, NULL,
                                                  &ts, NULL);

        // If Status is different to IN PROGRESS request flags shall false
        int32_t filteringRequested = SOPC_Atomic_Int_Get(&pubFilteringDsmEmissionRequest);
        // If a request is already in progress don't access shared memory
        if (SOPC_IsGoodStatus(code) && !filteringRequested)
        {
            /* Command processing */
            uint64_t PubId = inputArgs[0].Value.Uint64;
            uint16_t WgId = inputArgs[1].Value.Uint16;
            uint16_t dsmId = inputArgs[2].Value.Uint16;
            bool enableEmission = inputArgs[3].Value.Boolean;
            pubFilteringDsmId.pubId.data.uint = PubId;
            pubFilteringDsmId.writerGroupId = WgId;
            pubFilteringDsmId.dataSetWriterId = dsmId;
            pubFilteringDsmId.enableEmission = enableEmission;
            SOPC_Atomic_Int_Set(&pubFilteringDsmEmissionRequest, true);
        }
    }
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    SOPC_NodeId_Clear(nidDataSetMessageFilteringStatus);
    SOPC_Free(nidDataSetMessageFilteringStatus);
    nidDataSetMessageFilteringStatus = NULL;
    return code;
}

/**
 * Add OPC UA demo methods into the given method call manager
 */
static SOPC_ReturnStatus Server_AddMethods(SOPC_MethodCallManager* mcm)
{
    if (NULL == mcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* sNodeId;
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr* methodFunc;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Add methods implementation in the method call manager used */
    sNodeId = "ns=1;s=AcyclicSend";
    methodId = SOPC_NodeId_FromCString(sNodeId);
    if (NULL != methodId)
    {
        methodFunc = &Server_Method_Func_AcyclicSend;
        status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, NULL, NULL);
        SOPC_NodeId_Clear(methodId);
        SOPC_Free(methodId);
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "ns=1;s=DataSetMessageFiltering";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &Server_Method_Func_DataSetMessageFiltering;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, NULL, NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

static SOPC_ReturnStatus Server_InitDefaultCallMethodService(void)
{
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        // Note: in case of Nano compliant server this manager will never be used
        // since CallMethod service is not available
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_AddMethods(mcm);
    }

    return status;
}

SOPC_ReturnStatus Server_CreateServerConfig(void)
{
    SOPC_ReturnStatus status = SOPC_ServerConfigHelper_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    /* Load server endpoints configuration
     * use an embedded default demo server configuration.
     */

    SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(ENDPOINT_URL, true);
    SOPC_SecurityPolicy* spNone = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
    SOPC_SecurityPolicy* spSecu = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

    if (NULL == ep || NULL == spNone || NULL == spSecu)
    {
        SOPC_Free(ep);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* 1st Security policy is None with Anonymous user */
    status = SOPC_SecurityConfig_SetSecurityModes(spNone, SOPC_SecurityModeMask_None);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(spNone, &SOPC_UserTokenPolicy_Anonymous);
    }

    /* 2nd Security policy is Basic256Sha256 with anonymous user */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_SetSecurityModes(spSecu, SOPC_SecurityModeMask_SignAndEncrypt);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(spSecu, &SOPC_UserTokenPolicy_Anonymous);
    }

    // Server certificates configuration
    SOPC_PKIProvider* pkiProvider = NULL;
#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_CertificateList* static_cacert = NULL;
    SOPC_CRLList* static_cacrl = NULL;

    /* Load client/server certificates and server key from C source files (no filesystem needed) */
    status = SOPC_ServerConfigHelper_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                             sizeof(server_2k_key), server_2k_key);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &static_cacert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_cacrl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(static_cacert, static_cacrl, NULL, NULL, &pkiProvider);
    }

    /* Clean in all cases */
    SOPC_KeyManager_Certificate_Free(static_cacert);
    SOPC_KeyManager_CRL_Free(static_cacrl);

#else // WITH_STATIC_SECURITY_DATA == false
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the server key user password callback");
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_ServerConfigHelper_SetKeyCertPairFromPath(SERVER_CERT_PATH, SERVER_KEY_PATH, ENCRYPTED_SERVER_KEY);
    }

    // Set PKI configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(PKI_PATH, &pkiProvider);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed loading certificates and key (check paths are valid)");
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set namespaces
        const char* namespaces[] = {APPLICATION_URI};
        status = SOPC_ServerConfigHelper_SetNamespaces(1, namespaces);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed setting namespaces");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(APPLICATION_URI, PRODUCT_URI, SERVER_DESCRIPTION,
                                                                   NULL, OpcUa_ApplicationType_Server);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed setting application description");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(&Server_Treat_Local_Service_Response);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_InitDefaultCallMethodService();
    }

    return status;
}

#ifdef PUBSUB_STATIC_CONFIG

// static address space.
static SOPC_ReturnStatus Server_SetAddressSpace(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    address_space = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    if (NULL == address_space)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cannot load static address space");
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
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
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
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
    SOPC_ReturnStatus status = Server_SetAddressSpace();

    /* Set address space and set its notification callback */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to set the address space configuration");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetWriteNotifCallback(&Server_Event_AddressSpace);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Failed to configure the address space notification callback");
        }
    }

    return status;
}

static void SOPC_ServerStopped_Cbk(SOPC_ReturnStatus status)
{
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server stopped with status %d", status);
    SOPC_Atomic_Int_Set(&serverOnline, false);
}

SOPC_ReturnStatus Server_StartServer(void)
{
    /* Starts the server */
    SOPC_ReturnStatus status = SOPC_ServerHelper_StartServer(&SOPC_ServerStopped_Cbk);
    SOPC_Atomic_Int_Set(&serverOnline, true);
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Server started");

    /* TODO: Integrate runtime variables */
    return status;
}

bool Server_IsRunning(void)
{
    return true == SOPC_Atomic_Int_Get(&serverOnline);
}

void Server_PubSubStop_RequestRestart(void)
{
    SOPC_Atomic_Int_Set(&pubSubStopRequested, true);
    SOPC_Atomic_Int_Set(&pubSubStartRequested, true);
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
    bool requested = SOPC_Atomic_Int_Get(&pubSubStartRequested) && !SOPC_Atomic_Int_Get(&pubSubStopRequested);
    if (requested)
    {
        // Reset since request is transmitted on return
        SOPC_Atomic_Int_Set(&pubSubStartRequested, false);
    }
    return requested;
}

struct networkMessageIdentifier Server_PubAcyclicSend_Requested(void)
{
    int32_t acyclicSendReq = SOPC_Atomic_Int_Get(&pubAcyclicSendRequest);
    struct networkMessageIdentifier nmIdentifier = {.pubId = {.type = SOPC_Null_PublisherId}, .writerGroupId = 0};
    if (acyclicSendReq)
    {
        nmIdentifier = pubAcyclicSendNetworkMessageId;
        if (nmIdentifier.pubId.data.uint == 0 && nmIdentifier.writerGroupId == 0)
        {
            Server_request_change_sendAcyclicStatus(PUBLISHER_METHOD_ERROR);
            printf("# Warning : [publisherId, writerGroupId] cannot be equal to [0, 0]\n");
        }
        /* Reset since request is transmitted on return */
        SOPC_Atomic_Int_Set(&pubAcyclicSendRequest, 0);
    }
    return nmIdentifier;
}

struct publisherDsmIdentifier Server_PubFilteringDataSetMessage_Requested(void)
{
    int32_t enableDsmRequest = SOPC_Atomic_Int_Get(&pubFilteringDsmEmissionRequest);
    struct publisherDsmIdentifier pubDsmId = {
        .pubId = {.type = SOPC_Null_PublisherId}, .writerGroupId = 0, .dataSetWriterId = 0};
    if (enableDsmRequest)
    {
        // Access shared memory protected by request flag
        pubDsmId = pubFilteringDsmId;
        if (pubDsmId.pubId.data.uint == 0 && pubDsmId.writerGroupId == 0 && pubDsmId.dataSetWriterId == 0)
        {
            Server_request_change_DsmFilteringStatus(PUBLISHER_METHOD_ERROR);
            printf("# Warning : [publisherId, writerGroupId, dataSetWriterId] can't be equal to [0, 0, 0]\n");
        }
        /* Reset since reqest is transmitted on return */
        SOPC_Atomic_Int_Set(&pubFilteringDsmEmissionRequest, 0);
    }
    return pubDsmId;
}

SOPC_ReturnStatus Server_WritePubSubNodes(void)
{
    SOPC_NodeId* nidConfig = SOPC_NodeId_FromCString(NODEID_PUBSUB_CONFIG);
    SOPC_NodeId* nidCommand = SOPC_NodeId_FromCString(NODEID_PUBSUB_COMMAND);
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

        status = Helpers_AsyncLocalWrite(lpNid, lAttrId, lpDv, 2, Server_Event_Write);
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

bool Server_Trigger_Publisher(struct networkMessageIdentifier networkMessageId)
{
    if (!Server_IsRunning())
    {
        return false;
    }

    bool res = SOPC_PubScheduler_AcyclicSend(&networkMessageId.pubId, networkMessageId.writerGroupId);
    if (res)
    {
        Server_request_change_sendAcyclicStatus(PUBLISHER_METHOD_SUCCESS);
    }
    else
    {
        Server_request_change_sendAcyclicStatus(PUBLISHER_METHOD_ERROR);
    }
    return res;
}

bool Server_Trigger_FilteringDsmEmission(struct publisherDsmIdentifier pubDsmId)
{
    if (!Server_IsRunning())
    {
        return false;
    }
    bool res = false;
    if (pubDsmId.enableEmission)
    {
        res = SOPC_PubScheduler_Enable_DataSetMessage(&pubDsmId.pubId, pubDsmId.writerGroupId,
                                                      pubDsmId.dataSetWriterId) == SOPC_STATUS_OK;
    }
    else
    {
        res = SOPC_PubScheduler_Disable_DataSetMessage(&pubDsmId.pubId, pubDsmId.writerGroupId,
                                                       pubDsmId.dataSetWriterId) == SOPC_STATUS_OK;
    }
    if (res)
    {
        Server_request_change_DsmFilteringStatus(PUBLISHER_METHOD_SUCCESS);
    }
    else
    {
        Server_request_change_DsmFilteringStatus(PUBLISHER_METHOD_ERROR);
    }
    return res;
}

void Server_StopAndClear(void)
{
    SOPC_UNUSED_RESULT(SOPC_ServerHelper_StopServer());

    uint32_t loopCpt = 0;
    uint32_t loopTimeout = 5000; // 5 seconds
    while (Server_IsRunning() && loopCpt * SLEEP_TIMEOUT <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(SLEEP_TIMEOUT);
    }
    SOPC_ServerConfigHelper_Clear();

    if (NULL != lastPubSubConfigPath)
    {
        SOPC_Free(lastPubSubConfigPath);
        lastPubSubConfigPath = NULL;
    }
}

static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      OpcUa_WriteValue* writeValue,
                                      SOPC_StatusCode opStatus)
{
    SOPC_UNUSED_ARG(callCtxPtr);
    /* Watch modifications of configuration paths and the start/stop command */
    if ((opStatus & SOPC_GoodStatusOppositeMask) != 0)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                 "Received address space event which status code is not good: 0x%08X. Ignored",
                                 (unsigned int) opStatus);
        return;
    }

    Server_Event_Write((OpcUa_WriteValue*) writeValue);
}

static void Server_request_change_sendAcyclicStatus(PublisherMethodStatus state)
{
    /* Create a WriteRequest with a single WriteValue */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    OpcUa_WriteValue* wv = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));

    /* Avoid the creation of the NodeId each call of the function */
    static SOPC_NodeId* nidSendStatus = NULL;
    if (NULL == nidSendStatus)
    {
        nidSendStatus = SOPC_NodeId_FromCString(NODEID_ACYCLICPUB_SEND_STATUS);
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
        val->BuiltInTypeId = SOPC_Int32_Id;
        val->ArrayType = SOPC_VariantArrayType_SingleValue;
        val->Value.Int32 = (int32_t) state;

        status = SOPC_NodeId_Copy(&wv->NodeId, nidSendStatus);
        SOPC_NodeId_Clear(nidSendStatus);
        SOPC_Free(nidSendStatus);
        nidSendStatus = NULL;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerHelper_LocalServiceAsync(request, 0);
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

static void Server_request_change_DsmFilteringStatus(PublisherMethodStatus state)
{
    /* Create a WriteRequest with a single WriteValue */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    OpcUa_WriteValue* wv = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));

    /* Avoid the creation of the NodeId each call of the function */
    static SOPC_NodeId* nidDsmFilteringStatus = NULL;
    if (NULL == nidDsmFilteringStatus)
    {
        nidDsmFilteringStatus = SOPC_NodeId_FromCString(NODEID_DSM_FILTERING_STATUS);
    }

    if (NULL == request || NULL == wv || NULL == nidDsmFilteringStatus)
    {
        SOPC_Free(wv);
        SOPC_Free(nidDsmFilteringStatus);
    }
    else
    {
        SOPC_DataValue* dv = &wv->Value;
        SOPC_Variant* val = &dv->Value;

        request->NoOfNodesToWrite = 1;
        request->NodesToWrite = wv;

        wv->AttributeId = 13;
        dv->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        val->BuiltInTypeId = SOPC_Int32_Id;
        val->ArrayType = SOPC_VariantArrayType_SingleValue;
        val->Value.Int32 = (int32_t) state;

        status = SOPC_NodeId_Copy(&wv->NodeId, nidDsmFilteringStatus);
        SOPC_NodeId_Clear(nidDsmFilteringStatus);
        SOPC_Free(nidDsmFilteringStatus);
        nidDsmFilteringStatus = NULL;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerHelper_LocalServiceAsync(request, 0);
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
    if (NULL == nidConfig || NULL == nidCommand)
    {
        nidConfig = SOPC_NodeId_FromCString(NODEID_PUBSUB_CONFIG);
        nidCommand = SOPC_NodeId_FromCString(NODEID_PUBSUB_COMMAND);
    }

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

static void Server_SetSubStatus(bool sync, SOPC_PubSubState state)
{
    if (!Server_IsRunning())
    {
        return;
    }

    /* Create a WriteRequest with a single WriteValue */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    OpcUa_WriteValue* wv = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));

    /* Avoid the creation of the NodeId each call of the function */
    static SOPC_NodeId* nidStatus = NULL;
    if (NULL == nidStatus)
    {
        nidStatus = SOPC_NodeId_FromCString(NODEID_PUBSUB_STATUS);
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
        if (sync)
        {
            OpcUa_WriteResponse* response = NULL;
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &response);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);
            }
        }
        else
        {
            status = SOPC_ServerHelper_LocalServiceAsync(request, 0);
        }
    }
    else
    {
        SOPC_Free(wv);
        wv = NULL;
        SOPC_Free(request);
        request = NULL;
    }
}

void Server_SetSubStatusAsync(SOPC_PubSubState state)
{
    Server_SetSubStatus(false, state);
}

void Server_SetSubStatusSync(SOPC_PubSubState state)
{
    Server_SetSubStatus(true, state);
}

bool Server_SetTargetVariables(const OpcUa_WriteValue* lwv, const int32_t nbValues)
{
    if (!Server_IsRunning())
    {
        return true;
    }

    /* Encapsulate the WriteValues in a WriteRequest and send it as a local service,
     * acknowledge before the toolkit answers */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (NULL == request)
    {
        return false;
    }

    request->NoOfNodesToWrite = nbValues;
    request->NodesToWrite = SOPC_Calloc((size_t) request->NoOfNodesToWrite, sizeof(*request->NodesToWrite));
    SOPC_ASSERT(NULL != request->NodesToWrite);
    for (int i = 0; i < request->NoOfNodesToWrite; i++)
    {
        SOPC_EncodeableObject_Initialize(lwv->encodeableType, &request->NodesToWrite[i]);
        status = SOPC_EncodeableObject_Copy(lwv->encodeableType, &request->NodesToWrite[i], &lwv[i]);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    status = SOPC_ServerHelper_LocalServiceAsync(request, 0);

    return true;
}

SOPC_DataValue* Server_GetSourceVariables(const OpcUa_ReadValueId* lrv, const int32_t nbValues)
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
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        SOPC_Calloc(1, sizeof(SOPC_PubSheduler_GetVariableRequestContext));

    if (NULL == request || NULL == requestContext)
    {
        SOPC_UNUSED_RESULT(SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &request));
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
    request->NodesToRead = SOPC_Calloc((size_t) request->NoOfNodesToRead, sizeof(*request->NodesToRead));
    SOPC_ASSERT(NULL != request->NodesToRead);
    for (int i = 0; i < request->NoOfNodesToRead; i++)
    {
        SOPC_EncodeableObject_Initialize(lrv->encodeableType, &request->NodesToRead[i]);
        status = SOPC_EncodeableObject_Copy(lrv->encodeableType, &request->NodesToRead[i], &lrv[i]);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    SOPC_Mutex_Lock(&requestContext->mut);

    status = SOPC_ServerHelper_LocalServiceAsync(request, (uintptr_t) requestContext);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

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

void Server_Treat_Local_Service_Response(SOPC_EncodeableType* type, void* response, uintptr_t userContext)
{
    OpcUa_WriteResponse* writeResponse = NULL;
    OpcUa_ReadResponse* readResponse = NULL;
    SOPC_ReturnStatus statusCopy = SOPC_STATUS_NOK;

    /* Listen for WriteResponses, which only contain status codes */

    SOPC_PubSheduler_GetVariableRequestContext* ctx = (SOPC_PubSheduler_GetVariableRequestContext*) userContext;
    if (&OpcUa_ReadResponse_EncodeableType == type && NULL != ctx)
    {
        SOPC_Mutex_Lock(&ctx->mut);

        statusCopy = SOPC_STATUS_OK;
        readResponse = (OpcUa_ReadResponse*) response;

        if (NULL != readResponse) // Response if deleted by scheduler !!!
        {
            // Allocate data values
            ctx->ldv = SOPC_Calloc((size_t) ctx->NoOfNodesToRead, sizeof(SOPC_DataValue));

            // Copy to response
            if (NULL != ctx->ldv)
            {
                for (size_t i = 0; i < (size_t) ctx->NoOfNodesToRead && SOPC_STATUS_OK == statusCopy; ++i)
                {
                    statusCopy = SOPC_DataValue_Copy(&ctx->ldv[i], &readResponse->Results[i]);
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
    else if (&OpcUa_WriteResponse_EncodeableType == type)
    {
        writeResponse = response;
        // Service should have succeeded
        SOPC_ASSERT(0 == (SOPC_GoodStatusOppositeMask & writeResponse->ResponseHeader.ServiceResult));
    }
    else
    {
        SOPC_ASSERT(false);
    }
}
