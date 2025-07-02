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
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sks_device_push_methods.h"
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

#include "device_config.h"
#include "device_server.h"
#include "helpers.h"

/* These variables could be stored in a struct Server_Context, which is then passed to all functions.
 * This would mimic class instances and avoid global variables.
 */
int32_t serverOnline = 0;

typedef enum PublisherMethodStatus
{
    PUBLISHER_METHOD_NOT_TRIGGERED = 0,
    PUBLISHER_METHOD_IN_PROGRESS = 1,
    PUBLISHER_METHOD_SUCCESS = 2,
    PUBLISHER_METHOD_ERROR = 3,
} PublisherMethodStatus;

/* SOPC_ReturnStatus Server_SetRuntimeVariables(void); */

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

/**
 * Add OPC UA demo methods into the given method call manager
 */
static SOPC_ReturnStatus Server_AddMethods(SOPC_MethodCallManager* mcm)
{
    SOPC_NodeId methodTypeId;
    SOPC_NodeId methodId;
    SOPC_NodeId_Initialize(&methodTypeId);
    SOPC_NodeId_Initialize(&methodId);

    methodId.Namespace = 1;
    methodId.Data.Numeric = OpcUaId_PublishSubscribeType_SetSecurityKeys;
    methodTypeId.Data.Numeric = OpcUaId_PublishSubscribeType_SetSecurityKeys;

    return SOPC_MethodCallManager_AddMethodWithType(mcm, &methodId, &methodTypeId,
                                                    &SOPC_Method_Func_PublishSubscribe_SetSecurityKeys, NULL, NULL);
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

    status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    // Server certificates configuration
    if (SOPC_STATUS_OK == status)
    {
        // TODO: make copy in build with cmake
        status = SOPC_ServerConfigHelper_ConfigureFromXML(SERVER_CONFIG_XML, ADDRESS_SPACE_PATH, NULL, NULL);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the server key user password callback");
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
