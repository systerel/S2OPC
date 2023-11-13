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
static int32_t pubAcyclicSendWriterId = 0;

static SOPC_AddressSpace* address_space = NULL;
static uint8_t lastPubSubCommand = 0;
static char* lastPubSubConfigPath = NULL;

typedef enum PublisherSendStatus
{
    PUBLISHER_ACYCLIC_NOT_TRIGGERED = 0,
    PUBLISHER_ACYCLIC_IN_PROGRESS = 1,
    PUBLISHER_ACYCLIC_SENT = 2,
    PUBLISHER_ACYCLIC_ERROR = 3,
} PublisherSendStatus;

static SOPC_ReturnStatus Server_SetAddressSpace(void);

static void Server_Event_AddressSpace(const SOPC_CallContext* callCtxPtr,
                                      OpcUa_WriteValue* writeValue,
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
    bool res = SOPC_EndpointConfig_SetListeningMode(ep, SOPC_Endpoint_ListenAllInterfaces);
    SOPC_SecurityPolicy* spNone = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
    SOPC_SecurityPolicy* spSecu = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

    if (!res || NULL == ep || NULL == spNone || NULL == spSecu)
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

static void Server_SetSubStatus(bool sync, SOPC_PubSubState state)
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
        if (sync)
        {
            OpcUa_WriteResponse* response = NULL;
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &response);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Encodeable_Delete(response->encodeableType, (void**) &response);
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
    status = SOPC_ServerHelper_LocalServiceAsync(request, 0);

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

    status = SOPC_ServerHelper_LocalServiceAsync(request, (uintptr_t) requestContext);

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
