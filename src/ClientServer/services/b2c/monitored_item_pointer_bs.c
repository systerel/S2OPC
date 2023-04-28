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

#include "monitored_item_pointer_bs.h"

#include "address_space_impl.h"
#include "monitored_item_pointer_impl.h"

#include <inttypes.h>
#include <math.h>
#include <string.h>

#include "sopc_address_space_utils_internal.h"
#include "sopc_assert.h"
#include "sopc_dict.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "util_b2c.h"

static const SOPC_NodeId Number_DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Number};

static void SOPC_InternalMonitoredItem_Free(void* data)
{
    SOPC_InternalMonitoredItem* mi = (SOPC_InternalMonitoredItem*) data;
    if (NULL != mi)
    {
        SOPC_NumericRange_Delete(mi->indexRange);
        SOPC_NodeId_Clear(mi->nid);
        SOPC_Free(mi->nid);
        SOPC_String_Clear(mi->indexRangeString);
        SOPC_Free(mi->indexRangeString);
        OpcUa_DataChangeFilter_Clear(mi->filter);
        SOPC_Free(mi->filter);
        SOPC_Variant_Delete(mi->lastCachedValueForFilter);
        SOPC_Free(mi);
    }
}

static void SOPC_InternalMonitoredItemId_Free(void* data)
{
    SOPC_UNUSED_ARG(data);
    // Nothing to do: uintptr_t value
}

static uint64_t SOPC_InternalMonitoredItemId_Hash(const void* data)
{
    uintptr_t id = (uintptr_t) data;
    return (uint64_t) id;
}

static bool SOPC_InternalMonitoredItemId_Equal(const void* a, const void* b)
{
    // Compare uintptr_t id values
    return a == b;
}

static SOPC_Dict* monitoredItemIdDict = NULL;
static SOPC_SLinkedList* monitoredItemIdFreed = NULL;

static uint32_t monitoredItemIdMax = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_pointer_bs__INITIALISATION(void)
{
    monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();

    monitoredItemIdDict = SOPC_Dict_Create(constants_bs__c_monitoredItemId_indet, SOPC_InternalMonitoredItemId_Hash,
                                           SOPC_InternalMonitoredItemId_Equal, SOPC_InternalMonitoredItemId_Free,
                                           SOPC_InternalMonitoredItem_Free);
    SOPC_ASSERT(monitoredItemIdDict != NULL);
    monitoredItemIdFreed = SOPC_SLinkedList_Create(0);
    SOPC_ASSERT(monitoredItemIdFreed != NULL);
}

void monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION(void)
{
    if (monitoredItemIdDict != NULL)
    {
        SOPC_Dict_Delete(monitoredItemIdDict);
        monitoredItemIdDict = NULL;
    }

    if (monitoredItemIdFreed != NULL)
    {
        SOPC_SLinkedList_Delete(monitoredItemIdFreed);
        monitoredItemIdFreed = NULL;
    }

    monitoredItemIdMax = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
#define InputArguments_BrowseName "EURange"

static bool is_EURange(const OpcUa_VariableNode* node)
{
    if (NULL == node || &OpcUa_VariableNode_EncodeableType != node->encodeableType)
    {
        return false;
    }

    /* Type shall be Range */
    if (!(SOPC_IdentifierType_Numeric == node->DataType.IdentifierType && OpcUaId_Range == node->DataType.Data.Numeric))
    {
        return false;
    }

    /* Browse shall be EURange (and should be in NS=O but we don't check it) */
    return (strcmp(SOPC_String_GetRawCString(&node->BrowseName.Name), InputArguments_BrowseName) == 0);
}

static bool check_percent_deadband_filter_allowed(SOPC_AddressSpace_Node* variableNode, OpcUa_Range** range)
{
    SOPC_AddressSpace_Node* targetNode = NULL;
    bool found = false;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, variableNode);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, variableNode);
    SOPC_Variant* euRangeVariant = NULL;
    OpcUa_Range* euRangeValue = NULL;
    for (int32_t i = 0; i < *n_refs && NULL == euRangeVariant; ++i)
    { /* stop when input argument is found */
        OpcUa_ReferenceNode* ref = &(*refs)[i];
        if (SOPC_AddressSpaceUtil_IsProperty(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                targetNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &ref->TargetId.NodeId, &found);
                if (found && NULL != targetNode && OpcUa_NodeClass_Variable == targetNode->node_class)
                {
                    if (is_EURange(&targetNode->data.variable))
                    {
                        euRangeVariant = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, targetNode);
                    }
                }
            }
        }
    }
    if (euRangeVariant != NULL && SOPC_VariantArrayType_SingleValue == euRangeVariant->ArrayType &&
        SOPC_ExtensionObject_Id == euRangeVariant->BuiltInTypeId &&
        SOPC_ExtObjBodyEncoding_Object == euRangeVariant->Value.ExtObject->Encoding &&
        &OpcUa_Range_EncodeableType == euRangeVariant->Value.ExtObject->Body.Object.ObjType)
    {
        /* Important note: here we make the choice that the EURange for the MI is the one during MI
         * creation/modification, the EURange value / node might be changed in the future but we will ignore it. */
        euRangeValue = (OpcUa_Range*) euRangeVariant->Value.ExtObject->Body.Object.Value;
        if (euRangeValue->High >= euRangeValue->Low)
        {
            *range = euRangeValue;
            return true;
        }
    }
    return false;
}

void monitored_item_pointer_bs__check_monitored_item_filter_valid(
    const constants__t_Node_i monitored_item_pointer_bs__p_node,
    const constants__t_monitoringFilter_i monitored_item_pointer_bs__p_filter,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode,
    constants__t_monitoringFilterCtx_i* const monitored_item_pointer_bs__filterAbsDeadbandCtx)
{
    *monitored_item_pointer_bs__filterAbsDeadbandCtx = constants_bs__c_monitoringFilterCtx_indet;
    *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
    SOPC_ASSERT(NULL != monitored_item_pointer_bs__p_filter);
    OpcUa_NodeClass* ncl = NULL;
    SOPC_NodeId* dataTypeId = NULL;
    bool result = false;
    OpcUa_Range* euRange = NULL;
    switch (monitored_item_pointer_bs__p_filter->DeadbandType)
    {
    case OpcUa_DeadbandType_None:
        *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
        break;
    case OpcUa_DeadbandType_Absolute:
        ncl = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, monitored_item_pointer_bs__p_node);
        SOPC_ASSERT(NULL != ncl);
        if (OpcUa_NodeClass_Variable == *ncl)
        {
            dataTypeId = SOPC_AddressSpace_Get_DataType(address_space_bs__nodes, monitored_item_pointer_bs__p_node);
            result = SOPC_NodeId_Equal(dataTypeId, &Number_DataType);
            if (!result)
            {
                result = SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(address_space_bs__nodes, RECURSION_LIMIT,
                                                                            dataTypeId, dataTypeId, &Number_DataType);
            }
            /* Keep the absolute deadband value as context */
            *monitored_item_pointer_bs__filterAbsDeadbandCtx = monitored_item_pointer_bs__p_filter->DeadbandValue;
            if (result)
            {
                *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
            }
        }
        break;
    case OpcUa_DeadbandType_Percent:
        ncl = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, monitored_item_pointer_bs__p_node);
        SOPC_ASSERT(NULL != ncl);
        if (OpcUa_NodeClass_Variable == *ncl)
        {
            result = check_percent_deadband_filter_allowed(monitored_item_pointer_bs__p_node, &euRange);
            if (result)
            {
                /* Pre-compute the deadband as an absolute value using formula from Part 8 */
                *monitored_item_pointer_bs__filterAbsDeadbandCtx =
                    (monitored_item_pointer_bs__p_filter->DeadbandValue / 100.0) * (euRange->High - euRange->Low);
                *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
            }
        }
        break;
    default:
        // Already checked when retrieved in message
        SOPC_ASSERT(false && "invalid deadband type");
    }
}

void monitored_item_pointer_bs__create_monitored_item_pointer(
    const constants__t_subscription_i monitored_item_pointer_bs__p_subscription,
    const constants__t_NodeId_i monitored_item_pointer_bs__p_nid,
    const constants__t_AttributeId_i monitored_item_pointer_bs__p_aid,
    const constants__t_IndexRange_i monitored_item_pointer_bs__p_indexRange,
    const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
    const constants__t_monitoringMode_i monitored_item_pointer_bs__p_monitoringMode,
    const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
    const constants__t_monitoringFilter_i monitored_item_pointer_bs__p_filter,
    const constants__t_monitoringFilterCtx_i monitored_item_pointer_bs__p_filterAbsDeadbandCtx,
    const t_bool monitored_item_pointer_bs__p_discardOldest,
    const t_entier4 monitored_item_pointer_bs__p_queueSize,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode,
    constants__t_monitoredItemPointer_i* const monitored_item_pointer_bs__monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__monitoredItemId)
{
    *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    uint32_t freshId = 0;
    SOPC_InternalMonitoredItem* monitItem = SOPC_Calloc(1, sizeof(SOPC_InternalMonitoredItem));
    SOPC_NodeId* nid = SOPC_Malloc(sizeof(*nid));
    SOPC_String* rangeStr = NULL;
    if (NULL != monitored_item_pointer_bs__p_indexRange)
    {
        rangeStr = SOPC_Malloc(sizeof(*rangeStr));
    }
    SOPC_NumericRange* range = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_NOK;

    if (NULL == monitItem || NULL == nid || (NULL == rangeStr && NULL != monitored_item_pointer_bs__p_indexRange))
    {
        SOPC_Free(monitItem);
        SOPC_Free(nid);
        SOPC_Free(rangeStr);
        return;
    }

    SOPC_NodeId_Initialize(nid);
    retStatus = SOPC_NodeId_Copy(nid, monitored_item_pointer_bs__p_nid);

    if (SOPC_STATUS_OK == retStatus && monitored_item_pointer_bs__p_indexRange != NULL)
    {
        SOPC_String_Initialize(rangeStr);
        retStatus = SOPC_String_Copy(rangeStr, monitored_item_pointer_bs__p_indexRange);
    }
    if (SOPC_STATUS_OK == retStatus && monitored_item_pointer_bs__p_indexRange != NULL)
    {
        retStatus = SOPC_NumericRange_Parse(SOPC_String_GetRawCString(monitored_item_pointer_bs__p_indexRange), &range);

        if (SOPC_STATUS_OK != retStatus)
        {
            *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_index_range_invalid;
        }
    }
    if (SOPC_STATUS_OK == retStatus)
    {
        if (NULL != monitored_item_pointer_bs__p_filter)
        {
            monitItem->filter = SOPC_Malloc(sizeof(*monitored_item_pointer_bs__p_filter));
            if (NULL == monitItem->filter)
            {
                retStatus = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                *monitItem->filter = *monitored_item_pointer_bs__p_filter;
            }
        }
    }
    if (SOPC_STATUS_OK == retStatus)
    {
        bool dictInsertionOK = false;

        monitItem->subId = monitored_item_pointer_bs__p_subscription;
        monitItem->nid = nid;
        monitItem->aid = monitored_item_pointer_bs__p_aid;
        monitItem->indexRangeString = rangeStr;
        monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
        monitItem->monitoringMode = monitored_item_pointer_bs__p_monitoringMode;
        monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;
        monitItem->indexRange = range;
        monitItem->filterAbsoluteDeadbandContext = monitored_item_pointer_bs__p_filterAbsDeadbandCtx;
        monitItem->lastCachedValueForFilter = NULL;
        monitItem->discardOldest = monitored_item_pointer_bs__p_discardOldest;
        monitItem->queueSize = monitored_item_pointer_bs__p_queueSize;

        if (0 == SOPC_SLinkedList_GetLength(monitoredItemIdFreed))
        {
            // No free unique Id, create a new one
            if (monitoredItemIdMax < UINT32_MAX)
            {
                monitoredItemIdMax++;
                monitItem->monitoredItemId = monitoredItemIdMax;
                dictInsertionOK =
                    SOPC_Dict_Insert(monitoredItemIdDict, (void*) (uintptr_t) monitoredItemIdMax, monitItem);
            } // else: all Ids already in use
        }
        else
        {
            // Reuse freed id
            freshId = (uint32_t)(uintptr_t) SOPC_SLinkedList_PopHead(monitoredItemIdFreed);
            if (freshId != 0)
            {
                monitItem->monitoredItemId = freshId;
                dictInsertionOK = SOPC_Dict_Insert(monitoredItemIdDict, (void*) (uintptr_t) freshId, monitItem);
            }
        }

        if (!dictInsertionOK)
        {
            retStatus = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
        *monitored_item_pointer_bs__monitoredItemPointer = monitItem;
        *monitored_item_pointer_bs__monitoredItemId = monitItem->monitoredItemId;

        char* nidStr = SOPC_NodeId_ToCString(nid);
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "monitored_item_pointer_bs_create_monitored_item_pointer: subscriptionId=%" PRIu32
                               " monitoredItemId=%" PRIu32 " creation for NodeId=%s AttributeId=%d",
                               monitItem->subId, monitItem->monitoredItemId, nidStr, monitItem->aid);
        SOPC_Free(nidStr);
    }
    else
    {
        SOPC_NumericRange_Delete(range);
        SOPC_Free(monitItem->filter);
        SOPC_Free(monitItem);
        SOPC_NodeId_Clear(nid);
        SOPC_Free(nid);
        SOPC_String_Clear(rangeStr);
        SOPC_Free(rangeStr);
    }
}

void monitored_item_pointer_bs__modify_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
    const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
    const constants__t_monitoringFilter_i monitored_item_pointer_bs__p_filter,
    const constants__t_monitoringFilterCtx_i monitored_item_pointer_bs__p_filterAbsDeadbandCtx,
    const t_bool monitored_item_pointer_bs__p_discardOldest,
    const t_entier4 monitored_item_pointer_bs__p_queueSize,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode)
{
    *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
    monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;
    OpcUa_DataChangeFilter_Clear(monitItem->filter);
    SOPC_Free(monitItem->filter);
    monitItem->filter = NULL;
    if (NULL != monitored_item_pointer_bs__p_filter)
    {
        monitItem->filter = SOPC_Malloc(sizeof(*monitored_item_pointer_bs__p_filter));
        if (NULL == monitItem->filter)
        {
            *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }
        else
        {
            *monitItem->filter = *monitored_item_pointer_bs__p_filter;
        }
    }
    monitItem->filterAbsoluteDeadbandContext = monitored_item_pointer_bs__p_filterAbsDeadbandCtx;
    monitItem->discardOldest = monitored_item_pointer_bs__p_discardOldest;
    monitItem->queueSize = monitored_item_pointer_bs__p_queueSize;

    // If a cached value exists and no filter defined, reset the last cache value for filter
    if (NULL != monitItem->lastCachedValueForFilter &&
        (NULL == monitItem->filter || OpcUa_DeadbandType_None == monitItem->filter->DeadbandType))
    {
        SOPC_Variant_Delete(monitItem->lastCachedValueForFilter);
        monitItem->lastCachedValueForFilter = NULL;
    }
}

void monitored_item_pointer_bs__delete_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer)
{
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                           "monitored_item_pointer_bs__delete_monitored_item_pointer: monitoredItemId=%" PRIu32
                           " deletion",
                           monitItem->monitoredItemId);

    uintptr_t appended = (uintptr_t) SOPC_SLinkedList_Append(monitoredItemIdFreed, monitItem->monitoredItemId,
                                                             (void*) (uintptr_t) monitItem->monitoredItemId);

    if (appended != (uintptr_t) monitItem->monitoredItemId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "monitored_item_pointer_bs__delete_monitored_item_pointer: monitoredItemId %" PRIu32
                               " cannot be added to freed set",
                               monitItem->monitoredItemId);
    }

    // Reset monitored item associated
    // (Caution: it frees the monitItem pointer)
    bool inserted = SOPC_Dict_Insert(monitoredItemIdDict, (void*) (uintptr_t) monitItem->monitoredItemId, NULL);

    if (!inserted)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "monitored_item_pointer_bs__delete_monitored_item_pointer: monitoredItemId %" PRIu32
                               " cannot be removed from defined set",
                               monitItem->monitoredItemId);
    }
}

void monitored_item_pointer_bs__set_monit_mode_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_monitoringMode_i monitored_item_pointer_bs__p_monitoring_mode)
{
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    monitItem->monitoringMode = monitored_item_pointer_bs__p_monitoring_mode;
}

void monitored_item_pointer_bs__getall_monitoredItemId(
    const constants__t_monitoredItemId_i monitored_item_pointer_bs__p_monitoredItemId,
    t_bool* const monitored_item_pointer_bs__bres,
    constants__t_monitoredItemPointer_i* const monitored_item_pointer_bs__p_monitoredItemPointer)
{
    *monitored_item_pointer_bs__bres = false;
    *monitored_item_pointer_bs__p_monitoredItemPointer = NULL;
    if (monitored_item_pointer_bs__p_monitoredItemId != constants_bs__c_monitoredItemId_indet)
    {
        void* miPointer =
            SOPC_Dict_Get(monitoredItemIdDict, (void*) (uintptr_t) monitored_item_pointer_bs__p_monitoredItemId,
                          monitored_item_pointer_bs__bres);
        if (*monitored_item_pointer_bs__bres && NULL != miPointer)
        {
            *monitored_item_pointer_bs__p_monitoredItemPointer = miPointer;
        }
        else
        {
            *monitored_item_pointer_bs__bres = false;
            *monitored_item_pointer_bs__p_monitoredItemPointer = NULL;
        }
    }
}

void monitored_item_pointer_bs__getall_monitoredItemPointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__p_monitoredItemId,
    constants__t_subscription_i* const monitored_item_pointer_bs__p_subscription,
    constants__t_NodeId_i* const monitored_item_pointer_bs__p_nid,
    constants__t_AttributeId_i* const monitored_item_pointer_bs__p_aid,
    constants__t_IndexRange_i* const monitored_item_pointer_bs__p_indexRange,
    constants__t_TimestampsToReturn_i* const monitored_item_pointer_bs__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_pointer_bs__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_pointer_bs__p_clientHandle)
{
    SOPC_ASSERT(NULL != monitored_item_pointer_bs__p_monitoredItemPointer); // Guaranteed by B model
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    *monitored_item_pointer_bs__p_monitoredItemId = monitItem->monitoredItemId;
    *monitored_item_pointer_bs__p_subscription = monitItem->subId;
    *monitored_item_pointer_bs__p_nid = monitItem->nid;
    *monitored_item_pointer_bs__p_aid = monitItem->aid;
    *monitored_item_pointer_bs__p_indexRange = monitItem->indexRangeString;
    *monitored_item_pointer_bs__p_timestampToReturn = monitItem->timestampToReturn;
    *monitored_item_pointer_bs__p_monitoringMode = monitItem->monitoringMode;
    *monitored_item_pointer_bs__p_clientHandle = monitItem->clientHandle;
}

// Note: both < and > operators always return False with NaN, testing both allow to consider NaN values equal
#define COMPARE_DEADBAND_ABSOLUTE_NUMERIC_VALUE(sopcTypeid, ntype, tmpVarType) \
    case sopcTypeid:                                                           \
        left##tmpVarType = *(const ntype*) left;                               \
        right##tmpVarType = *(const ntype*) right;                             \
        if (left##tmpVarType > right##tmpVarType)                              \
        {                                                                      \
            compareValue = 1;                                                  \
            diff = (double) (left##tmpVarType - right##tmpVarType);            \
        }                                                                      \
        else if (right##tmpVarType > left##tmpVarType)                         \
        {                                                                      \
            compareValue = -1;                                                 \
            diff = (double) (right##tmpVarType - left##tmpVarType);            \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            compareValue = 0;                                                  \
        }                                                                      \
        break;

#define FOR_EACH_NUMERIC_TYPE(x)                                                                                       \
    x(SOPC_Byte_Id, SOPC_Byte, uint64_t) x(SOPC_UInt16_Id, uint16_t, uint64_t) x(SOPC_UInt32_Id, uint32_t, uint64_t)   \
        x(SOPC_UInt64_Id, uint64_t, uint64_t) x(SOPC_SByte_Id, SOPC_SByte, int64_t) x(SOPC_Int16_Id, int16_t, int64_t) \
            x(SOPC_Int32_Id, int32_t, int64_t) x(SOPC_Int64_Id, int64_t, int64_t) x(SOPC_Float_Id, float, double)      \
                x(SOPC_Double_Id, double, double)

static SOPC_ReturnStatus compare_deadband_absolute(const void* customContext,
                                                   SOPC_BuiltinId builtInTypeId,
                                                   const void* left,
                                                   const void* right,
                                                   int32_t* compResult)
{
    SOPC_ASSERT(NULL != customContext);
    double deadband = *(const double*) customContext;
    // Checked on filter creation
    SOPC_ASSERT(!(deadband < 0.0));
    int32_t compareValue = 0;
    uint64_t leftuint64_t = 0;
    uint64_t rightuint64_t = 0;
    int64_t leftint64_t = 0;
    int64_t rightint64_t = 0;
    double leftdouble = 0;
    double rightdouble = 0;
    double diff = 0.0;
    switch (builtInTypeId)
    {
        FOR_EACH_NUMERIC_TYPE(COMPARE_DEADBAND_ABSOLUTE_NUMERIC_VALUE)
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (compareValue != 0)
    {
        // Check absolute value of the difference
        diff = fabs(diff);
        if (diff > deadband)
        {
            *compResult = compareValue;
        }
        else
        {
            *compResult = 0;
        }
    }
    else
    {
        // Equality detected (including NaN equality for FP)
        *compResult = compareValue;
    }
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus compare_monitored_item_values(const SOPC_NumericRange* numRange,
                                                       const OpcUa_DataChangeFilter* filter,
                                                       const void* filterAbsDeadandCtx,
                                                       const SOPC_Variant* oldValue,
                                                       const SOPC_Variant* newValue,
                                                       int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL != filter && OpcUa_DeadbandType_None != filter->DeadbandType)
    {
        switch (filter->DeadbandType)
        {
        case OpcUa_DeadbandType_None:
            SOPC_ASSERT(false && "already evaluated case");
            break;
        case OpcUa_DeadbandType_Absolute:
            /* Variable DataType already verified to have (sub)type Number.
             * \p filterAbsDeadandCtx contains the absolute deadband.
             */
        case OpcUa_DeadbandType_Percent:
            /* Variable had a valid EURange property that was used to compute the \p filterAbsDeadandCtx
             * absolute deadband using percent value and range.
             * Note: we did not check the type was AnalogItemType and thus allow any variable with a valid EURange
             * property.
             */
            status = SOPC_Variant_CompareCustomRange(&compare_deadband_absolute, filterAbsDeadandCtx, oldValue,
                                                     newValue, numRange, comparison);
            break;
        default:
            // Already checked when retrieved in message
            SOPC_ASSERT(false && "invalid deadband type");
        }
    }
    else
    {
        // No filter active
        status = SOPC_Variant_CompareRange(oldValue, newValue, numRange, comparison);
    }
    return status;
}

static SOPC_ReturnStatus monitored_item_update_last_cached_value(SOPC_InternalMonitoredItem* monitItem,
                                                                 const SOPC_Variant* lastNotifiedValue)
{
    // See part 4 DataChangeFilterDataChangeFilter definition for cache necessity:
    // The last cached value is defined as the last value pushed to the queue [of notification]

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Cache the last notified value when filter are active (needed for next comparison)
    if (NULL != monitItem->filter && OpcUa_DeadbandType_None != monitItem->filter->DeadbandType)
    {
        SOPC_Variant_Clear(monitItem->lastCachedValueForFilter);
        if (NULL == monitItem->lastCachedValueForFilter)
        {
            monitItem->lastCachedValueForFilter = SOPC_Variant_Create();
            status = (NULL == monitItem->lastCachedValueForFilter) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Copy(monitItem->lastCachedValueForFilter, lastNotifiedValue);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(monitItem->lastCachedValueForFilter);
                monitItem->lastCachedValueForFilter = NULL;
            }
        }
    }
    return status;
}

static const SOPC_Variant* monitored_item_get_last_cached_value(const SOPC_InternalMonitoredItem* monitItem,
                                                                const SOPC_Variant* oldAddressSpaceValue)
{
    if (monitItem->filter != NULL && OpcUa_DeadbandType_None != monitItem->filter->DeadbandType)
    {
        // We shall use the last cached value if available
        if (NULL != monitItem->lastCachedValueForFilter)
        {
            return monitItem->lastCachedValueForFilter;
        }
    }
    // Previous address space value <=> cached value
    return oldAddressSpaceValue;
}

void monitored_item_pointer_bs__is_notification_triggered(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_old_wv_pointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_new_wv_pointer,
    t_bool* const monitored_item_pointer_bs__bres)
{
    *monitored_item_pointer_bs__bres = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t dtCompare = 0;
    SOPC_InternalMonitoredItem* monitItem = monitored_item_pointer_bs__p_monitoredItemPointer;
    OpcUa_DataChangeFilter* filter = monitItem->filter;
    const SOPC_Variant* lastCachedValue = NULL;

    if (monitItem->aid != constants__c_AttributeId_indet &&
        monitored_item_pointer_bs__p_new_wv_pointer->AttributeId == (uint32_t) monitItem->aid)
    {
        // Compare statuses first: DataChangeTrigger contains at least Status
        if (monitored_item_pointer_bs__p_old_wv_pointer->Value.Status ==
            monitored_item_pointer_bs__p_new_wv_pointer->Value.Status)
        {
            // If DataChangeTrigger defined, check if timestamp is included in change detection
            if (NULL != filter && OpcUa_DataChangeTrigger_StatusValueTimestamp == filter->Trigger &&
                OpcUa_DeadbandType_None == filter->DeadbandType)
            {
                if (monitored_item_pointer_bs__p_old_wv_pointer->Value.SourceTimestamp !=
                        monitored_item_pointer_bs__p_new_wv_pointer->Value.SourceTimestamp ||
                    monitored_item_pointer_bs__p_old_wv_pointer->Value.SourcePicoSeconds !=
                        monitored_item_pointer_bs__p_new_wv_pointer->Value.SourcePicoSeconds)
                {
                    // Timestamp change
                    dtCompare = -1;
                }
            }
            // If no changed detected and value change detection is active, compare values
            if (0 == dtCompare && (NULL == filter || OpcUa_DataChangeTrigger_StatusValue == filter->Trigger ||
                                   OpcUa_DataChangeTrigger_StatusValueTimestamp == filter->Trigger))
            {
                lastCachedValue = monitored_item_get_last_cached_value(
                    monitItem, &monitored_item_pointer_bs__p_old_wv_pointer->Value.Value);
                status = compare_monitored_item_values(
                    monitItem->indexRange, filter, &monitItem->filterAbsoluteDeadbandContext, lastCachedValue,
                    &monitored_item_pointer_bs__p_new_wv_pointer->Value.Value, &dtCompare);
            }
        }
        else
        {
            // Statuses are differents
            dtCompare = -1;
        }
        if (SOPC_STATUS_OK == status)
        {
            if (dtCompare != 0)
            {
                // Generate a notification if change detected
                *monitored_item_pointer_bs__bres = true;
                // Cache last value notified if value filtered
                monitored_item_update_last_cached_value(monitItem,
                                                        &monitored_item_pointer_bs__p_new_wv_pointer->Value.Value);
            }
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "MonitoredItem notification trigger: comparison of MI id=%" PRIu32
                " data values failed with (deadband, type, array type)=(%" PRIu32 ", %d, %d)",
                monitItem->monitoredItemId,
                NULL == monitItem->filter ? OpcUa_DeadbandType_None : monitItem->filter->DeadbandType,
                (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.BuiltInTypeId,
                (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.ArrayType);
        }
    }
}
