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

/** \file
 *
 * \brief Tests the ServerDiagnostics EnabledFlag management:
 *        - diagnostics support computed from EnabledFlag initial value and AccessLevel (configuration only),
 *        - EnabledFlag without value in address space: diagnostics disabled, nodes have OpcUa_BadNotReadable status,
 *        - diagnostics values kept up to date while disabled but address space not updated,
 *        - EnabledFlag written to TRUE: nodes updated with current values,
 *        - CurrentSessionCount: session opened while disabled counted once enabled, 2 simultaneous sessions
 *          counted, value decremented on each session closure,
 *        - EnabledFlag written to FALSE: nodes set to 0 with OpcUa_BadNotReadable status,
 *        - counters reset on server stop: a client creates a session, a subscription and a rejected request
 *          (ServiceFault), then the server is restarted: EnabledFlag state is kept and cumulated counters restart
 *          from 0,
 *        - unmanaged diagnostics nodes always have OpcUa_BadNotReadable status.
 *
 *        When compiled with TEST_DIAGNOSTICS_INHIBITED, the server is started with a FALSE and not writable
 *        EnabledFlag: diagnostics are inhibited, a local write of EnabledFlag has no effect and diagnostics remain
 *        inhibited after a server restart.
 */

#include <stdio.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_threads.h"

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_diagnostics.h"
#include "libs2opc_server_internal.h"

#include "embedded/sopc_addspace_loader.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost:product"
#define CLIENT_APPLICATION_URI "urn:S2OPC:localhost:client"

#define SLEEP_TIMEOUT_MS 50
#define WAIT_TIMEOUT_MS 5000

// Number of client connections (one session each) used simultaneously
#define NB_CLIENT_CONNECTIONS 2

// AccessLevel values of EnabledFlag node: CurrentRead and CurrentRead | CurrentWrite
#define ACCESS_LEVEL_READ 1
#define ACCESS_LEVEL_READ_WRITE 3

// EnabledFlag node value to set in address space
typedef enum
{
    ENABLED_FLAG_NO_VALUE,
    ENABLED_FLAG_FALSE,
    ENABLED_FLAG_TRUE
} EnabledFlagValue;

static const SOPC_NodeId enabledFlagNodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_EnabledFlag);
static const SOPC_NodeId rejectedRequestsNodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_RejectedRequestsCount);
static const SOPC_NodeId securityRejectedRequestsNodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_SecurityRejectedRequestsCount);
static const SOPC_NodeId currentSessionCountNodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CurrentSessionCount);

// Cumulated counters incremented once by the client activity
static const SOPC_NodeId cumulatedCountersNodeIds[] = {
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CumulatedSessionCount),
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_CumulatedSubscriptionCount),
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary_RejectedRequestsCount)};
#define NB_CUMULATED_COUNTERS (sizeof(cumulatedCountersNodeIds) / sizeof(cumulatedCountersNodeIds[0]))

// Expected status and value of a UInt32 diagnostic node
typedef struct
{
    const SOPC_NodeId* nodeId;
    SOPC_StatusCode status;
    uint32_t value;
} ExpectedNode;

// Expected diagnostic nodes evaluated by wait_for
typedef struct
{
    const ExpectedNode* nodes;
    size_t nbNodes;
} ExpectedNodes;

// Diagnostic nodes expected when diagnostics are disabled: value 0 with OpcUa_BadNotReadable status
static const ExpectedNode disabledNodes[] = {{&rejectedRequestsNodeId, OpcUa_BadNotReadable, 0},
                                             {&securityRejectedRequestsNodeId, OpcUa_BadNotReadable, 0},
                                             {&currentSessionCountNodeId, OpcUa_BadNotReadable, 0}};
#define NB_DISABLED_NODES (sizeof(disabledNodes) / sizeof(disabledNodes[0]))

// Defined in the generated embedded address space: true if generated as constant (WITH_CONST_ADDSPACE)
extern const bool sopc_embedded_is_const_addspace;

static int32_t serverStopped = 0;

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&serverStopped, 1);
}

static void SOPC_ClientConnectionEventCb(SOPC_ClientConnection* config,
                                         SOPC_ClientConnectionEvent event,
                                         SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
}

static void SOPC_ClientSubscriptionNotificationCb(const SOPC_ClientHelper_Subscription* subscription,
                                                  SOPC_StatusCode status,
                                                  SOPC_EncodeableType* notificationType,
                                                  uint32_t nbNotifElts,
                                                  const void* notification,
                                                  uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(notificationType);
    SOPC_UNUSED_ARG(nbNotifElts);
    SOPC_UNUSED_ARG(notification);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);
}

// Condition evaluated periodically by wait_for
typedef bool WaitCondition_Fct(void* context);

// Waits until the condition is true with a maximum duration of WAIT_TIMEOUT_MS, returns false on timeout
static bool wait_for(WaitCondition_Fct* condition, void* context)
{
    for (uint32_t waitedMs = 0; waitedMs <= WAIT_TIMEOUT_MS; waitedMs += SLEEP_TIMEOUT_MS)
    {
        if (condition(context))
        {
            return true;
        }
        SOPC_Sleep(SLEEP_TIMEOUT_MS);
    }
    return false;
}

static bool is_server_stopped(void* context)
{
    SOPC_UNUSED_ARG(context);
    return 0 != SOPC_Atomic_Int_Get(&serverStopped);
}

static bool set_enabled_flag_node(SOPC_AddressSpace* addressSpace, SOPC_Byte accessLevel, EnabledFlagValue value)
{
    bool found = false;
    SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addressSpace, &enabledFlagNodeId, &found);
    if (!found || NULL == node || OpcUa_NodeClass_Variable != node->node_class)
    {
        return false;
    }
    node->data.variable.AccessLevel = accessLevel;
    SOPC_Variant* variant = SOPC_AddressSpace_Get_Value(addressSpace, node);
    SOPC_Variant_Clear(variant);
    SOPC_Variant_Initialize(variant);
    if (ENABLED_FLAG_NO_VALUE != value)
    {
        variant->BuiltInTypeId = SOPC_Boolean_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Boolean = (ENABLED_FLAG_TRUE == value);
    }
    return true;
}

/* Returns true if configuring the given EnabledFlag node leads to the expected diagnostics support and enabled
 * states. Shall be called before server configuration initialization: callback registration is then refused
 * (SOPC_STATUS_INVALID_STATE), which allows to check it is only attempted when diagnostics are supported.
 *
 * Note: SOPC_STATUS_INVALID_STATE status case never happens when server configuration is initialized
 *       and is only used as a probe for testing.
 */
static bool check_configure_case(SOPC_Byte accessLevel,
                                 EnabledFlagValue value,
                                 bool expSupported,
                                 bool expEnabled,
                                 bool expCbRegistration)
{
    SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    bool result = NULL != addressSpace && set_enabled_flag_node(addressSpace, accessLevel, value);
    if (result)
    {
        const SOPC_ReturnStatus expStatus = expCbRegistration ? SOPC_STATUS_INVALID_STATE : SOPC_STATUS_OK;
        result = expStatus == SOPC_ServerInternal_DiagnosticsConfigure(addressSpace) &&
                 expSupported == SOPC_ServerInternal_IsDiagnosticsSupported() &&
                 expEnabled == SOPC_ServerInternal_IsDiagnosticsEnabled();
    }
    SOPC_AddressSpace_Delete(addressSpace);
    SOPC_ServerInternal_DiagnosticsClear();
    return result && !SOPC_ServerInternal_IsDiagnosticsSupported() && !SOPC_ServerInternal_IsDiagnosticsEnabled();
}

static SOPC_ReturnStatus check_configure(void)
{
    bool result = check_configure_case(ACCESS_LEVEL_READ, ENABLED_FLAG_FALSE, false, false, false) &&
                  check_configure_case(ACCESS_LEVEL_READ, ENABLED_FLAG_TRUE, true, true, true) &&
                  check_configure_case(ACCESS_LEVEL_READ, ENABLED_FLAG_NO_VALUE, false, false, false) &&
                  check_configure_case(ACCESS_LEVEL_READ_WRITE, ENABLED_FLAG_FALSE, true, false, true) &&
                  check_configure_case(ACCESS_LEVEL_READ_WRITE, ENABLED_FLAG_TRUE, true, true, true) &&
                  check_configure_case(ACCESS_LEVEL_READ_WRITE, ENABLED_FLAG_NO_VALUE, true, false, true);

    // EnabledFlag node absent: diagnostics not supported
    if (result)
    {
        SOPC_AddressSpace* emptyAddressSpace = SOPC_AddressSpace_Create(true);
        result = NULL != emptyAddressSpace &&
                 SOPC_STATUS_OK == SOPC_ServerInternal_DiagnosticsConfigure(emptyAddressSpace) &&
                 !SOPC_ServerInternal_IsDiagnosticsSupported() && !SOPC_ServerInternal_IsDiagnosticsEnabled();
        SOPC_AddressSpace_Delete(emptyAddressSpace);
        SOPC_ServerInternal_DiagnosticsClear();
    }
    printf("<Test_Server_Diagnostics_Enabled_Flag: diagnostics support configuration: %s\n", result ? "OK" : "NOK");
    return result ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

static SOPC_ReturnStatus configure_server(SOPC_Byte accessLevel, bool expSupported)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
    // An unsecured endpoint (no certificate nor PKI needed) is sufficient for the test
    SOPC_SecurityPolicy* sp = (NULL == ep) ? NULL : SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
    if (NULL == sp)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_None);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC diagnostics enabled flag test server", NULL,
                                                                   OpcUa_ApplicationType_Server);
    }
    // Reduce shutdown phase duration for tests (server is restarted)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetShutdownCountdown(0);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
        // Set EnabledFlag = False + accessLevel parameter value
        status = (NULL != addressSpace && set_enabled_flag_node(addressSpace, accessLevel, ENABLED_FLAG_FALSE))
                     ? SOPC_ServerConfigHelper_SetAddressSpace(addressSpace)
                     : SOPC_STATUS_NOK;
        if (SOPC_STATUS_OK != status)
        {
            SOPC_AddressSpace_Delete(addressSpace);
        }
    }
    // EnabledFlag is FALSE: diagnostics are disabled
    if (SOPC_STATUS_OK == status &&
        (SOPC_ServerInternal_IsDiagnosticsEnabled() || expSupported != SOPC_ServerInternal_IsDiagnosticsSupported()))
    {
        status = SOPC_STATUS_NOK;
    }
    printf("<Test_Server_Diagnostics_Enabled_Flag: server configuration: %s\n",
           SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

static SOPC_ReturnStatus write_enabled_flag_sync(bool enabled)
{
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    dv.Value.BuiltInTypeId = SOPC_Boolean_Id;
    dv.Value.Value.Boolean = enabled;

    OpcUa_WriteRequest* writeReq = SOPC_WriteRequest_Create(1);
    OpcUa_WriteResponse* writeResp = NULL;
    SOPC_ReturnStatus status = (NULL == writeReq) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_WriteRequest_SetWriteValue(writeReq, 0, &enabledFlagNodeId, SOPC_AttributeId_Value, NULL, &dv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(writeReq, (void**) &writeResp);
        writeReq = NULL; // freed by toolkit
    }
    if (SOPC_STATUS_OK == status)
    {
        status = (&OpcUa_WriteResponse_EncodeableType == writeResp->encodeableType &&
                  SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult) && 1 == writeResp->NoOfResults &&
                  SOPC_IsGoodStatus(writeResp->Results[0]))
                     ? SOPC_STATUS_OK
                     : SOPC_STATUS_NOK;
    }
    if (NULL != writeResp)
    {
        SOPC_EncodeableObject_Delete(writeResp->encodeableType, (void**) &writeResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &writeReq);
    return status;
}

/* Returns true if all the given UInt32 diagnostic nodes have the expected value and status:
 * a Good expected status matches any Good status, any other expected status shall be equal */
static bool check_nodes(const ExpectedNode* nodes, size_t nbNodes)
{
    OpcUa_ReadRequest* readReq = SOPC_ReadRequest_Create(nbNodes, OpcUa_TimestampsToReturn_Neither);
    OpcUa_ReadResponse* readResp = NULL;
    SOPC_ReturnStatus status = (NULL == readReq) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    for (size_t i = 0; SOPC_STATUS_OK == status && i < nbNodes; i++)
    {
        status = SOPC_ReadRequest_SetReadValue(readReq, i, nodes[i].nodeId, SOPC_AttributeId_Value, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(readReq, (void**) &readResp);
        readReq = NULL; // freed by toolkit
    }
    bool result = SOPC_STATUS_OK == status && &OpcUa_ReadResponse_EncodeableType == readResp->encodeableType &&
                  SOPC_IsGoodStatus(readResp->ResponseHeader.ServiceResult) &&
                  nbNodes == (size_t) readResp->NoOfResults;
    for (int32_t i = 0; result && i < readResp->NoOfResults; i++)
    {
        const SOPC_DataValue* dv = &readResp->Results[i];
        const ExpectedNode* expected = &nodes[i];
        const bool statusOk =
            SOPC_IsGoodStatus(expected->status) ? SOPC_IsGoodStatus(dv->Status) : expected->status == dv->Status;
        result = statusOk && SOPC_UInt32_Id == dv->Value.BuiltInTypeId && expected->value == dv->Value.Value.Uint32;
    }
    if (NULL != readResp)
    {
        SOPC_EncodeableObject_Delete(readResp->encodeableType, (void**) &readResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &readReq);
    return result;
}

static bool are_nodes(void* context)
{
    const ExpectedNodes* expected = (const ExpectedNodes*) context;
    return check_nodes(expected->nodes, expected->nbNodes);
}

// Waits until all the given UInt32 diagnostic nodes have the expected value and status, returns false on timeout
static bool wait_nodes(const ExpectedNode* nodes, size_t nbNodes)
{
    ExpectedNodes expected = {nodes, nbNodes};
    return wait_for(are_nodes, &expected);
}

// Returns true if the diagnostic nodes have the disabled diagnostics value 0 with OpcUa_BadNotReadable status
static bool check_disabled_nodes(void)
{
    return check_nodes(disabledNodes, NB_DISABLED_NODES);
}

// Waits until the diagnostic nodes have the disabled diagnostics value 0 with OpcUa_BadNotReadable status
static bool wait_disabled_nodes(void)
{
    return wait_nodes(disabledNodes, NB_DISABLED_NODES);
}

/* Returns SOPC_STATUS_OK if all the cumulated counters reach the expected cumulated value and CurrentSessionCount
 * reaches the expected current sessions value, with a Good status */
static SOPC_ReturnStatus wait_counters(uint32_t cumulated, uint32_t currentSessions)
{
    ExpectedNode nodes[NB_CUMULATED_COUNTERS + 1];
    for (size_t i = 0; i < NB_CUMULATED_COUNTERS; i++)
    {
        nodes[i].nodeId = &cumulatedCountersNodeIds[i];
        nodes[i].status = SOPC_GoodGenericStatus;
        nodes[i].value = cumulated;
    }
    nodes[NB_CUMULATED_COUNTERS].nodeId = &currentSessionCountNodeId;
    nodes[NB_CUMULATED_COUNTERS].status = SOPC_GoodGenericStatus;
    nodes[NB_CUMULATED_COUNTERS].value = currentSessions;
    return wait_nodes(nodes, NB_CUMULATED_COUNTERS + 1) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

// Returns true if all the unmanaged diagnostic nodes have the OpcUa_BadNotReadable status
static bool check_unmanaged_nodes(void)
{
    const SOPC_NodeId unmanagedNodeIds[] = {
        SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_SubscriptionDiagnosticsArray),
        SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerDiagnostics_SessionsDiagnosticsSummary_SessionDiagnosticsArray),
        SOPC_NODEID_NS0_NUMERIC(
            OpcUaId_Server_ServerDiagnostics_SessionsDiagnosticsSummary_SessionSecurityDiagnosticsArray),
    };
    const size_t nbNodes = sizeof(unmanagedNodeIds) / sizeof(unmanagedNodeIds[0]);
    OpcUa_ReadRequest* readReq = SOPC_ReadRequest_Create(nbNodes, OpcUa_TimestampsToReturn_Neither);
    OpcUa_ReadResponse* readResp = NULL;
    SOPC_ReturnStatus status = (NULL == readReq) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    for (size_t i = 0; SOPC_STATUS_OK == status && i < nbNodes; i++)
    {
        status = SOPC_ReadRequest_SetReadValue(readReq, i, &unmanagedNodeIds[i], SOPC_AttributeId_Value, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(readReq, (void**) &readResp);
        readReq = NULL; // freed by toolkit
    }
    bool result = SOPC_STATUS_OK == status && &OpcUa_ReadResponse_EncodeableType == readResp->encodeableType &&
                  SOPC_IsGoodStatus(readResp->ResponseHeader.ServiceResult) &&
                  (int32_t) nbNodes == readResp->NoOfResults;
    for (int32_t i = 0; result && i < readResp->NoOfResults; i++)
    {
        result = OpcUa_BadNotReadable == readResp->Results[i].Status;
    }
    if (NULL != readResp)
    {
        SOPC_EncodeableObject_Delete(readResp->encodeableType, (void**) &readResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &readReq);
    printf("<Test_Server_Diagnostics_Enabled_Flag: unmanaged nodes not readable: %s\n", result ? "OK" : "NOK");
    return result;
}

static SOPC_ReturnStatus configure_client(SOPC_SecureConnection_Config* secureConnConfigs[NB_CLIENT_CONNECTIONS])
{
    const char* userIds[NB_CLIENT_CONNECTIONS] = {"1", "2"};
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetApplicationDescription(
        CLIENT_APPLICATION_URI, CLIENT_APPLICATION_URI, "S2OPC diagnostics enabled flag test client", NULL,
        OpcUa_ApplicationType_Client);
    // One secure connection configuration per simultaneous connection
    for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_CLIENT_CONNECTIONS; i++)
    {
        secureConnConfigs[i] = SOPC_ClientConfigHelper_CreateSecureConnection(
            userIds[i], DEFAULT_ENDPOINT_URL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);
        status = (NULL == secureConnConfigs[i]) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }
    printf("<Test_Server_Diagnostics_Enabled_Flag: client configuration: %s\n",
           SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

// Sends a request rejected by the server with a ServiceFault (Read request without node: BadNothingToDo)
static SOPC_ReturnStatus send_rejected_request(SOPC_ClientConnection* connection)
{
    OpcUa_ReadRequest* readReq = NULL;
    void* resp = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &readReq);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(connection, readReq, &resp);
        readReq = NULL; // freed by client wrapper
    }
    // ServiceFault expected
    if (SOPC_STATUS_OK == status)
    {
        status =
            (&OpcUa_ServiceFault_EncodeableType == *(SOPC_EncodeableType**) resp) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }
    if (NULL != resp)
    {
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) resp, &resp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &readReq);
    return status;
}

/* Creates a subscription, sends a rejected request and deletes the subscription in the given session:
 * cumulated subscription and rejected requests counters are incremented by 1 */
static SOPC_ReturnStatus run_session_activity(SOPC_ClientConnection* connection)
{
    SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelper_CreateSubscription(
        connection, SOPC_CreateSubscriptionRequest_CreateDefault(), SOPC_ClientSubscriptionNotificationCb, 0);
    SOPC_ReturnStatus status = (NULL == subscription) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = send_rejected_request(connection);
    }
    if (NULL != subscription)
    {
        SOPC_ReturnStatus delStatus = SOPC_ClientHelper_DeleteSubscription(&subscription);
        status = (SOPC_STATUS_OK == status) ? delStatus : status;
    }
    return status;
}

/* Disconnects the client if connected (even if the given status is not OK):
 * returns the given status if not OK, the disconnection status otherwise */
static SOPC_ReturnStatus disconnect_client(SOPC_ClientConnection** connection, SOPC_ReturnStatus status)
{
    if (NULL != *connection)
    {
        SOPC_ReturnStatus discoStatus = SOPC_ClientHelper_Disconnect(connection);
        status = (SOPC_STATUS_OK == status) ? discoStatus : status;
    }
    return status;
}

/* Connects a client (1 session), creates a subscription, sends a rejected request and disconnects:
 * each cumulated counter is incremented by 1.
 * Note: diagnostics nodes update is enqueued as priority request by the services thread before the next event. */
static SOPC_ReturnStatus run_client_activity(SOPC_SecureConnection_Config* secureConnConfig)
{
    SOPC_ClientConnection* connection = NULL;
    SOPC_ReturnStatus status = SOPC_ClientHelper_Connect(secureConnConfig, SOPC_ClientConnectionEventCb, &connection);
    if (SOPC_STATUS_OK == status)
    {
        status = run_session_activity(connection);
    }
    return disconnect_client(&connection, status);
}

static SOPC_ReturnStatus stop_server(void)
{
    SOPC_ReturnStatus status = SOPC_ServerHelper_StopServer();
    if (SOPC_STATUS_OK == status && !wait_for(is_server_stopped, NULL))
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    return status;
}

static SOPC_ReturnStatus restart_server(void)
{
    SOPC_ReturnStatus status = stop_server();
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Atomic_Int_Set(&serverStopped, 0);
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    }
    printf("<Test_Server_Diagnostics_Enabled_Flag: server restart: %s\n", SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

/* Shall be called in server started state.
 * Each client connection has a session activity: cumulated counters equal to 2 on return. */
static SOPC_ReturnStatus check_enabled_flag_started(
    SOPC_SecureConnection_Config* secureConnConfigs[NB_CLIENT_CONNECTIONS])
{
    SOPC_ClientConnection* connections[NB_CLIENT_CONNECTIONS] = {NULL, NULL};

    // Initial state: diagnostics disabled
    SOPC_ReturnStatus status = wait_disabled_nodes() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    printf("<Test_Server_Diagnostics_Enabled_Flag: initial disabled state: %s\n",
           SOPC_STATUS_OK == status ? "OK" : "NOK");

    // Diagnostics values updated while disabled (session kept active): address space not updated
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(secureConnConfigs[0], SOPC_ClientConnectionEventCb, &connections[0]);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_session_activity(connections[0]);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Diagnostics nodes update is a priority request: a local read is treated after any address space update
        status = check_disabled_nodes() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        printf("<Test_Server_Diagnostics_Enabled_Flag: no update while disabled: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }

    // Enable diagnostics: address space updated with values counted while disabled, including the active session
    if (SOPC_STATUS_OK == status)
    {
        status = write_enabled_flag_sync(true);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(1, 1);
        printf("<Test_Server_Diagnostics_Enabled_Flag: enabled: %s\n", SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    // Unmanaged nodes are not readable even if diagnostics are enabled
    if (SOPC_STATUS_OK == status)
    {
        status = check_unmanaged_nodes() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Second simultaneous session
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(secureConnConfigs[1], SOPC_ClientConnectionEventCb, &connections[1]);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_session_activity(connections[1]);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(2, 2);
        printf("<Test_Server_Diagnostics_Enabled_Flag: 2 current sessions: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }

    // CurrentSessionCount decremented on each session closure
    status = disconnect_client(&connections[1], status);
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(2, 1);
    }
    status = disconnect_client(&connections[0], status);
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(2, 0);
        printf("<Test_Server_Diagnostics_Enabled_Flag: current sessions decremented: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }

    // Disable diagnostics: address space values set to 0 with BadNotReadable status
    if (SOPC_STATUS_OK == status)
    {
        status = write_enabled_flag_sync(false);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = wait_disabled_nodes() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        printf("<Test_Server_Diagnostics_Enabled_Flag: disabled: %s\n", SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    return status;
}

// Shall be called in server started state with diagnostics inhibited
static SOPC_ReturnStatus check_inhibited_started(void)
{
    SOPC_ReturnStatus status =
        (!SOPC_ServerInternal_IsDiagnosticsSupported() && wait_disabled_nodes()) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    printf("<Test_Server_Diagnostics_Enabled_Flag: initial inhibited state: %s\n",
           SOPC_STATUS_OK == status ? "OK" : "NOK");
    if (SOPC_STATUS_OK == status)
    {
        status = check_unmanaged_nodes() ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Local services bypass the AccessLevel check: write succeeds but has no effect on diagnostics
    if (SOPC_STATUS_OK == status)
    {
        status = write_enabled_flag_sync(true);
    }
    /* The write is synchronous and no asynchronous effect is possible: no write behavior callback is registered when
     * diagnostics are inhibited (checked above) */
    if (SOPC_STATUS_OK == status)
    {
        status =
            (!SOPC_ServerInternal_IsDiagnosticsEnabled() && check_disabled_nodes()) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        printf("<Test_Server_Diagnostics_Enabled_Flag: local write without effect: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    return status;
}

/* Shall be called after ::check_enabled_flag_started (cumulated counters equal to 2):
 * checks counters are reset on server stop and EnabledFlag state is kept on server restart */
static SOPC_ReturnStatus check_reset_on_restart(SOPC_SecureConnection_Config* secureConnConfig)
{
    // Enable diagnostics and generate client activity: cumulated counters equal to 3
    SOPC_ReturnStatus status = write_enabled_flag_sync(true);
    if (SOPC_STATUS_OK == status)
    {
        status = run_client_activity(secureConnConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(3, 0);
        printf("<Test_Server_Diagnostics_Enabled_Flag: counters incremented: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = restart_server();
    }
    // EnabledFlag state kept on restart and cumulated counters reset to 0
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerInternal_IsDiagnosticsEnabled() ? wait_counters(0, 0) : SOPC_STATUS_NOK;
        printf("<Test_Server_Diagnostics_Enabled_Flag: counters reset on restart: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    // Same client activity after restart: cumulated counters equal to 1 (not 4)
    if (SOPC_STATUS_OK == status)
    {
        status = run_client_activity(secureConnConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = wait_counters(1, 0);
        printf("<Test_Server_Diagnostics_Enabled_Flag: counters incremented after restart: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    return status;
}

// Shall be called in server started state with diagnostics inhibited: diagnostics remain inhibited after restart
static SOPC_ReturnStatus check_inhibited_on_restart(void)
{
    SOPC_ReturnStatus status = restart_server();
    if (SOPC_STATUS_OK == status)
    {
        status = (!SOPC_ServerInternal_IsDiagnosticsSupported() && !SOPC_ServerInternal_IsDiagnosticsEnabled() &&
                  wait_disabled_nodes())
                     ? SOPC_STATUS_OK
                     : SOPC_STATUS_NOK;
        printf("<Test_Server_Diagnostics_Enabled_Flag: inhibited after restart: %s\n",
               SOPC_STATUS_OK == status ? "OK" : "NOK");
    }
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    /* The test modifies the EnabledFlag node (AccessLevel and value) and reloads the embedded address space:
     * not possible with a constant address space (WITH_CONST_ADDSPACE) */
    if (sopc_embedded_is_const_addspace)
    {
        printf("<Test_Server_Diagnostics_Enabled_Flag: ignored since address space is constant\n");
        return 0;
    }

    SOPC_SecureConnection_Config* secureConnConfigs[NB_CLIENT_CONNECTIONS] = {NULL, NULL};

    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath =
        "./toolkit_test_server_diagnostics_enabled_flag_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);

#ifdef TEST_DIAGNOSTICS_INHIBITED
    const SOPC_Byte accessLevel = ACCESS_LEVEL_READ;
    const bool expSupported = false;
#else
    const SOPC_Byte accessLevel = ACCESS_LEVEL_READ_WRITE;
    const bool expSupported = true;
#endif

    if (SOPC_STATUS_OK == status)
    {
        status = check_configure();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = configure_server(accessLevel, expSupported);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = configure_client(secureConnConfigs);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = expSupported ? check_enabled_flag_started(secureConnConfigs) : check_inhibited_started();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = expSupported ? check_reset_on_restart(secureConnConfigs[0]) : check_inhibited_on_restart();
    }

    SOPC_ReturnStatus stopStatus = stop_server();

    SOPC_ClientConfigHelper_Clear();
    SOPC_ServerConfigHelper_Clear();
    // Diagnostics state is reset on clear
    if (SOPC_STATUS_OK == status &&
        (SOPC_ServerInternal_IsDiagnosticsSupported() || SOPC_ServerInternal_IsDiagnosticsEnabled()))
    {
        status = SOPC_STATUS_NOK;
    }
    SOPC_CommonHelper_Clear();

    printf("<Test_Server_Diagnostics_Enabled_Flag: final result: %s\n",
           SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus ? "OK" : "NOK");
    return (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus) ? 0 : 1;
}
