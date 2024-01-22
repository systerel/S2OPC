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

#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_request_builder.h"

#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/* Default publish period */
#ifndef SOPC_DEFAULT_PUBLISH_PERIOD_MS
#define SOPC_DEFAULT_PUBLISH_PERIOD_MS 500
#endif
/* Default max keep alive count */
#ifndef SOPC_DEFAULT_MAX_KEEP_ALIVE_COUNT
#define SOPC_DEFAULT_MAX_KEEP_ALIVE_COUNT 3
#endif
/* Default max lifetime Count of subscriptions */
#ifndef SOPC_DEFAULT_MAX_LIFETIME_COUNT
#define SOPC_DEFAULT_MAX_LIFETIME_COUNT 10
#endif

/* Default max lifetime Count of subscriptions */
#ifndef SOPC_DEFAULT_MAX_NOTIFS_PER_PUBLISH
#define SOPC_DEFAULT_MAX_NOTIFS_PER_PUBLISH 1000
#endif

/* Default sampling interval of monitored items */
#ifndef SOPC_DEFAULT_MI_SAMPLING_INTERVAL_MS
#define SOPC_DEFAULT_MI_SAMPLING_INTERVAL_MS (-1)
#endif

/* Default queue size of monitored items */
#ifndef SOPC_DEFAULT_MI_QUEUE_SIZE
#define SOPC_DEFAULT_MI_QUEUE_SIZE 100
#endif

/* Default discard oldest flag of monitored items */
#ifndef SOPC_DEFAULT_MI_DISCARD_OLDEST
#define SOPC_DEFAULT_MI_DISCARD_OLDEST true
#endif

// Macro used to check if msgPtr is valid and element index is valid in this message
#define CHECK_ELEMENT_EXISTS(msgPtr, fieldNoOf, index) \
    (NULL != (msgPtr) && ((msgPtr)->fieldNoOf) > 0 && (index) < (size_t)((msgPtr)->fieldNoOf))

static inline SOPC_AttributeId SOPC_TypeHelperInternal_CheckAttributeId(SOPC_AttributeId attrId)
{
    switch (attrId)
    {
    case SOPC_AttributeId_Invalid:
    case SOPC_AttributeId_NodeId:
    case SOPC_AttributeId_NodeClass:
    case SOPC_AttributeId_BrowseName:
    case SOPC_AttributeId_DisplayName:
    case SOPC_AttributeId_Description:
    case SOPC_AttributeId_WriteMask:
    case SOPC_AttributeId_UserWriteMask:
    case SOPC_AttributeId_IsAbstract:
    case SOPC_AttributeId_Symmetric:
    case SOPC_AttributeId_InverseName:
    case SOPC_AttributeId_ContainsNoLoops:
    case SOPC_AttributeId_EventNotifier:
    case SOPC_AttributeId_Value:
    case SOPC_AttributeId_DataType:
    case SOPC_AttributeId_ValueRank:
    case SOPC_AttributeId_ArrayDimensions:
    case SOPC_AttributeId_AccessLevel:
    case SOPC_AttributeId_UserAccessLevel:
    case SOPC_AttributeId_MinimumSamplingInterval:
    case SOPC_AttributeId_Historizing:
    case SOPC_AttributeId_Executable:
    case SOPC_AttributeId_UserExecutable:
    case SOPC_AttributeId_DataTypeDefinition:
    case SOPC_AttributeId_RolePermissions:
    case SOPC_AttributeId_UserRolePermissions:
    case SOPC_AttributeId_AccessRestrictions:
    case SOPC_AttributeId_AccessLevelEx:
        return attrId;
    default:
        return SOPC_AttributeId_Invalid;
    }
}

static inline bool SOPC_TypeHelperInternal_CheckBrowseDirection(OpcUa_BrowseDirection bd)
{
    switch (bd)
    {
    case OpcUa_BrowseDirection_Forward:
    case OpcUa_BrowseDirection_Inverse:
    case OpcUa_BrowseDirection_Both:
        return true;
    default:
        return false;
    }
}

const OpcUa_NodeClass NodeClassMask_ALL =
    OpcUa_NodeClass_Object | OpcUa_NodeClass_Variable | OpcUa_NodeClass_Method | OpcUa_NodeClass_ObjectType |
    OpcUa_NodeClass_VariableType | OpcUa_NodeClass_ReferenceType | OpcUa_NodeClass_DataType | OpcUa_NodeClass_View;

static inline bool SOPC_TypeHelperInternal_CheckNodeClassMask(OpcUa_NodeClass ncm)
{
    OpcUa_NodeClass filtered_ncm = ncm & NodeClassMask_ALL;

    if (filtered_ncm != ncm)
    {
        // Contains invalid mask values
        return false;
    }
    return true;
}

static inline bool SOPC_TypeHelperInternal_CheckResultMask(OpcUa_BrowseResultMask rm)
{
    OpcUa_BrowseResultMask filtered_rm = rm & OpcUa_BrowseResultMask_All;

    if (filtered_rm != rm)
    {
        // Contains invalid mask values
        return false;
    }
    return true;
}

OpcUa_ReadRequest* SOPC_ReadRequest_Create(size_t nbReadValues, OpcUa_TimestampsToReturn tsToReturn)
{
    OpcUa_ReadRequest* req = NULL;
    if (nbReadValues > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->NodesToRead = SOPC_Calloc(nbReadValues, sizeof(*req->NodesToRead));
    if (NULL != req->NodesToRead)
    {
        req->NoOfNodesToRead = (int32_t) nbReadValues;
        req->TimestampsToReturn = tsToReturn;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfNodesToRead; i++)
        {
            OpcUa_ReadValueId_Initialize(&req->NodesToRead[i]);
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &req);
    }
    return req;
}

SOPC_ReturnStatus SOPC_ReadRequest_SetMaxAge(OpcUa_ReadRequest* readRequest, double maxAge)
{
    if (NULL == readRequest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    readRequest->MaxAge = maxAge;
    return SOPC_STATUS_OK;
}

static OpcUa_ReadValueId* ReadRequest_InitializeReadvalPointer(OpcUa_ReadRequest* readRequest,
                                                               size_t index,
                                                               SOPC_AttributeId attribute)
{
    if (!CHECK_ELEMENT_EXISTS(readRequest, NoOfNodesToRead, index) ||
        SOPC_AttributeId_Invalid == SOPC_TypeHelperInternal_CheckAttributeId(attribute))
    {
        return NULL;
    }
    OpcUa_ReadValueId* readValResult = &readRequest->NodesToRead[index];
    readValResult->AttributeId = attribute;
    return readValResult;
}

SOPC_ReturnStatus SOPC_ReadRequest_SetReadValueFromStrings(OpcUa_ReadRequest* readRequest,
                                                           size_t index,
                                                           const char* nodeId,
                                                           SOPC_AttributeId attribute,
                                                           const char* indexRange)
{
    OpcUa_ReadValueId* readVal = ReadRequest_InitializeReadvalPointer(readRequest, index, attribute);
    if (NULL == readVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(&readVal->NodeId, nodeId, (int32_t) strlen(nodeId));
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_CopyFromCString(&readVal->IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_ReadValueId_Clear(readVal);
    }
    return status;
}

SOPC_ReturnStatus SOPC_ReadRequest_SetReadValue(OpcUa_ReadRequest* readRequest,
                                                size_t index,
                                                const SOPC_NodeId* nodeId,
                                                SOPC_AttributeId attribute,
                                                const SOPC_String* indexRange)
{
    OpcUa_ReadValueId* readVal = ReadRequest_InitializeReadvalPointer(readRequest, index, attribute);
    if (NULL == readVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&readVal->NodeId, nodeId);
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_Copy(&readVal->IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_ReadValueId_Clear(readVal);
    }
    return status;
}

SOPC_ReturnStatus SOPC_ReadRequest_SetReadValueDataEncoding(OpcUa_ReadRequest* readRequest,
                                                            size_t index,
                                                            const SOPC_QualifiedName* dataEncoding)
{
    if (!CHECK_ELEMENT_EXISTS(readRequest, NoOfNodesToRead, index))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_QualifiedName_Copy(&readRequest->NodesToRead[index].DataEncoding, dataEncoding);
}

// Write request builder
OpcUa_WriteRequest* SOPC_WriteRequest_Create(size_t nbWriteValues)
{
    OpcUa_WriteRequest* req = NULL;
    if (nbWriteValues > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->NodesToWrite = SOPC_Calloc(nbWriteValues, sizeof(*req->NodesToWrite));
    if (NULL != req->NodesToWrite)
    {
        req->NoOfNodesToWrite = (int32_t) nbWriteValues;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfNodesToWrite; i++)
        {
            OpcUa_WriteValue_Initialize(&req->NodesToWrite[i]);
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &req);
    }
    return req;
}

static OpcUa_WriteValue* WriteRequest_InitializeWritevalPointer(OpcUa_WriteRequest* writeRequest,
                                                                size_t index,
                                                                SOPC_AttributeId attribute)
{
    if (!CHECK_ELEMENT_EXISTS(writeRequest, NoOfNodesToWrite, index) ||
        SOPC_AttributeId_Invalid == SOPC_TypeHelperInternal_CheckAttributeId(attribute))
    {
        return NULL;
    }
    OpcUa_WriteValue* writeValResult = &writeRequest->NodesToWrite[index];
    writeValResult->AttributeId = attribute;
    return writeValResult;
}

SOPC_ReturnStatus SOPC_WriteRequest_SetWriteValueFromStrings(OpcUa_WriteRequest* writeRequest,
                                                             size_t index,
                                                             const char* nodeId,
                                                             SOPC_AttributeId attribute,
                                                             const char* indexRange,
                                                             const SOPC_DataValue* value)
{
    OpcUa_WriteValue* writeVal = WriteRequest_InitializeWritevalPointer(writeRequest, index, attribute);
    if (NULL == writeVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(&writeVal->NodeId, nodeId, (int32_t) strlen(nodeId));
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_InitializeFromCString(&writeVal->IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_DataValue_Copy(&writeVal->Value, value);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_WriteValue_Clear(writeVal);
    }
    return status;
}

SOPC_ReturnStatus SOPC_WriteRequest_SetWriteValue(OpcUa_WriteRequest* writeRequest,
                                                  size_t index,
                                                  const SOPC_NodeId* nodeId,
                                                  SOPC_AttributeId attribute,
                                                  const SOPC_String* indexRange,
                                                  const SOPC_DataValue* value)
{
    OpcUa_WriteValue* writeVal = WriteRequest_InitializeWritevalPointer(writeRequest, index, attribute);
    if (NULL == writeVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&writeVal->NodeId, nodeId);
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_Copy(&writeVal->IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_DataValue_Copy(&writeVal->Value, value);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_WriteValue_Clear(writeVal);
    }
    return status;
}

OpcUa_BrowseRequest* SOPC_BrowseRequest_Create(size_t nbNodesToBrowse,
                                               size_t maxReferencesPerNode,
                                               const OpcUa_ViewDescription* optView)
{
    OpcUa_BrowseRequest* req = NULL;
    if (nbNodesToBrowse > INT32_MAX || maxReferencesPerNode > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_BrowseRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->NodesToBrowse = SOPC_Calloc(nbNodesToBrowse, sizeof(*req->NodesToBrowse));
    if (NULL != req->NodesToBrowse)
    {
        req->NoOfNodesToBrowse = (int32_t) nbNodesToBrowse;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status && NULL != optView)
    {
        req->View.Timestamp = optView->Timestamp;
        req->View.ViewVersion = optView->ViewVersion;
        status = SOPC_NodeId_Copy(&req->View.ViewId, &optView->ViewId);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfNodesToBrowse; i++)
        {
            OpcUa_BrowseDescription_Initialize(&req->NodesToBrowse[i]);
        }
        req->RequestedMaxReferencesPerNode = (uint32_t) maxReferencesPerNode;
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_BrowseRequest_EncodeableType, (void**) &req);
    }

    return req;
}

static OpcUa_BrowseDescription* BrowseRequest_InitializeBrowsedescPointer(OpcUa_BrowseRequest* browseRequest,
                                                                          size_t index,
                                                                          OpcUa_BrowseDirection browseDirection,
                                                                          bool includeSubtypes,
                                                                          OpcUa_NodeClass nodeClassMask,
                                                                          OpcUa_BrowseResultMask resultMask)
{
    if (!CHECK_ELEMENT_EXISTS(browseRequest, NoOfNodesToBrowse, index) ||
        !SOPC_TypeHelperInternal_CheckBrowseDirection(browseDirection) ||
        !SOPC_TypeHelperInternal_CheckNodeClassMask(nodeClassMask) ||
        !SOPC_TypeHelperInternal_CheckResultMask(resultMask))
    {
        return NULL;
    }
    OpcUa_BrowseDescription* browseDescrResult = &browseRequest->NodesToBrowse[index];
    browseDescrResult->BrowseDirection = browseDirection;
    browseDescrResult->IncludeSubtypes = includeSubtypes;
    browseDescrResult->NodeClassMask = nodeClassMask;
    browseDescrResult->ResultMask = resultMask;
    return browseDescrResult;
}

SOPC_ReturnStatus SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(OpcUa_BrowseRequest* browseRequest,
                                                                     size_t index,
                                                                     const char* nodeId,
                                                                     OpcUa_BrowseDirection browseDirection,
                                                                     const char* referenceTypeId,
                                                                     bool includeSubtypes,
                                                                     OpcUa_NodeClass nodeClassMask,
                                                                     OpcUa_BrowseResultMask resultMask)
{
    OpcUa_BrowseDescription* browseDesc = BrowseRequest_InitializeBrowsedescPointer(
        browseRequest, index, browseDirection, includeSubtypes, nodeClassMask, resultMask);
    if (NULL == browseDesc)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(&browseDesc->NodeId, nodeId, (int32_t) strlen(nodeId));
    if (SOPC_STATUS_OK == status && NULL != referenceTypeId)
    {
        status = SOPC_NodeId_InitializeFromCString(&browseDesc->ReferenceTypeId, referenceTypeId,
                                                   (int32_t) strlen(referenceTypeId));
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_BrowseDescription_Clear(browseDesc);
    }
    return status;
}

SOPC_ReturnStatus SOPC_BrowseRequest_SetBrowseDescription(OpcUa_BrowseRequest* browseRequest,
                                                          size_t index,
                                                          const SOPC_NodeId* nodeId,
                                                          OpcUa_BrowseDirection browseDirection,
                                                          const SOPC_NodeId* referenceTypeId,
                                                          bool includeSubtypes,
                                                          OpcUa_NodeClass nodeClassMask,
                                                          OpcUa_BrowseResultMask resultMask)
{
    OpcUa_BrowseDescription* browseDesc = BrowseRequest_InitializeBrowsedescPointer(
        browseRequest, index, browseDirection, includeSubtypes, nodeClassMask, resultMask);
    if (NULL == browseDesc)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&browseDesc->NodeId, nodeId);
    if (SOPC_STATUS_OK == status && NULL != referenceTypeId)
    {
        status = SOPC_NodeId_Copy(&browseDesc->ReferenceTypeId, referenceTypeId);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_BrowseDescription_Clear(browseDesc);
    }
    return status;
}

OpcUa_BrowseNextRequest* SOPC_BrowseNextRequest_Create(bool releaseContinuationPoints, size_t nbContinuationPoints)
{
    OpcUa_BrowseNextRequest* req = NULL;
    if (nbContinuationPoints > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_BrowseNextRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->ContinuationPoints = SOPC_Calloc(nbContinuationPoints, sizeof(*req->ContinuationPoints));
    if (NULL != req->ContinuationPoints)
    {
        req->NoOfContinuationPoints = (int32_t) nbContinuationPoints;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfContinuationPoints; i++)
        {
            SOPC_ByteString_Initialize(&req->ContinuationPoints[i]);
        }
        req->ReleaseContinuationPoints = releaseContinuationPoints;
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_BrowseNextRequest_EncodeableType, (void**) &req);
    }

    return req;
}

SOPC_ReturnStatus SOPC_BrowseNextRequest_SetContinuationPoint(OpcUa_BrowseNextRequest* browseNextRequest,
                                                              size_t index,
                                                              const SOPC_ByteString* continuationPoint)
{
    if (!CHECK_ELEMENT_EXISTS(browseNextRequest, NoOfContinuationPoints, index) || NULL == continuationPoint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ByteString* cp = &browseNextRequest->ContinuationPoints[index];
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Copy(cp, continuationPoint);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_ByteString_Clear(cp);
    }
    return status;
}

OpcUa_TranslateBrowsePathsToNodeIdsRequest* SOPC_TranslateBrowsePathsRequest_Create(size_t nbTranslateBrowsePaths)
{
    if (nbTranslateBrowsePaths > INT32_MAX)
    {
        return NULL;
    }
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* req = NULL;
    SOPC_ReturnStatus status =
        SOPC_Encodeable_Create(&OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    req->BrowsePaths = SOPC_Calloc(nbTranslateBrowsePaths, sizeof(*req->BrowsePaths));
    if (NULL != req->BrowsePaths)
    {
        req->NoOfBrowsePaths = (int32_t) nbTranslateBrowsePaths;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfBrowsePaths; i++)
        {
            OpcUa_BrowsePath_Initialize(&req->BrowsePaths[i]);
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType, (void**) &req);
    }

    return req;
}

static OpcUa_BrowsePath* TranslateBPRequest_InitializeBrowsePathPointer(
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpRequest,
    size_t index,
    const void* startingNodeId,
    size_t nbPathElements,
    OpcUa_RelativePathElement* pathElements)
{
    if (!CHECK_ELEMENT_EXISTS(tbpRequest, NoOfBrowsePaths, index) || NULL == startingNodeId || 0 == nbPathElements ||
        nbPathElements > INT32_MAX || NULL == pathElements)
    {
        return NULL;
    }
    OpcUa_BrowsePath* browsePathResult = &tbpRequest->BrowsePaths[index];
    return browsePathResult;
}

SOPC_ReturnStatus SOPC_TranslateBrowsePathRequest_SetPathFromString(
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpRequest,
    size_t index,
    const char* startingNodeId,
    size_t nbPathElements,
    OpcUa_RelativePathElement* pathElements)
{
    OpcUa_BrowsePath* browsePath = TranslateBPRequest_InitializeBrowsePathPointer(
        tbpRequest, index, (const void*) startingNodeId, nbPathElements, pathElements);
    if (NULL == browsePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status =
        SOPC_NodeId_InitializeFromCString(&browsePath->StartingNode, startingNodeId, (int32_t) strlen(startingNodeId));
    if (SOPC_STATUS_OK == status)
    {
        browsePath->RelativePath.Elements = pathElements;
        browsePath->RelativePath.NoOfElements = (int32_t) nbPathElements;
    }
    else
    {
        OpcUa_BrowsePath_Clear(browsePath);
    }
    return status;
}

SOPC_ReturnStatus SOPC_TranslateBrowsePathRequest_SetPath(OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpRequest,
                                                          size_t index,
                                                          SOPC_NodeId* startingNodeId,
                                                          size_t nbPathElements,
                                                          OpcUa_RelativePathElement* pathElements)
{
    OpcUa_BrowsePath* browsePath = TranslateBPRequest_InitializeBrowsePathPointer(
        tbpRequest, index, (const void*) startingNodeId, nbPathElements, pathElements);
    if (NULL == browsePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&browsePath->StartingNode, startingNodeId);
    if (SOPC_STATUS_OK == status)
    {
        browsePath->RelativePath.Elements = pathElements;
        browsePath->RelativePath.NoOfElements = (int32_t) nbPathElements;
    }
    else
    {
        OpcUa_BrowsePath_Clear(browsePath);
    }
    return status;
}

OpcUa_RelativePathElement* SOPC_RelativePathElements_Create(size_t nbPathElements)
{
    OpcUa_RelativePathElement* res = NULL;
    if (nbPathElements > INT32_MAX)
    {
        return res;
    }
    res = SOPC_Calloc(nbPathElements, sizeof(OpcUa_RelativePathElement));
    if (NULL != res)
    { // Initialize elements
        for (size_t i = 0; i < nbPathElements; i++)
        {
            OpcUa_RelativePathElement_Initialize(&res[i]);
        }
    }

    return res;
}

SOPC_ReturnStatus SOPC_RelativePathElements_SetPathElement(OpcUa_RelativePathElement* pathElementsArray,
                                                           size_t index,
                                                           const SOPC_NodeId* referenceTypeId,
                                                           bool isInverse,
                                                           bool includeSubtypes,
                                                           uint16_t targetNsIndex,
                                                           const char* targetName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pathElementsArray || NULL == targetName || index > INT32_MAX)
    {
        return status;
    }

    OpcUa_RelativePathElement* rpe = &pathElementsArray[index];

    status = SOPC_STATUS_OK;
    if (NULL != referenceTypeId)
    {
        status = SOPC_NodeId_Copy(&rpe->ReferenceTypeId, referenceTypeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&rpe->TargetName.Name, targetName);
    }

    if (SOPC_STATUS_OK == status)
    {
        rpe->TargetName.NamespaceIndex = targetNsIndex;
        rpe->IsInverse = isInverse;
        rpe->IncludeSubtypes = includeSubtypes;
    }
    else
    {
        OpcUa_RelativePathElement_Clear(rpe);
    }

    return status;
}

OpcUa_GetEndpointsRequest* SOPC_GetEndpointsRequest_Create(const char* endpointURL)
{
    OpcUa_GetEndpointsRequest* getEndpointReq = NULL;
    SOPC_ReturnStatus status =
        SOPC_Encodeable_Create(&OpcUa_GetEndpointsRequest_EncodeableType, (void**) &getEndpointReq);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&getEndpointReq->EndpointUrl, endpointURL);
    }
    return getEndpointReq;
}

static SOPC_String* SOPC_HelperInternal_AllocAndCopyCstringInArray(size_t stringArrayLength,
                                                                   char* const* cStringsToCopy)
{
    SOPC_String* newStringArray = SOPC_Calloc(sizeof(SOPC_String), stringArrayLength);

    if (NULL == newStringArray)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (size_t i = 0; i < stringArrayLength; i++)
    {
        SOPC_String_Initialize(&newStringArray[i]);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_CopyFromCString(&newStringArray[i], cStringsToCopy[i]);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        int32_t length = (int32_t) stringArrayLength;
        SOPC_Clear_Array(&length, (void**) &newStringArray, sizeof(SOPC_String), SOPC_String_ClearAux);
    }

    return newStringArray;
}

SOPC_ReturnStatus SOPC_GetEndpointsRequest_SetPreferredLocales(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                               size_t nbLocales,
                                                               char* const* localeIds)
{
    if (NULL == getEndpointsReq || 0 == nbLocales || NULL == localeIds)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (getEndpointsReq->NoOfLocaleIds != 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    getEndpointsReq->LocaleIds = SOPC_HelperInternal_AllocAndCopyCstringInArray(nbLocales, localeIds);

    if (NULL == getEndpointsReq->LocaleIds)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    getEndpointsReq->NoOfLocaleIds = (int32_t) nbLocales;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_GetEndpointsRequest_SetProfileURIs(OpcUa_GetEndpointsRequest* getEndpointsReq,
                                                          size_t nbProfiles,
                                                          char* const* profileURIs)
{
    if (NULL == getEndpointsReq || NULL == profileURIs)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (getEndpointsReq->NoOfProfileUris > 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    getEndpointsReq->ProfileUris = SOPC_HelperInternal_AllocAndCopyCstringInArray(nbProfiles, profileURIs);

    if (NULL == getEndpointsReq->ProfileUris)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    getEndpointsReq->NoOfProfileUris = (int32_t) nbProfiles;

    return SOPC_STATUS_OK;
}

OpcUa_RegisterServer2Request* SOPC_RegisterServer2Request_CreateFromServerConfiguration(void)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    OpcUa_ApplicationDescription* srcDesc = &pConfig->serverConfig.serverDescription;
    OpcUa_MdnsDiscoveryConfiguration* mdnsObj = NULL;
    OpcUa_RegisterServer2Request* request = SOPC_Calloc(1, sizeof(OpcUa_RegisterServer2Request));
    OpcUa_RegisterServer2Request_Initialize(request);
    if (NULL == request)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    request->DiscoveryConfiguration = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    status = (NULL == request->DiscoveryConfiguration ? SOPC_STATUS_NOK : SOPC_STATUS_OK);

    if (SOPC_STATUS_OK == status)
    {
        request->NoOfDiscoveryConfiguration = 1;
        status = SOPC_Encodeable_CreateExtension(request->DiscoveryConfiguration,
                                                 &OpcUa_MdnsDiscoveryConfiguration_EncodeableType, (void**) &mdnsObj);
    }

    if (SOPC_STATUS_OK == status && srcDesc->NoOfDiscoveryUrls > 0)
    {
        request->Server.DiscoveryUrls = SOPC_Calloc((size_t) srcDesc->NoOfDiscoveryUrls, sizeof(SOPC_String));

        if (request->Server.DiscoveryUrls != NULL)
        {
            request->Server.NoOfDiscoveryUrls = srcDesc->NoOfDiscoveryUrls;
            for (int32_t i = 0; SOPC_STATUS_OK == status && i < srcDesc->NoOfDiscoveryUrls; i++)
            {
                status = SOPC_String_AttachFrom(&request->Server.DiscoveryUrls[i], &srcDesc->DiscoveryUrls[i]);
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LocalizedText_CopyToArray(&request->Server.ServerNames, &request->Server.NoOfServerNames,
                                                &srcDesc->ApplicationName);
    }

    if (SOPC_STATUS_OK == status && srcDesc->GatewayServerUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&request->Server.GatewayServerUri, &srcDesc->GatewayServerUri);
    }

    if (SOPC_STATUS_OK == status && srcDesc->ProductUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&request->Server.ProductUri, &srcDesc->ProductUri);
    }

    if (SOPC_STATUS_OK == status && srcDesc->ApplicationUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&request->Server.ServerUri, &srcDesc->ApplicationUri);
    }

    if (SOPC_STATUS_OK == status && srcDesc->ApplicationName.defaultText.Length > 0)
    {
        status = SOPC_String_AttachFrom(&mdnsObj->MdnsServerName, &srcDesc->ApplicationName.defaultText);
    }

    if (SOPC_STATUS_OK == status)
    {
        mdnsObj->ServerCapabilities = SOPC_Calloc(1, sizeof(SOPC_String));
        if (NULL == mdnsObj->ServerCapabilities)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            mdnsObj->NoOfServerCapabilities = 1;
            status = SOPC_String_AttachFromCstring(mdnsObj->ServerCapabilities, "DA"); // Supports only DA
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        request->Server.IsOnline = true;
        request->Server.ServerType = srcDesc->ApplicationType;
    }
    else
    {
        OpcUa_RegisterServer2Request_Clear(request);
        SOPC_Free(request);
        request = NULL;
    }

    return request;
}

OpcUa_AddNodesRequest* SOPC_AddNodesRequest_Create(size_t nbAddNodes)
{
    if (nbAddNodes > INT32_MAX)
    {
        return NULL;
    }
    OpcUa_AddNodesRequest* req = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_AddNodesRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    req->NodesToAdd = SOPC_Calloc(nbAddNodes, sizeof(*req->NodesToAdd));
    if (NULL != req->NodesToAdd)
    {
        req->NoOfNodesToAdd = (int32_t) nbAddNodes;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Initialize elements
        for (int32_t i = 0; i < req->NoOfNodesToAdd; i++)
        {
            OpcUa_AddNodesItem_Initialize(&req->NodesToAdd[i]);
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &req);
    }
    return req;
}

SOPC_ReturnStatus SOPC_AddNodeRequest_SetVariableAttributes(OpcUa_AddNodesRequest* addNodesRequest,
                                                            size_t index,
                                                            const SOPC_ExpandedNodeId* parentNodeId,
                                                            const SOPC_NodeId* referenceTypeId,
                                                            const SOPC_ExpandedNodeId* optRequestedNodeId,
                                                            const SOPC_QualifiedName* browseName,
                                                            const SOPC_ExpandedNodeId* typeDefinition,
                                                            const SOPC_LocalizedText* optDisplayName,
                                                            const SOPC_LocalizedText* optDescription,
                                                            const uint32_t* optWriteMask,
                                                            const uint32_t* optUserWriteMask,
                                                            const SOPC_Variant* optValue,
                                                            const SOPC_NodeId* optDataType,
                                                            const int32_t* optValueRank,
                                                            int32_t noOfArrayDimensions,
                                                            const uint32_t* optArrayDimensions,
                                                            const SOPC_Byte* optAccessLevel,
                                                            const SOPC_Byte* optUserAccessLevel,
                                                            const double* optMinimumSamplingInterval,
                                                            SOPC_Boolean* optHistorizing)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (!CHECK_ELEMENT_EXISTS(addNodesRequest, NoOfNodesToAdd, index) || NULL == parentNodeId ||
        NULL == referenceTypeId || (NULL != optRequestedNodeId && 0 != optRequestedNodeId->ServerIndex) ||
        NULL == browseName || NULL == typeDefinition || (noOfArrayDimensions > 0 && NULL != optArrayDimensions))
    {
        return status;
    }
    OpcUa_AddNodesItem* item = &addNodesRequest->NodesToAdd[index];
    OpcUa_VariableAttributes* varAttrs = NULL;

    // item is a Variable node
    item->NodeClass = OpcUa_NodeClass_Variable;

    status = SOPC_Encodeable_CreateExtension(&item->NodeAttributes, &OpcUa_VariableAttributes_EncodeableType,
                                             (void**) &varAttrs);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ExpandedNodeId_Copy(&item->ParentNodeId, parentNodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&item->ReferenceTypeId, referenceTypeId);
    }
    if (SOPC_STATUS_OK == status && NULL != optRequestedNodeId)
    {
        status = SOPC_ExpandedNodeId_Copy(&item->RequestedNewNodeId, optRequestedNodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_QualifiedName_Copy(&item->BrowseName, browseName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ExpandedNodeId_Copy(&item->TypeDefinition, typeDefinition);
    }
    if (SOPC_STATUS_OK == status && NULL != optDisplayName)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_DisplayName;
        status = SOPC_LocalizedText_Copy(&varAttrs->DisplayName, optDisplayName);
    }
    if (SOPC_STATUS_OK == status && NULL != optDescription)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_Description;
        status = SOPC_LocalizedText_Copy(&varAttrs->Description, optDescription);
    }
    if (SOPC_STATUS_OK == status && NULL != optWriteMask)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_WriteMask;
        varAttrs->WriteMask = *optWriteMask;
    }
    if (SOPC_STATUS_OK == status && NULL != optUserWriteMask)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_UserWriteMask;
        varAttrs->UserWriteMask = *optUserWriteMask;
    }
    if (SOPC_STATUS_OK == status && NULL != optValue)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_Value;
        status = SOPC_Variant_Copy(&varAttrs->Value, optValue);
    }
    if (SOPC_STATUS_OK == status && NULL != optDataType)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_DataType;
        status = SOPC_NodeId_Copy(&varAttrs->DataType, optDataType);
    }
    if (SOPC_STATUS_OK == status && NULL != optValueRank)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_ValueRank;
        varAttrs->ValueRank = *optValueRank;
    }
    if (SOPC_STATUS_OK == status && NULL != optArrayDimensions)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_ArrayDimensions;
        varAttrs->ArrayDimensions = SOPC_Calloc((size_t) noOfArrayDimensions, sizeof(*varAttrs->ArrayDimensions));
        if (NULL == varAttrs->ArrayDimensions)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            for (int32_t i = 0; i < noOfArrayDimensions; i++)
            {
                varAttrs->ArrayDimensions[i] = optArrayDimensions[i];
            }
        }
    }
    if (SOPC_STATUS_OK == status && NULL != optAccessLevel)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_AccessLevel;
        varAttrs->AccessLevel = *optAccessLevel;
    }
    if (SOPC_STATUS_OK == status && NULL != optUserAccessLevel)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_UserAccessLevel;
        varAttrs->UserAccessLevel = *optUserAccessLevel;
    }
    if (SOPC_STATUS_OK == status && NULL != optMinimumSamplingInterval)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_MinimumSamplingInterval;
        varAttrs->MinimumSamplingInterval = *optMinimumSamplingInterval;
    }
    if (SOPC_STATUS_OK == status && NULL != optHistorizing)
    {
        varAttrs->SpecifiedAttributes |= OpcUa_NodeAttributesMask_Historizing;
        varAttrs->Historizing = *optHistorizing;
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_AddNodesItem_Clear(item);
    }
    return status;
}

OpcUa_CreateSubscriptionRequest* SOPC_CreateSubscriptionRequest_Create(double reqPublishingInterval,
                                                                       uint32_t reqLifetimeCount,
                                                                       uint32_t reqMaxKeepAliveCount,
                                                                       uint32_t maxNotifPerPublish,
                                                                       SOPC_Boolean publishingEnabled,
                                                                       SOPC_Byte priority)
{
    OpcUa_CreateSubscriptionRequest* req = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_CreateSubscriptionRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    req->RequestedPublishingInterval = reqPublishingInterval;
    req->RequestedLifetimeCount = reqLifetimeCount;
    req->RequestedMaxKeepAliveCount = reqMaxKeepAliveCount;
    req->MaxNotificationsPerPublish = maxNotifPerPublish;
    req->PublishingEnabled = publishingEnabled;
    req->Priority = priority;
    return req;
}

OpcUa_CreateSubscriptionRequest* SOPC_CreateSubscriptionRequest_CreateDefault(void)
{
    return SOPC_CreateSubscriptionRequest_Create(SOPC_DEFAULT_PUBLISH_PERIOD_MS, SOPC_DEFAULT_MAX_LIFETIME_COUNT,
                                                 SOPC_DEFAULT_MAX_KEEP_ALIVE_COUNT, SOPC_DEFAULT_MAX_NOTIFS_PER_PUBLISH,
                                                 true, 0);
}

OpcUa_CreateMonitoredItemsRequest* SOPC_CreateMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           OpcUa_TimestampsToReturn ts)
{
    OpcUa_CreateMonitoredItemsRequest* req = NULL;
    if (nbMonitoredItems > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->ItemsToCreate = SOPC_Calloc(nbMonitoredItems, sizeof(*req->ItemsToCreate));
    if (NULL != req->ItemsToCreate)
    {
        // Initialize elements
        for (size_t i = 0; i < nbMonitoredItems; i++)
        {
            OpcUa_MonitoredItemCreateRequest_Initialize(&req->ItemsToCreate[i]);
        }
        req->SubscriptionId = subscriptionId;
        req->TimestampsToReturn = ts;
        req->NoOfItemsToCreate = (int32_t) nbMonitoredItems;
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &req);
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return req;
}

static OpcUa_MonitoredItemCreateRequest* CreateMIrequest_InitializeCreateMonitPointer(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    SOPC_AttributeId attribute)
{
    if (!CHECK_ELEMENT_EXISTS(createMIrequest, NoOfItemsToCreate, index) ||
        SOPC_AttributeId_Invalid == SOPC_TypeHelperInternal_CheckAttributeId(attribute))
    {
        return NULL;
    }
    OpcUa_MonitoredItemCreateRequest* createMIresult = &createMIrequest->ItemsToCreate[index];
    createMIresult->ItemToMonitor.AttributeId = attribute;
    createMIresult->MonitoringMode = OpcUa_MonitoringMode_Reporting;
    createMIresult->RequestedParameters.SamplingInterval = SOPC_DEFAULT_MI_SAMPLING_INTERVAL_MS;
    createMIresult->RequestedParameters.QueueSize = SOPC_DEFAULT_MI_QUEUE_SIZE;
    createMIresult->RequestedParameters.DiscardOldest = SOPC_DEFAULT_MI_DISCARD_OLDEST;
    return createMIresult;
}

SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    const char* nodeId,
    SOPC_AttributeId attribute,
    const char* indexRange)
{
    OpcUa_MonitoredItemCreateRequest* createMI =
        CreateMIrequest_InitializeCreateMonitPointer(createMIrequest, index, attribute);
    if (NULL == createMI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status =
        SOPC_NodeId_InitializeFromCString(&createMI->ItemToMonitor.NodeId, nodeId, (int32_t) strlen(nodeId));
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_CopyFromCString(&createMI->ItemToMonitor.IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_MonitoredItemCreateRequest_Clear(createMI);
    }
    return status;
}

SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    const SOPC_NodeId* nodeId,
    SOPC_AttributeId attribute,
    const SOPC_String* indexRange)
{
    OpcUa_MonitoredItemCreateRequest* createMI =
        CreateMIrequest_InitializeCreateMonitPointer(createMIrequest, index, attribute);
    if (NULL == createMI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&createMI->ItemToMonitor.NodeId, nodeId);
    if (SOPC_STATUS_OK == status && NULL != indexRange)
    {
        status = SOPC_String_Copy(&createMI->ItemToMonitor.IndexRange, indexRange);
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_MonitoredItemCreateRequest_Clear(createMI);
    }
    return status;
}

OpcUa_CreateMonitoredItemsRequest* SOPC_CreateMonitoredItemsRequest_CreateDefault(uint32_t subscriptionId,
                                                                                  size_t nbMonitoredItems,
                                                                                  const SOPC_NodeId* nodeIdsToMonitor,
                                                                                  OpcUa_TimestampsToReturn ts)
{
    if (NULL == nodeIdsToMonitor)
    {
        return NULL;
    }
    OpcUa_CreateMonitoredItemsRequest* req =
        SOPC_CreateMonitoredItemsRequest_Create(subscriptionId, nbMonitoredItems, ts);
    if (NULL != req)
    {
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbMonitoredItems; i++)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(req, i, &nodeIdsToMonitor[i],
                                                                         SOPC_AttributeId_Value, NULL);
        }
        if (SOPC_STATUS_OK != status)
        {
            OpcUa_CreateMonitoredItemsRequest_Clear(req);
            SOPC_Free(req);
            req = NULL;
        }
    }
    return req;
}

OpcUa_CreateMonitoredItemsRequest* SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
    uint32_t subscriptionId,
    size_t nbMonitoredItems,
    char* const* nodeIdsToMonitor,
    OpcUa_TimestampsToReturn ts)
{
    if (NULL == nodeIdsToMonitor)
    {
        return NULL;
    }
    OpcUa_CreateMonitoredItemsRequest* req =
        SOPC_CreateMonitoredItemsRequest_Create(subscriptionId, nbMonitoredItems, ts);
    if (NULL != req)
    {
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbMonitoredItems; i++)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(req, i, nodeIdsToMonitor[i],
                                                                                    SOPC_AttributeId_Value, NULL);
        }
        if (SOPC_STATUS_OK != status)
        {
            OpcUa_CreateMonitoredItemsRequest_Clear(req);
            SOPC_Free(req);
            req = NULL;
        }
    }
    return req;
}

SOPC_ExtensionObject* SOPC_MonitoredItem_DataChangeFilter(OpcUa_DataChangeTrigger trigger,
                                                          OpcUa_DeadbandType deadbandType,
                                                          double deadbandValue)
{
    if (deadbandValue < 0.0 || deadbandType < OpcUa_DeadbandType_None || deadbandType > OpcUa_DeadbandType_Percent ||
        trigger < OpcUa_DataChangeTrigger_Status || trigger > OpcUa_DataChangeTrigger_StatusValueTimestamp)
    {
        return NULL;
    }
    OpcUa_DataChangeFilter* filter = NULL;
    SOPC_ExtensionObject* filterExt = SOPC_Calloc(1, sizeof(*filterExt));
    if (NULL == filterExt)
    {
        return NULL;
    }
    SOPC_ReturnStatus status =
        SOPC_Encodeable_CreateExtension(filterExt, &OpcUa_DataChangeFilter_EncodeableType, (void**) &filter);

    if (SOPC_STATUS_OK == status)
    {
        filter->DeadbandType = deadbandType;
        filter->DeadbandValue = deadbandValue;
        filter->Trigger = trigger;
    }
    else
    {
        SOPC_Free(filterExt);
        filterExt = NULL;
    }
    return filterExt;
}

SOPC_ReturnStatus SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
    OpcUa_CreateMonitoredItemsRequest* createMIrequest,
    size_t index,
    OpcUa_MonitoringMode monitoringMode,
    uint32_t clientHandle,
    double samplingInterval,
    SOPC_ExtensionObject* optFilter,
    uint32_t queueSize,
    SOPC_Boolean discardOldest)
{
    if (NULL == createMIrequest || !CHECK_ELEMENT_EXISTS(createMIrequest, NoOfItemsToCreate, index))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_MonitoredItemCreateRequest* createMI = &createMIrequest->ItemsToCreate[index];

    createMI->MonitoringMode = monitoringMode;
    createMI->RequestedParameters.ClientHandle = clientHandle;
    createMI->RequestedParameters.SamplingInterval = samplingInterval;
    if (NULL != optFilter)
    {
        createMI->RequestedParameters.Filter = *optFilter;
        SOPC_Free(optFilter);
    }
    createMI->RequestedParameters.QueueSize = queueSize;
    createMI->RequestedParameters.DiscardOldest = discardOldest;

    return SOPC_STATUS_OK;
}

OpcUa_ModifyMonitoredItemsRequest* SOPC_ModifyMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           OpcUa_TimestampsToReturn ts)
{
    OpcUa_ModifyMonitoredItemsRequest* req = NULL;
    if (nbMonitoredItems > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_ModifyMonitoredItemsRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->ItemsToModify = SOPC_Calloc(nbMonitoredItems, sizeof(*req->ItemsToModify));
    if (NULL != req->ItemsToModify)
    {
        // Initialize elements
        for (size_t i = 0; i < nbMonitoredItems; i++)
        {
            OpcUa_ModifyMonitoredItemsRequest_Initialize(&req->ItemsToModify[i]);
        }
        req->SubscriptionId = subscriptionId;
        req->TimestampsToReturn = ts;
        req->NoOfItemsToModify = (int32_t) nbMonitoredItems;
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_ModifyMonitoredItemsRequest_EncodeableType, (void**) &req);
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return req;
}

SOPC_ReturnStatus SOPC_ModifyMonitoredItemsRequest_SetMonitoredItemParams(
    OpcUa_ModifyMonitoredItemsRequest* modifyMIrequest,
    size_t index,
    uint32_t monitoredItemId,
    uint32_t clientHandle,
    double samplingInterval,
    SOPC_ExtensionObject* optFilter,
    uint32_t queueSize,
    SOPC_Boolean discardOldest)
{
    if (NULL == modifyMIrequest || !CHECK_ELEMENT_EXISTS(modifyMIrequest, NoOfItemsToModify, index))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_MonitoredItemModifyRequest* modifyMI = &modifyMIrequest->ItemsToModify[index];

    modifyMI->MonitoredItemId = monitoredItemId;
    modifyMI->RequestedParameters.ClientHandle = clientHandle;
    modifyMI->RequestedParameters.SamplingInterval = samplingInterval;
    if (NULL != optFilter)
    {
        modifyMI->RequestedParameters.Filter = *optFilter;
        SOPC_Free(optFilter);
    }
    modifyMI->RequestedParameters.QueueSize = queueSize;
    modifyMI->RequestedParameters.DiscardOldest = discardOldest;

    return SOPC_STATUS_OK;
}

OpcUa_DeleteMonitoredItemsRequest* SOPC_DeleteMonitoredItemsRequest_Create(uint32_t subscriptionId,
                                                                           size_t nbMonitoredItems,
                                                                           const uint32_t* optMonitoredItemIds)
{
    OpcUa_DeleteMonitoredItemsRequest* req = NULL;
    if (nbMonitoredItems > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->SubscriptionId = subscriptionId;
    req->NoOfMonitoredItemIds = (int32_t) nbMonitoredItems;
    req->MonitoredItemIds = SOPC_Calloc(nbMonitoredItems, sizeof(*req->MonitoredItemIds));
    if (NULL != req->MonitoredItemIds)
    {
        // Initialize elements
        if (NULL != optMonitoredItemIds)
        {
            for (size_t i = 0; i < nbMonitoredItems; i++)
            {
                req->MonitoredItemIds[i] = optMonitoredItemIds[i];
            }
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &req);
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return req;
}

SOPC_ReturnStatus SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(
    OpcUa_DeleteMonitoredItemsRequest* deleteMIrequest,
    size_t index,
    uint32_t monitoredItemId)
{
    if (NULL == deleteMIrequest || !CHECK_ELEMENT_EXISTS(deleteMIrequest, NoOfMonitoredItemIds, index))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    deleteMIrequest->MonitoredItemIds[index] = monitoredItemId;
    return SOPC_STATUS_OK;
}

OpcUa_CallRequest* SOPC_CallRequest_Create(size_t nbMethodsToCalls)
{
    OpcUa_CallRequest* req = NULL;
    if (nbMethodsToCalls > INT32_MAX)
    {
        return req;
    }
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_CallRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status)
    {
        return req;
    }
    req->NoOfMethodsToCall = (int32_t) nbMethodsToCalls;
    req->MethodsToCall = SOPC_Calloc(nbMethodsToCalls, sizeof(*req->MethodsToCall));
    if (NULL != req->MethodsToCall)
    {
        for (size_t i = 0; i < nbMethodsToCalls; i++)
        {
            OpcUa_CallMethodRequest_Initialize(&req->MethodsToCall[i]);
        }
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return req;
}

SOPC_ReturnStatus SOPC_CallRequest_SetMethodToCall(OpcUa_CallRequest* callRequest,
                                                   size_t index,
                                                   const SOPC_NodeId* objectId,
                                                   const SOPC_NodeId* methodId,
                                                   int32_t nbOfInputArguments,
                                                   const SOPC_Variant* inputArguments)
{
    if (NULL == callRequest || !CHECK_ELEMENT_EXISTS(callRequest, NoOfMethodsToCall, index))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_CallMethodRequest* callMethod = &callRequest->MethodsToCall[index];
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&callMethod->ObjectId, objectId);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&callMethod->MethodId, methodId);
    }
    if (SOPC_STATUS_OK == status)
    {
        callMethod->InputArguments = SOPC_Calloc((size_t) nbOfInputArguments, sizeof(*callMethod->InputArguments));
        status = (NULL != callMethod->InputArguments ? status : SOPC_STATUS_OUT_OF_MEMORY);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Copy_Array(nbOfInputArguments, callMethod->InputArguments, inputArguments, sizeof(SOPC_Variant),
                                 &SOPC_Variant_CopyAux);
        callMethod->NoOfInputArguments = nbOfInputArguments;
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_CallMethodRequest_Clear(callMethod);
    }
    return status;
}

#undef CHECK_ELEMENT_EXISTS
