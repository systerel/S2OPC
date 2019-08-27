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
#include "sopc_array.h"

#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client_common.h"
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

/* Timeout for requests */
#define SYNCHRONOUS_REQUEST_TIMEOUT 10000
/* Max number of simultaneous connections */
#define MAX_SIMULTANEOUS_CONNECTIONS 200
/* Max number of subscribed items per connection */
#define MAX_SUBSCRIBED_ITEMS 200
/* Max BrowseNext requests iteration number */
#define MAX_BROWSENEXT_REQUESTS 200

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

    SOPC_DataValue** values;
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

typedef struct
{
    Mutex mutex; /* protect this context */
    Condition condition;

    SOPC_StatusCode* writeResults;
    int32_t nbElements;
    SOPC_StatusCode status;
    bool finish;
} WriteContext;

static SOPC_ReturnStatus SOPC_WriteContext_Initialization(WriteContext* ctx)
{
    SOPC_ReturnStatus status = Mutex_Initialization(&ctx->mutex);
    ctx->writeResults = NULL;
    ctx->nbElements = 0;
    ctx->status = SOPC_STATUS_NOK;
    ctx->finish = false;
    return status;
}

typedef struct
{
    Mutex mutex; /* protect this context */
    Condition condition;

    SOPC_StatusCode* statusCodes;
    SOPC_Array** browseResults;
    SOPC_ByteString** continuationPoints;
    int32_t nbElements;
    SOPC_StatusCode status;
    bool finish;
} BrowseContext;

static SOPC_ReturnStatus SOPC_BrowseContext_Initialization(BrowseContext* ctx)
{
    SOPC_ReturnStatus status = Mutex_Initialization(&ctx->mutex);
    ctx->statusCodes = NULL;
    ctx->browseResults = NULL;
    ctx->continuationPoints = NULL;
    ctx->nbElements = 0;
    ctx->status = SOPC_STATUS_NOK;
    ctx->finish = false;
    return status;
}

/* Callbacks */
static void log_callback(const SOPC_Toolkit_Log_Level log_level, SOPC_LibSub_CstString text);
static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);
static void SOPC_ClientHelper_GenericCallback(SOPC_LibSub_ConnectionId c_id,
                                              SOPC_LibSub_ApplicativeEvent event,
                                              SOPC_StatusCode status,
                                              const void* response,
                                              uintptr_t responseContext);
/* static functions */

static int32_t ConnectHelper_CreateConfiguration(SOPC_LibSub_ConnectionCfg* cfg_con,
                                                 const char* endpointUrl,
                                                 SOPC_ClientHelper_Security security);
static SOPC_ReturnStatus ReadHelper_Initialize(SOPC_ReturnStatus status,
                                               size_t nbElements,
                                               OpcUa_ReadValueId *nodesToRead,
                                               SOPC_ClientHelper_ReadValue *readValues,
                                               SOPC_DataValue **values);
static SOPC_ReturnStatus WriteHelper_InitialiazeValues(size_t nbElements,
                                                       SOPC_ReturnStatus status,
                                                       OpcUa_WriteValue *nodesToWrite,
                                                       SOPC_ClientHelper_WriteValue *writeValues);
static SOPC_ReturnStatus BrowseHelper_InitializeContinuationPoints(size_t nbElements,
                                                                   SOPC_ReturnStatus status,
                                                                   SOPC_ByteString **continuationPointsArray);
static SOPC_ReturnStatus BrowseHelper_InitializeNodesToBrowse(size_t nbElements,
                                                              SOPC_ReturnStatus status,
                                                              OpcUa_BrowseDescription *nodesToBrowse,
                                                              SOPC_ClientHelper_BrowseRequest *browseRequests);
static SOPC_ReturnStatus BrowseHelper_InitializeBrowseResults(size_t nbElements,
                                                              SOPC_ReturnStatus status,
                                                              SOPC_Array **browseResultsListArray);
static void GenericCallbackHelper_Read(SOPC_StatusCode status,
                                       const void* response,
                                       uintptr_t responseContext);
static void GenericCallbackHelper_Write(SOPC_StatusCode status,
                                        const void* response,
                                        uintptr_t responseContext);
static void GenericCallbackHelper_Browse(SOPC_StatusCode status,
                                         const void* response,
                                         uintptr_t responseContext);
static void GenericCallbackHelper_BrowseNext(SOPC_StatusCode status,
                                             const void* response,
                                             uintptr_t responseContext);
static bool ContainsContinuationPoints(SOPC_ByteString** continuationPointsArray, size_t nbElements);
static SOPC_ReturnStatus BrowseNext(int32_t connectionId,
                                    SOPC_StatusCode* statusCodes,
                                    SOPC_Array** browseResultsListArray,
                                    size_t nbElements,
                                    SOPC_ByteString** continuationPoints);

/* Functions */

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
        log_path = "./logs/";
    }

    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = log_callback,
                                     .disconnect_callback = disconnect_callback,
                                     .toolkit_logger = {.level = level, .log_path = log_path, .maxBytes = 1048576, .maxFiles = 50}};

    SOPC_ReturnStatus status = SOPC_ClientCommon_Initialize(&cfg_cli);

    if (!log_level_set)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_WARNING, "Invalid log level provided, set to level 3 (debug) by default.");
    }

    if (!log_path_set)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_WARNING, "No log path provided, set to './logs/' by default.");
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
    SOPC_ClientCommon_Clear();
    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Toolkit closed.");
}

static int32_t ConnectHelper_CreateConfiguration(SOPC_LibSub_ConnectionCfg* cfg_con,
                                                 const char* endpointUrl,
                                                 SOPC_ClientHelper_Security security)
{
    OpcUa_MessageSecurityMode secuMode = OpcUa_MessageSecurityMode_Invalid;
    bool disable_verification = false;
    const char* cert_auth = security.path_cert_auth;
    const char* cert_srv = security.path_cert_srv;
    const char* cert_cli = security.path_cert_cli;
    const char* key_cli = security.path_key_cli;

    if (NULL == cfg_con)
    {
        return -1;
    }

    switch (security.security_mode)
    {
        case OpcUa_MessageSecurityMode_None:
            if (strncmp(security.security_policy, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) != 0)
            {
                return -11;
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
            return -12;
    }

    if (!disable_verification && NULL == cert_auth)
    {
        return -13;
    }
    if (!disable_verification && NULL == cert_srv)
    {
        return -14;
    }
    if (!disable_verification && NULL == cert_cli)
    {
        return -15;
    }
    if (!disable_verification && NULL == key_cli)
    {
        return -16;
    }
    if (NULL == security.policyId)
    {
        return -17;
    }

    cfg_con->server_url = endpointUrl;
    cfg_con->security_policy = security.security_policy;
    cfg_con->security_mode = secuMode;
    cfg_con->disable_certificate_verification = disable_verification;
    cfg_con->path_cert_auth = cert_auth;
    cfg_con->path_cert_srv = cert_srv;
    cfg_con->path_cert_cli = cert_cli;
    cfg_con->path_key_cli = key_cli;
    cfg_con->path_crl = NULL;
    cfg_con->policyId = security.policyId;
    cfg_con->username = security.username;
    cfg_con->password = security.password;
    cfg_con->publish_period_ms = PUBLISH_PERIOD_MS;
    cfg_con->n_max_keepalive = MAX_KEEP_ALIVE_COUNT;
    cfg_con->n_max_lifetime = MAX_LIFETIME_COUNT;
    cfg_con->data_change_callback = NULL;
    cfg_con->timeout_ms = TIMEOUT_MS;
    cfg_con->sc_lifetime = SC_LIFETIME_MS;
    cfg_con->token_target = PUBLISH_N_TOKEN;
    cfg_con->generic_response_callback = SOPC_ClientHelper_GenericCallback;

    if (cfg_con->security_mode != OpcUa_MessageSecurityMode_None)
    {
        cfg_con->path_cert_srv = PATH_SERVER_PUBL;
        cfg_con->path_cert_cli = PATH_CLIENT_PUBL;
        cfg_con->path_key_cli = PATH_CLIENT_PRIV;
    }
    if (!cfg_con->disable_certificate_verification)
    {
        cfg_con->path_cert_auth = PATH_CACERT_PUBL;
    }

    return 0;
}

// Return connection Id > 0 if succeeded, -<n> with <n> argument number (starting from 1) if invalid argument detected
// or '-100' if connection failed
int32_t SOPC_ClientHelper_Connect(const char* endpointUrl,
                                  SOPC_ClientHelper_Security security)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == endpointUrl)
    {
        return -1;
    }

    if (NULL == security.security_policy)
    {
        return -11;
    }

    SOPC_LibSub_ConnectionCfg cfg_con;
    int32_t res = ConnectHelper_CreateConfiguration(&cfg_con, endpointUrl, security);

    if (0 != res)
    {
        return res;
    }

    SOPC_LibSub_ConfigurationId cfg_id = 0;
    SOPC_LibSub_ConnectionId con_id = 0;

    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Connecting to \"%s\"", cfg_con.server_url);

    status = SOPC_ClientCommon_ConfigureConnection(&cfg_con, &cfg_id);
    if (SOPC_STATUS_OK != status)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not configure connection.");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientCommon_Configured();
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not configure the toolkit.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientCommon_Connect(cfg_id, &con_id);
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

static void GenericCallbackHelper_Read(SOPC_StatusCode status,
                                       const void* response,
                                       uintptr_t responseContext)
{
        ReadContext* ctx = (ReadContext*) responseContext;
        const OpcUa_ReadResponse* readResp = (const OpcUa_ReadResponse*) response;

        ctx->status = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);

        ctx->status = status;
        if (ctx->nbElements != readResp->NoOfResults)
        {
            // TODO log error
            ctx->status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == ctx->status)
        {
            for (int32_t i = 0; i < readResp->NoOfResults && SOPC_STATUS_OK == ctx->status; i++)
            {
                ctx->status = SOPC_DataValue_Copy(ctx->values[i], &readResp->Results[i]);
            }
            if (ctx->status == SOPC_STATUS_NOK)
            {
                for (int32_t i = 0; i < readResp->NoOfResults; i++)
                {
                    SOPC_DataValue_Clear(ctx->values[i]);
                }
                ctx->nbElements = 0;
            }
        }
        ctx->finish = true;

        ctx->status = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);
        /* Signal that the response is available */
        status = Condition_SignalAll(&ctx->condition);
        assert(SOPC_STATUS_OK == status);
}

static void GenericCallbackHelper_Write(SOPC_StatusCode status,
                                        const void* response,
                                        uintptr_t responseContext)
{
        WriteContext* ctx = (WriteContext*) responseContext;
        const OpcUa_WriteResponse* writeResp = (const OpcUa_WriteResponse*) response;

        ctx->status = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);

        ctx->status = status;
        if (ctx->nbElements != writeResp->NoOfResults)
        {
            // TODO log error
            ctx->status = SOPC_STATUS_NOK;
        }
        if (NULL == ctx->writeResults)
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == ctx->status)
        {
            for (int32_t i = 0; i < writeResp->NoOfResults; i++)
            {
                ctx->writeResults[i] = writeResp->Results[i];
            }
        }
        ctx->finish = true;

        ctx->status = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);
        /* Signal that the response is available */
        status = Condition_SignalAll(&ctx->condition);
        assert(SOPC_STATUS_OK == status);
}

static void GenericCallbackHelper_Browse(SOPC_StatusCode status,
                                         const void* response,
                                         uintptr_t responseContext)
{
        BrowseContext* ctx = (BrowseContext*) responseContext;
        const OpcUa_BrowseResponse* browseResp = (const OpcUa_BrowseResponse*) response;

        ctx->status = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);

        ctx->status = status;
        if (ctx->nbElements != browseResp->NoOfResults)
        {
            // TODO log error
            ctx->status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == ctx->status)
        {
            for (int32_t i = 0; i < browseResp->NoOfResults; i++)
            {
                ctx->statusCodes[i] = browseResp->Results[i].StatusCode;
                SOPC_ByteString_Copy(ctx->continuationPoints[i],
                                     &browseResp->Results[i].ContinuationPoint);
                for (int32_t j = 0; j < browseResp->Results[i].NoOfReferences; j++)
                {
                    SOPC_ClientHelper_BrowseResultReference resultReference;
                    OpcUa_ReferenceDescription* reference = &browseResp->Results[i].References[j];

                    //TODO check mallocs ?
                    resultReference.referenceTypeId = SOPC_NodeId_ToCString(&reference->ReferenceTypeId);
                    resultReference.nodeId = SOPC_NodeId_ToCString(&reference->NodeId.NodeId);
                    resultReference.browseName = SOPC_String_GetCString(&reference->BrowseName.Name);
                    resultReference.displayName = SOPC_String_GetCString(&reference->DisplayName.Text);
                    resultReference.isForward = reference->IsForward;
                    resultReference.nodeClass = reference->NodeClass;
                    SOPC_Array_Append(ctx->browseResults[i], resultReference);
                }
            }
        }
        ctx->finish = true;

        ctx->status = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);
        /* Signal that the response is available */
        status = Condition_SignalAll(&ctx->condition);
        assert(SOPC_STATUS_OK == status);
}

static void GenericCallbackHelper_BrowseNext(SOPC_StatusCode status,
                                         const void* response,
                                         uintptr_t responseContext)
{
        BrowseContext* ctx = (BrowseContext*) responseContext;
        const OpcUa_BrowseNextResponse* browseNextResp = (const OpcUa_BrowseNextResponse*) response;

        ctx->status = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);

        ctx->status = status;
        if (ctx->nbElements < browseNextResp->NoOfResults)
        {
            // TODO log error
            ctx->status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == ctx->status)
        {
            int32_t index = 0;
            for (int32_t i = 0; i < browseNextResp->NoOfResults && index < ctx->nbElements; i++)
            {
                bool found = false;
                while (index < ctx->nbElements && !found)
                {
                    if (0 < ctx->continuationPoints[index]->Length)
                    {
                        found = true;
                    }
                    else
                    {
                        index++;
                    }
                }

                ctx->statusCodes[index] = browseNextResp->Results[i].StatusCode;

                SOPC_ByteString_Delete(ctx->continuationPoints[index]);
                ctx->continuationPoints[index] = SOPC_ByteString_Create();
                SOPC_ByteString_Copy(ctx->continuationPoints[index],
                                   &browseNextResp->Results[i].ContinuationPoint);
                for (int32_t j = 0; j < browseNextResp->Results[i].NoOfReferences; j++)
                {
                    SOPC_ClientHelper_BrowseResultReference resultReference;
                    OpcUa_ReferenceDescription* reference = &browseNextResp->Results[i].References[j];

                    //TODO check mallocs ?
                    resultReference.referenceTypeId = SOPC_NodeId_ToCString(&reference->ReferenceTypeId);
                    resultReference.nodeId = SOPC_NodeId_ToCString(&reference->NodeId.NodeId);
                    resultReference.browseName = SOPC_String_GetCString(&reference->BrowseName.Name);
                    resultReference.displayName = SOPC_String_GetCString(&reference->DisplayName.Text);
                    resultReference.isForward = reference->IsForward;
                    resultReference.nodeClass = reference->NodeClass;
                    SOPC_Array_Append(ctx->browseResults[index], resultReference);
                }

                index += 1;
            }
        }
        ctx->finish = true;

        ctx->status = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == ctx->status);
        /* Signal that the response is available */
        status = Condition_SignalAll(&ctx->condition);
        assert(SOPC_STATUS_OK == status);
}

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
        GenericCallbackHelper_Read(status, response, responseContext);
    }
    else if (pEncType == &OpcUa_WriteResponse_EncodeableType)
    {
        GenericCallbackHelper_Write(status, response, responseContext);
    }
    else if (pEncType == &OpcUa_BrowseResponse_EncodeableType)
    {
        GenericCallbackHelper_Browse(status, response, responseContext);
    }
    else if (pEncType == &OpcUa_BrowseNextResponse_EncodeableType)
    {
        GenericCallbackHelper_BrowseNext(status, response, responseContext);
    }
}

static SOPC_ReturnStatus ReadHelper_Initialize(SOPC_ReturnStatus status, size_t nbElements,
        OpcUa_ReadValueId *nodesToRead, SOPC_ClientHelper_ReadValue *readValues,
        SOPC_DataValue **values)
{
    if (SOPC_STATUS_OK == status)
    {
        size_t i = 0;
        for (i = 0; i < nbElements && SOPC_STATUS_OK == status; i++)
        {
            OpcUa_ReadValueId_Initialize(&nodesToRead[i]);
            nodesToRead[i].AttributeId = readValues[i].attributeId;
            if (NULL == readValues[i].indexRange)
            {
                nodesToRead[i].IndexRange.Length = 0;
                nodesToRead[i].IndexRange.DoNotClear = true;
                nodesToRead[i].IndexRange.Data = NULL;
            }
            else
            {
                status = SOPC_String_CopyFromCString(&nodesToRead[i].IndexRange,
                        readValues[i].indexRange);
            }
            if (SOPC_STATUS_OK == status)
            {
                // create an instance of NodeId
                SOPC_NodeId *nodeId = SOPC_NodeId_FromCString(readValues[i].nodeId,
                        (int) strlen(readValues[i].nodeId));
                if (NULL == nodeId)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_NodeId_Copy(&nodesToRead[i].NodeId, nodeId);
                    SOPC_NodeId_Clear(nodeId);
                    SOPC_Free(nodeId);
                }
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        /* alloc values */
        int i = 0;
        for (i = 0; i < (int) nbElements && SOPC_STATUS_OK == status; i++)
        {
            values[i] = SOPC_Malloc(sizeof(SOPC_DataValue));
            if (NULL == values[i])
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            for (int j = 0; j < i + 1; j++)
            {
                SOPC_Free(values[j]);
            }
        }
    }
    return status;
}

int32_t SOPC_ClientHelper_Read(int32_t connectionId,
                               SOPC_ClientHelper_ReadValue* readValues,
                               size_t nbElements,
                               SOPC_DataValue** values)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (connectionId <= 0)
    {
        return -1;
    }
    if (NULL == readValues || nbElements < 1 || nbElements > INT32_MAX)
    {
        return -2;
    }
    if (NULL == values)
    {
        return -3;
    }

    OpcUa_ReadRequest* request = (OpcUa_ReadRequest*) SOPC_Malloc(sizeof(OpcUa_ReadRequest));
    if (NULL == request)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    ReadContext* ctx = (ReadContext*) SOPC_Malloc(sizeof(ReadContext));

    if (NULL == ctx)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_ReadRequest_Initialize(request);
        request->MaxAge = 0; /* Not manage by S2OPC */
        request->TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
        request->NoOfNodesToRead = (int32_t) nbElements; // check is done before
    }

    /* Set NodesToRead. This is deallocated by toolkit
       when call SOPC_LibSub_AsyncSendRequestOnSession */
    OpcUa_ReadValueId* nodesToRead = SOPC_Calloc(nbElements, sizeof(OpcUa_ReadValueId));
    if (NULL == nodesToRead)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

	status = ReadHelper_Initialize(status, nbElements, nodesToRead, readValues, values);
    /* set context */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ReadContext_Initialization(ctx);
        ctx->values = values;
        ctx->nbElements = request->NoOfNodesToRead;
        ctx->status = SOPC_STATUS_NOK;
        ctx->finish = false;
    }

    /* send request */
    SOPC_ReturnStatus statusMutex = SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        request->NodesToRead = nodesToRead;
        /* Prepare the synchronous context */
        Condition_Init(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);

        status =
            SOPC_ClientCommon_AsyncSendRequestOnSession((SOPC_LibSub_ConnectionId) connectionId, request, (uintptr_t) ctx);

        /* Wait for the response */
        while (SOPC_STATUS_OK == status && !ctx->finish)
        {
            statusMutex = Mutex_UnlockAndTimedWaitCond(&ctx->condition, &ctx->mutex, SYNCHRONOUS_REQUEST_TIMEOUT);
            assert(SOPC_STATUS_TIMEOUT != statusMutex); /* TODO return error */
            assert(SOPC_STATUS_OK == statusMutex);
        }
        status = ctx->status;

        /* Free the context */
        statusMutex = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Condition_Clear(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Clear(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
    }

    if (SOPC_STATUS_OUT_OF_MEMORY == status)
    {
        SOPC_Free(request);
        SOPC_Free(nodesToRead);
    }
    SOPC_Free(ctx);

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
    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_DEBUG, "SOPC_ClientHelper_CreateSubscription");
    int32_t res = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (connectionId <= 0)
    {
        return -1;
    }
    else if (NULL == callback)
    {
        return -2;
    }

    status = SOPC_ClientCommon_CreateSubscription((SOPC_LibSub_ConnectionId) connectionId, callback);
    if (SOPC_STATUS_OK != status)
    {
        return -100;
    }

    return res;
}

int32_t SOPC_ClientHelper_AddMonitoredItems(int32_t connectionId, char** nodeIds, size_t nbNodeIds)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (connectionId <= 0)
    {
        return -1;
    }
    else if (NULL == nodeIds || nbNodeIds <= 0 || nbNodeIds > INT32_MAX)
    {
        return -2;
    }
    else
    {
        /*
        for (size_t i = 0; i < nbNodeIds; i++)
        {
            if (NULL == nodeIds[i])
            {
                return -3 - (int32_t) i;
            }
        }*/
    }

    SOPC_LibSub_AttributeId* lAttrIds = SOPC_Calloc(nbNodeIds, sizeof(SOPC_LibSub_AttributeId));
    if (NULL == lAttrIds)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        for (size_t i = 0; i < nbNodeIds; ++i)
        {
            lAttrIds[i] = SOPC_LibSub_AttributeId_Value;
        }
    }

    SOPC_LibSub_DataId* lDataId = SOPC_Calloc(nbNodeIds, sizeof(SOPC_LibSub_DataId));
    if (NULL == lDataId)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientCommon_AddToSubscription(
                (SOPC_LibSub_ConnectionId) connectionId, (const char* const*) nodeIds, lAttrIds, (int32_t) nbNodeIds, lDataId);
    }
    if (SOPC_STATUS_OK != status)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not create monitored items.");
        return -100;
    }
    else
    {
        for (size_t i = 0; i < nbNodeIds; ++i)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Created MonIt for \"%s\" with data_id %" PRIu32 ".", nodeIds[i],
                        lDataId[i]);
        }
    }

    SOPC_Free(lAttrIds);
    SOPC_Free(lDataId);

    if (SOPC_STATUS_OK != status)
    {
        return 3;
    }

    return 0;
}

int32_t SOPC_ClientHelper_Unsubscribe(int32_t connectionId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 >= connectionId)
    {
        return -1;
    }

    status = SOPC_ClientCommon_DeleteSubscription((SOPC_LibSub_ConnectionId) connectionId);

    if (SOPC_STATUS_OK != status)
    {
        return -100;
    }

    return 0;
}

int32_t SOPC_ClientHelper_Disconnect(int32_t connectionId)
{
    if (connectionId <= 0)
    {
        return -1;
    }

    Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "Closing the connection %" PRIi32, connectionId);
    SOPC_ReturnStatus status = SOPC_ClientCommon_Disconnect((SOPC_LibSub_ConnectionId) connectionId);

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

static SOPC_ReturnStatus WriteHelper_InitialiazeValues(size_t nbElements, SOPC_ReturnStatus status,
        OpcUa_WriteValue *nodesToWrite, SOPC_ClientHelper_WriteValue *writeValues)
{
    for (size_t i = 0; i < nbElements && SOPC_STATUS_OK == status; i++)
    {
        OpcUa_WriteValue_Initialize(&nodesToWrite[i]);
        nodesToWrite[i].AttributeId = 13; // TODO find corresponding constant for value attribute
        SOPC_DataValue_Copy(&nodesToWrite[i].Value, writeValues[i].value);
        if (NULL == writeValues[i].indexRange)
        {
            nodesToWrite[i].IndexRange.Length = 0;
            nodesToWrite[i].IndexRange.DoNotClear = true;
            nodesToWrite[i].IndexRange.Data = NULL;
        }
        else
        {
            status = SOPC_String_CopyFromCString(&nodesToWrite[i].IndexRange,
                    writeValues[i].indexRange);
        }
        if (SOPC_STATUS_OK == status)
        {
            // create an instance of NodeId
            SOPC_NodeId *nodeId = SOPC_NodeId_FromCString(writeValues[i].nodeId,
                    (int) strlen(writeValues[i].nodeId));
            if (NULL == nodeId)
            {
                Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "nodeId NULL");
            }
            status = SOPC_NodeId_Copy(&nodesToWrite[i].NodeId, nodeId);
            SOPC_NodeId_Clear(nodeId);
            SOPC_Free(nodeId);
        }
    }
    return status;
}

int32_t SOPC_ClientHelper_Write(int32_t connectionId,
                                SOPC_ClientHelper_WriteValue* writeValues,
                                size_t nbElements,
                                SOPC_StatusCode* writeResults)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (connectionId <= 0)
    {
        return -1;
    }
    if (NULL == writeValues || nbElements < 1 || nbElements > INT32_MAX)
    {
        return -2;
    }
    if (NULL == writeResults)
    {
        return -3;
    }

    OpcUa_WriteRequest* request = (OpcUa_WriteRequest*) SOPC_Malloc(sizeof(OpcUa_WriteRequest));
    if (NULL == request)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    WriteContext* ctx = (WriteContext*) SOPC_Malloc(sizeof(WriteContext));
    if (NULL == ctx)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_WriteRequest_Initialize(request);
        request->NoOfNodesToWrite = (int32_t) nbElements;
    }

    OpcUa_WriteValue* nodesToWrite = SOPC_Calloc(nbElements, sizeof(OpcUa_WriteValue));
    if (NULL == nodesToWrite)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = WriteHelper_InitialiazeValues(nbElements, status, nodesToWrite, writeValues);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_WriteContext_Initialization(ctx);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set context */
        ctx->nbElements = request->NoOfNodesToWrite;
        ctx->writeResults = (SOPC_StatusCode*) SOPC_Malloc(sizeof(SOPC_StatusCode) * (size_t) request->NoOfNodesToWrite);
        if (NULL == ctx->writeResults)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        ctx->status = SOPC_STATUS_NOK;
        ctx->finish = false;
    }

    if (SOPC_STATUS_OK == status)
    {
        request->NodesToWrite = nodesToWrite;
        /* Prepare the synchronous context */
        SOPC_ReturnStatus statusMutex = Condition_Init(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);

        status = SOPC_ClientCommon_AsyncSendRequestOnSession((SOPC_LibSub_ConnectionId) connectionId, request, (uintptr_t) ctx);

        /* Wait for the response */
        while (SOPC_STATUS_OK == status && !ctx->finish)
        {
            statusMutex = Mutex_UnlockAndTimedWaitCond(&ctx->condition, &ctx->mutex, SYNCHRONOUS_REQUEST_TIMEOUT);
            assert(SOPC_STATUS_TIMEOUT != statusMutex);
            assert(SOPC_STATUS_OK == statusMutex);
        }
        status = ctx->status;

        /* fill write results */
        for (int i=0; i < ctx->nbElements; i++)
        {
            writeResults[i] = ctx->writeResults[i];
        }

        /* Free the context */
        statusMutex = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Condition_Clear(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Clear(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        SOPC_Free(ctx->writeResults);
    }

    if (SOPC_STATUS_OUT_OF_MEMORY == status)
    {
        SOPC_Free(request);
        SOPC_Free(nodesToWrite);
    }
    SOPC_Free(ctx);

    if (SOPC_STATUS_OK == status)
    {
        return 0;
    }
    else
    {
        return -100;
    }
}

static SOPC_ReturnStatus BrowseHelper_InitializeContinuationPoints(size_t nbElements,
        SOPC_ReturnStatus status, SOPC_ByteString **continuationPointsArray)
{
    size_t i = 0;
    for (; i < nbElements && SOPC_STATUS_OK == status; i++)
    {
        continuationPointsArray[i] = SOPC_ByteString_Create();
        if (NULL == continuationPointsArray)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        for (size_t j = 0; j < i; j++)
        {
            SOPC_ByteString_Delete(continuationPointsArray[j]);
            continuationPointsArray[j] = NULL;
        }
    }
    return status;
}

static SOPC_ReturnStatus BrowseHelper_InitializeNodesToBrowse(size_t nbElements, SOPC_ReturnStatus status,
        OpcUa_BrowseDescription *nodesToBrowse, SOPC_ClientHelper_BrowseRequest *browseRequests)
{
    for (size_t i = 0; i < nbElements && SOPC_STATUS_OK == status; i++)
    {
        OpcUa_BrowseDescription_Initialize(&nodesToBrowse[i]);
        // create an instance of NodeId
        SOPC_NodeId *nodeId = SOPC_NodeId_FromCString(browseRequests[i].nodeId,
                (int) strlen(browseRequests[i].nodeId));
        if (NULL == nodeId)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "nodeId NULL");
        }
        status = SOPC_NodeId_Copy(&nodesToBrowse[i].NodeId, nodeId);
        SOPC_NodeId_Clear(nodeId);
        SOPC_Free(nodeId);
        if (SOPC_STATUS_OK == status)
        {
            // create an instance of NodeId
            SOPC_NodeId *refNodeId = SOPC_NodeId_FromCString(browseRequests[i].referenceTypeId,
                    (int) strlen(browseRequests[i].referenceTypeId));
            if (NULL == refNodeId)
            {
                Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_INFO, "refNodeId NULL");
            }
            status = SOPC_NodeId_Copy(&nodesToBrowse[i].ReferenceTypeId, refNodeId);
            SOPC_NodeId_Clear(refNodeId);
            SOPC_Free(refNodeId);
        }
        if (SOPC_STATUS_OK == status)
        {
            nodesToBrowse[i].BrowseDirection = browseRequests[i].direction;
            nodesToBrowse[i].IncludeSubtypes = browseRequests[i].includeSubtypes;
            nodesToBrowse[i].NodeClassMask = 0; //all //TODO correct ?
            nodesToBrowse[i].ResultMask = 0x3f; //all //TODO correct ?
        }
    }
    return status;
}

static SOPC_ReturnStatus BrowseHelper_InitializeBrowseResults(size_t nbElements, SOPC_ReturnStatus status,
        SOPC_Array **browseResultsListArray)
{
    size_t i = 0;
    for (; i < nbElements && SOPC_STATUS_OK == status; i++)
    {
        browseResultsListArray[i] = SOPC_Array_Create(
                sizeof(SOPC_ClientHelper_BrowseResultReference), 10, SOPC_Free);
        if (NULL == browseResultsListArray)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        for (size_t j = 0; j < i; j++)
        {
            SOPC_Array_Delete(browseResultsListArray[j]);
            browseResultsListArray[j] = NULL;
        }
    }
    return status;
}

int32_t SOPC_ClientHelper_Browse(int32_t connectionId,
                                 SOPC_ClientHelper_BrowseRequest* browseRequests,
                                 size_t nbElements,
                                 SOPC_ClientHelper_BrowseResult* browseResults)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (connectionId <= 0)
    {
        return -1;
    }
    if (NULL == browseRequests || nbElements < 1 || nbElements > INT32_MAX)
    {
        return -2;
    }
    if (NULL == browseResults)
    {
        return -3;
    }
    OpcUa_BrowseRequest* request = (OpcUa_BrowseRequest*)
        SOPC_Calloc(1, sizeof(OpcUa_BrowseRequest));
    if (NULL == request)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    BrowseContext* ctx = (BrowseContext*) SOPC_Calloc(1, sizeof(BrowseContext));
    if (NULL == ctx)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Convert browseRequests to an actual request */
    OpcUa_BrowseDescription* nodesToBrowse = SOPC_Calloc(nbElements, sizeof(OpcUa_BrowseDescription));
    if (NULL == nodesToBrowse)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Create array of singly linked lists, containing browse results */
    SOPC_Array** browseResultsListArray = (SOPC_Array**)
        SOPC_Calloc(nbElements, sizeof(SOPC_Array*));
    if (NULL == browseResultsListArray)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* create an array of status code to store request results */
    SOPC_StatusCode* statusCodes = SOPC_Calloc(nbElements, sizeof(SOPC_StatusCode));
    if (NULL == statusCodes)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = BrowseHelper_InitializeBrowseResults(nbElements, status, browseResultsListArray);
    }

    /* create array of continuationPoints */
    SOPC_ByteString** continuationPointsArray = (SOPC_ByteString**)
        SOPC_Calloc(nbElements, sizeof(SOPC_ByteString*));
    if (NULL == continuationPointsArray)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = BrowseHelper_InitializeContinuationPoints(nbElements, status,
                continuationPointsArray);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = BrowseHelper_InitializeNodesToBrowse(nbElements, status, nodesToBrowse,
                browseRequests);
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_BrowseRequest_Initialize(request);
        OpcUa_ViewDescription_Initialize(&request->View);
        request->NodesToBrowse = nodesToBrowse;
        request->NoOfNodesToBrowse = (int32_t) nbElements;
        request->RequestedMaxReferencesPerNode = 0; //unlimited
    }

    /* Create context */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_BrowseContext_Initialization(ctx);
        ctx->nbElements = (int32_t) nbElements;
        ctx->statusCodes = statusCodes;
        ctx->browseResults = browseResultsListArray;
        ctx->continuationPoints = continuationPointsArray;
    }
    /* Send Browse Request */
    if (SOPC_STATUS_OK == status)
    {
        /* Prepare the synchronous context */
        SOPC_ReturnStatus statusMutex = Condition_Init(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);

        status = SOPC_ClientCommon_AsyncSendRequestOnSession((SOPC_LibSub_ConnectionId) connectionId, request, (uintptr_t) ctx);

        /* Wait for the response */
        while (SOPC_STATUS_OK == status && !ctx->finish)
        {
            statusMutex = Mutex_UnlockAndTimedWaitCond(&ctx->condition, &ctx->mutex, SYNCHRONOUS_REQUEST_TIMEOUT);
            assert(SOPC_STATUS_TIMEOUT != statusMutex);
            assert(SOPC_STATUS_OK == statusMutex);
        }
        status = ctx->status;

        /* Free the context */
        statusMutex = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Condition_Clear(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Clear(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
    }

    /* Send Browse Next Request until there are no continuation or limit is reached */
    bool containsContinuationPoint = ContainsContinuationPoints(continuationPointsArray, nbElements);
    if (SOPC_STATUS_OK == status && containsContinuationPoint)
    {
        for (int i = 0; i < MAX_BROWSENEXT_REQUESTS && containsContinuationPoint && SOPC_STATUS_OK == status; i++)
        {
            status = BrowseNext(connectionId, statusCodes, browseResultsListArray, nbElements, continuationPointsArray);
            containsContinuationPoint = ContainsContinuationPoints(continuationPointsArray, nbElements);
        }
    }

    /* process browseResultsListArray */
    for (size_t i = 0; i < nbElements; i++)
    {
        browseResults[i].statusCode = statusCodes[i];
        browseResults[i].nbOfReferences = (int32_t) SOPC_Array_Size(browseResultsListArray[i]);
        browseResults[i].references = SOPC_Array_Into_Raw(browseResultsListArray[i]);
    }

    /* Free memory */
    if (SOPC_STATUS_OUT_OF_MEMORY == status)
    {
        SOPC_Free(nodesToBrowse);
        SOPC_Free(request);
    }
    SOPC_Free(statusCodes);
    SOPC_Free(browseResultsListArray);

    for (size_t i = 0; i < nbElements; i++)
    {
        SOPC_ByteString_Delete(continuationPointsArray[i]);
    }
    SOPC_Free(continuationPointsArray);

    SOPC_Free(ctx);

    if (SOPC_STATUS_OK == status)
    {
        return 0;
    }
    else
    {
        return -100;
    }
}

static SOPC_ReturnStatus BrowseNext(int32_t connectionId,
                                    SOPC_StatusCode* statusCodes,
                                    SOPC_Array** browseResultsListArray,
                                    size_t nbElements,
                                    SOPC_ByteString** continuationPoints)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 > connectionId || NULL == statusCodes
        || NULL == browseResultsListArray || 1 > nbElements || NULL == continuationPoints)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* allocate memory */
    OpcUa_BrowseNextRequest* request = (OpcUa_BrowseNextRequest*)
        SOPC_Calloc(1, sizeof(OpcUa_BrowseNextRequest));
    if (NULL == request)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    BrowseContext* ctx = (BrowseContext*) SOPC_Calloc(1, sizeof(BrowseContext));
    if (NULL == ctx)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    // TODO alloc correct size
    SOPC_ByteString* nextContinuationPoints = SOPC_Calloc(nbElements, sizeof(SOPC_ByteString));

    if (NULL == nextContinuationPoints)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    int32_t count = 0;
    if (SOPC_STATUS_OK == status)
    {
        for (int32_t i = 0; i < (int32_t) nbElements; i++)
        {
            if (NULL != continuationPoints[i] && 0 < continuationPoints[i]->Length)
            {
                SOPC_ByteString_Copy(&nextContinuationPoints[count], continuationPoints[i]);
                count++;
            }
        }
    }

    /* craft request */
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_BrowseNextRequest_Initialize(request);
        request->ReleaseContinuationPoints = false;
        request->NoOfContinuationPoints = count;
        request->ContinuationPoints = nextContinuationPoints;
    }
    /* setup context */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_BrowseContext_Initialization(ctx);
        ctx->nbElements = (int32_t) nbElements;
        ctx->statusCodes = statusCodes;
        ctx->browseResults = browseResultsListArray;
        ctx->continuationPoints = continuationPoints;
    }
    /* send request */
    if (SOPC_STATUS_OK == status)
    {
        /* Prepare the synchronous context */
        SOPC_ReturnStatus statusMutex = Condition_Init(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Lock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);

        status =
            SOPC_ClientCommon_AsyncSendRequestOnSession((SOPC_LibSub_ConnectionId) connectionId, request, (uintptr_t) ctx);

        /* Wait for the response */
        while (SOPC_STATUS_OK == status && !ctx->finish)
        {
            statusMutex = Mutex_UnlockAndTimedWaitCond(&ctx->condition, &ctx->mutex, SYNCHRONOUS_REQUEST_TIMEOUT);
            assert(SOPC_STATUS_TIMEOUT != statusMutex);
            assert(SOPC_STATUS_OK == statusMutex);
        }
        status = ctx->status;

        /* Free the context */
        statusMutex = Mutex_Unlock(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Condition_Clear(&ctx->condition);
        assert(SOPC_STATUS_OK == statusMutex);
        statusMutex = Mutex_Clear(&ctx->mutex);
        assert(SOPC_STATUS_OK == statusMutex);
    }

    /* free memory */
    SOPC_Free(ctx);

    return status;
}

static bool ContainsContinuationPoints(SOPC_ByteString** continuationPointsArray, size_t nbElements)
{
    bool result = false;

    if (NULL == continuationPointsArray)
    {
        return false;
    }

    for (size_t i = 0; i < nbElements && !result; i++)
    {
        result = result || (NULL != continuationPointsArray[i] && 0 < continuationPointsArray[i]->Length);
    }

    return result;
}
