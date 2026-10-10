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

#ifndef SOPC_MONITORED_ITEM_POINTER_IMPL_H_
#define SOPC_MONITORED_ITEM_POINTER_IMPL_H_

#include "constants.h"

#include "sopc_circular_queue.h"
#include "sopc_macros.h"
#include "sopc_numeric_range.h"

typedef struct SOPC_InternalMonitoredItemFilterCtx
{
    bool isDataFilter;
    union
    {
        struct
        {
            OpcUa_DataChangeFilter dataFilter;
            double filterAbsoluteDeadbandContext;
            SOPC_Variant* lastCachedValueForFilter;
        } Data;
        struct
        {
            OpcUa_EventFilter* eventFilter;
            char** qnPathStrSelectClauses;               // eventFilter->NoOfSelectClauses clauses
            SOPC_NumericRange** indexRangeSelectClauses; // eventFilter->NoOfSelectClauses clauses
            SOPC_NodeId whereClauseTypeId; // Set to non-NULL id if eventFilter->WhereClause.Element[0] was a valid of
                                           // OfType expression
        } Event;
    } Filter;
} SOPC_InternalMonitoredItemFilterCtx;

/**
 * \brief Deletes a data or event filter context, including a partially built event filter context.
 *
 * \param filterCtx  The filter context to delete, might be NULL
 */
void SOPC_InternalMonitoredFilter_Free(SOPC_InternalMonitoredItemFilterCtx* filterCtx);

typedef struct SOPC_InternalMonitoredItem
{
    uint32_t monitoredItemId;
    constants__t_subscription_i subId;
    constants__t_NodeId_i nid;
    constants__t_AttributeId_i aid;
    constants__t_IndexRange_i indexRangeString;
    constants__t_TimestampsToReturn_i timestampToReturn;
    constants__t_monitoringMode_i monitoringMode;
    constants__t_client_handle_i clientHandle;
    SOPC_NumericRange* indexRange;
    SOPC_InternalMonitoredItemFilterCtx* filterCtx;
    bool discardOldest;
    int32_t queueSize;
    SOPC_CircularQueue* notifQueue;
    bool queueOverflowEventTriggered;
} SOPC_InternalMonitoredItem;

/**
 * \brief Sets the limits on the number of MonitoredItems, 0 meaning no limit.
 *        The limits are initialized with ::SOPC_MAX_MONITORED_ITEMS and ::SOPC_MAX_MONITORED_ITEMS_PER_SUBSCRIPTION
 *        on module INITIALISATION. Already created MonitoredItems are kept when a limit is lowered.
 *
 * \note Internal use only (tests): shall be called from the Services thread or before it is started.
 */
void SOPC_MonitoredItemPointer_SetLimits(uint32_t maxMonitoredItems, uint32_t maxMonitoredItemsPerSubscription);

/** \brief Returns the number of MonitoredItems currently created in the server. */
uint32_t SOPC_MonitoredItemPointer_GetNbMonitoredItems(void);

/** \brief Returns the number of MonitoredItems currently created in the given subscription. */
uint32_t SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(constants__t_subscription_i subscription);

// Non-static because this inlined function is used in inlined extern one
static SOPC_STRONG_INLINE bool is_eventMI(SOPC_InternalMonitoredItem* monitoredItemPointer)
{
    return (NULL != monitoredItemPointer->filterCtx && !monitoredItemPointer->filterCtx->isDataFilter);
}

// Number of total fallback allocation since initialization
size_t getNotifEltsPoolTotalFallback(void);

#endif /* SOPC_MONITORED_ITEM_POINTER_IMPL_H_ */
