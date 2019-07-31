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

#include "libs2opc_client_cmds.h"

#include "sopc_builtintypes.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO add documentation */
#define SYNCHRONOUS_READ_TIMEOUT 10000

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Connection global timeout */
#define TIMEOUT_MS 10000
/* Secure Channel lifetime */
#define SC_LIFETIME_MS 3600000
/* Default publish period */
#define PUBLISH_PERIOD_MS 500
/* Default max keep alive count */
#define MAX_KEEP_ALIVE_COUNT 3
/* Lifetime Count of subscriptions */
#define MAX_LIFETIME_COUNT 10
/* Number of targetted publish token */
#define PUBLISH_N_TOKEN 3

/* Path to the certificate authority */
#define PATH_CACERT_PUBL "./trusted/cacert.der"
/* Path to the server certificate */
#define PATH_SERVER_PUBL "./server_public/server_2k_cert.der"
/* Path to the client certificate */
#define PATH_CLIENT_PUBL "./client_public/client_2k_cert.der"
/* Path to the client private key */
#define PATH_CLIENT_PRIV "./client_private/client_2k_key.pem"

/* Default policy Id */
#define POLICY_ID "anonymous"

/* Command line helpers and arguments */
enum
{
    OPT_HELP,
    OPT_ENDPOINT,
    OPT_POLICYID,
    OPT_USERNAME,
    OPT_PASSWORD,
    OPT_PUBLISH_PERIOD,
    OPT_TOKEN_TARGET,
    OPT_DISABLE_SECU,
    OPT_KEEPALIVE,
} cmd_line_option_values_t;

typedef struct
{
    char* endpoint_url;
    char* policyId;
    char* username;
    char* password;
    char* publish_period_str;
    int64_t publish_period;
    char* token_target_str;
    uint16_t token_target;
    int node_ids_size;
    char** node_ids;
    bool disable_certificate_verification;
    char* n_max_keepalive_str;
    uint32_t n_max_keepalive;
} cmd_line_options_t;

typedef struct
{
    Mutex mutex; /* protect this context */
    Condition condition;

    SOPC_DataValue* values;
    int32_t nbElements;
    SOPC_StatusCode status;
    bool finish;
} ReadContext;

static SOPC_ReturnStatus SOPC_ReadContext_Initialization(ReadContext* ctx)
{
    SOPC_ReturnStatus status = Mutex_Initialization(&ctx->mutex);
    ctx->values = NULL;
    ctx->nbElements = 0;
    ctx->status = SOPC_STATUS_NOK;
    ctx->finish = false;
    return status;
}


/* Callbacks */
static void log_callback(const SOPC_Toolkit_Log_Level log_level, SOPC_LibSub_CstString text);
static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);

// Return 0 if succeeded
int32_t SOPC_ClientHelper_Initialize(const char* log_path, int32_t log_level)
{
    SOPC_Toolkit_Log_Level level = SOPC_TOOLKIT_LOG_LEVEL_DEBUG;
    bool log_level_set = true;
    bool log_path_set = true;

    switch (log_level)
    {
    case 0:
        level = SOPC_TOOLKIT_LOG_LEVEL_ERROR;
        break;
    case 1:
        level = SOPC_TOOLKIT_LOG_LEVEL_WARNING;
        break;
    case 2:
        level = SOPC_TOOLKIT_LOG_LEVEL_INFO;
        break;
    case 3:
        level = SOPC_TOOLKIT_LOG_LEVEL_DEBUG;
        break;
    default:
        log_level_set = false;
        break; // Keep DEBUG level
    }

    if (log_path == NULL)
    {
        log_path_set = false;
        log_path = "./logs";
    }

    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = log_callback,
                                     .disconnect_callback = disconnect_callback,
                                     .toolkit_logger = {.level = level, .log_path = log_path}};

    SOPC_ReturnStatus status = SOPC_LibSub_Initialize(&cfg_cli);

    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, SOPC_LibSub_GetVersion());

    if (!log_level_set)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_WARNING, "Invalid log level provided, set to level 3 (debug) by default.");
    }

    if (!log_path_set)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_WARNING, "No log path provided, set to './logs' by default.");
    }

    if (SOPC_STATUS_OK != status)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not initialize library.");
        return 2;
    }

    return 0;
}

void SOPC_ClientHelper_Finalize(void)
{
    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Closing the Toolkit.");
    SOPC_LibSub_Clear();
    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Toolkit closed.");
}

// Return connection Id > 0 if succeeded, -<n> with <n> argument number (starting from 1) if invalid arguement detected
// or '-100' if connection failed
int32_t SOPC_ClientHelper_Connect(const char* endpointUrl,
                                  SOPC_ClientHelper_Security security)
{
    /* TODO use structure instead of redefining */
    const char* security_policy = security.security_policy;
    int32_t security_mode = security.security_mode;
    const char* path_cert_auth = security.path_cert_auth;
    const char* path_cert_srv = security.path_cert_srv;
    const char* path_cert_cli = security.path_cert_cli;
    const char* path_key_cli = security.path_key_cli;
    const char* policyId = security.policyId;
    const char* username = security.username;
    const char* password = security.password;

    SOPC_LibSub_DataChangeCbk callback = NULL;

    OpcUa_MessageSecurityMode secuMode = OpcUa_MessageSecurityMode_Invalid;
    bool disable_verification = false;
    const char* cert_auth = path_cert_auth;
    const char* cert_srv = path_cert_srv;
    const char* cert_cli = path_cert_cli;
    const char* key_cli = path_key_cli;

    if (NULL == endpointUrl)
    {
        return -1;
    }

    if (NULL == security_policy)
    {
        return -2;
    }

    switch (security_mode)
    {
    case OpcUa_MessageSecurityMode_None:
        if (strncmp(security_policy, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) != 0)
        {
            return -2;
        }
        disable_verification = true;
        secuMode = OpcUa_MessageSecurityMode_None;
        cert_auth = NULL;
        cert_srv = NULL;
        cert_cli = NULL;
        key_cli = NULL;
        break;
    case OpcUa_MessageSecurityMode_Sign:
        secuMode = OpcUa_MessageSecurityMode_Sign;
        break;
    case OpcUa_MessageSecurityMode_SignAndEncrypt:
        secuMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
        break;
    default:
        return -3;
    }

    if (!disable_verification && NULL == cert_auth)
    {
        return -4;
    }
    if (!disable_verification && NULL == cert_srv)
    {
        return -5;
    }
    if (!disable_verification && NULL == cert_cli)
    {
        return -6;
    }
    if (!disable_verification && NULL == key_cli)
    {
        return -7;
    }

    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = endpointUrl,
                                         .security_policy = security_policy,
                                         .security_mode = secuMode,
                                         .disable_certificate_verification = disable_verification,
                                         .path_cert_auth = cert_auth,
                                         .path_cert_srv = cert_srv,
                                         .path_cert_cli = cert_cli,
                                         .path_key_cli = key_cli,
                                         .path_crl = NULL,
                                         .policyId = policyId,
                                         .username = username,
                                         .password = password,
                                         .publish_period_ms = PUBLISH_PERIOD_MS,
                                         .n_max_keepalive = MAX_KEEP_ALIVE_COUNT,
                                         .n_max_lifetime = MAX_LIFETIME_COUNT,
                                         .data_change_callback = callback,
                                         .timeout_ms = TIMEOUT_MS,
                                         .sc_lifetime = SC_LIFETIME_MS,
                                         .token_target = PUBLISH_N_TOKEN,
                                         .generic_response_callback = NULL};
    SOPC_LibSub_ConfigurationId cfg_id = 0;
    SOPC_LibSub_ConnectionId con_id = 0;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (cfg_con.security_mode != OpcUa_MessageSecurityMode_None)
    {
        cfg_con.path_cert_srv = PATH_SERVER_PUBL;
        cfg_con.path_cert_cli = PATH_CLIENT_PUBL;
        cfg_con.path_key_cli = PATH_CLIENT_PRIV;
    }
    if (!cfg_con.disable_certificate_verification)
    {
        cfg_con.path_cert_auth = PATH_CACERT_PUBL;
    }

    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Connecting to \"%s\"", cfg_con.server_url);

    status = SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id);
    if (SOPC_STATUS_OK != status)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not configure connection.");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LibSub_Configured();
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not configure the toolkit.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LibSub_Connect(cfg_id, &con_id);
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not connect with given configuration id.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Connected.");
    }
    else
    {
        return -100;
    }

    assert(con_id > 0);
    assert(con_id <= INT32_MAX);
    return (int32_t) con_id;
}

// TODO Add Mutex protection. See SOPC_Mutex
void SOPC_ClientHelper_GenericCallback(SOPC_LibSub_ConnectionId c_id,
                                       SOPC_LibSub_ApplicativeEvent event,
                                       SOPC_StatusCode status,
                                       const void* response,
                                       uintptr_t responseContext)
{
    // unused
    (void) c_id;

    if (SOPC_LibSub_ApplicativeEvent_Response != event)
    {
        return;
    }

    const SOPC_EncodeableType* pEncType = *(SOPC_EncodeableType* const*) response;

    if (pEncType == &OpcUa_ReadResponse_EncodeableType)
    {
        ReadContext* ctx = (ReadContext*) responseContext;
        const OpcUa_ReadResponse* readResp = *(OpcUa_ReadResponse* const*) response;

        ctx->status = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);

        ctx->status = status;
        if (ctx->nbElements != readResp->NoOfResults)
        {
            // TODO log error
            ctx->status = SOPC_STATUS_NOK;
        }
        for (int32_t i = 0; i < readResp->NoOfResults && ctx->status; i++)
        {
            ctx->status = SOPC_DataValue_Copy(&ctx->values[i], &readResp->Results[i]);
        }
        if (ctx->status == SOPC_STATUS_NOK)
        {
            for (int32_t i = 0; i < readResp->NoOfResults; i++)
            {
                SOPC_DataValue_Clear(&ctx->values[i]);
            }
            ctx->nbElements = 0;
        }
        ctx->finish = true;

        /* Signal that the response is available */
        status = Condition_SignalAll(&ctx->condition);
        assert(SOPC_STATUS_OK == status);
    }
}

int32_t SOPC_ClientHelper_Read(int32_t connectionId,
                               SOPC_ClientHelper_ReadValue* readValues,
                               size_t nbElements,
                               SOPC_DataValue* values)
{
    if (connectionId <= 0)
    {
        return -1;
    }
    if (NULL == readValues || nbElements < 1 || nbElements > INT32_MAX)
    {
        return -2;
    }

    OpcUa_ReadRequest request;
    OpcUa_ReadValueId* nodesToRead;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    ReadContext ctx;

    OpcUa_ReadRequest_Initialize(&request);
    request.MaxAge = 0; /* Not manage by S2OPC */
    request.TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
    request.NoOfNodesToRead = (int32_t) nbElements; // check is done before

    /* Set NodesToRead. This is deallocated by toolkit
       when call SOPC_LibSub_AsyncSendRequestOnSession */
    nodesToRead = SOPC_Calloc(nbElements, sizeof(OpcUa_ReadValueId));
    for (size_t i = 0; i < nbElements && SOPC_STATUS_OK == status; i++)
    {
        OpcUa_ReadValueId_Initialize(&nodesToRead[i]);
        nodesToRead[i].AttributeId = readValues[i].attributeId;
        status = SOPC_String_CopyFromCString(&nodesToRead[i].IndexRange, readValues[i].indexRange);
        if (SOPC_STATUS_OK == status)
        {
            // create an instance of NodeId
            SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(readValues[i].nodeId, 1);
            status = SOPC_NodeId_Copy(&nodesToRead[i].NodeId, nodeId);
            SOPC_NodeId_Clear(nodeId);
        }
    }
    request.NodesToRead = nodesToRead;

    if (SOPC_STATUS_OK == status)
    {
        /* Prepare the synchronous context */
        /* TODO: assert that the SOPC_STATUS_OK != statusMutex always avoid deadlocks in production code */
        SOPC_ReturnStatus statusMutex = SOPC_ReadContext_Initialization(&ctx);
        assert(SOPC_STATUS_OK == statusMutex);
        Condition_Init(&ctx.condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Lock(&ctx.mutex);
        assert(SOPC_STATUS_OK == statusMutex);

        /* Set context */
        ctx.values = values;
        ctx.nbElements = request.NoOfNodesToRead;
        ctx.status = SOPC_STATUS_NOK;
        ctx.finish = false;
        status =
            SOPC_LibSub_AsyncSendRequestOnSession((SOPC_LibSub_ConnectionId) connectionId, &request, (uintptr_t) &ctx);

        /* Wait for the response */
        while (SOPC_STATUS_OK == status && ctx.finish)
        {
            statusMutex = Mutex_UnlockAndTimedWaitCond(&ctx.condition, &ctx.mutex, SYNCHRONOUS_READ_TIMEOUT);
            assert(SOPC_STATUS_TIMEOUT != statusMutex); /* TODO return error */
            assert(SOPC_STATUS_OK == statusMutex);
        }
        status = ctx.status;

        /* Free the context */
        statusMutex = Mutex_Unlock(&ctx.mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Condition_Clear(&ctx.condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Clear(&ctx.mutex);
        assert(SOPC_STATUS_OK == statusMutex);
    }

    if (SOPC_STATUS_OK == status)
    {
        return 0;
    }
    else
    {
        return -100;
    }
}

int32_t SOPC_ClientHelper_CreateSubscription(int32_t connectionId, SOPC_ClientHelper_DataChangeCbk callback)
{
    if (connectionId <= 0)
    {
        return -1;
    }
    else if (NULL == callback)
    {
        return -2;
    }
    //else if (NULL == nodeIds || nbNodeIds <= 0 || nbNodeIds > INT32_MAX)
    //{
    //    return -2;
    //}
    //else
    //{
    //    /*
    //    for (size_t i = 0; i < nbNodeIds; i++)
    //    {
    //        if (NULL == nodeIds[i])
    //        {
    //            return -3 - (int32_t) i;
    //        }
    //    }*/
    //}

    //SOPC_LibSub_AttributeId* lAttrIds = calloc(nbNodeIds, sizeof(SOPC_LibSub_AttributeId));
    //assert(NULL != lAttrIds);
    //for (size_t i = 0; i < nbNodeIds; ++i)
    //{
    //    lAttrIds[i] = SOPC_LibSub_AttributeId_Value;
    //}
    //SOPC_LibSub_DataId* lDataId = calloc(nbNodeIds, sizeof(SOPC_LibSub_DataId));
    //assert(NULL != lDataId);
    //SOPC_ReturnStatus status = SOPC_LibSub_AddToSubscription(
    //    (SOPC_LibSub_ConnectionId) connectionId, (const char* const*) nodeIds, lAttrIds, (int32_t) nbNodeIds, lDataId);
    //if (SOPC_STATUS_OK != status)
    //{
    //    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not create monitored items.");
    //}
    //else
    //{
    //    for (size_t i = 0; i < nbNodeIds; ++i)
    //    {
    //        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Created MonIt for \"%s\" with data_id %" PRIu32 ".", nodeIds[i],
    //                    lDataId[i]);
    //    }
    //}
    //free(lAttrIds);
    //free(lDataId);

    //if (SOPC_STATUS_OK != status)
    //{
    //    return 3;
    //}

    return -100;
}

int32_t SOPC_ClientHelper_AddMonitoredItems(int32_t connectionId, char** nodeIds, size_t nbNodeIds)
{
    // TODO implement this function
    (void) connectionId;
    (void) nodeIds;
    (void) nbNodeIds;
    return -100;
}

int32_t SOPC_ClientHelper_Unsubscribe(int32_t connectionId)
{
    // TODO implement this function
    (void) connectionId;
    return -100;
}

int32_t SOPC_ClientHelper_Disconnect(int32_t connectionId)
{
    if (connectionId <= 0)
    {
        return -1;
    }

    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Closing the connection %" PRIi32, connectionId);
    SOPC_ReturnStatus status = SOPC_LibSub_Disconnect((SOPC_LibSub_ConnectionId) connectionId);

    if (SOPC_STATUS_OK != status)
    {
        return 4;
    }
    return 0;
}

static void log_callback(const SOPC_Toolkit_Log_Level log_level, SOPC_LibSub_CstString text)
{
    Helpers_LoggerStdout(log_level, text);
}

static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Client %" PRIu32 " disconnected.", c_id);
}

int32_t SOPC_ClientHelper_Write(int32_t connectionId, SOPC_ClientHelper_WriteValue* writeValues, size_t nbElements)
{
    // TODO
    (void) connectionId;
    (void) writeValues;
    (void) nbElements;
    return -100;
}
int32_t SOPC_ClientHelper_Browse(int32_t connectionId,
                                 SOPC_ClientHelper_BrowseRequest* browseRequests,
                                 size_t nbElements,
                                 SOPC_ClientHelper_BrowseResult* browseResults)
{
    // TODO
    (void) connectionId;
    (void) browseRequests;
    (void) nbElements;
    (void) browseResults;
    return -100;
}
