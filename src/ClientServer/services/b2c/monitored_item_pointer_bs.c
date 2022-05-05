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
#include "monitored_item_pointer_impl.h"

#include <assert.h>
#include <inttypes.h>

#include "sopc_dict.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

static void SOPC_InternalMonitoredItem_Free(void* data)
{
    SOPC_InternalMontitoredItem* mi = (SOPC_InternalMontitoredItem*) data;
    if (NULL != mi)
    {
        SOPC_NumericRange_Delete(mi->indexRange);
        SOPC_NodeId_Clear(mi->nid);
        SOPC_Free(mi->nid);
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

static uintptr_t monitoredItemIdMax = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_pointer_bs__INITIALISATION(void)
{
    monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();

    monitoredItemIdDict = SOPC_Dict_Create(0, SOPC_InternalMonitoredItemId_Hash, SOPC_InternalMonitoredItemId_Equal,
                                           SOPC_InternalMonitoredItemId_Free, SOPC_InternalMonitoredItem_Free);
    assert(monitoredItemIdDict != NULL);
    monitoredItemIdFreed = SOPC_SLinkedList_Create(0);
    assert(monitoredItemIdFreed != NULL);
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
void monitored_item_pointer_bs__create_monitored_item_pointer(
    const constants__t_subscription_i monitored_item_pointer_bs__p_subscription,
    const constants__t_NodeId_i monitored_item_pointer_bs__p_nid,
    const constants__t_AttributeId_i monitored_item_pointer_bs__p_aid,
    const constants__t_IndexRange_i monitored_item_pointer_bs__p_indexRange,
    const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
    const constants__t_monitoringMode_i monitored_item_pointer_bs__p_monitoringMode,
    const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode,
    constants__t_monitoredItemPointer_i* const monitored_item_pointer_bs__monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__monitoredItemId)
{
    *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    uintptr_t freshId = 0;
    SOPC_InternalMontitoredItem* monitItem = SOPC_Malloc(sizeof(SOPC_InternalMontitoredItem));
    SOPC_NodeId* nid = SOPC_Malloc(sizeof(SOPC_NodeId));
    SOPC_NumericRange* range = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_NOK;

    if (NULL == monitItem || NULL == nid)
    {
        SOPC_Free(monitItem);
        SOPC_Free(nid);
        return;
    }

    SOPC_NodeId_Initialize(nid);
    retStatus = SOPC_NodeId_Copy(nid, monitored_item_pointer_bs__p_nid);

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
        bool dictInsertionOK = false;

        monitItem->subId = monitored_item_pointer_bs__p_subscription;
        monitItem->nid = nid;
        monitItem->aid = monitored_item_pointer_bs__p_aid;
        monitItem->indexRange = range;
        monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
        monitItem->monitoringMode = monitored_item_pointer_bs__p_monitoringMode;
        monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;

        if (0 == SOPC_SLinkedList_GetLength(monitoredItemIdFreed))
        {
            // No free unique Id, create a new one
            if (monitoredItemIdMax < UINTPTR_MAX) // Note: we already enforced that UINTPTR_MAX >= UINT32_MAX
            {
                monitoredItemIdMax++;
                monitItem->monitoredItemId = (uint32_t) monitoredItemIdMax;
                dictInsertionOK = SOPC_Dict_Insert(monitoredItemIdDict, (void*) monitoredItemIdMax, monitItem);
            } // else: all Ids already in use
        }
        else
        {
            // Reuse freed id
            freshId = (uintptr_t) SOPC_SLinkedList_PopHead(monitoredItemIdFreed);
            if (freshId != 0)
            {
                monitItem->monitoredItemId = (uint32_t) freshId;
                dictInsertionOK = SOPC_Dict_Insert(monitoredItemIdDict, (void*) freshId, monitItem);
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
    }
    else
    {
        SOPC_NumericRange_Delete(range);
        SOPC_Free(monitItem);
        SOPC_Free(nid);
    }
}

void monitored_item_pointer_bs__delete_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer)
{
    SOPC_InternalMontitoredItem* monitItem =
        (SOPC_InternalMontitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
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

void monitored_item_pointer_bs__getall_monitoredItemPointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__p_monitoredItemId,
    constants__t_subscription_i* const monitored_item_pointer_bs__p_subscription,
    constants__t_NodeId_i* const monitored_item_pointer_bs__p_nid,
    constants__t_AttributeId_i* const monitored_item_pointer_bs__p_aid,
    constants__t_TimestampsToReturn_i* const monitored_item_pointer_bs__p_timestampToReturn,
    constants__t_monitoringMode_i* const monitored_item_pointer_bs__p_monitoringMode,
    constants__t_client_handle_i* const monitored_item_pointer_bs__p_clientHandle)
{
    assert(NULL != monitored_item_pointer_bs__p_monitoredItemPointer); // Guaranteed by B model
    SOPC_InternalMontitoredItem* monitItem =
        (SOPC_InternalMontitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    *monitored_item_pointer_bs__p_monitoredItemId = monitItem->monitoredItemId;
    *monitored_item_pointer_bs__p_subscription = monitItem->subId;
    *monitored_item_pointer_bs__p_nid = monitItem->nid;
    *monitored_item_pointer_bs__p_aid = monitItem->aid;
    *monitored_item_pointer_bs__p_timestampToReturn = monitItem->timestampToReturn;
    *monitored_item_pointer_bs__p_monitoringMode = monitItem->monitoringMode;
    *monitored_item_pointer_bs__p_clientHandle = monitItem->clientHandle;
}

void monitored_item_pointer_bs__is_notification_triggered(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_old_wv_pointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_new_wv_pointer,
    t_bool* const monitored_item_pointer_bs__bres)
{
    *monitored_item_pointer_bs__bres = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    int32_t dtCompare = 0;
    SOPC_InternalMontitoredItem* monitItem = monitored_item_pointer_bs__p_monitoredItemPointer;
    // We already know that it is the same NodeId, check for attribute Id
    // TODO: check index if monitored item on an index of an array !

    if (monitItem->aid != constants__c_AttributeId_indet &&
        monitored_item_pointer_bs__p_new_wv_pointer->AttributeId == (uint32_t) monitItem->aid)
    {
        // Same attribute, now compare values (no filters managed for now: STATUS_VALUE_1 behavior)
        if (monitored_item_pointer_bs__p_old_wv_pointer->Value.Status ==
            monitored_item_pointer_bs__p_new_wv_pointer->Value.Status)
        {
            status = SOPC_Variant_CompareRange(&monitored_item_pointer_bs__p_old_wv_pointer->Value.Value,
                                               &monitored_item_pointer_bs__p_new_wv_pointer->Value.Value,
                                               monitItem->indexRange, &dtCompare);
        }
        else
        {
            // Statuses are differents
            status = SOPC_STATUS_OK;
            dtCompare = -1;
        }
        if (SOPC_STATUS_OK == status)
        {
            if (dtCompare != 0)
            {
                // Generate a notification
                *monitored_item_pointer_bs__bres = true;
            }
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "MonitoredItem notification trigger: comparison of data values failed with (type, array type)=(%d, %d)",
                (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.BuiltInTypeId,
                (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.ArrayType);
        }
    }
}
