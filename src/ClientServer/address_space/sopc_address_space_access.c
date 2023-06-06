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

#include "sopc_address_space_access_internal.h"

#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_node_mgt_helper_internal.h"
#include "sopc_platform_time.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "util_variant.h"

struct _SOPC_AddressSpaceAccess
{
    SOPC_AddressSpace* addSpaceRef;
    bool recordOperations;
    SOPC_SLinkedList* operations; // SOPC_AddressSpaceAccessOperation* prepended operations since creation
};

SOPC_AddressSpaceAccess* SOPC_AddressSpaceAccess_Create(SOPC_AddressSpace* addSpaceRef, bool recordOperations)
{
    SOPC_ASSERT(NULL != addSpaceRef);
    SOPC_AddressSpaceAccess* newAccess = SOPC_Calloc(1, sizeof(*newAccess));
    if (NULL == newAccess)
    {
        return NULL;
    }
    newAccess->addSpaceRef = addSpaceRef;
    if (recordOperations)
    {
        newAccess->recordOperations = true;
        newAccess->operations = SOPC_SLinkedList_Create(0);
        if (NULL == newAccess->operations)
        {
            SOPC_Free(newAccess);
            return NULL;
        }
    }
    return newAccess;
}

SOPC_SLinkedList* SOPC_AddressSpaceAccess_GetOperations(SOPC_AddressSpaceAccess* addSpaceAccess)
{
    SOPC_ASSERT(NULL != addSpaceAccess);
    SOPC_SLinkedList* operations = NULL;
    if (addSpaceAccess->recordOperations)
    {
        operations = addSpaceAccess->operations;
        addSpaceAccess->operations = NULL;
    }
    return operations;
}

static void SOPC_InternalAddressSpaceAccess_FreeOperation(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_AddressSpaceAccessOperation* op = (SOPC_AddressSpaceAccessOperation*) val;
    switch (op->operation)
    {
    case SOPC_ADDSPACE_WRITE:
        SOPC_Encodeable_Delete(&OpcUa_WriteValue_EncodeableType, &op->param1);
        SOPC_Encodeable_Delete(&OpcUa_WriteValue_EncodeableType, &op->param2);
        break;
    case SOPC_ADDSPACE_CHANGE_NODE:
        SOPC_NodeId_Clear((SOPC_NodeId*) op->param1);
        SOPC_Free(op->param1);
        SOPC_ASSERT(NULL == op->param2);
        break;
    default:
        SOPC_ASSERT(false);
    }
    SOPC_Free(op);
}

void SOPC_AddressSpaceAccess_Delete(SOPC_AddressSpaceAccess** ppAddSpaceAccess)
{
    SOPC_ASSERT(NULL != ppAddSpaceAccess);
    SOPC_AddressSpaceAccess* access = *ppAddSpaceAccess;
    SOPC_ASSERT(NULL != access);
    if (access->recordOperations && NULL != access->operations)
    {
        SOPC_SLinkedList_Apply(access->operations, &SOPC_InternalAddressSpaceAccess_FreeOperation);
        SOPC_SLinkedList_Delete(access->operations);
        access->operations = NULL;
    }
    SOPC_ASSERT(NULL == access->operations);
    SOPC_Free(access);
    *ppAddSpaceAccess = NULL;
}

static SOPC_AddressSpace_Node* SOPC_InternalAddressSpaceAccess_GetNode(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                                       const SOPC_NodeId* nodeId)
{
    SOPC_ASSERT(NULL != addSpaceAccess);
    SOPC_ASSERT(NULL != nodeId);

    bool found = false;
    SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addSpaceAccess->addSpaceRef, nodeId, &found);
    if (!found)
    {
        return NULL;
    }
    return node;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_ReadAttribute(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                      const SOPC_NodeId* nodeId,
                                                      SOPC_AttributeId attribId,
                                                      SOPC_Variant** outValue)
{
    if (NULL == addSpaceAccess || NULL == nodeId || NULL == outValue)
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_AddressSpace_Node* node = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, nodeId);
    if (NULL == node)
    {
        return OpcUa_BadNodeIdUnknown;
    }
    if (!SOPC_AddressSpace_Has_Attribute(addSpaceAccess->addSpaceRef, node, attribId))
    {
        return OpcUa_BadAttributeIdInvalid;
    }

    SOPC_Variant* val = NULL;
    switch (attribId)
    {
    case SOPC_AttributeId_NodeId:
        val = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_NodeClass:
        val = util_variant__new_Variant_from_NodeClass(
            *SOPC_AddressSpace_Get_NodeClass(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_BrowseName:
        val = util_variant__new_Variant_from_QualifiedName(
            SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_DisplayName:
        val = util_variant__new_Variant_from_LocalizedText(
            SOPC_AddressSpace_Get_DisplayName(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_Description:
        val = util_variant__new_Variant_from_LocalizedText(
            SOPC_AddressSpace_Get_Description(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_WriteMask:
        val =
            util_variant__new_Variant_from_uint32(*SOPC_AddressSpace_Get_WriteMask(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_UserWriteMask:
        val = util_variant__new_Variant_from_uint32(
            *SOPC_AddressSpace_Get_UserWriteMask(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_IsAbstract:
        val = util_variant__new_Variant_from_Bool(*SOPC_AddressSpace_Get_IsAbstract(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_Value:
        val = util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_DataType:
        val = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Get_DataType(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_ValueRank:
        val = util_variant__new_Variant_from_int32(*SOPC_AddressSpace_Get_ValueRank(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_ArrayDimensions:
        val = util_variant__new_Variant_from_uint32(
            *SOPC_AddressSpace_Get_ArrayDimensions(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_AccessLevel:
        val = util_variant__new_Variant_from_Byte(SOPC_AddressSpace_Get_AccessLevel(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_Executable:
        val = util_variant__new_Variant_from_Bool(SOPC_AddressSpace_Get_Executable(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_ContainsNoLoops:
    case SOPC_AttributeId_InverseName:
    case SOPC_AttributeId_Symmetric:
    case SOPC_AttributeId_EventNotifier:
    case SOPC_AttributeId_MinimumSamplingInterval:
    case SOPC_AttributeId_Historizing:
    case SOPC_AttributeId_UserAccessLevel:
    case SOPC_AttributeId_UserExecutable:
    default:
        return OpcUa_BadNotImplemented;
    }
    *outValue = val;
    if (NULL == val)
    {
        return OpcUa_BadOutOfMemory;
    }
    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_ReadValue(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                  const SOPC_NodeId* nodeId,
                                                  const SOPC_NumericRange* optNumRange,
                                                  SOPC_DataValue** outDataValue)
{
    if (NULL == addSpaceAccess || NULL == nodeId || NULL == outDataValue)
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_AddressSpace_Node* node = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, nodeId);
    if (NULL == node)
    {
        return OpcUa_BadNodeIdUnknown;
    }
    OpcUa_NodeClass nodeClass = *SOPC_AddressSpace_Get_NodeClass(addSpaceAccess->addSpaceRef, node);
    if (nodeClass != OpcUa_NodeClass_Variable && nodeClass != OpcUa_NodeClass_VariableType)
    {
        return OpcUa_BadAttributeIdInvalid;
    }
    SOPC_DataValue* dv = SOPC_Calloc(1, sizeof(*dv));
    SOPC_Variant* src = SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, node);
    if (NULL == dv || NULL == src)
    {
        SOPC_Free(dv);
        return OpcUa_BadOutOfMemory;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_StatusCode returnCode = OpcUa_BadInternalError;
    SOPC_DataValue_Initialize(dv);
    if (NULL == optNumRange)
    {
        status = SOPC_Variant_Copy(&dv->Value, src);
    }
    else
    {
        bool hasRange = false;
        status = SOPC_Variant_HasRange(src, optNumRange, false, &hasRange);
        if (SOPC_STATUS_OK == status && hasRange)
        {
            status = SOPC_Variant_GetRange(&dv->Value, src, optNumRange);
        }
        else if (SOPC_STATUS_OK != status)
        {
            returnCode = OpcUa_BadIndexRangeInvalid;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
            returnCode = OpcUa_BadIndexRangeNoData;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set Status and Source TS. Server TS is always the current date, not set for local access.
        dv->Status = SOPC_AddressSpace_Get_StatusCode(addSpaceAccess->addSpaceRef, node);
        SOPC_Value_Timestamp sourceTs = SOPC_AddressSpace_Get_SourceTs(addSpaceAccess->addSpaceRef, node);
        dv->SourceTimestamp = sourceTs.timestamp;
        dv->ServerPicoSeconds = sourceTs.picoSeconds;

        returnCode = SOPC_GoodGenericStatus;
        *outDataValue = dv;
    }
    else
    {
        SOPC_DataValue_Clear(dv);
        SOPC_Free(dv);
    }
    return returnCode;
}

static SOPC_ReturnStatus SOPC_InternalRecordOperation_Write(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                            SOPC_AddressSpace_Node* node,
                                                            const SOPC_NodeId* nodeId,
                                                            SOPC_AttributeId attribId,
                                                            SOPC_Variant* previousValue,
                                                            SOPC_StatusCode prevStatusCode,
                                                            SOPC_Value_Timestamp prevSourceTs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_ASSERT(NULL != addSpaceAccess->operations);
    SOPC_Variant* currentValue = SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, node);
    SOPC_StatusCode currentStatusCode = SOPC_AddressSpace_Get_StatusCode(addSpaceAccess->addSpaceRef, node);
    SOPC_Value_Timestamp currentSourceTs = SOPC_AddressSpace_Get_SourceTs(addSpaceAccess->addSpaceRef, node);

    void* addedOp = NULL;
    SOPC_AddressSpaceAccessOperation* op = SOPC_Calloc(1, sizeof(*op));
    OpcUa_WriteValue* prevWV = SOPC_Calloc(1, sizeof(*prevWV));
    OpcUa_WriteValue* newWV = SOPC_Calloc(1, sizeof(*newWV));
    if (NULL == op || NULL == prevWV || NULL == newWV)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        addedOp = (void*) SOPC_SLinkedList_Prepend(addSpaceAccess->operations, 0, (uintptr_t) op);
        status = (NULL == addedOp ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        OpcUa_WriteValue_Initialize(prevWV);
        OpcUa_WriteValue_Initialize(newWV);
        op->operation = SOPC_ADDSPACE_WRITE;
        op->param1 = prevWV;
        op->param2 = newWV;
    }
    if (SOPC_STATUS_OK == status)
    {
        prevWV->AttributeId = attribId;
        status = SOPC_NodeId_Copy(&prevWV->NodeId, nodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        newWV->AttributeId = attribId;
        status = SOPC_NodeId_Copy(&newWV->NodeId, nodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        newWV->Value.Status = currentStatusCode;
        newWV->Value.SourceTimestamp = currentSourceTs.timestamp;
        newWV->Value.SourcePicoSeconds = currentSourceTs.picoSeconds;
        status = SOPC_Variant_Copy(&newWV->Value.Value, currentValue);
    }
    if (SOPC_STATUS_OK == status)
    {
        prevWV->Value.Status = prevStatusCode;
        prevWV->Value.SourceTimestamp = prevSourceTs.timestamp;
        prevWV->Value.SourcePicoSeconds = prevSourceTs.picoSeconds;
        SOPC_Variant_Move(&prevWV->Value.Value, previousValue);
    }
    if (SOPC_STATUS_OK != status)
    {
        if (NULL != addedOp)
        {
            SOPC_SLinkedList_PopHead(addSpaceAccess->operations);
        }
        SOPC_Free(op);
        OpcUa_WriteValue_Clear(prevWV);
        OpcUa_WriteValue_Clear(newWV);
        SOPC_Free(prevWV);
        SOPC_Free(newWV);
    }
    return status;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_WriteValue(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   const SOPC_NodeId* nodeId,
                                                   const SOPC_NumericRange* optNumRange,
                                                   const SOPC_Variant* value,
                                                   const SOPC_StatusCode* optStatus,
                                                   const SOPC_DateTime* optSourceTimestamp,
                                                   const uint16_t* optSourcePicoSeconds)
{
    if (NULL == addSpaceAccess || NULL == nodeId || NULL == value ||
        (NULL != optSourcePicoSeconds && NULL == optSourceTimestamp))
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_AddressSpace_Node* node = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, nodeId);
    if (NULL == node)
    {
        return OpcUa_BadNodeIdUnknown;
    }
    SOPC_Variant previousValue;
    SOPC_Variant_Initialize(&previousValue);
    SOPC_Variant* currentValue = SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, node);

    SOPC_StatusCode prevStatusCode = SOPC_AddressSpace_Get_StatusCode(addSpaceAccess->addSpaceRef, node);
    SOPC_Value_Timestamp prevSourceTs = SOPC_AddressSpace_Get_SourceTs(addSpaceAccess->addSpaceRef, node);

    if (NULL != optStatus)
    {
        bool res = SOPC_AddressSpace_Set_StatusCode(addSpaceAccess->addSpaceRef, node, *optStatus);
        if (!res)
        {
            return OpcUa_BadWriteNotSupported;
        }
    }

    if (NULL != optSourceTimestamp)
    {
        SOPC_Value_Timestamp newSourceTs;
        newSourceTs.timestamp = *optSourceTimestamp;
        if (NULL != optSourcePicoSeconds)
        {
            newSourceTs.picoSeconds = *optSourcePicoSeconds;
        }
        else
        {
            newSourceTs.picoSeconds = 0;
        }
        // If both defined to 0, set current time as source
        if (0 == newSourceTs.timestamp && 0 == newSourceTs.picoSeconds)
        {
            newSourceTs.timestamp = SOPC_Time_GetCurrentTimeUTC();
        }
        bool res = SOPC_AddressSpace_Set_SourceTs(addSpaceAccess->addSpaceRef, node, newSourceTs);
        if (!res)
        {
            // Note: no need to restore StatusCode, if address space was read only it shall have failed when setting it.
            return OpcUa_BadWriteNotSupported;
        }
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_StatusCode returnCode = SOPC_BadStatusMask;
    if (NULL == optNumRange)
    {
        // Overwrite current value
        SOPC_Variant_Move(&previousValue, currentValue);
        SOPC_Variant_Clear(currentValue);
        status = SOPC_Variant_Copy(currentValue, value);
        returnCode = (SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadOutOfMemory);
    }
    else
    {
        // Write given value with index range
        bool hasRange = false;
        status = SOPC_Variant_HasRange(currentValue, optNumRange, true, &hasRange);
        returnCode = (SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadIndexRangeInvalid);
        if (SOPC_STATUS_OK == status && !hasRange)
        {
            status = SOPC_STATUS_INVALID_STATE;
            returnCode = OpcUa_BadIndexRangeNoData;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Copy(&previousValue, currentValue);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Variant_SetRange(currentValue, value, optNumRange);
            }
            returnCode = (SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadOutOfMemory);
        }
    }

    if (SOPC_STATUS_OK == status && addSpaceAccess->recordOperations)
    {
        status = SOPC_InternalRecordOperation_Write(addSpaceAccess, node, nodeId, SOPC_AttributeId_Value,
                                                    &previousValue, prevStatusCode, prevSourceTs);
        returnCode = (SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadOutOfMemory);
    }

    if (SOPC_STATUS_OK != status)
    {
        // Write failed, restore previous values
        SOPC_Variant_Clear(currentValue);
        SOPC_Variant_Move(currentValue, &previousValue);
        bool res = SOPC_AddressSpace_Set_StatusCode(addSpaceAccess->addSpaceRef, node, prevStatusCode);
        SOPC_UNUSED_RESULT(res); // if false it means there is nothing to restore (constant)
        res = SOPC_AddressSpace_Set_SourceTs(addSpaceAccess->addSpaceRef, node, prevSourceTs);
        SOPC_UNUSED_RESULT(res); // if false it means there is nothing to restore (constant)
    }
    SOPC_Variant_Clear(&previousValue);

    return returnCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_AddVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                        const SOPC_ExpandedNodeId* parentNodeId,
                                                        const SOPC_NodeId* refTypeId,
                                                        const SOPC_NodeId* newNodeId,
                                                        const SOPC_QualifiedName* browseName,
                                                        const OpcUa_VariableAttributes* varAttributes,
                                                        const SOPC_ExpandedNodeId* typeDefId)
{
    if (NULL == addSpaceAccess || NULL == parentNodeId || NULL == refTypeId || NULL == newNodeId ||
        NULL == browseName || NULL == varAttributes || NULL == typeDefId)
    {
        return OpcUa_BadInvalidArgument;
    }

    if (!SOPC_AddressSpace_AreNodesReleasable(addSpaceAccess->addSpaceRef) ||
        SOPC_AddressSpace_AreReadOnlyNodes(addSpaceAccess->addSpaceRef))
    {
        return OpcUa_BadServiceUnsupported;
    }

    bool nodeIdAlreadyExsists = false;
    SOPC_UNUSED_RESULT(SOPC_AddressSpace_Get_Node(addSpaceAccess->addSpaceRef, newNodeId, &nodeIdAlreadyExsists));
    if (nodeIdAlreadyExsists)
    {
        return OpcUa_BadNodeIdExists;
    }

    SOPC_StatusCode retCode = SOPC_NodeMgtHelperInternal_CheckConstraints_AddVariable(
        addSpaceAccess->addSpaceRef, parentNodeId, refTypeId, browseName, typeDefId);
    if (!SOPC_IsGoodStatus(retCode))
    {
        return retCode;
    }

    retCode = OpcUa_BadOutOfMemory;
    SOPC_AddressSpace_Node* newNode = SOPC_Calloc(1, sizeof(*newNode));
    if (NULL == newNode)
    {
        return retCode;
    }
    SOPC_AddressSpace_Node_Initialize(addSpaceAccess->addSpaceRef, newNode, OpcUa_NodeClass_Variable);

    // Copy the main parameters not included in NodeAttributes structure
    OpcUa_VariableNode* varNode = &newNode->data.variable;
    // NodeID
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&varNode->NodeId, newNodeId);
    SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);

    // BrowseName
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_QualifiedName_Copy(&varNode->BrowseName, browseName);
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    // References from new node (backward to parent and forward to type)
    if (SOPC_STATUS_OK == status)
    {
        varNode->References = SOPC_Calloc(2, sizeof(*varNode->References));
        if (NULL == varNode->References)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            varNode->NoOfReferences = 2;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set HasTypeDefinition
        OpcUa_ReferenceNode* hasTypeDef = &varNode->References[0];
        hasTypeDef->IsInverse = false;
        hasTypeDef->ReferenceTypeId.Namespace = 0;
        hasTypeDef->ReferenceTypeId.IdentifierType = SOPC_IdentifierType_Numeric;
        hasTypeDef->ReferenceTypeId.Data.Numeric = OpcUaId_HasTypeDefinition;
        status = SOPC_ExpandedNodeId_Copy(&hasTypeDef->TargetId, typeDefId);
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set hierarchical reference to parent
        OpcUa_ReferenceNode* hierarchicalRef = &varNode->References[1];
        hierarchicalRef->IsInverse = true;
        status = SOPC_NodeId_Copy(&hierarchicalRef->ReferenceTypeId, &typeDefId->NodeId);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ExpandedNodeId_Copy(&hierarchicalRef->TargetId, parentNodeId);
        }
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    // Manage NodeAttributes
    if (SOPC_STATUS_OK == status)
    {
        // Note: set retCode in case of failure
        status = SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes(addSpaceAccess->addSpaceRef, newNode, varNode,
                                                                      varAttributes, &retCode);
    }
    // Set reciprocal reference from parent and add node to address space
    if (SOPC_STATUS_OK == status)
    {
        // reset retCode to default failure value
        retCode = OpcUa_BadOutOfMemory;
        status = SOPC_NodeMgtHelperInternal_AddRefChildToParentNode(addSpaceAccess->addSpaceRef, &parentNodeId->NodeId,
                                                                    newNodeId, refTypeId);
        // Add node to address space
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AddressSpace_Append(addSpaceAccess->addSpaceRef, newNode);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_ASSERT(SOPC_STATUS_OUT_OF_MEMORY == status);
                // Rollback reference added in parent
                SOPC_NodeMgtHelperInternal_RemoveLastRefInParentNode(addSpaceAccess->addSpaceRef,
                                                                     &parentNodeId->NodeId);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        retCode = SOPC_GoodGenericStatus;
    }
    else
    {
        // Clear and dealloc node
        SOPC_AddressSpace_Node_Clear(addSpaceAccess->addSpaceRef, newNode);
        SOPC_Free(newNode);
    }
    return retCode;
}
