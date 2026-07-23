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
 * \brief Helpers to validate OPC UA service response
 */

#include <inttypes.h>
#if defined(_MSC_VER)
#include <string.h>

#include "sopc_assert.h"
#endif

#include "libs2opc_response_helper.h"

#include "opcua_statuscodes.h"
#include "sopc_builtintypes.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

typedef SOPC_ReturnStatus (*SOPC_ServiceResponse_GetCount_Fct)(const void* msg, int32_t* outCount);

typedef SOPC_ReturnStatus (*SOPC_ServiceResponse_CustomSubResultCountCheck_Fct)(
    const SOPC_ServiceResponse_ReqContext* reqCtx,
    const void* response);

typedef struct SOPC_ServiceResponse_CountDescriptor
{
    SOPC_EncodeableType* requestType;
    SOPC_EncodeableType* responseType;
    SOPC_ServiceResponse_GetCount_Fct getRequestCount;
    SOPC_ServiceResponse_GetCount_Fct getResponseCount;
    const char* serviceName;
    SOPC_ServiceResponse_CustomSubResultCountCheck_Fct customSubResultCountCheck;
} SOPC_ServiceResponse_CountDescriptor;

struct SOPC_ServiceResponse_ReqContext
{
    const SOPC_ServiceResponse_CountDescriptor* descriptor;
    int32_t operationCount;
    uint32_t browseMaxReferencesPerNode;
    int32_t* callNoOfInputArguments;
    int32_t callNoOfMethodsToCall;
};

#define SOPC_SERVICE_RESPONSE_GET_COUNT(msgType, field)                                                  \
    static SOPC_ReturnStatus SOPC_ServiceResponse_GetCount_##msgType(const void* msg, int32_t* outCount) \
    {                                                                                                    \
        *outCount = ((const msgType*) msg)->field;                                                       \
        return SOPC_STATUS_OK;                                                                           \
    }

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_ReadRequest, NoOfNodesToRead)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_ReadResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_WriteRequest, NoOfNodesToWrite)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_WriteResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_BrowseRequest, NoOfNodesToBrowse)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_BrowseResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_BrowseNextRequest, NoOfContinuationPoints)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_BrowseNextResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_TranslateBrowsePathsToNodeIdsRequest, NoOfBrowsePaths)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_TranslateBrowsePathsToNodeIdsResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_CallRequest, NoOfMethodsToCall)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_CallResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_HistoryReadRequest, NoOfNodesToRead)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_HistoryReadResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_AddNodesRequest, NoOfNodesToAdd)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_AddNodesResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteNodesRequest, NoOfNodesToDelete)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteNodesResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_CreateMonitoredItemsRequest, NoOfItemsToCreate)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_CreateMonitoredItemsResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_ModifyMonitoredItemsRequest, NoOfItemsToModify)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_ModifyMonitoredItemsResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_SetMonitoringModeRequest, NoOfMonitoredItemIds)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_SetMonitoringModeResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteMonitoredItemsRequest, NoOfMonitoredItemIds)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteMonitoredItemsResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_SetPublishingModeRequest, NoOfSubscriptionIds)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_SetPublishingModeResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_TransferSubscriptionsRequest, NoOfSubscriptionIds)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_TransferSubscriptionsResponse, NoOfResults)

SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteSubscriptionsRequest, NoOfSubscriptionIds)
SOPC_SERVICE_RESPONSE_GET_COUNT(OpcUa_DeleteSubscriptionsResponse, NoOfResults)

#undef SOPC_SERVICE_RESPONSE_GET_COUNT

static void SOPC_ServiceResponse_InternalFreeReqContext(SOPC_ServiceResponse_ReqContext** ppReqCtx)
{
    if (NULL != ppReqCtx && NULL != *ppReqCtx)
    {
        SOPC_Free((*ppReqCtx)->callNoOfInputArguments);
        SOPC_Free(*ppReqCtx);
        *ppReqCtx = NULL;
    }
}

static SOPC_ReturnStatus SOPC_ServiceResponse_CheckBrowseSubResultCounts(const SOPC_ServiceResponse_ReqContext* reqCtx,
                                                                         const void* response)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const OpcUa_BrowseResponse* browseResp = (const OpcUa_BrowseResponse*) response;

    if (0 < reqCtx->browseMaxReferencesPerNode)
    {
        const uint32_t maxReferencesPerNode = reqCtx->browseMaxReferencesPerNode;

        for (int32_t i = 0; SOPC_STATUS_OK == status && i < browseResp->NoOfResults; ++i)
        {
            const int32_t noOfReferences = browseResp->Results[i].NoOfReferences;

            if ((uint32_t) noOfReferences > maxReferencesPerNode)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "Browse: Results[%" PRIi32 "].NoOfReferences=%" PRIi32
                                         " exceeds RequestedMaxReferencesPerNode=%" PRIu32,
                                         i, noOfReferences, maxReferencesPerNode);
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
    }

    return status;
}

static SOPC_ReturnStatus SOPC_ServiceResponse_CheckCallSubResultCounts(const SOPC_ServiceResponse_ReqContext* reqCtx,
                                                                       const void* response)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const OpcUa_CallResponse* callResp = (const OpcUa_CallResponse*) response;

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < callResp->NoOfResults; ++i)
    {
        const int32_t noOfInputArguments = (NULL != reqCtx->callNoOfInputArguments && i < reqCtx->callNoOfMethodsToCall)
                                               ? reqCtx->callNoOfInputArguments[i]
                                               : 0;
        const OpcUa_CallMethodResult* methodResult = &callResp->Results[i];
        const int32_t noOfInputArgResults = methodResult->NoOfInputArgumentResults;
        const int32_t noOfOutputArguments = methodResult->NoOfOutputArguments;
        const SOPC_StatusCode methodStatus = methodResult->StatusCode;

        if (noOfInputArgResults < 0 || noOfOutputArguments < 0)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Call: Results[%" PRIi32 "] negative InputArgumentResults or OutputArguments count",
                                   i);
            status = SOPC_STATUS_NOK;
        }
        else if (noOfInputArgResults > noOfInputArguments)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Call: Results[%" PRIi32 "].NoOfInputArgumentResults=%" PRIi32
                                     " exceeds NoOfInputArguments=%" PRIi32,
                                     i, noOfInputArgResults, noOfInputArguments);
            status = SOPC_STATUS_INVALID_STATE;
        }
        else if (OpcUa_BadInvalidArgument == methodStatus)
        {
            if (noOfInputArgResults != noOfInputArguments)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "Call: Results[%" PRIi32
                                         "] Bad_InvalidArgument expects NoOfInputArgumentResults=%" PRIi32
                                         ", got %" PRIi32,
                                         i, noOfInputArguments, noOfInputArgResults);
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        else if (0 != noOfInputArgResults)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Call: Results[%" PRIi32
                                     "] InputArgumentResults shall be empty unless "
                                     "StatusCode is Bad_InvalidArgument (got %" PRIi32 ")",
                                     i, noOfInputArgResults);
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status && SOPC_IsBadStatus(methodStatus) && 0 != noOfOutputArguments)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Call: Results[%" PRIi32
                                     "] OutputArguments shall be empty when method "
                                     "StatusCode severity is Bad (got %" PRIi32 ")",
                                     i, noOfOutputArguments);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    return status;
}

#if defined(_MSC_VER)

#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_SLOT_COUNT 16U

static SOPC_ServiceResponse_CountDescriptor
    SOPC_gServiceResponseCountDescriptors[SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_SLOT_COUNT];
static bool SOPC_gServiceResponseCountDescriptorsInitialized = false;

#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_OPEN                                                    \
    static void SOPC_ServiceResponse_InitCountDescriptors(void)                                         \
    {                                                                                                   \
        if (SOPC_gServiceResponseCountDescriptorsInitialized)                                           \
        {                                                                                               \
            return;                                                                                     \
        }                                                                                               \
                                                                                                        \
        /* Note: cannot use a file-scope static const initializer on Windows (MSVC C2099): addresses of \
         *       static functions are not constant expressions (same constraint as ticket #911). */     \
        const SOPC_ServiceResponse_CountDescriptor entries[] = {
#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_CLOSE                                                       \
    ;                                                                                                       \
    SOPC_ASSERT(sizeof(entries) / sizeof(entries[0]) == SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_SLOT_COUNT); \
    memcpy(SOPC_gServiceResponseCountDescriptors, entries, sizeof(entries));                                \
    SOPC_gServiceResponseCountDescriptorsInitialized = true;                                                \
    }

#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_COUNT SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_SLOT_COUNT

#else /* !defined(_MSC_VER) */

#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_OPEN \
    static const SOPC_ServiceResponse_CountDescriptor SOPC_gServiceResponseCountDescriptors[] = {
#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_CLOSE \
    }                                                 \
    ;                                                 \
    static void SOPC_ServiceResponse_InitCountDescriptors(void) {}

#define SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_COUNT \
    (sizeof(SOPC_gServiceResponseCountDescriptors) / sizeof(SOPC_gServiceResponseCountDescriptors[0]))

#endif /* defined(_MSC_VER) */

SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_OPEN{&OpcUa_ReadRequest_EncodeableType,
                                             &OpcUa_ReadResponse_EncodeableType,
                                             SOPC_ServiceResponse_GetCount_OpcUa_ReadRequest,
                                             SOPC_ServiceResponse_GetCount_OpcUa_ReadResponse,
                                             "Read",
                                             NULL},
    {&OpcUa_WriteRequest_EncodeableType,
     &OpcUa_WriteResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_WriteRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_WriteResponse,
     "Write",
     NULL},
    {&OpcUa_BrowseRequest_EncodeableType,
     &OpcUa_BrowseResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_BrowseRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_BrowseResponse,
     "Browse",
     SOPC_ServiceResponse_CheckBrowseSubResultCounts},
    {&OpcUa_BrowseNextRequest_EncodeableType,
     &OpcUa_BrowseNextResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_BrowseNextRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_BrowseNextResponse,
     "BrowseNext",
     NULL},
    {&OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
     &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_TranslateBrowsePathsToNodeIdsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_TranslateBrowsePathsToNodeIdsResponse,
     "TranslateBrowsePathsToNodeIds",
     NULL},
    {&OpcUa_CallRequest_EncodeableType,
     &OpcUa_CallResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_CallRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_CallResponse,
     "Call",
     SOPC_ServiceResponse_CheckCallSubResultCounts},
    {&OpcUa_HistoryReadRequest_EncodeableType,
     &OpcUa_HistoryReadResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_HistoryReadRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_HistoryReadResponse,
     "HistoryRead",
     NULL},
    {&OpcUa_AddNodesRequest_EncodeableType,
     &OpcUa_AddNodesResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_AddNodesRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_AddNodesResponse,
     "AddNodes",
     NULL},
    {&OpcUa_DeleteNodesRequest_EncodeableType,
     &OpcUa_DeleteNodesResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteNodesRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteNodesResponse,
     "DeleteNodes",
     NULL},
    {&OpcUa_CreateMonitoredItemsRequest_EncodeableType,
     &OpcUa_CreateMonitoredItemsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_CreateMonitoredItemsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_CreateMonitoredItemsResponse,
     "CreateMonitoredItems",
     NULL},
    {&OpcUa_ModifyMonitoredItemsRequest_EncodeableType,
     &OpcUa_ModifyMonitoredItemsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_ModifyMonitoredItemsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_ModifyMonitoredItemsResponse,
     "ModifyMonitoredItems",
     NULL},
    {&OpcUa_SetMonitoringModeRequest_EncodeableType,
     &OpcUa_SetMonitoringModeResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_SetMonitoringModeRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_SetMonitoringModeResponse,
     "SetMonitoringMode",
     NULL},
    {&OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
     &OpcUa_DeleteMonitoredItemsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteMonitoredItemsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteMonitoredItemsResponse,
     "DeleteMonitoredItems",
     NULL},
    {&OpcUa_SetPublishingModeRequest_EncodeableType,
     &OpcUa_SetPublishingModeResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_SetPublishingModeRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_SetPublishingModeResponse,
     "SetPublishingMode",
     NULL},
    {&OpcUa_TransferSubscriptionsRequest_EncodeableType,
     &OpcUa_TransferSubscriptionsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_TransferSubscriptionsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_TransferSubscriptionsResponse,
     "TransferSubscriptions",
     NULL},
    {&OpcUa_DeleteSubscriptionsRequest_EncodeableType,
     &OpcUa_DeleteSubscriptionsResponse_EncodeableType,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteSubscriptionsRequest,
     SOPC_ServiceResponse_GetCount_OpcUa_DeleteSubscriptionsResponse,
     "DeleteSubscriptions",
     NULL},
    SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_CLOSE

#undef SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_OPEN
#undef SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTORS_CLOSE

    static const SOPC_ServiceResponse_CountDescriptor*
    SOPC_ServiceResponse_FindDescriptor(SOPC_EncodeableType* requestType)
{
    const SOPC_ServiceResponse_CountDescriptor* result = NULL;

    SOPC_ServiceResponse_InitCountDescriptors();

    for (size_t i = 0; NULL == result && i < SOPC_SERVICE_RESPONSE_COUNT_DESCRIPTOR_COUNT; ++i)
    {
        if (requestType == SOPC_gServiceResponseCountDescriptors[i].requestType)
        {
            result = &SOPC_gServiceResponseCountDescriptors[i];
        }
    }

    return result;
}

bool SOPC_ServiceResponse_CheckResultCountMatches(int32_t responseCount,
                                                  int32_t expectedCount,
                                                  const char* contextLabel)
{
    bool matches = false;

    if (responseCount == expectedCount && 0 <= responseCount)
    {
        matches = true;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "%s: number of results %" PRIi32 " != expected %" PRIi32,
                               contextLabel, responseCount, expectedCount);
    }

    return matches;
}

SOPC_ServiceResponse_ReqContext* SOPC_ServiceResponse_CreateRequestContext(const void* request)
{
    SOPC_ServiceResponse_ReqContext* reqCtx = NULL;
    SOPC_EncodeableType* requestType = NULL;
    const SOPC_ServiceResponse_CountDescriptor* descriptor = NULL;

    if (NULL == request)
    {
        return NULL;
    }

    requestType = *(SOPC_EncodeableType* const*) request;
    descriptor = SOPC_ServiceResponse_FindDescriptor(requestType);
    if (NULL == descriptor)
    {
        return NULL;
    }

    reqCtx = SOPC_Calloc(1, sizeof(*reqCtx));
    if (NULL == reqCtx)
    {
        return NULL;
    }

    reqCtx->descriptor = descriptor;
    if (SOPC_STATUS_OK != descriptor->getRequestCount(request, &reqCtx->operationCount))
    {
        SOPC_ServiceResponse_InternalFreeReqContext(&reqCtx);
        return NULL;
    }

    if (&OpcUa_BrowseRequest_EncodeableType == descriptor->requestType)
    {
        reqCtx->browseMaxReferencesPerNode = ((const OpcUa_BrowseRequest*) request)->RequestedMaxReferencesPerNode;
    }
    else if (&OpcUa_CallRequest_EncodeableType == descriptor->requestType)
    {
        const OpcUa_CallRequest* callReq = (const OpcUa_CallRequest*) request;

        reqCtx->callNoOfMethodsToCall = callReq->NoOfMethodsToCall;
        if (0 < callReq->NoOfMethodsToCall)
        {
            reqCtx->callNoOfInputArguments =
                SOPC_Calloc((size_t) callReq->NoOfMethodsToCall, sizeof(*reqCtx->callNoOfInputArguments));
            if (NULL == reqCtx->callNoOfInputArguments)
            {
                SOPC_ServiceResponse_InternalFreeReqContext(&reqCtx);
                return NULL;
            }
            for (int32_t i = 0; i < callReq->NoOfMethodsToCall; ++i)
            {
                reqCtx->callNoOfInputArguments[i] = callReq->MethodsToCall[i].NoOfInputArguments;
            }
        }
    }

    return reqCtx;
}

void SOPC_ServiceResponse_DeleteRequestContext(SOPC_ServiceResponse_ReqContext** ppReqCtx)
{
    SOPC_ServiceResponse_InternalFreeReqContext(ppReqCtx);
}

SOPC_ReturnStatus SOPC_ServiceResponse_ValidateResultCount(SOPC_ServiceResponse_ReqContext** ppReqCtx,
                                                           const void* response)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ServiceResponse_ReqContext* reqCtx = NULL;
    SOPC_EncodeableType* responseType = NULL;
    int32_t responseCount = 0;

    if (NULL == ppReqCtx || NULL == *ppReqCtx || NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    reqCtx = *ppReqCtx;

    if (!SOPC_IsGoodStatus(((const OpcUa_ServiceFault*) response)->ResponseHeader.ServiceResult))
    {
        status = SOPC_STATUS_WOULD_BLOCK;
    }

    if (SOPC_STATUS_OK == status)
    {
        responseType = *(SOPC_EncodeableType* const*) response;

        if (responseType != reqCtx->descriptor->responseType)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_ServiceResponse_ValidateResultCount: %s: response type %s does not match expected type %s",
                reqCtx->descriptor->serviceName, (NULL != responseType) ? responseType->TypeName : "NULL",
                reqCtx->descriptor->responseType->TypeName);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            status = reqCtx->descriptor->getResponseCount(response, &responseCount);
            if (SOPC_STATUS_OK == status && !SOPC_ServiceResponse_CheckResultCountMatches(
                                                responseCount, reqCtx->operationCount, reqCtx->descriptor->serviceName))
            {
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK == status && NULL != reqCtx->descriptor->customSubResultCountCheck)
            {
                status = reqCtx->descriptor->customSubResultCountCheck(reqCtx, response);
            }
        }
    }

    SOPC_ServiceResponse_InternalFreeReqContext(ppReqCtx);
    return status;
}
