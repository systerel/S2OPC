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

/**
 * \file monitored_item_pointer_bs.c
 * \brief This module manages storage of the information associated with created monitored items.
 *        A monitored item id is associated to a monitored item information structure pointer
 *        and a table provide lookup for this association.
 *
 * MonitoredItemId to \ref SOPC_InternalMonitoredItem mapping uses a 1-based pointer
 * table indexed by (id - 1), pre-allocated at init. Freed ids are recycled via
 * \c monitoredItemIdFreed.
 */

#include "monitored_item_pointer_bs.h"

#include "monitored_item_pointer_impl.h"

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "util_variant.h"

static void SOPC_InternalMonitoredFilter_Free(SOPC_InternalMonitoredItemFilterCtx* filterCtx)
{
    if (NULL != filterCtx)
    {
        if (filterCtx->isDataFilter)
        {
            SOPC_Variant_Delete(filterCtx->Filter.Data.lastCachedValueForFilter);
            filterCtx->Filter.Data.lastCachedValueForFilter = NULL;
        }
        else
        {
            for (int32_t i = 0; i < filterCtx->Filter.Event.eventFilter->NoOfSelectClauses; i++)
            {
                SOPC_NumericRange_Delete(filterCtx->Filter.Event.indexRangeSelectClauses[i]);
                SOPC_Free(filterCtx->Filter.Event.qnPathStrSelectClauses[i]);
            }
            SOPC_Free(filterCtx->Filter.Event.indexRangeSelectClauses);
            SOPC_Free(filterCtx->Filter.Event.qnPathStrSelectClauses);
            SOPC_NodeId_Clear(&filterCtx->Filter.Event.whereClauseTypeId);
            SOPC_ReturnStatus status = SOPC_EncodeableObject_Delete(filterCtx->Filter.Event.eventFilter->encodeableType,
                                                                    (void**) &filterCtx->Filter.Event.eventFilter);
            SOPC_UNUSED_RESULT(status);
        }
        SOPC_Free(filterCtx);
    }
}

static void SOPC_InternalMonitoredItem_Free(SOPC_InternalMonitoredItem* mi)
{
    if (NULL != mi)
    {
        SOPC_NumericRange_Delete(mi->indexRange);
        SOPC_NodeId_Clear(mi->nid);
        SOPC_Free(mi->nid);
        SOPC_String_Clear(mi->indexRangeString);
        SOPC_Free(mi->indexRangeString);
        SOPC_InternalMonitoredFilter_Free(mi->filterCtx);
        SOPC_Free(mi);
    }
}

/* Initial slot count; table is allocated at INITIALISATION (Calloc zeroes slots). */
#define MI_TABLE_INITIAL_CAPACITY ((size_t) 16)

/* MonitoredItemId (1-based) -> pointer table. Slot (id - 1) is NULL when the id
 * was deleted and is available for reuse (listed in monitoredItemIdFreed).
 * Capacity is a high-water mark: it never shrinks on delete. */
static SOPC_InternalMonitoredItem** monitoredItemById = NULL;
static size_t monitoredItemByIdCapacity = 0;
/* Ids popped on create, pushed on delete; avoids scanning NULL slots for reuse. */
static SOPC_SLinkedList* monitoredItemIdFreed = NULL;

/* Highest id ever issued when the freed list is empty. */
static uint32_t monitoredItemIdMax = 0;

/**
 * \brief Ensures the MonitoredItemId table can hold \p id (index \p id - 1).
 * \param id  MonitoredItemId (must not be 0 or UINT32_MAX).
 * \return \c true on success, \c false on invalid id or allocation failure.
 *
 * \warning table never shrinks: capacity tracks max id ever stored.
 */
static bool mi_table_ensure(uint32_t id)
{
    if (id == 0 || id == UINT32_MAX)
    {
        /* 0 is c_monitoredItemId_indet; UINT32_MAX is the id exhaustion sentinel. */
        return false;
    }
    if (id <= monitoredItemByIdCapacity)
    {
        return true;
    }
    size_t newCap = monitoredItemByIdCapacity;
    while (newCap < (size_t) id)
    {
        if (newCap > SIZE_MAX / 2)
        {
            return false;
        }
        newCap *= 2;
    }
    size_t oldBytes = monitoredItemByIdCapacity * sizeof(SOPC_InternalMonitoredItem*);
    size_t newBytes = newCap * sizeof(SOPC_InternalMonitoredItem*);
    SOPC_InternalMonitoredItem** p = SOPC_Realloc(monitoredItemById, oldBytes, newBytes);
    if (p == NULL)
    {
        return false;
    }
    memset(p + monitoredItemByIdCapacity, 0,
           (newCap - monitoredItemByIdCapacity) * sizeof(SOPC_InternalMonitoredItem*));
    /* New slots must be NULL: mi_table_get treats NULL as "no live item". */
    monitoredItemById = p;
    monitoredItemByIdCapacity = newCap;
    return true;
}

/**
 * \brief Looks up a monitored item by MonitoredItemId.
 * \param id  MonitoredItemId (1-based).
 * \return The item pointer, or \c NULL if \p id is invalid, out of range, or deleted.
 */
static SOPC_InternalMonitoredItem* mi_table_get(uint32_t id)
{
    if (id == 0 || id > monitoredItemByIdCapacity)
    {
        /* Never issued (id above capacity) or invalid (0). */
        return NULL;
    }
    /* NULL slot: deleted id or not yet reused; getall_monitoredItemId maps to bres=false. */
    return monitoredItemById[id - 1];
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_pointer_bs__INITIALISATION(void)
{
    monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();

    monitoredItemIdFreed = SOPC_SLinkedList_Create(0);
    SOPC_ASSERT(monitoredItemIdFreed != NULL);
    monitoredItemById = SOPC_Calloc(MI_TABLE_INITIAL_CAPACITY, sizeof(SOPC_InternalMonitoredItem*));
    SOPC_ASSERT(monitoredItemById != NULL);
    monitoredItemByIdCapacity = MI_TABLE_INITIAL_CAPACITY;
}

void monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION(void)
{
    if (monitoredItemById != NULL)
    {
        /* Free live items only; NULL slots are deleted ids (already freed on delete). */
        for (size_t i = 0; i < monitoredItemByIdCapacity; ++i)
        {
            if (monitoredItemById[i] != NULL)
            {
                SOPC_InternalMonitoredItem_Free(monitoredItemById[i]);
                monitoredItemById[i] = NULL;
            }
        }
        SOPC_Free(monitoredItemById);
        monitoredItemById = NULL;
    }
    monitoredItemByIdCapacity = 0;

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
    const constants__t_monitoringFilterCtx_i monitored_item_pointer_bs__p_filterCtx,
    const t_bool monitored_item_pointer_bs__p_discardOldest,
    const t_entier4 monitored_item_pointer_bs__p_queueSize,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode,
    constants__t_monitoredItemPointer_i* const monitored_item_pointer_bs__monitoredItemPointer,
    constants__t_monitoredItemId_i* const monitored_item_pointer_bs__monitoredItemId)
{
    SOPC_ASSERT(NULL != monitored_item_pointer_bs__p_filterCtx ||
                constants__e_aid_EventNotifier != monitored_item_pointer_bs__p_aid);

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
    SOPC_InternalMonitoredItemFilterCtx* filterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_pointer_bs__p_filterCtx;
    if (NULL == monitItem || NULL == nid || (NULL == rangeStr && NULL != monitored_item_pointer_bs__p_indexRange))
    {
        SOPC_Free(monitItem);
        SOPC_Free(nid);
        SOPC_Free(rangeStr);
        SOPC_Free(filterCtx);
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
        SOPC_ASSERT((constants__e_aid_EventNotifier != monitored_item_pointer_bs__p_aid &&
                     (NULL == filterCtx || filterCtx->isDataFilter)) ||
                    (constants__e_aid_EventNotifier == monitored_item_pointer_bs__p_aid && !filterCtx->isDataFilter));

        bool tableInsertionOK = false;

        monitItem->subId = monitored_item_pointer_bs__p_subscription;
        monitItem->nid = nid;
        monitItem->aid = monitored_item_pointer_bs__p_aid;
        monitItem->indexRangeString = rangeStr;
        monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
        monitItem->monitoringMode = monitored_item_pointer_bs__p_monitoringMode;
        monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;
        monitItem->indexRange = range;
        monitItem->filterCtx = filterCtx;
        monitItem->discardOldest = monitored_item_pointer_bs__p_discardOldest;
        monitItem->queueSize = monitored_item_pointer_bs__p_queueSize;

        if (0 == SOPC_SLinkedList_GetLength(monitoredItemIdFreed))
        {
            // No free unique Id, create a new one
            if (monitoredItemIdMax < UINT32_MAX)
            {
                monitoredItemIdMax++;
                monitItem->monitoredItemId = monitoredItemIdMax;
            } // else: all Ids already in use
        }
        else
        {
            // Reuse freed id
            freshId = (uint32_t) SOPC_SLinkedList_PopHead(monitoredItemIdFreed);
            if (freshId != 0)
            {
                monitItem->monitoredItemId = freshId;
            }
        }

        if (monitItem->monitoredItemId != 0)
        {
            /* Same path for fresh and reused ids: grow table if needed, then store pointer. */
            tableInsertionOK = mi_table_ensure(monitItem->monitoredItemId);
            if (tableInsertionOK)
            {
                monitoredItemById[monitItem->monitoredItemId - 1] = monitItem;
            }
        }
        /* monitoredItemId stays 0 when all ids are in use (UINT32_MAX reached). */

        if (!tableInsertionOK)
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
        SOPC_Free(filterCtx);
        SOPC_Free(monitItem);
        SOPC_NodeId_Clear(nid);
        SOPC_Free(nid);
        SOPC_String_Clear(rangeStr);
        SOPC_Free(rangeStr);
    }
}

static void check_and_move_if_last_cached_value_shall_be_kept(SOPC_InternalMonitoredItemFilterCtx* oldFilterCtx,
                                                              SOPC_InternalMonitoredItemFilterCtx* newFilterCtx)
{
    if (NULL == oldFilterCtx || NULL == newFilterCtx)
    {
        return;
    }
    if (oldFilterCtx->isDataFilter && newFilterCtx->isDataFilter &&
        oldFilterCtx->Filter.Data.dataFilter.DeadbandType != OpcUa_DeadbandType_None &&
        newFilterCtx->Filter.Data.dataFilter.DeadbandType != OpcUa_DeadbandType_None)
    {
        newFilterCtx->Filter.Data.lastCachedValueForFilter = oldFilterCtx->Filter.Data.lastCachedValueForFilter;
        oldFilterCtx->Filter.Data.lastCachedValueForFilter = NULL;
    }
}

void monitored_item_pointer_bs__modify_monitored_item_pointer(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
    const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
    const constants__t_monitoringFilterCtx_i monitored_item_pointer_bs__p_filterCtx,
    const t_bool monitored_item_pointer_bs__p_discardOldest,
    const t_entier4 monitored_item_pointer_bs__p_queueSize,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_pointer_bs__StatusCode)
{
    *monitored_item_pointer_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    SOPC_ASSERT(NULL != monitored_item_pointer_bs__p_filterCtx || constants__e_aid_EventNotifier != monitItem->aid);
    SOPC_ASSERT(NULL != monitItem->filterCtx || constants__e_aid_EventNotifier != monitItem->aid);

    monitItem->timestampToReturn = monitored_item_pointer_bs__p_timestampToReturn;
    monitItem->clientHandle = monitored_item_pointer_bs__p_clientHandle;

    SOPC_InternalMonitoredItemFilterCtx* newFilterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_pointer_bs__p_filterCtx;

    check_and_move_if_last_cached_value_shall_be_kept(monitItem->filterCtx, newFilterCtx);

    monitItem->discardOldest = monitored_item_pointer_bs__p_discardOldest;
    monitItem->queueSize = monitored_item_pointer_bs__p_queueSize;
    SOPC_InternalMonitoredFilter_Free(monitItem->filterCtx);
    monitItem->filterCtx = newFilterCtx;
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

    uintptr_t appended = SOPC_SLinkedList_Append(monitoredItemIdFreed, monitItem->monitoredItemId,
                                                 (uintptr_t) monitItem->monitoredItemId);

    if (appended != (uintptr_t) monitItem->monitoredItemId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "monitored_item_pointer_bs__delete_monitored_item_pointer: monitoredItemId %" PRIu32
                               " cannot be added to freed set",
                               monitItem->monitoredItemId);
    }

    /* Copy id before free; callers must not dereference monitItem after this call. */
    uint32_t id = monitItem->monitoredItemId;
    SOPC_InternalMonitoredItem_Free(monitItem);
    /* Clear table slot so getall_monitoredItemId rejects this id until reuse. */
    if (id > 0 && id <= monitoredItemByIdCapacity)
    {
        monitoredItemById[id - 1] = NULL;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "monitored_item_pointer_bs__delete_monitored_item_pointer: monitoredItemId %" PRIu32
                               " out of table range",
                               id);
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

void monitored_item_pointer_bs__get_monitoredItemFilter(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    constants__t_monitoringFilterCtx_i* const monitored_item_pointer_bs__p_filter)
{
    SOPC_InternalMonitoredItem* monitItem =
        (SOPC_InternalMonitoredItem*) monitored_item_pointer_bs__p_monitoredItemPointer;
    *monitored_item_pointer_bs__p_filter = monitItem->filterCtx;
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
        /* Lookup by id for Modify/Delete/SetMonitoringMode (subscription_core). */
        SOPC_InternalMonitoredItem* mi = mi_table_get(monitored_item_pointer_bs__p_monitoredItemId);
        if (mi != NULL)
        {
            *monitored_item_pointer_bs__bres = true;
            *monitored_item_pointer_bs__p_monitoredItemPointer = mi;
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
    else if (builtInTypeId == sopcTypeid)                                      \
    {                                                                          \
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
    }

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
    if (false)
    {
        // Necessary to use the following macro that generates "else if"
    }
    FOR_EACH_NUMERIC_TYPE(COMPARE_DEADBAND_ABSOLUTE_NUMERIC_VALUE)
    else { return SOPC_STATUS_INVALID_PARAMETERS; }
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

static SOPC_ReturnStatus compare_monitored_item_LT_values(char** localeIds,
                                                          const SOPC_NumericRange* numRange,
                                                          const SOPC_Variant* oldValue,
                                                          const SOPC_Variant* newValue,
                                                          int32_t* comparison)
{
    SOPC_Variant* tmpOldValue = util_variant__new_Variant_from_Variant(oldValue, false);
    SOPC_Variant* tmpNewValue = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    if (NULL != tmpOldValue)
    {
        // Get preferred localized text(s) for old value
        tmpOldValue =
            util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(&tmpOldValue, localeIds, NULL);
    }
    if (NULL != tmpOldValue)
    {
        tmpNewValue = util_variant__new_Variant_from_Variant(newValue, false);
    }
    if (NULL != tmpNewValue)
    {
        // Get preferred localized text(s) for new value
        tmpNewValue =
            util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(&tmpNewValue, localeIds, NULL);
    }
    if (NULL != tmpNewValue)
    {
        status = SOPC_Variant_CompareRange(tmpOldValue, tmpNewValue, numRange, comparison);
    }
    SOPC_Variant_Delete(tmpOldValue);
    SOPC_Variant_Delete(tmpNewValue);
    return status;
}

static SOPC_STRONG_INLINE SOPC_ReturnStatus compare_monitored_item_values(char** localeIds,
                                                                          const SOPC_NumericRange* numRange,
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
        // Check if value has localized text type and apply locales
        if (SOPC_LocalizedText_Id == oldValue->BuiltInTypeId && SOPC_LocalizedText_Id == newValue->BuiltInTypeId)
        {
            status = compare_monitored_item_LT_values(localeIds, numRange, oldValue, newValue, comparison);
        }
        else
        {
            status = SOPC_Variant_CompareRange(oldValue, newValue, numRange, comparison);
        }
    }
    return status;
}

static SOPC_ReturnStatus monitored_item_update_last_cached_value(SOPC_InternalMonitoredItem* monitItem,
                                                                 const SOPC_Variant* lastNotifiedValue)
{
    // See part 4 DataChangeFilterDataChangeFilter definition for cache necessity:
    // The last cached value is defined as the last value pushed to the queue [of notification]
    SOPC_ASSERT(NULL == monitItem->filterCtx || monitItem->filterCtx->isDataFilter);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Cache the last notified value when filter are active (needed for next comparison)
    if (NULL != monitItem->filterCtx &&
        OpcUa_DeadbandType_None != monitItem->filterCtx->Filter.Data.dataFilter.DeadbandType)
    {
        SOPC_Variant* lastValue = monitItem->filterCtx->Filter.Data.lastCachedValueForFilter;
        SOPC_Variant_Clear(lastValue);
        if (NULL == lastValue)
        {
            lastValue = SOPC_Variant_Create();
            status = (NULL == lastValue) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Copy(lastValue, lastNotifiedValue);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(lastValue);
                lastValue = NULL;
            }
        }
        monitItem->filterCtx->Filter.Data.lastCachedValueForFilter = lastValue;
    }
    return status;
}

static SOPC_STRONG_INLINE const SOPC_Variant* monitored_item_get_last_cached_value(
    const SOPC_InternalMonitoredItem* monitItem,
    const SOPC_Variant* oldAddressSpaceValue)
{
    if (monitItem->filterCtx != NULL && monitItem->filterCtx->isDataFilter &&
        OpcUa_DeadbandType_None != monitItem->filterCtx->Filter.Data.dataFilter.DeadbandType)
    {
        // We shall use the last cached value if available
        if (NULL != monitItem->filterCtx->Filter.Data.lastCachedValueForFilter)
        {
            return monitItem->filterCtx->Filter.Data.lastCachedValueForFilter;
        }
    }
    // Previous address space value <=> cached value
    return oldAddressSpaceValue;
}

void monitored_item_pointer_bs__is_event_monitoredItem(
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    t_bool* const monitored_item_pointer_bs__p_isEvent)
{
    SOPC_InternalMonitoredItem* monitItem = monitored_item_pointer_bs__p_monitoredItemPointer;
    *monitored_item_pointer_bs__p_isEvent = false;
    if (NULL != monitItem->filterCtx)
    {
        *monitored_item_pointer_bs__p_isEvent = !monitItem->filterCtx->isDataFilter;
    }
}

void monitored_item_pointer_bs__is_notification_triggered(
    const constants__t_LocaleIds_i monitored_item_pointer_bs__p_localeIds,
    const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_old_wv_pointer,
    const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_new_wv_pointer,
    t_bool* const monitored_item_pointer_bs__bres)
{
    *monitored_item_pointer_bs__bres = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t dtCompare = 0;
    SOPC_InternalMonitoredItem* monitItem = monitored_item_pointer_bs__p_monitoredItemPointer;
    SOPC_ASSERT(NULL == monitItem->filterCtx || monitItem->filterCtx->isDataFilter);
    OpcUa_DataChangeFilter* filter =
        (NULL == monitItem->filterCtx) ? NULL : &monitItem->filterCtx->Filter.Data.dataFilter;
    const void* filterAbsDeadandCtx =
        (NULL == monitItem->filterCtx) ? NULL : &monitItem->filterCtx->Filter.Data.filterAbsoluteDeadbandContext;
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
                    monitored_item_pointer_bs__p_localeIds, monitItem->indexRange, filter, filterAbsDeadandCtx,
                    lastCachedValue, &monitored_item_pointer_bs__p_new_wv_pointer->Value.Value, &dtCompare);
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
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "MonitoredItem notification trigger: comparison of MI id=%" PRIu32
                                   " data values failed with (deadband, type, array type)=(%" PRIu32 ", %d, %d)",
                                   monitItem->monitoredItemId,
                                   NULL == monitItem->filterCtx
                                       ? OpcUa_DeadbandType_None
                                       : monitItem->filterCtx->Filter.Data.dataFilter.DeadbandType,
                                   (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.BuiltInTypeId,
                                   (int) monitored_item_pointer_bs__p_new_wv_pointer->Value.Value.ArrayType);
        }
    }
}
