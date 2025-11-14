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

#include "sopc_address_space_access_internal.h"

#include "sopc_address_space_utils_internal.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_node_mgt_helper_internal.h"
#include "sopc_toolkit_config_constants.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "util_variant.h"

// NodeIds used for DeleteNodes
#if 0 != S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES
static const SOPC_NodeId organizesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);
#endif
static const SOPC_NodeId hasComponentType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
static const SOPC_NodeId hasChildType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasChild);
static const SOPC_NodeId aggregatesType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Aggregates);
static const SOPC_NodeId hasSubtypeType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasSubtype);
static const SOPC_NodeId hasTypeDefinitionType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasTypeDefinition);
static const SOPC_NodeId hasModellingRuleType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasModellingRule);
static const SOPC_NodeId modellingRuleMandatoryType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ModellingRule_Mandatory);
static const SOPC_NodeId modellingRuleOptionalType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ModellingRule_Optional);

static SOPC_StatusCode AddSingleVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                             const SOPC_ExpandedNodeId* parentNodeId,
                                             const SOPC_NodeId* refToParentTypeId,
                                             const SOPC_NodeId* newNodeId,
                                             const SOPC_QualifiedName* browseName,
                                             const OpcUa_VariableAttributes* varAttributes,
                                             const SOPC_ExpandedNodeId* typeDefId);

static SOPC_StatusCode AddSingleObjectNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                           const SOPC_ExpandedNodeId* parentNodeId,
                                           const SOPC_NodeId* refToParentTypeId,
                                           const SOPC_NodeId* newNodeId,
                                           const SOPC_QualifiedName* browseName,
                                           const OpcUa_ObjectAttributes* objAttributes,
                                           const SOPC_ExpandedNodeId* typeDefId);

/* Log macros used for references between two SOPC_AddressSpace_Node */
#define LOG_NODE_REF(level, addSpace, msg, sourceNode, targetNode)                                         \
    {                                                                                                      \
        SOPC_ASSERT(NULL != sourceNode && NULL != targetNode && NULL != addSpace);                         \
        const SOPC_NodeId* sourceNodeId = SOPC_AddressSpace_Get_NodeId(addSpace, sourceNode);              \
        const SOPC_NodeId* targetNodeId = SOPC_AddressSpace_Get_NodeId(addSpace, targetNode);              \
        SOPC_ASSERT(NULL != sourceNodeId && NULL != targetNodeId);                                         \
        char* sourceNodeIdStr = SOPC_NodeId_ToCString(sourceNodeId);                                       \
        char* targetNodeIdStr = SOPC_NodeId_ToCString(targetNodeId);                                       \
        SOPC_ASSERT(NULL != sourceNodeIdStr && NULL != targetNodeIdStr);                                   \
        switch (level)                                                                                     \
        {                                                                                                  \
        case SOPC_LOG_LEVEL_ERROR:                                                                         \
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, msg, sourceNodeIdStr, targetNodeIdStr);   \
            break;                                                                                         \
        case SOPC_LOG_LEVEL_DEBUG:                                                                         \
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, msg, sourceNodeIdStr, targetNodeIdStr);   \
            break;                                                                                         \
        case SOPC_LOG_LEVEL_WARNING:                                                                       \
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, msg, sourceNodeIdStr, targetNodeIdStr); \
            break;                                                                                         \
        default:                                                                                           \
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, msg, sourceNodeIdStr, targetNodeIdStr);   \
            break;                                                                                         \
        }                                                                                                  \
        SOPC_Free(sourceNodeIdStr);                                                                        \
        SOPC_Free(targetNodeIdStr);                                                                        \
    }

struct _SOPC_AddressSpaceAccess
{
    SOPC_AddressSpace* addSpaceRef;
    bool recordOperations;
    SOPC_SLinkedList* operations; // SOPC_AddressSpaceAccessOperation* prepended operations since creation
};

// Constant to define initial capacity of array of reference description.
#define BROWSE_REFERENCE_DESCRIPTION_RESULT_LEN_ARRAY 10

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
        SOPC_EncodeableObject_Delete(&OpcUa_WriteValue_EncodeableType, (void**) &op->param1);
        SOPC_EncodeableObject_Delete(&OpcUa_WriteValue_EncodeableType, (void**) &op->param2);
        break;
    case SOPC_ADDSPACE_CHANGE_NODE:
        SOPC_NodeId_Clear((SOPC_NodeId*) op->param2);
        SOPC_Free((void*) op->param2);
        op->param2 = (uintptr_t) NULL;
        break;
    default:
        SOPC_ASSERT(false);
    }
    SOPC_Free(op);
}

static bool is_type_or_subtype(SOPC_AddressSpace* addSpace,
                               const SOPC_NodeId* actualType,
                               const SOPC_NodeId* expectedType)
{
    return SOPC_NodeId_Equal(actualType, expectedType) ||
           SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(addSpace, SOPC_RECURSION_LIMIT, actualType, actualType,
                                                              expectedType);
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
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, node),
                                                    true);
        break;
    case SOPC_AttributeId_NodeClass:
        val = util_variant__new_Variant_from_NodeClass(
            *SOPC_AddressSpace_Get_NodeClass(addSpaceAccess->addSpaceRef, node));
        break;
    case SOPC_AttributeId_BrowseName:
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_QualifiedName(
            SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, node), true);
        break;
    case SOPC_AttributeId_DisplayName:
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_LocalizedText(
            SOPC_AddressSpace_Get_DisplayName(addSpaceAccess->addSpaceRef, node), true);
        break;
    case SOPC_AttributeId_Description:
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_LocalizedText(
            SOPC_AddressSpace_Get_Description(addSpaceAccess->addSpaceRef, node), true);
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
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, node),
                                                     true);
        break;
    case SOPC_AttributeId_DataType:
        // Make a deep copy to avoid any possible address space change side-effect,  WriteValue shall be used for that
        val = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Get_DataType(addSpaceAccess->addSpaceRef, node),
                                                    true);
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
        val = util_variant__new_Variant_from_Bool(SOPC_AddressSpace_Get_Historizing(addSpaceAccess->addSpaceRef, node));
        break;
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
        op->param1 = (uintptr_t) prevWV;
        op->param2 = (uintptr_t) newWV;
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

static SOPC_StatusCode check_valid_params_and_state(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                    const SOPC_ExpandedNodeId* parentNodeId,
                                                    const SOPC_NodeId* refToParentTypeId,
                                                    const SOPC_NodeId* newNodeId,
                                                    const SOPC_QualifiedName* browseName,
                                                    const OpcUa_NodeAttributes* NodeAttributes,
                                                    const SOPC_ExpandedNodeId* typeDefId,
                                                    const OpcUa_NodeClass nodeClass)
{
    if (NULL == addSpaceAccess || NULL == parentNodeId || NULL == refToParentTypeId || NULL == newNodeId ||
        NULL == browseName || NULL == NodeAttributes ||
        ((OpcUa_NodeClass_Object == nodeClass || OpcUa_NodeClass_Variable == nodeClass) && NULL == typeDefId))
    {
        return OpcUa_BadInvalidArgument;
    }

    if (!SOPC_AddressSpace_AreNodesReleasable(addSpaceAccess->addSpaceRef) ||
        SOPC_AddressSpace_AreReadOnlyNodes(addSpaceAccess->addSpaceRef))
    {
        return OpcUa_BadServiceUnsupported;
    }

    bool nodeIdAlreadyExists = false;
    SOPC_AddressSpace_Node* maybeNode =
        SOPC_AddressSpace_Get_Node(addSpaceAccess->addSpaceRef, newNodeId, &nodeIdAlreadyExists);
    SOPC_UNUSED_RESULT(maybeNode);
    if (nodeIdAlreadyExists)
    {
        return OpcUa_BadNodeIdExists;
    }

    return 0;
}

// Set reciprocal reference from parent and add node to address space
static SOPC_ReturnStatus add_node_and_set_reciprocal_reference(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                               const SOPC_NodeId* parentNodeId,
                                                               const SOPC_NodeId* childNodeId,
                                                               SOPC_AddressSpace_Node* newNode,
                                                               const SOPC_NodeId* refToParentTypeId,
                                                               const SOPC_NodeId* refTypeDefId)
{
    SOPC_ReturnStatus status = SOPC_NodeMgtHelperInternal_AddRefToNode(addSpaceAccess->addSpaceRef, parentNodeId,
                                                                       childNodeId, refToParentTypeId, false);
    uint8_t nbRefAdded = SOPC_STATUS_OK == status ? 1 : 0;

#if 0 != S2OPC_NODE_ADD_INVERSE_TYPEDEF
    if (SOPC_STATUS_OK == status && refTypeDefId != NULL)
    {
        status = SOPC_NodeMgtHelperInternal_AddRefToNode(addSpaceAccess->addSpaceRef, refTypeDefId, childNodeId,
                                                         &hasTypeDefinitionType, true);
        nbRefAdded = SOPC_STATUS_OK == status ? (uint8_t)(nbRefAdded + 1) : nbRefAdded;
    }
#else
    SOPC_UNUSED_ARG(refTypeDefId);
#endif // S2OPC_NODE_ADD_INVERSE_TYPEDEF

    // Add node to address space
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AddressSpace_Append(addSpaceAccess->addSpaceRef, newNode);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_ASSERT(SOPC_STATUS_OUT_OF_MEMORY == status);
        if (nbRefAdded >= 1)
        {
            // Rollback reference added in parent
            SOPC_NodeMgtHelperInternal_RemoveLastRefInTargetNode(addSpaceAccess->addSpaceRef, parentNodeId);
        }
        if (nbRefAdded == 2)
        {
            // Rollback reference added in type
            SOPC_NodeMgtHelperInternal_RemoveLastRefInTargetNode(addSpaceAccess->addSpaceRef, refTypeDefId);
        }
    }
    else
    {
        // Record operation if required (node added)
        if (addSpaceAccess->recordOperations)
        {
            SOPC_ASSERT(NULL != addSpaceAccess->operations);
            SOPC_AddressSpaceAccessOperation* op = SOPC_Calloc(1, sizeof(*op));
            SOPC_NodeId* nodeIdCopy = SOPC_Calloc(1, sizeof(SOPC_NodeId));
            status = SOPC_NodeId_Copy(nodeIdCopy, childNodeId);
            const void* addedOp = NULL;
            if (NULL != op && SOPC_STATUS_OK == status && NULL != nodeIdCopy)
            {
                addedOp = (const void*) SOPC_SLinkedList_Prepend(addSpaceAccess->operations, 0, (uintptr_t) op);
            }
            if (NULL != addedOp)
            {
                op->operation = SOPC_ADDSPACE_CHANGE_NODE;
                op->param1 = true; // true for add
                op->param2 = (uintptr_t) nodeIdCopy;
            }
            else
            {
                SOPC_Free(op);
                SOPC_NodeId_Clear(nodeIdCopy);
                SOPC_Free(nodeIdCopy);
            }
        }
    }

    return status;
}

/**
 * \brief Searchs 1st \p expectedRefType reference of the \p nodeAddspace node. Returns NULL if not found.
 */
static SOPC_ExpandedNodeId* GetExpectedReference(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                 SOPC_AddressSpace_Node* nodeAddspace,
                                                 const SOPC_NodeId* expectedRefType,
                                                 const bool isInverse)
{
    SOPC_ExpandedNodeId* refTargetExpId = NULL;
    bool isExpectedRef = false;
    int32_t indexRefChild = 0;
    OpcUa_ReferenceNode* reference = NULL;
    const int32_t* n_refsChild = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, nodeAddspace);
    OpcUa_ReferenceNode** refsChild = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, nodeAddspace);
    SOPC_ASSERT(NULL != refsChild && NULL != n_refsChild);

    while (!isExpectedRef && indexRefChild < *n_refsChild)
    {
        reference = &(*refsChild)[indexRefChild];
        isExpectedRef = is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId, expectedRefType);
        indexRefChild++;
    }
    if (isExpectedRef && reference->IsInverse == isInverse)
    {
        refTargetExpId = &reference->TargetId;
    }
    return refTargetExpId;
}

/**
 * \brief Searches among all references of a target node whether it has a reference to modeling rules.
 *        If it is found, it checks whether it is mandatory or optional and returns the result of this search
 *        in function of the compilation variable ::S2OPC_NODE_ADD_OPTIONAL.
 *
 * \param addSpaceAccess  The AddressSpace Access used for AddNodes operation
 * \param targetNodeId    The pointer of target nodeId on which the search is performed.
 *
 * \return True if reference is found, false otherwise
 */
static bool IsModellingRuleToAdd(SOPC_AddressSpaceAccess* addSpaceAccess, const SOPC_NodeId* targetNodeId)
{
    bool resMandatory = false;
    bool resOptional = false;
    bool res = false;
    SOPC_AddressSpace_Node* targetNodeAddspace = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, targetNodeId);
    const SOPC_ExpandedNodeId* modellingRuleTargetRefId =
        GetExpectedReference(addSpaceAccess, targetNodeAddspace, &hasModellingRuleType, false);
    if (modellingRuleTargetRefId != NULL)
    {
        resMandatory = SOPC_NodeId_Equal(&modellingRuleTargetRefId->NodeId, &modellingRuleMandatoryType);
        resOptional = !resMandatory && SOPC_NodeId_Equal(&modellingRuleTargetRefId->NodeId, &modellingRuleOptionalType);
    }
    res = resMandatory;
#if 0 != S2OPC_NODE_ADD_OPTIONAL
    res = res || resOptional;
#else
    SOPC_UNUSED_ARG(resOptional); // To avoid compilation error when S2OPC_NODE_ADD_OPTIONAL selected
#endif // S2OPC_NODE_ADD_OPTIONAL
    return res;
}

static SOPC_StatusCode AddSingleObjectNodeFromInstanceDeclaration(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                                  const SOPC_ExpandedNodeId* parentNodeId,
                                                                  const SOPC_NodeId* refToParentTypeId,
                                                                  const SOPC_NodeId* newNodeId,
                                                                  SOPC_AddressSpace_Node* nodeInstDecl)
{
    // Setup attributes
    // Attribute WriteMask is not supported
    OpcUa_ObjectAttributes objAttributes;
    OpcUa_ObjectAttributes_Initialize(&objAttributes);
    objAttributes.SpecifiedAttributes = OpcUa_NodeAttributesMask_DisplayName | OpcUa_NodeAttributesMask_Description |
                                        OpcUa_NodeAttributesMask_EventNotifier;
    const SOPC_LocalizedText* displayName =
        SOPC_AddressSpace_Get_DisplayName(addSpaceAccess->addSpaceRef, nodeInstDecl);
    SOPC_LocalizedText_Copy(&objAttributes.DisplayName, displayName);
    const SOPC_LocalizedText* description =
        SOPC_AddressSpace_Get_Description(addSpaceAccess->addSpaceRef, nodeInstDecl);
    SOPC_LocalizedText_Copy(&objAttributes.Description, description);
#if 0 != S2OPC_EVENT_MANAGEMENT
    const SOPC_Byte eventNotifier = SOPC_AddressSpace_Get_EventNotifier(addSpaceAccess->addSpaceRef, nodeInstDecl);
    objAttributes.EventNotifier = eventNotifier;
#endif // S2OPC_EVENT_MANAGEMENT

    const SOPC_QualifiedName* browseName = SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_ExpandedNodeId* typeDefId =
        GetExpectedReference(addSpaceAccess, nodeInstDecl, &hasTypeDefinitionType, false);

    // Add Node
    SOPC_StatusCode stCode = AddSingleObjectNode(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                                 &objAttributes, typeDefId);
    OpcUa_ObjectAttributes_Clear(&objAttributes);
    return stCode;
}

static SOPC_StatusCode AddSingleVariableNodeFromInstanceDeclaration(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                                    const SOPC_ExpandedNodeId* parentNodeId,
                                                                    const SOPC_NodeId* refToParentTypeId,
                                                                    const SOPC_NodeId* newNodeId,
                                                                    SOPC_AddressSpace_Node* nodeInstDecl)
{
    // Get instance declaration attribute values
    const SOPC_LocalizedText* displayName =
        SOPC_AddressSpace_Get_DisplayName(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_LocalizedText* description =
        SOPC_AddressSpace_Get_Description(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_Variant* value = SOPC_AddressSpace_Get_Value(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_NodeId* dataType = SOPC_AddressSpace_Get_DataType(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const int32_t* valueRank = SOPC_AddressSpace_Get_ValueRank(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const int32_t noOfArrayDimensions =
        SOPC_AddressSpace_Get_NoOfArrayDimensions(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_Byte accessLevel = SOPC_AddressSpace_Get_AccessLevel(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_QualifiedName* browseName = SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_ExpandedNodeId* typeDefId =
        GetExpectedReference(addSpaceAccess, nodeInstDecl, &hasTypeDefinitionType, false);

    // Setup attributes for new variable node
    // Note: attribute WriteMask is not supported in add operation
    OpcUa_VariableAttributes varAttributes;
    OpcUa_VariableAttributes_Initialize(&varAttributes);
    varAttributes.SpecifiedAttributes = OpcUa_NodeAttributesMask_DisplayName | OpcUa_NodeAttributesMask_Description |
                                        OpcUa_NodeAttributesMask_Value | OpcUa_NodeAttributesMask_DataType |
                                        OpcUa_NodeAttributesMask_ValueRank | OpcUa_NodeAttributesMask_ArrayDimensions |
                                        OpcUa_NodeAttributesMask_AccessLevel;

    SOPC_ReturnStatus status = SOPC_LocalizedText_Copy(&varAttributes.DisplayName, displayName);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LocalizedText_Copy(&varAttributes.Description, description);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Variant_Initialize(&varAttributes.Value);
        status = SOPC_Variant_ShallowCopy(&varAttributes.Value, value);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&varAttributes.DataType, dataType);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Array dimensions copy
        if (noOfArrayDimensions > 0)
        {
            const uint32_t* arrayDimensions =
                SOPC_AddressSpace_Get_ArrayDimensions(addSpaceAccess->addSpaceRef, nodeInstDecl);
            varAttributes.ArrayDimensions = SOPC_Calloc((size_t) noOfArrayDimensions, sizeof(uint32_t));
            status = SOPC_Copy_Array(noOfArrayDimensions, (void*) varAttributes.ArrayDimensions,
                                     (const void*) arrayDimensions, sizeof(uint32_t), &SOPC_UInt32_CopyAux);
            if (SOPC_STATUS_OK == status)
            {
                varAttributes.NoOfArrayDimensions = noOfArrayDimensions;
            }
            else
            {
                SOPC_Free(varAttributes.ArrayDimensions);
                varAttributes.ArrayDimensions = NULL;
                varAttributes.NoOfArrayDimensions = 0;
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        varAttributes.ValueRank = *valueRank;
        varAttributes.AccessLevel = accessLevel;
    }

    SOPC_StatusCode stCode = OpcUa_BadOutOfMemory;
    if (SOPC_STATUS_OK == status)
    {
        // Add Node
        stCode = AddSingleVariableNode(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                       &varAttributes, typeDefId);
    }
    OpcUa_VariableAttributes_Clear(&varAttributes);
    return stCode;
}

static SOPC_StatusCode AddSingleMethodNodeFromInstanceDeclaration(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                                  const SOPC_ExpandedNodeId* parentNodeId,
                                                                  const SOPC_NodeId* refToParentTypeId,
                                                                  const SOPC_NodeId* newNodeId,
                                                                  SOPC_AddressSpace_Node* nodeInstDecl)
{
    // Get instance declaration attribute values
    const SOPC_LocalizedText* displayName =
        SOPC_AddressSpace_Get_DisplayName(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_LocalizedText* description =
        SOPC_AddressSpace_Get_Description(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_Boolean executable = SOPC_AddressSpace_Get_Executable(addSpaceAccess->addSpaceRef, nodeInstDecl);
    const SOPC_QualifiedName* browseName = SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, nodeInstDecl);

    // Setup attributes for new variable node
    // Attribute WriteMask is not supported
    OpcUa_MethodAttributes metAttributes;
    OpcUa_MethodAttributes_Initialize(&metAttributes);
    metAttributes.SpecifiedAttributes = OpcUa_NodeAttributesMask_DisplayName | OpcUa_NodeAttributesMask_Description |
                                        OpcUa_NodeAttributesMask_Executable;
    SOPC_ReturnStatus status = SOPC_LocalizedText_Copy(&metAttributes.DisplayName, displayName);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LocalizedText_Copy(&metAttributes.Description, description);
    }
    if (SOPC_STATUS_OK == status)
    {
        metAttributes.Executable = executable;
    }
    // Add Node
    SOPC_StatusCode stCode = OpcUa_BadOutOfMemory;
    if (SOPC_STATUS_OK == status)
    {
        stCode = SOPC_AddressSpaceAccess_AddMethodNode(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId,
                                                       browseName, &metAttributes);
    }
    OpcUa_MethodAttributes_Clear(&metAttributes);
    return stCode;
}

static SOPC_StatusCode AddChildNodesRecursive(SOPC_AddressSpaceAccess* addSpaceAccess,
                                              const SOPC_ExpandedNodeId* rootNode,
                                              const SOPC_NodeId* typeNodeId,
                                              int32_t recursionLimit)
{
    // Check recursion limit
    recursionLimit--;
    if (recursionLimit < 0)
    {
        return OpcUa_BadWouldBlock;
    }
    const int recursionLevel = SOPC_RECURSION_LIMIT - recursionLimit;
    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;

    // Get parent type references
    // Note: typeNodeId is either a type node or its instance declaration children
    SOPC_AddressSpace_Node* parentType_NodeAddspace =
        SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, typeNodeId);
    const int32_t* parentType_noOfRefs =
        SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, parentType_NodeAddspace);
    OpcUa_ReferenceNode** parentType_refs =
        SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, parentType_NodeAddspace);
    SOPC_ASSERT(NULL != parentType_refs && NULL != parentType_noOfRefs);
    // For each references
    SOPC_StatusCode firstChildErrorCode = SOPC_GoodGenericStatus;
    for (int parentType_indexRef = 0; parentType_indexRef < *parentType_noOfRefs && SOPC_IsGoodStatus(stCode);
         parentType_indexRef++)
    {
        const OpcUa_ReferenceNode* parentType_ref = &((*parentType_refs)[parentType_indexRef]);
        bool isAggregatesRef =
            is_type_or_subtype(addSpaceAccess->addSpaceRef, &parentType_ref->ReferenceTypeId, &aggregatesType);
        // For a forward 'Aggregates' type or sub-type reference
        if (isAggregatesRef && !parentType_ref->IsInverse)
        {
            // Get Child Node
            SOPC_ExpandedNodeId childType_ExpNode = parentType_ref->TargetId;
            char* childTypeNodeStr = SOPC_NodeId_ToCString(&childType_ExpNode.NodeId);

            // Check whether a child node should be added according to the modeling rule.
            bool hasModellingRuleToAdd = IsModellingRuleToAdd(addSpaceAccess, &childType_ExpNode.NodeId);
            bool ignoreChildAddedForLog = false;
            if (hasModellingRuleToAdd)
            {
                SOPC_StatusCode resCodeAddChildNode = SOPC_GoodGenericStatus;
                // Child node needs to be added. Create it
                SOPC_NodeId* newChildNodeId =
                    SOPC_AddressSpace_GetFreshNodeId(addSpaceAccess->addSpaceRef, rootNode->NodeId.Namespace);
                if (NULL != newChildNodeId)
                {
                    SOPC_AddressSpace_Node* childTypeNode =
                        SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, &childType_ExpNode.NodeId);
                    const SOPC_QualifiedName* browseName =
                        SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, childTypeNode);
                    const OpcUa_NodeClass* childType_NodeNodeClass =
                        SOPC_AddressSpace_Get_NodeClass(addSpaceAccess->addSpaceRef, childTypeNode);
                    // Add new child node
                    switch (*childType_NodeNodeClass)
                    {
                    case OpcUa_NodeClass_Object:
                        resCodeAddChildNode = AddSingleObjectNodeFromInstanceDeclaration(
                            addSpaceAccess, rootNode, &parentType_ref->ReferenceTypeId, newChildNodeId, childTypeNode);
                        break;
                    case OpcUa_NodeClass_Variable:
                        resCodeAddChildNode = AddSingleVariableNodeFromInstanceDeclaration(
                            addSpaceAccess, rootNode, &parentType_ref->ReferenceTypeId, newChildNodeId, childTypeNode);
                        break;
                    case OpcUa_NodeClass_Method:
                        resCodeAddChildNode = AddSingleMethodNodeFromInstanceDeclaration(
                            addSpaceAccess, rootNode, &parentType_ref->ReferenceTypeId, newChildNodeId, childTypeNode);
                        break;
                    default:
                        ignoreChildAddedForLog = true;
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                                 "AddNodes children (level=%d): Unexpected node class for child node: "
                                                 "%s ! Ignoring node ...",
                                                 recursionLevel, childTypeNodeStr);
                        break;
                    }
                    // Recursion : Add Child child node
                    if (SOPC_IsGoodStatus(resCodeAddChildNode))
                    {
                        if (!ignoreChildAddedForLog)
                        {
                            char* newChildNodeIdStr = SOPC_NodeId_ToCString(newChildNodeId);
                            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                                  "AddNodes children (level=%d): child node id=%s based on instance "
                                                  "declaration node id=%s successfully added",
                                                  recursionLevel,
                                                  NULL != newChildNodeIdStr ? newChildNodeIdStr : "null",
                                                  childTypeNodeStr);
                            SOPC_Free(newChildNodeIdStr);
                        }
                        SOPC_ExpandedNodeId newChildExpNode;
                        SOPC_ExpandedNodeId_Initialize(&newChildExpNode);
                        newChildExpNode.NodeId = *newChildNodeId;
                        stCode = AddChildNodesRecursive(addSpaceAccess, &newChildExpNode, &childType_ExpNode.NodeId,
                                                        recursionLimit);
                    }
                    else if (OpcUa_BadBrowseNameDuplicated == resCodeAddChildNode)
                    {
                        SOPC_Logger_TraceDebug(
                            SOPC_LOG_MODULE_CLIENTSERVER,
                            "AddNodes children (level=%d): ignored node id=%s due to identical browseName=%s",
                            recursionLevel, childTypeNodeStr, SOPC_String_GetRawCString(&browseName->Name));
                    }
                    else
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                                 "AddNodes children (level=%d): child node based on instance "
                                                 "declaration node id=%s addition failed with status 0x%08" PRIX32
                                                 ". Ignoring node.",
                                                 recursionLevel, childTypeNodeStr, stCode);
                        firstChildErrorCode =
                            SOPC_IsGoodStatus(firstChildErrorCode) ? resCodeAddChildNode : firstChildErrorCode;
                        stCode = SOPC_GoodGenericStatus; // reset status
                    }
                    SOPC_NodeId_Clear(newChildNodeId);
                    SOPC_Free(newChildNodeId);
                    newChildNodeId = NULL;
                }
                else
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "AddNodes children (level=%d): Failed to get a fresh nodeId to add child node based on "
                        "instance declaration id=%s. "
                        "SOPC_FRESH_NODEID_MAX_RETRIES might be increased to find a fresh one (currently = %d)",
                        recursionLevel, childTypeNodeStr, SOPC_FRESH_NODEID_MAX_RETRIES);
                    stCode = OpcUa_BadNodeIdInvalid;
                }
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AddNodes children (level=%d): %s, not added as not mandatory", recursionLevel,
                                       childTypeNodeStr);
            }
            SOPC_Free(childTypeNodeStr);
        }
    }
    if (SOPC_IsGoodStatus(stCode) && !SOPC_IsGoodStatus(firstChildErrorCode))
    {
        stCode = firstChildErrorCode;
    }
    return stCode;
}

static SOPC_StatusCode AddChildNodesRecursiveFromType(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                      const SOPC_NodeId* rootNode,
                                                      const SOPC_NodeId* typeNode,
                                                      int32_t recursionLimit)
{
    if (addSpaceAccess == NULL || rootNode == NULL || typeNode == NULL)
    {
        return OpcUa_BadInvalidArgument;
    }
    // Check recursion limit
    recursionLimit--;
    if (recursionLimit < 0)
    {
        return OpcUa_BadWouldBlock;
    }

    // Set the new expanded node id.
    SOPC_ExpandedNodeId rootNodeExpId;
    SOPC_ExpandedNodeId_Initialize(&rootNodeExpId);
    rootNodeExpId.NodeId = *rootNode;

    // Debug log trace to indicate the type recursion level
    if (SOPC_LOG_LEVEL_DEBUG == SOPC_Logger_GetTraceLogLevel())
    {
        // Get string representation of node ids
        const int recursionLevel = SOPC_RECURSION_LIMIT - recursionLimit;
        char* rootNodeIdStr = SOPC_NodeId_ToCString(rootNode);
        char* typeNodeIdStr = SOPC_NodeId_ToCString(typeNode);
        SOPC_Logger_TraceDebug(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "AddChildNodesRecursiveFromType (type level=%d): Automatically adding children to node %s based "
            "on its (super)type id=%s",
            recursionLevel, rootNodeIdStr, typeNodeIdStr);
        SOPC_Free(rootNodeIdStr);
        SOPC_Free(typeNodeIdStr);
    }

    // add nodes from direct type
    SOPC_StatusCode stCode = AddChildNodesRecursive(addSpaceAccess, &rootNodeExpId, typeNode, SOPC_RECURSION_LIMIT);

    // add nodes from parent type recursively (overloaded nodes will be ignored as they have same browse name)
    SOPC_AddressSpace_Node* parentType_NodeAddspace = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, typeNode);
    const SOPC_ExpandedNodeId* superType_ExpNode =
        GetExpectedReference(addSpaceAccess, parentType_NodeAddspace, &hasSubtypeType, true);
    if (SOPC_IsGoodStatus(stCode) && superType_ExpNode != NULL)
    {
        stCode = AddChildNodesRecursiveFromType(addSpaceAccess, rootNode, &superType_ExpNode->NodeId, recursionLimit);
    }
    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_GetFreshNodeId(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                       uint16_t nsIndex,
                                                       SOPC_NodeId* freshNodeId)
{
    if (NULL == addSpaceAccess || NULL == freshNodeId)
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_NodeId* freshNid = SOPC_AddressSpace_GetFreshNodeId(addSpaceAccess->addSpaceRef, nsIndex);
    if (freshNid != NULL)
    {
        // Avoid check-code error: freshNodeId is not a NULL pointer already checked in stCode.
        SOPC_ASSERT(freshNodeId != NULL);
        *freshNodeId = *freshNid;
        SOPC_NodeId_Initialize(freshNid);
        SOPC_Free(freshNid);
        freshNid = NULL;
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "AddNodes: Fail to get a fresh nodeId. You can increase the number of tries by modifying "
            "SOPC_FRESH_NODEID_MAX_RETRIES (currently = %d)",
            SOPC_FRESH_NODEID_MAX_RETRIES);
        stCode = OpcUa_BadNodeIdInvalid;
    }

    return stCode;
}

static SOPC_StatusCode AddSingleVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                             const SOPC_ExpandedNodeId* parentNodeId,
                                             const SOPC_NodeId* refToParentTypeId,
                                             const SOPC_NodeId* newNodeId,
                                             const SOPC_QualifiedName* browseName,
                                             const OpcUa_VariableAttributes* varAttributes,
                                             const SOPC_ExpandedNodeId* typeDefId)
{
    SOPC_StatusCode stCode =
        check_valid_params_and_state(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                     (const OpcUa_NodeAttributes*) varAttributes, typeDefId, OpcUa_NodeClass_Variable);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return stCode;
    }

    stCode = SOPC_NodeMgtHelperInternal_CheckConstraints_AddNode(
        OpcUa_NodeClass_Variable, addSpaceAccess->addSpaceRef, parentNodeId, refToParentTypeId, browseName, typeDefId);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return stCode;
    }

    stCode = OpcUa_BadOutOfMemory;
    SOPC_AddressSpace_Node* newNode = SOPC_Calloc(1, sizeof(*newNode));
    if (NULL == newNode)
    {
        return stCode;
    }
    SOPC_AddressSpace_Node_Initialize(addSpaceAccess->addSpaceRef, newNode, OpcUa_NodeClass_Variable);

    // Copy the main parameters not included in NodeAttributes structure
    OpcUa_VariableNode* varNode = &newNode->data.variable;
    SOPC_ReturnStatus status = SOPC_NodeMgtHelperInternal_CopyDataInNode((OpcUa_Node*) varNode, parentNodeId, newNodeId,
                                                                         refToParentTypeId, browseName, typeDefId);

    // Manage NodeAttributes
    if (SOPC_STATUS_OK == status)
    {
        // Note: set stCode in case of failure
        status = SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes(addSpaceAccess->addSpaceRef, newNode, varNode,
                                                                      varAttributes, &stCode);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = add_node_and_set_reciprocal_reference(addSpaceAccess, &parentNodeId->NodeId, newNodeId, newNode,
                                                       refToParentTypeId, &typeDefId->NodeId);
        // reset stCode to default failure value
        stCode = OpcUa_BadOutOfMemory;
    }

    if (SOPC_STATUS_OK == status)
    {
        stCode = SOPC_GoodGenericStatus;
    }
    else
    {
        // Clear and dealloc node
        SOPC_AddressSpace_Node_Clear(addSpaceAccess->addSpaceRef, newNode);
        SOPC_Free(newNode);
    }
    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_AddVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                        const SOPC_ExpandedNodeId* parentNodeId,
                                                        const SOPC_NodeId* refToParentTypeId,
                                                        const SOPC_NodeId* newNodeId,
                                                        const SOPC_QualifiedName* browseName,
                                                        const OpcUa_VariableAttributes* varAttributes,
                                                        const SOPC_ExpandedNodeId* typeDefId,
                                                        const bool addChildNodesFromType)
{
    // All parameters are verified in AddSingleVariableNode. Here, only those used before are checked.
    if (NULL == addSpaceAccess || NULL == newNodeId || NULL == typeDefId)
    {
        return OpcUa_BadInvalidArgument;
    }

    char* newNodeIdStr = SOPC_NodeId_ToCString(newNodeId);
    char* typeDefIdStr = SOPC_NodeId_ToCString(&typeDefId->NodeId);

    // Add new node
    SOPC_StatusCode stCode = AddSingleVariableNode(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId,
                                                   browseName, varAttributes, typeDefId);

    if (SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "AddNode: Variable id=%s of type: %s successfully added. %s", newNodeIdStr, typeDefIdStr,
                              addChildNodesFromType ? " Next step will automatically add child nodes from node type."
                                                    : " End of treatment.");
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddNode: Variable id=%s of type: %s failed with status: 0x%08" PRIX32 ".", newNodeIdStr,
                               typeDefIdStr, stCode);
    }
    // Add child nodes if required
    if (SOPC_IsGoodStatus(stCode) && addChildNodesFromType)
    {
        SOPC_StatusCode stCodeLog =
            AddChildNodesRecursiveFromType(addSpaceAccess, newNodeId, &typeDefId->NodeId, SOPC_RECURSION_LIMIT);
        if (!SOPC_IsGoodStatus(stCodeLog))
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "AddNode: Variable id=%s: Failure occurred adding child nodes based on type. The root node "
                "was created successfully but uncertain number of children was created (see previous log details).",
                newNodeIdStr);
            stCode = SOPC_UncertainStatusMask; /* Generic Uncertain status */
        }
    }
    SOPC_Free(typeDefIdStr);
    SOPC_Free(newNodeIdStr);

    return stCode;
}

static SOPC_StatusCode AddSingleObjectNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                           const SOPC_ExpandedNodeId* parentNodeId,
                                           const SOPC_NodeId* refToParentTypeId,
                                           const SOPC_NodeId* newNodeId,
                                           const SOPC_QualifiedName* browseName,
                                           const OpcUa_ObjectAttributes* objAttributes,
                                           const SOPC_ExpandedNodeId* typeDefId)
{
    SOPC_StatusCode stCode =
        check_valid_params_and_state(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                     (const OpcUa_NodeAttributes*) objAttributes, typeDefId, OpcUa_NodeClass_Object);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return stCode;
    }

    stCode = SOPC_NodeMgtHelperInternal_CheckConstraints_AddNode(
        OpcUa_NodeClass_Object, addSpaceAccess->addSpaceRef, parentNodeId, refToParentTypeId, browseName, typeDefId);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return stCode;
    }

    stCode = OpcUa_BadOutOfMemory;
    SOPC_AddressSpace_Node* newNode = SOPC_Calloc(1, sizeof(*newNode));
    if (NULL == newNode)
    {
        return stCode;
    }

    // ADD OBJECT
    SOPC_AddressSpace_Node_Initialize(addSpaceAccess->addSpaceRef, newNode, OpcUa_NodeClass_Object);
    // Copy the main parameters not included in NodeAttributes structure
    OpcUa_ObjectNode* objNode = &newNode->data.object;
    SOPC_ReturnStatus status = SOPC_NodeMgtHelperInternal_CopyDataInNode((OpcUa_Node*) objNode, parentNodeId, newNodeId,
                                                                         refToParentTypeId, browseName, typeDefId);

    // Manage NodeAttributes
    if (SOPC_STATUS_OK == status)
    {
        // Note: set stCode in case of failure
        status = SOPC_NodeMgtHelperInternal_AddObjectNodeAttributes(objNode, objAttributes, &stCode);
    }

    // COMMON PART WITH ADD VARIABLE

    // Set reciprocal reference from parent and add node to address space
    if (SOPC_STATUS_OK == status)
    {
        // reset stCode to default failure value
        stCode = OpcUa_BadOutOfMemory;
        status = add_node_and_set_reciprocal_reference(addSpaceAccess, &parentNodeId->NodeId, newNodeId, newNode,
                                                       refToParentTypeId, &typeDefId->NodeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        stCode = SOPC_GoodGenericStatus;
    }
    else
    {
        // Clear and dealloc node
        SOPC_AddressSpace_Node_Clear(addSpaceAccess->addSpaceRef, newNode);
        SOPC_Free(newNode);
    }

    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_AddObjectNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                      const SOPC_ExpandedNodeId* parentNodeId,
                                                      const SOPC_NodeId* refToParentTypeId,
                                                      const SOPC_NodeId* newNodeId,
                                                      const SOPC_QualifiedName* browseName,
                                                      const OpcUa_ObjectAttributes* objAttributes,
                                                      const SOPC_ExpandedNodeId* typeDefId,
                                                      const bool addChildNodesFromType)
{
    // All parameters are verified in AddSingleVariableNode. Here, only those used before are checked.
    if (NULL == addSpaceAccess || NULL == newNodeId || NULL == typeDefId)
    {
        return OpcUa_BadInvalidArgument;
    }

    char* newNodeIdStr = SOPC_NodeId_ToCString(newNodeId);
    char* typeDefIdStr = SOPC_NodeId_ToCString(&typeDefId->NodeId);
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "AddNodes: > Request to add object node: %s of type: %s%s",
                          newNodeIdStr, typeDefIdStr, addChildNodesFromType ? " (recursively)." : ".");
    // Add new node
    SOPC_StatusCode stCode = AddSingleObjectNode(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                                 objAttributes, typeDefId);

    if (SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "AddNode: Object id=%s of type: %s successfully added. %s",
                              newNodeIdStr, typeDefIdStr,
                              addChildNodesFromType ? " Next step will automatically add child nodes from node type."
                                                    : " End of treatment.");
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddNode: Object id=%s of type: %s failed (recursively=%s) with status: 0x%08" PRIX32
                               ".",
                               newNodeIdStr, typeDefIdStr, addChildNodesFromType ? "true" : "false", stCode);
    }
    // Add child nodes if required
    if (SOPC_IsGoodStatus(stCode) && addChildNodesFromType)
    {
        SOPC_StatusCode stCodeLog =
            AddChildNodesRecursiveFromType(addSpaceAccess, newNodeId, &typeDefId->NodeId, SOPC_RECURSION_LIMIT);
        if (!SOPC_IsGoodStatus(stCodeLog))
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "AddNode: Object id=%s: Failure occurred adding child nodes based on type. The root node "
                "was created successfully but uncertain number of children was created (see previous log details).",
                newNodeIdStr);
            stCode = SOPC_UncertainStatusMask; /* Generic Uncertain status */
        }
    }
    SOPC_Free(newNodeIdStr);
    SOPC_Free(typeDefIdStr);

    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_AddMethodNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                      const SOPC_ExpandedNodeId* parentNodeId,
                                                      const SOPC_NodeId* refToParentTypeId,
                                                      const SOPC_NodeId* newNodeId,
                                                      const SOPC_QualifiedName* browseName,
                                                      const OpcUa_MethodAttributes* metAttributes)
{
    if (NULL == addSpaceAccess || NULL == newNodeId)
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_StatusCode stCode =
        check_valid_params_and_state(addSpaceAccess, parentNodeId, refToParentTypeId, newNodeId, browseName,
                                     (const OpcUa_NodeAttributes*) metAttributes, NULL, OpcUa_NodeClass_Method);

    /* §5.7.1 Part 3 (1.05): A Method shall always be the TargetNode of at least one HasComponent Reference.
     */
    if (SOPC_IsGoodStatus(stCode))
    {
        bool isHasComponentRef = is_type_or_subtype(addSpaceAccess->addSpaceRef, refToParentTypeId, &hasComponentType);
        if (!isHasComponentRef)
        {
            // Added Method node has no HasComponent Reference
            stCode = OpcUa_BadReferenceNotAllowed;
        }
    }

    if (SOPC_IsGoodStatus(stCode))
    {
        stCode = SOPC_NodeMgtHelperInternal_CheckConstraints_AddNode(
            OpcUa_NodeClass_Method, addSpaceAccess->addSpaceRef, parentNodeId, refToParentTypeId, browseName, NULL);
    }
    if (!SOPC_IsGoodStatus(stCode))
    {
        return stCode;
    }

    // If all check are good, Add node.
    stCode = OpcUa_BadOutOfMemory;
    SOPC_AddressSpace_Node* newNode = SOPC_Calloc(1, sizeof(*newNode));
    if (NULL == newNode)
    {
        return stCode;
    }

    // ADD METHOD, init its NodeClass
    SOPC_AddressSpace_Node_Initialize(addSpaceAccess->addSpaceRef, newNode, OpcUa_NodeClass_Method);
    // Copy the main parameters not included in NodeAttributes structure
    OpcUa_MethodNode* metNode = &newNode->data.method;
    SOPC_ReturnStatus status = SOPC_NodeMgtHelperInternal_CopyDataInNode((OpcUa_Node*) metNode, parentNodeId, newNodeId,
                                                                         refToParentTypeId, browseName, NULL);

    // Manage NodeAttributes
    if (SOPC_STATUS_OK == status)
    {
        // Note: set stCode in case of failure
        status = SOPC_NodeMgtHelperInternal_AddMethodNodeAttributes(metNode, metAttributes, &stCode);
    }

    // COMMON PART WITH ADD VARIABLE

    // Set reciprocal reference from parent and add node to address space
    if (SOPC_STATUS_OK == status)
    {
        // reset stCode to default failure value
        stCode = OpcUa_BadOutOfMemory;
        status = add_node_and_set_reciprocal_reference(addSpaceAccess, &parentNodeId->NodeId, newNodeId, newNode,
                                                       refToParentTypeId, NULL);
    }

    if (SOPC_STATUS_OK == status)
    {
        stCode = SOPC_GoodGenericStatus;
    }
    else
    {
        // Clear and dealloc node
        SOPC_AddressSpace_Node_Clear(addSpaceAccess->addSpaceRef, newNode);
        SOPC_Free(newNode);
    }
    char* newNodeIdStr = SOPC_NodeId_ToCString(newNodeId);
    if (SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "AddNode: Method id=%s with browseName=%s successfully added.", newNodeIdStr,
                              SOPC_String_GetRawCString(&browseName->Name));
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddNode: Method id=%s with browseName=%s failed with status: 0x%08" PRIX32 ".",
                               newNodeIdStr, SOPC_String_GetRawCString(&browseName->Name), stCode);
    }
    SOPC_Free(newNodeIdStr);
    return stCode;
}

/* Browse referenced Nodes from startingNode and return the node of the first targeted node that match with
 relativePathElement criteria. The relativePathElement criteria are isInverse, referenceTypeId, IncludeSubtypes and
 targetName which is the browseName of the targeted Node.
 */
static SOPC_AddressSpace_Node* findNextStartingNode(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                    const OpcUa_RelativePathElement* relativePathElement,
                                                    SOPC_AddressSpace_Node* startingNode)
{
    if (NULL == addSpaceAccess || NULL == relativePathElement || NULL == startingNode)
    {
        return NULL;
    }
    SOPC_AddressSpace_Node* nextStartingNode = NULL;
    SOPC_AddressSpace_Node* nodeBrowsed = NULL;
    SOPC_QualifiedName* nodeBrowseName = NULL;
    int32_t comp = 0;
    OpcUa_ReferenceNode* reference = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Check successively each reference and targetNode by this reference until matching with criteria
    int32_t* noOfReferences = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, startingNode);
    OpcUa_ReferenceNode** references = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, startingNode);
    if (noOfReferences != NULL && 0 < *noOfReferences && NULL != references)
    {
        bool found = false;
        for (int32_t indexReference = 0; indexReference < *noOfReferences && !found; indexReference++)
        {
            status = SOPC_STATUS_OK;
            // Check if the reference have the same direction
            reference = &(*references)[indexReference];

            if (relativePathElement->IsInverse != reference->IsInverse)
            {
                status = SOPC_STATUS_NOK;
            }

            if (SOPC_STATUS_OK == status)
            {
                bool res = false;
                // Check if the reference have the same referenceType or is a Subtype if activated
                if (relativePathElement->IncludeSubtypes)
                {
                    res = is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId,
                                             &relativePathElement->ReferenceTypeId);
                }
                else
                {
                    res = SOPC_NodeId_Equal(&reference->ReferenceTypeId, &relativePathElement->ReferenceTypeId);
                }
                if (!res)
                {
                    status = SOPC_STATUS_NOK;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                // If the targeted Node is not in the same server don't fetch it
                if (0 != reference->TargetId.ServerIndex)
                {
                    status = SOPC_STATUS_NOK;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                // Get the node of the element referenced
                nodeBrowsed = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, &reference->TargetId.NodeId);
                if (NULL == nodeBrowsed)
                {
                    status = SOPC_STATUS_NOK;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                // Get the browse Name of the element referenced
                nodeBrowseName = SOPC_AddressSpace_Get_BrowseName(addSpaceAccess->addSpaceRef, nodeBrowsed);
                if (NULL == nodeBrowseName)
                {
                    status = SOPC_STATUS_NOK;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                // Check that expected browseName and browseName of the browsed node are the same.
                // If yes then return
                status = SOPC_QualifiedName_Compare(nodeBrowseName, &relativePathElement->TargetName, &comp);
                if (SOPC_STATUS_OK == status && 0 == comp)
                {
                    nextStartingNode = nodeBrowsed;
                    found = true;
                }
            }
        }
    }
    return nextStartingNode;
}

/* Recursive browse on relativePathElement. This function return the node targeted by the last browse element
 in relativePathElement array and NULL if something went wrong.
 Note: number of elements is limited to SOPC_RECURSION_LIMIT and decremented at each step which give guarantee it
 terminates.
*/
static SOPC_AddressSpace_Node* Recursive_BrowseRelativePath(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                            const int32_t remainingElements,
                                                            const OpcUa_RelativePathElement* elements,
                                                            SOPC_AddressSpace_Node* startingNode)
{
    if (0 >= remainingElements || NULL == elements || SOPC_RECURSION_LIMIT < remainingElements || NULL == startingNode)
    {
        return NULL;
    }
    SOPC_AddressSpace_Node* targetNode = NULL;

    // Find the next node referenced by the first relativePathElement
    SOPC_AddressSpace_Node* nextStartingNode = findNextStartingNode(addSpaceAccess, elements, startingNode);
    if (remainingElements > 1)
    {
        // If fail to get nextStartingNode stop recursion
        if (NULL != nextStartingNode)
        {
            targetNode =
                Recursive_BrowseRelativePath(addSpaceAccess, remainingElements - 1, elements + 1, nextStartingNode);
        }
    }
    else
    {
        // Ending point of the recursion find the targetNode.
        targetNode = nextStartingNode;
    }

    return targetNode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_TranslateBrowsePath(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                            const SOPC_NodeId* startingNode,
                                                            const OpcUa_RelativePath* relativePath,
                                                            const SOPC_NodeId** targetId)
{
    if (NULL == addSpaceAccess || NULL == startingNode || NULL == relativePath || NULL == targetId || NULL != *targetId)
    {
        return OpcUa_BadInvalidArgument;
    }
    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_AddressSpace_Node* startingNodeAddspace =
        SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, startingNode);
    if (NULL == startingNodeAddspace)
    {
        stCode = OpcUa_BadNodeIdUnknown;
    }
    else
    {
        SOPC_AddressSpace_Node* targetIdNode = Recursive_BrowseRelativePath(
            addSpaceAccess, relativePath->NoOfElements, relativePath->Elements, startingNodeAddspace);
        if (NULL != targetIdNode)
        {
            (*targetId) = SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, targetIdNode);
        }
        else
        {
            stCode = OpcUa_BadNoMatch;
        }
    }
    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_BrowseNode(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   const SOPC_NodeId* nodeId,
                                                   const OpcUa_BrowseDirection browseDirection,
                                                   const SOPC_NodeId* referenceTypeId,
                                                   const bool includeSubtypes,
                                                   const OpcUa_NodeClass nodeClassMask,
                                                   const OpcUa_BrowseResultMask resultMask,
                                                   OpcUa_ReferenceDescription** references,
                                                   int32_t* noOfReferences)
{
    SOPC_UNUSED_ARG(nodeClassMask);
    SOPC_UNUSED_ARG(resultMask);

    if (NULL == addSpaceAccess || NULL == nodeId || NULL == referenceTypeId || NULL == references ||
        NULL != *references || NULL == noOfReferences)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Check that referenceTypeId refer to a valid referenceType
    SOPC_AddressSpace_Node* referenceNode = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, referenceTypeId);
    if (NULL == referenceNode || OpcUa_NodeClass_ReferenceType != referenceNode->node_class)
    {
        return OpcUa_BadReferenceTypeIdInvalid;
    }

    // Get node and check if this one exist
    SOPC_AddressSpace_Node* node = NULL;
    node = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, nodeId);
    if (NULL == node)
    {
        return OpcUa_BadNodeIdUnknown;
    }

    // Create the array which will store all the references matching given criteria
    SOPC_Array* refsArr =
        SOPC_Array_Create(sizeof(OpcUa_ReferenceDescription), BROWSE_REFERENCE_DESCRIPTION_RESULT_LEN_ARRAY,
                          OpcUa_ReferenceDescription_Clear);
    if (NULL == refsArr)
    {
        return OpcUa_BadOutOfMemory;
    }

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    OpcUa_ReferenceDescription refDescription;
    OpcUa_ReferenceDescription_Initialize(&refDescription);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_ReferenceNode* ref = NULL;
    // Get all references attached to the node
    int32_t* addSpaceNbRef = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, node);
    OpcUa_ReferenceNode** addSpaceRefs = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, node);
    SOPC_ASSERT(NULL != addSpaceNbRef);
    SOPC_ASSERT(NULL != addSpaceRefs);
    for (int32_t i = 0; i < *addSpaceNbRef && SOPC_IsGoodStatus(stCode); i++)
    {
        status = SOPC_STATUS_OK;
        // Check references criteria
        ref = &(*addSpaceRefs)[i];

        // Check if referenceTypeId is equal or a subtype if allow
        bool res = false;
        if (includeSubtypes)
        {
            res = is_type_or_subtype(addSpaceAccess->addSpaceRef, &ref->ReferenceTypeId, referenceTypeId);
        }
        else
        {
            res = SOPC_NodeId_Equal(&ref->ReferenceTypeId, referenceTypeId);
        }
        if (!res)
        {
            status = SOPC_STATUS_NOK;
        }

        // Check the direction if needed
        if (SOPC_STATUS_OK == status)
        {
            switch (browseDirection)
            {
            case OpcUa_BrowseDirection_Forward:
                status = (!ref->IsInverse) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                break;
            case OpcUa_BrowseDirection_Inverse:
                status = (ref->IsInverse) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                break;
            case OpcUa_BrowseDirection_Both:
                break;
            default:
                status = SOPC_STATUS_NOK;
                stCode = OpcUa_BadBrowseDirectionInvalid;
                break;
            }
        }

        // The reference match all criteria add the reference to result
        if (SOPC_STATUS_OK == status)
        {
            OpcUa_ReferenceDescription_Initialize(&refDescription);

            // Fill the referenceDescription
            refDescription.IsForward = !ref->IsInverse;
            status = SOPC_ExpandedNodeId_Copy(&refDescription.NodeId, &ref->TargetId);
            if (SOPC_STATUS_OK != status)
            {
                stCode = OpcUa_BadOutOfMemory;
            }
            else
            {
                res = SOPC_Array_Append(refsArr, refDescription);
                if (!res)
                {
                    // Array append fail so we must free memory allocated
                    OpcUa_ReferenceDescription_Clear(&refDescription);
                    stCode = OpcUa_BadOutOfMemory;
                }
            }
        }
    }

    if (SOPC_IsGoodStatus(stCode))
    {
        *noOfReferences = (int32_t) SOPC_Array_Size(refsArr);
        // Array content is transferred to references, and memory responsibility too.
        *references = (OpcUa_ReferenceDescription*) SOPC_Array_Into_Raw(refsArr);
    }
    else
    {
        // Deallocate array in case of failure.
        SOPC_Array_Delete(refsArr);
    }

    return stCode;
}

static void delete_target_reference_in_source_node(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   SOPC_AddressSpace_Node* sourceNode,
                                                   SOPC_AddressSpace_Node* targetNode)
{
    SOPC_ASSERT(NULL != sourceNode);
    SOPC_ASSERT(NULL != targetNode);

    const int32_t* noOfReferences = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, sourceNode);
    OpcUa_ReferenceNode** references = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, sourceNode);
    OpcUa_ReferenceNode* reference = NULL;
    int32_t nodeId_comparison = -1;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (noOfReferences != NULL && 0 < *noOfReferences && NULL != references)
    {
        SOPC_SLinkedList* indexOfRefsToDelete = SOPC_SLinkedList_Create((size_t) *noOfReferences);
        SOPC_ASSERT(NULL != indexOfRefsToDelete);
        for (int32_t indexReference = 0; indexReference < *noOfReferences; indexReference++)
        {
            // For each reference, get the targetNode. If it is equal to target_node,
            // then delete the reference.
            reference = &(*references)[indexReference];
            const SOPC_NodeId* targetNodeId = SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, targetNode);
            SOPC_ASSERT(NULL != targetNodeId);
            status = SOPC_NodeId_Compare(targetNodeId, &reference->TargetId.NodeId, &nodeId_comparison);
            if (0 == nodeId_comparison && SOPC_STATUS_OK == status)
            {
                SOPC_SLinkedList_Append(indexOfRefsToDelete, 0,
                                        (uintptr_t)(indexReference + 1)); // prevent from adding value 0 and fail
            }
        }
        SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(indexOfRefsToDelete);
        while (SOPC_SLinkedList_HasNext(&it) && SOPC_STATUS_OK == status)
        {
            int32_t indexOfRef = (int32_t) SOPC_SLinkedList_Next(&it);
            // Delete reference
            bool is_ref_deleted =
                SOPC_NodeMgtHelperInternal_RemoveRefAtIndex(addSpaceAccess->addSpaceRef, sourceNode, indexOfRef - 1);
            if (is_ref_deleted)
            {
                LOG_NODE_REF(SOPC_LOG_LEVEL_DEBUG, addSpaceAccess->addSpaceRef,
                             "Success deleting reference from %s to %s", sourceNode, targetNode);
            }
            else
            {
                LOG_NODE_REF(SOPC_LOG_LEVEL_ERROR, addSpaceAccess->addSpaceRef,
                             "Failed deleting reference from %s to %s", sourceNode, targetNode);
            }
        }
        SOPC_SLinkedList_Delete(indexOfRefsToDelete);
        indexOfRefsToDelete = NULL;
    }
}

static void SOPC_AddressSpaceAccess_DeleteNodeOnly(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   SOPC_AddressSpace_Node* node,
                                                   SOPC_AddressSpaceAccessOperation* op)
{
    SOPC_ASSERT(NULL != node);
    SOPC_ASSERT(NULL != addSpaceAccess);

    // Add operation node deleted while address space access if necessary
    if (NULL != op)
    {
        const void* addedOp = NULL;
        SOPC_NodeId* nodeIdCopy = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        SOPC_ReturnStatus status =
            SOPC_NodeId_Copy(nodeIdCopy, SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, node));
        if (SOPC_STATUS_OK == status && NULL != nodeIdCopy)
        {
            addedOp = (const void*) SOPC_SLinkedList_Prepend(addSpaceAccess->operations, 0, (uintptr_t) op);
        }
        if (NULL != addedOp)
        {
            op->operation = SOPC_ADDSPACE_CHANGE_NODE;
            op->param1 = false; // false for delete
            op->param2 = (uintptr_t) nodeIdCopy;
        }
        else
        {
            SOPC_Free(op);
            SOPC_NodeId_Clear(nodeIdCopy);
            SOPC_Free(nodeIdCopy);
        }
    }

    // Delete the node
    SOPC_AddressSpace_Node_Delete(addSpaceAccess->addSpaceRef, node);
}

static bool is_single_parent(const SOPC_AddressSpaceAccess* addSpaceAccess,
                             SOPC_AddressSpace_Node* alreadyKnownParentNode,
                             SOPC_AddressSpace_Node* childNode)
{
    SOPC_ASSERT(NULL != childNode);
    SOPC_ASSERT(NULL != alreadyKnownParentNode);

    const int32_t* noOfReferences = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, childNode);
    OpcUa_ReferenceNode** references = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, childNode);
    const OpcUa_ReferenceNode* reference = NULL;
    bool b_initial_parent_found = false;
    bool b_otherParentFound = false;
    int32_t nodeId_comparison = -1;
    if (noOfReferences != NULL && 0 < *noOfReferences && NULL != references)
    {
        for (int32_t indexReference = 0; indexReference < *noOfReferences && !b_otherParentFound; indexReference++)
        {
            reference = &(*references)[indexReference];
            if (reference->IsInverse)
            {
                // Iterate on all inverse references of the child.
                // If it has one inverse reference with parent / child type (same type when checking childs)
                // that is different from the identified parent, b_otherParentFound = TRUE.
                bool b_ref_type_child_or_organizes =
                    is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId, &hasChildType);
#if 0 != S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES
                b_ref_type_child_or_organizes |= is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId, &organizesType);
#endif
                if (b_ref_type_child_or_organizes)
                {
                    // Check if the TargetNode is in same server.
                    SOPC_ReturnStatus status =
                        (reference->TargetId.ServerIndex == 0) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                    if (SOPC_STATUS_OK == status)
                    {
                        const SOPC_NodeId* alreadyKnownParentNodeId =
                            SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, alreadyKnownParentNode);
                        SOPC_ASSERT(NULL != alreadyKnownParentNodeId);
                        status = SOPC_NodeId_Compare(alreadyKnownParentNodeId, &reference->TargetId.NodeId,
                                                     &nodeId_comparison);
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        if (0 == nodeId_comparison)
                        {
                            b_initial_parent_found = true;
                        }
                        else
                        {
                            b_otherParentFound = true;
                        }
                    }
                }
            }
        }
    }

    if (!b_initial_parent_found)
    {
        LOG_NODE_REF(SOPC_LOG_LEVEL_WARNING, addSpaceAccess->addSpaceRef,
                     "DeleteNode: inverse reference from child node %s to parent node %s has not been found.",
                     childNode, alreadyKnownParentNode);
    }

    return !b_otherParentFound;
}

static SOPC_StatusCode SOPC_AddressSpaceAccess_DeleteNodeRec(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                             SOPC_AddressSpace_Node* nodeToDelete,
                                                             bool deleteTargetReferences,
                                                             bool deleteChildNodes,
                                                             int32_t recursionLimit)
{
    SOPC_ASSERT(NULL != addSpaceAccess);
    SOPC_ASSERT(NULL != nodeToDelete);
    SOPC_ASSERT(0 <= recursionLimit);

    if (0 == recursionLimit)
    {
        return OpcUa_BadWouldBlock;
    }

    // Prepare record operation
    SOPC_AddressSpaceAccessOperation* op = NULL;
    if (addSpaceAccess->recordOperations)
    {
        SOPC_ASSERT(NULL != addSpaceAccess->operations);
        op = SOPC_Calloc(1, sizeof(*op));
        if (NULL == op)
        {
            return OpcUa_BadOutOfMemory;
        }
    }

    const int recursionLevel = SOPC_RECURSION_LIMIT - recursionLimit;
    const SOPC_NodeId* nodeId = SOPC_AddressSpace_Get_NodeId(addSpaceAccess->addSpaceRef, nodeToDelete);
    SOPC_ASSERT(NULL != nodeId);
    char* nodeIdStr = SOPC_NodeId_ToCString(nodeId);

    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    // If the nodeId does not exist in the address space, ignore.
    const int32_t* noOfReferences = SOPC_AddressSpace_Get_NoOfReferences(addSpaceAccess->addSpaceRef, nodeToDelete);
    OpcUa_ReferenceNode** references = SOPC_AddressSpace_Get_References(addSpaceAccess->addSpaceRef, nodeToDelete);
    const OpcUa_ReferenceNode* reference = NULL;
    if (NULL != noOfReferences && 0 < *noOfReferences && NULL != references)
    {
        for (int32_t indexReference = 0; indexReference < *noOfReferences && SOPC_IsGoodStatus(stCode);
             indexReference++)
        {
            // Iterate on references.
            reference = &(*references)[indexReference];
            const SOPC_NodeId* targetNodeId = &reference->TargetId.NodeId;
            SOPC_AddressSpace_Node* targetNode = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, targetNodeId);
            if (0 == reference->TargetId.ServerIndex && NULL != targetNode)
            {
                // Recursively delete the TargetNode if:
                // - the reference type is HasChild or subtype, or Organizes or subtype if the
                //   option is set,
                // - the reference is a forward ref,
                // - delete target references is set,
                // - it has a single parent in the address space.
                bool b_ref_type_child_or_organizes =
                    is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId, &hasChildType);
#if 0 != S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES
                b_ref_type_child_or_organizes |= is_type_or_subtype(addSpaceAccess->addSpaceRef, &reference->ReferenceTypeId, &organizesType);
#endif
                bool deleteChilds = (deleteChildNodes && b_ref_type_child_or_organizes && !reference->IsInverse &&
                                     is_single_parent(addSpaceAccess, nodeToDelete, targetNode));
                if (deleteChilds)
                {
                    // Delete the targetNode (and thus all its references including those to nodeIdToDelete)
                    stCode = SOPC_AddressSpaceAccess_DeleteNodeRec(addSpaceAccess, targetNode, deleteTargetReferences,
                                                                   deleteChildNodes, recursionLimit - 1);
                }
                else if (deleteTargetReferences)
                {
                    // The targetNode will not be deleted. But we want to delete the reference to nodeToDelete
                    // if DeleteTargetReferences is set.
                    delete_target_reference_in_source_node(addSpaceAccess, targetNode, nodeToDelete);
                }
            } // No treatment if the TargetNode is not in the same server
        }
    }

    // Trace deletion of node at info level
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                          "DeleteNode (level=%d): node id=%s has been deleted from address space.", recursionLevel,
                          nodeIdStr);

    // Log if fail in deleting recursively child
    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "DeleteNode: Failure deleting child nodes of %s, only node %s was deleted.", nodeIdStr,
                                 nodeIdStr);
    }

    // Delete the node and its references as a source, and post "node deleted" event
    SOPC_AddressSpaceAccess_DeleteNodeOnly(addSpaceAccess, nodeToDelete, op);

    SOPC_Free(nodeIdStr);

    return stCode;
}

SOPC_StatusCode SOPC_AddressSpaceAccess_DeleteNode(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   const SOPC_NodeId* nodeIdToDelete,
                                                   bool deleteTargetReferences,
                                                   bool deleteChildNodes)
{
    if (NULL == addSpaceAccess || NULL == nodeIdToDelete)
    {
        return OpcUa_BadInvalidArgument;
    }
    SOPC_StatusCode stCode = SOPC_GoodGenericStatus;
    SOPC_AddressSpace_Node* root_node = SOPC_InternalAddressSpaceAccess_GetNode(addSpaceAccess, nodeIdToDelete);
    if (NULL == root_node)
    {
        // If the source node does not exist, return BadNodeIdUnknow.
        stCode = OpcUa_BadNodeIdUnknown;
    }
    else
    {
        // If the source node exists, delete it and maybe recursively delete its childs.
        stCode = SOPC_AddressSpaceAccess_DeleteNodeRec(addSpaceAccess, root_node, deleteTargetReferences,
                                                       deleteChildNodes, SOPC_RECURSION_LIMIT);
    }

    if (SOPC_IsGoodStatus(stCode) && deleteTargetReferences)
    {
        stCode = OpcUa_UncertainReferenceNotDeleted;
    }
    return stCode;
}
