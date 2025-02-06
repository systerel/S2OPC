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

#include "monitored_item_notification_queue_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include "monitored_item_pointer_impl.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"
#include "util_event.h"
#include "util_variant.h"

typedef struct SOPC_InternalNotificationElement
{
    SOPC_InternalMonitoredItem* monitoredItemPointer;
    OpcUa_WriteValue* value;
    OpcUa_EventFieldList* eventValues;
    bool isQueueOverflowEvent;
} SOPC_InternalNotificationElement;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_notification_queue_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static bool is_eventMI(SOPC_InternalMonitoredItem* monitoredItemPointer)
{
    return (NULL != monitoredItemPointer->filterCtx && !monitoredItemPointer->filterCtx->isDataFilter);
}

void monitored_item_notification_queue_bs__allocate_new_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const t_bool monitored_item_notification_queue_bs__p_isEvent,
    t_bool* const monitored_item_notification_queue_bs__bres,
    constants__t_notificationQueue_i* const monitored_item_notification_queue_bs__queue)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitoredItemPointer->queueSize > 0);
    SOPC_ASSERT(monitored_item_notification_queue_bs__p_isEvent == is_eventMI(monitoredItemPointer));
    if (monitored_item_notification_queue_bs__p_isEvent)
    {
        // 1 element added to maximum in order to be allowed to store an EventQueueOverflowEventType event
        monitoredItemPointer->notifQueue = SOPC_SLinkedList_Create((size_t) monitoredItemPointer->queueSize + 1);
    }
    else
    {
        monitoredItemPointer->notifQueue = SOPC_SLinkedList_Create((size_t) monitoredItemPointer->queueSize);
    }
    if (NULL == monitoredItemPointer->notifQueue)
    {
        *monitored_item_notification_queue_bs__bres = false;
    }
    else
    {
        *monitored_item_notification_queue_bs__queue = monitoredItemPointer->notifQueue;
        *monitored_item_notification_queue_bs__bres = true;
    }
}

static void SOPC_InternalNotificationElement_Free(SOPC_InternalNotificationElement* notifElt)
{
    if (NULL != notifElt)
    {
        OpcUa_WriteValue_Clear(notifElt->value);
        SOPC_Free(notifElt->value);
        SOPC_ReturnStatus status =
            SOPC_EncodeableObject_Delete(&OpcUa_EventFieldList_EncodeableType, (void**) &notifElt->eventValues);
        SOPC_ASSERT(NULL == notifElt->eventValues || SOPC_STATUS_OK == status);
        SOPC_Free(notifElt);
    }
}

static void SOPC_InternalNotificationQueueElement_Free(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_InternalNotificationElement_Free((SOPC_InternalNotificationElement*) val);
}

void monitored_item_notification_queue_bs__clear_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitoredItemPointer->notifQueue == monitored_item_notification_queue_bs__p_queue);
    SOPC_SLinkedList_Apply(monitoredItemPointer->notifQueue, SOPC_InternalNotificationQueueElement_Free);
    SOPC_SLinkedList_Clear(monitoredItemPointer->notifQueue);
}

void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitoredItemPointer->notifQueue == monitored_item_notification_queue_bs__p_queue);
    SOPC_SLinkedList_Apply(monitoredItemPointer->notifQueue, SOPC_InternalNotificationQueueElement_Free);
    SOPC_SLinkedList_Delete(monitoredItemPointer->notifQueue);
    monitoredItemPointer->notifQueue = NULL;
}

static SOPC_InternalNotificationElement* SOPC_InternalGetDiscardedNotification(SOPC_SLinkedList* notifQueue,
                                                                               bool discardOldest)
{
    SOPC_ASSERT(NULL != notifQueue);
    SOPC_ASSERT(SOPC_SLinkedList_GetLength(notifQueue) > 0);
    SOPC_InternalNotificationElement* discardedNotifElt = NULL;
    if (discardOldest)
    {
        discardedNotifElt = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_PopHead(notifQueue);
    }
    else
    {
        discardedNotifElt = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_PopLast(notifQueue);
    }
    SOPC_ASSERT(NULL != discardedNotifElt);
    return discardedNotifElt;
}

static void SOPC_InternalDiscardOneNotification(SOPC_SLinkedList* notifQueue, bool discardOldest)
{
    SOPC_InternalNotificationElement* discardedNotifElt =
        SOPC_InternalGetDiscardedNotification(notifQueue, discardOldest);
    SOPC_InternalNotificationElement_Free(discardedNotifElt);
}

static void SOPC_InternalSetOverflowBitAfterDiscard(SOPC_SLinkedList* notifQueue, bool discardOldest)
{
    SOPC_InternalNotificationElement* notifElt = NULL;

    /* Set the overflow bit in DataValue status code in value replacing discarded one */
    if (discardOldest)
    {
        /* New oldest notification DataValue status code should have bit set */
        notifElt = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_GetHead(notifQueue);
    }
    else
    { // New last notification DataValue status code should have bit set
        notifElt = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_GetLast(notifQueue);
    }
    SOPC_ASSERT(NULL != notifElt);

    /* The next notification of the one discarded should have overflow bit set */
    notifElt->value->Value.Status |= SOPC_DataValueOverflowStatusMask;
}

static SOPC_ReturnStatus SOPC_InternalAddCommonFinishAddNotifDataElt(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    SOPC_InternalNotificationElement* notifElt,
    const SOPC_String* indexRange,
    const SOPC_StatusCode valueStatus,
    const SOPC_Value_Timestamp monitored_item_notification_queue_bs__p_val_ts_src,
    const SOPC_Value_Timestamp monitored_item_notification_queue_bs__p_val_ts_srv,
    const SOPC_NodeId* monitored_item_notification_queue_bs__p_nid,
    const uint32_t attributeId)
{
    SOPC_ASSERT(notifElt != NULL);
    SOPC_InternalNotificationElement* checkAdded = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;

    retStatus = SOPC_String_Copy(&notifElt->value->IndexRange, indexRange);
    if (SOPC_STATUS_OK == retStatus)
    {
        retStatus = SOPC_NodeId_Copy(&notifElt->value->NodeId, monitored_item_notification_queue_bs__p_nid);
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        notifElt->value->AttributeId = attributeId;
        notifElt->value->Value.Status = valueStatus;
        notifElt->value->Value.SourceTimestamp = monitored_item_notification_queue_bs__p_val_ts_src.timestamp;
        notifElt->value->Value.SourcePicoSeconds = monitored_item_notification_queue_bs__p_val_ts_src.picoSeconds;
        notifElt->value->Value.ServerTimestamp = monitored_item_notification_queue_bs__p_val_ts_srv.timestamp;
        notifElt->value->Value.ServerPicoSeconds = monitored_item_notification_queue_bs__p_val_ts_srv.picoSeconds;

        checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Append(
            monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) notifElt);
        if (checkAdded != notifElt)
        {
            uint32_t capacity = SOPC_SLinkedList_GetCapacity(monitored_item_notification_queue_bs__p_queue);
            /* Discard a notification to add the new one */
            if (capacity > 0 && SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) == capacity)
            {
                SOPC_InternalDiscardOneNotification(monitored_item_notification_queue_bs__p_queue,
                                                    notifElt->monitoredItemPointer->discardOldest);
                checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Append(
                    monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) notifElt);
                if (checkAdded != notifElt)
                {
                    retStatus = SOPC_STATUS_NOK;
                }
                else if (SOPC_SLinkedList_GetCapacity(monitored_item_notification_queue_bs__p_queue) != 1)
                {
                    SOPC_InternalSetOverflowBitAfterDiscard(monitored_item_notification_queue_bs__p_queue,
                                                            notifElt->monitoredItemPointer->discardOldest);
                }
                /* else: no overflow bit to set since it is not considered as a queue if size is 1.
                 *
                 * From part 4 (v1.05) table 144:
                 * For values larger than one a first-in-first-out queue is to be used. The Server may
                 * limit the size in revisedQueueSize. In the case of a queue overflow, the Overflow bit
                 * (flag) in the InfoBits portion of the DataValue statusCode is set in the new value.
                 */
            }
            else
            {
                retStatus = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }

    return retStatus;
}

void monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_NodeId_i monitored_item_notification_queue_bs__p_nid,
    const constants__t_AttributeId_i monitored_item_notification_queue_bs__p_aid,
    const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer,
    const constants__t_RawStatusCode monitored_item_notification_queue_bs__p_ValueSc,
    const constants__t_Timestamp monitored_item_notification_queue_bs__p_val_ts_src,
    const constants__t_Timestamp monitored_item_notification_queue_bs__p_val_ts_srv,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    *monitored_item_notification_queue_bs__bres = false;
    SOPC_StatusCode valueStatus = monitored_item_notification_queue_bs__p_ValueSc;
    if (SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) >=
        INT32_MAX) // number of notifications returned in B model as a int32
    {
        return;
    }

    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    SOPC_InternalNotificationElement* notifElt = SOPC_Calloc(1, sizeof(SOPC_InternalNotificationElement));
    OpcUa_WriteValue* pNewWriteValue = SOPC_Malloc(sizeof(OpcUa_WriteValue));

    if (NULL == pNewWriteValue || NULL == notifElt)
    {
        SOPC_Free(notifElt);
        SOPC_Free(pNewWriteValue);
        return;
    }
    OpcUa_WriteValue_Initialize(pNewWriteValue);
    notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
    notifElt->value = pNewWriteValue;

    if (constants__c_Variant_indet != monitored_item_notification_queue_bs__p_VariantValuePointer)
    {
        retStatus = SOPC_Variant_Copy(&pNewWriteValue->Value.Value,
                                      monitored_item_notification_queue_bs__p_VariantValuePointer);
    }
    else
    {
        // The status shall be != Good if the variant is null
        if ((valueStatus & SOPC_GoodStatusOppositeMask) == 0)
        {
            // The status is good whereas no value is provided, it is unexpected
            valueStatus = OpcUa_BadInternalError;
        }
    }

    if (monitored_item_notification_queue_bs__p_aid == constants__c_AttributeId_indet)
    {
        retStatus = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        SOPC_String indexRangeString;
        SOPC_String_Initialize(&indexRangeString);
        retStatus = SOPC_InternalAddCommonFinishAddNotifDataElt(
            monitored_item_notification_queue_bs__p_queue, notifElt, &indexRangeString, valueStatus,
            monitored_item_notification_queue_bs__p_val_ts_src, monitored_item_notification_queue_bs__p_val_ts_srv,
            monitored_item_notification_queue_bs__p_nid, (uint32_t) monitored_item_notification_queue_bs__p_aid);
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        *monitored_item_notification_queue_bs__bres = true;
    }
    else
    {
        SOPC_Free(notifElt);
        OpcUa_WriteValue_Clear(pNewWriteValue);
        SOPC_Free(pNewWriteValue);
    }
}

void monitored_item_notification_queue_bs__add_monitored_item_data_notification_to_queue(
    const constants__t_LocaleIds_i monitored_item_notification_queue_bs__p_localeIds,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_TimestampsToReturn_i monitored_item_notification_queue_bs__p_timestampToReturn,
    const constants__t_WriteValuePointer_i monitored_item_notification_queue_bs__p_writeValuePointer,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitored_item_notification_queue_bs__p_queue == monitoredItemPointer->notifQueue);
    SOPC_ASSERT(!is_eventMI(monitoredItemPointer));
    *monitored_item_notification_queue_bs__bres = false;

    SOPC_ReturnStatus retStatus = SOPC_STATUS_NOK;
    SOPC_InternalNotificationElement* notifElt = SOPC_Calloc(1, sizeof(SOPC_InternalNotificationElement));
    OpcUa_WriteValue* pNewWriteValue = SOPC_Malloc(sizeof(OpcUa_WriteValue));
    SOPC_StatusCode valueStatus = monitored_item_notification_queue_bs__p_writeValuePointer->Value.Status;
    constants_statuscodes_bs__t_StatusCode_i readSC = constants_statuscodes_bs__c_StatusCode_indet;
    SOPC_Variant* newValue = &monitored_item_notification_queue_bs__p_writeValuePointer->Value.Value;
    if (NULL == notifElt || NULL == pNewWriteValue)
    {
        retStatus = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        retStatus = SOPC_STATUS_OK;
        OpcUa_WriteValue_Initialize((void*) pNewWriteValue);
        notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
        notifElt->value = pNewWriteValue;

        SOPC_ReturnStatus copyStatus = util_variant__copy_and_apply_locales_and_index_range(
            &pNewWriteValue->Value.Value, newValue, monitored_item_notification_queue_bs__p_localeIds,
            ((SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem)->indexRange);
        if (SOPC_STATUS_OK != copyStatus)
        {
            SOPC_ASSERT(pNewWriteValue->Value.Value.BuiltInTypeId == SOPC_StatusCode_Id);
            // The Bad status is copied as StatusCode Value in case of error:
            // move it as data value status code and clear the variant
            valueStatus = pNewWriteValue->Value.Value.Value.Status;
            SOPC_Variant_Clear(&pNewWriteValue->Value.Value);
        }

        if (SOPC_STATUS_OK == retStatus)
        {
            SOPC_Value_Timestamp srcTs = (SOPC_Value_Timestamp){
                monitored_item_notification_queue_bs__p_writeValuePointer->Value.SourceTimestamp,
                monitored_item_notification_queue_bs__p_writeValuePointer->Value.SourcePicoSeconds};
            SOPC_Value_Timestamp srvTs = (SOPC_Value_Timestamp){
                monitored_item_notification_queue_bs__p_writeValuePointer->Value.ServerTimestamp,
                monitored_item_notification_queue_bs__p_writeValuePointer->Value.ServerPicoSeconds};

            switch (monitored_item_notification_queue_bs__p_timestampToReturn)
            {
            case constants__e_ttr_source:
                srvTs = constants__c_Timestamp_null;
                break;
            case constants__e_ttr_server:
                srcTs = constants__c_Timestamp_null;
                break;
            case constants__e_ttr_neither:
                srcTs = constants__c_Timestamp_null;
                srvTs = constants__c_Timestamp_null;
                break;
            default:
                // Keep both in other cases
                break;
            }

            retStatus = SOPC_InternalAddCommonFinishAddNotifDataElt(
                monitored_item_notification_queue_bs__p_queue, notifElt,
                &monitored_item_notification_queue_bs__p_writeValuePointer->IndexRange, valueStatus, srcTs, srvTs,
                &monitored_item_notification_queue_bs__p_writeValuePointer->NodeId,
                monitored_item_notification_queue_bs__p_writeValuePointer->AttributeId);
        }
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        *monitored_item_notification_queue_bs__bres = true;
    }
    else
    {
        SOPC_Free(notifElt);
        OpcUa_WriteValue_Clear(pNewWriteValue);
        SOPC_Free(pNewWriteValue);

        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Services: add_monitored_item_data_notification_to_queue OOM for MI id="
            "%" PRIu32 " or read failed with sc=%d",
            ((SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem)->monitoredItemId,
            readSC);
    }
}

static bool SOPC_InternalCreateQueueOverflowNotif(SOPC_InternalMonitoredItem* monitoredItemPointer,
                                                  SOPC_InternalNotificationElement** queueOverflowNotifRes)
{
    bool res = false;
    SOPC_ASSERT(!monitoredItemPointer->queueOverflowEventTriggered);
    SOPC_ASSERT(NULL != queueOverflowNotifRes && NULL == *queueOverflowNotifRes);
    SOPC_InternalNotificationElement* queueOverflowNotif = SOPC_Calloc(1, sizeof(SOPC_InternalNotificationElement));
    res = (NULL != queueOverflowNotif);
    if (res)
    {
        queueOverflowNotif->monitoredItemPointer = monitoredItemPointer;
        queueOverflowNotif->isQueueOverflowEvent = true;
        res = util_event__gen_event_queue_overflow_notification(
            monitoredItemPointer->filterCtx, monitoredItemPointer->clientHandle, &queueOverflowNotif->eventValues);
    }
    if (res)
    {
        monitoredItemPointer->queueOverflowEventTriggered = true;
        *queueOverflowNotifRes = queueOverflowNotif;
    }
    else
    {
        SOPC_Free(queueOverflowNotif);
    }
    return res;
}

static bool SOPC_InternalAddCommonFinishAddNotifEventElt(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    SOPC_InternalNotificationElement* notifElt)
{
    /* See details in part 4 §5.12.1.5:
     * The queue size is the maximum value supported by the Server when monitoring Events.
     * If Events are lost, an Event of the type EventQueueOverflowEventType is placed
     * in the queue. This Event is generated when the first Event is discarded on a MonitoredItem subscribing for
     * Events. It is put into the Queue of the MonitoredItem in addition to the size of the Queue defined for this
     * MonitoredItem without discarding any other Event.
     * If discardOldest is set to TRUE it is put at the beginning of the queue and is never discarded,
     * otherwise at the end.
     */
    SOPC_ASSERT(notifElt != NULL);
    SOPC_InternalNotificationElement* checkAdded = NULL;
    uint32_t capacity = SOPC_SLinkedList_GetCapacity(monitored_item_notification_queue_bs__p_queue);
    // One element reserved for queue overflow event
    capacity = (capacity > 0 ? capacity - 1 : 0);
    uint32_t length = SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue);
    bool res = length < capacity;
    if (res)
    {
        checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Append(
            monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) notifElt);
        res = (checkAdded == notifElt);
    }
    if (!res && capacity > 0)
    {
        /* Discard a notification to add the new one */
        res = true;
        SOPC_InternalNotificationElement* queueOverflowNotif = NULL;
        SOPC_InternalMonitoredItem* monitoredItemPointer = notifElt->monitoredItemPointer;
        SOPC_InternalNotificationElement* actualNotifDiscarded = NULL;
        uint8_t discardedCount = 0;
        // Discard a notification event and possibly retrieve the queue overflow notification to restore it
        // (possible with discardOldest = TRUE case only)
        while (NULL == actualNotifDiscarded && discardedCount < 2)
        {
            actualNotifDiscarded = SOPC_InternalGetDiscardedNotification(monitored_item_notification_queue_bs__p_queue,
                                                                         monitoredItemPointer->discardOldest);
            if (actualNotifDiscarded->isQueueOverflowEvent)
            {
                queueOverflowNotif = actualNotifDiscarded;
                actualNotifDiscarded = NULL;
            }
            discardedCount++;
        }
        SOPC_ASSERT(NULL == queueOverflowNotif ||
                    monitoredItemPointer->discardOldest); // otherwise it cannot be discarded
        SOPC_ASSERT(NULL != actualNotifDiscarded);
        SOPC_InternalNotificationElement_Free(actualNotifDiscarded);

        // Create the first MI queue overflow event if first time it occurs
        if (!monitoredItemPointer->queueOverflowEventTriggered)
        {
            // Note: This Event is generated when the first Event is discarded on a MonitoredItem
            res = SOPC_InternalCreateQueueOverflowNotif(monitoredItemPointer, &queueOverflowNotif);
        }

        // May enqueue the overflow notification (previously discarded or newly created)
        // Note: It is put into the Queue of the MonitoredItem in addition to the size of the Queue
        if (res && NULL != queueOverflowNotif)
        {
            if (monitoredItemPointer->discardOldest)
            {
                // Enqueue the overflow notification (previously discarded or newly created) at the beginning
                checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Prepend(
                    monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) queueOverflowNotif);
            }
            else
            {
                // Enqueue the newly created overflow notification at the end
                checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Append(
                    monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) queueOverflowNotif);
            }
            res = (checkAdded == queueOverflowNotif);
        }
        // Add the new notification event since one has been discarded
        if (res)
        {
            checkAdded = (SOPC_InternalNotificationElement*) SOPC_SLinkedList_Append(
                monitored_item_notification_queue_bs__p_queue, 0, (uintptr_t) notifElt);
            res = (checkAdded == notifElt);
        }
        else
        {
            SOPC_InternalNotificationElement_Free(queueOverflowNotif);
        }
    }
    return res;
}

void monitored_item_notification_queue_bs__add_monitored_item_event_notification_to_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_eventFieldList_i monitored_item_notification_queue_bs__p_eventFieldList,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitored_item_notification_queue_bs__p_queue == monitoredItemPointer->notifQueue);
    SOPC_ASSERT(is_eventMI(monitoredItemPointer));
    *monitored_item_notification_queue_bs__bres = false;

    SOPC_InternalNotificationElement* notifElt = SOPC_Calloc(1, sizeof(SOPC_InternalNotificationElement));
    if (NULL == notifElt)
    {
        return;
    }
    notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
    notifElt->eventValues = monitored_item_notification_queue_bs__p_eventFieldList;

    bool res = SOPC_InternalAddCommonFinishAddNotifEventElt(monitored_item_notification_queue_bs__p_queue, notifElt);

    if (!res)
    {
        SOPC_InternalNotificationElement_Free(notifElt);

        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Services: add_monitored_item_event_notification_to_queue OOM for MI id="
            "%" PRIu32,
            ((SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem)->monitoredItemId);
    }

    *monitored_item_notification_queue_bs__bres = res;
}

static SOPC_InternalNotificationElement* continue_pop_iter_notif(SOPC_SLinkedList* notifQueue, bool* hasMoreElts)
{
    SOPC_InternalNotificationElement* notifElt =
        (SOPC_InternalNotificationElement*) SOPC_SLinkedList_PopHead(notifQueue);
    SOPC_ASSERT(notifElt != NULL);
    *hasMoreElts = SOPC_SLinkedList_GetLength(notifQueue) > 0;
    return notifElt;
}

void monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_data_notification(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__p_continue,
    constants__t_WriteValuePointer_i* const monitored_item_notification_queue_bs__p_writeValuePointer)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(!is_eventMI(monitoredItemPointer));
    *monitored_item_notification_queue_bs__p_continue = false;
    SOPC_InternalNotificationElement* notifElt = continue_pop_iter_notif(
        monitored_item_notification_queue_bs__p_queue, monitored_item_notification_queue_bs__p_continue);

    *monitored_item_notification_queue_bs__p_writeValuePointer = notifElt->value;
    SOPC_Free(notifElt);
}

void monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_event_notification(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__p_continue,
    constants__t_eventFieldList_i* const monitored_item_notification_queue_bs__p_eventFieldList)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(is_eventMI(monitoredItemPointer));
    *monitored_item_notification_queue_bs__p_continue = false;
    SOPC_InternalNotificationElement* notifElt = continue_pop_iter_notif(
        monitored_item_notification_queue_bs__p_queue, monitored_item_notification_queue_bs__p_continue);

    *monitored_item_notification_queue_bs__p_eventFieldList = notifElt->eventValues;
    SOPC_Free(notifElt);
}

void monitored_item_notification_queue_bs__free_first_monitored_item_notification_value(
    const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer)
{
    SOPC_Free(monitored_item_notification_queue_bs__p_VariantValuePointer);
}

void monitored_item_notification_queue_bs__get_length_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_mi_notif_queue,
    t_entier4* const monitored_item_notification_queue_bs__p_nb_available_notifs)
{
    SOPC_ASSERT(NULL != monitored_item_notification_queue_bs__p_mi_notif_queue);
    uint32_t length = SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_mi_notif_queue);
    SOPC_ASSERT(length <= INT32_MAX); // Guaranteed by add functions
    *monitored_item_notification_queue_bs__p_nb_available_notifs = (int32_t) length;
}

void monitored_item_notification_queue_bs__get_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    t_bool* const monitored_item_notification_queue_bs__bres,
    constants__t_notificationQueue_i* const monitored_item_notification_queue_bs__queue)
{
    *monitored_item_notification_queue_bs__bres = false;
    *monitored_item_notification_queue_bs__queue = NULL;
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    if (NULL != monitoredItemPointer->notifQueue)
    {
        *monitored_item_notification_queue_bs__bres = true;
        *monitored_item_notification_queue_bs__queue = monitoredItemPointer->notifQueue;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "MonitoredItem (%" PRIu32 ") notification queue is not allocated",
                               monitoredItemPointer->monitoredItemId);
    }
}

void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__p_continue,
    t_bool* const monitored_item_notification_queue_bs__p_isEvent)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    *monitored_item_notification_queue_bs__p_isEvent = is_eventMI(monitoredItemPointer);
    *monitored_item_notification_queue_bs__p_continue =
        SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) > 0;
}

void monitored_item_notification_queue_bs__is_event_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_mi_notif_queue,
    t_bool* const monitored_item_notification_queue_bs__p_is_event_queue)
{
    SOPC_UNUSED_ARG(monitored_item_notification_queue_bs__p_mi_notif_queue);
    *monitored_item_notification_queue_bs__p_is_event_queue =
        is_eventMI(monitored_item_notification_queue_bs__p_monitoredItem);
}

void monitored_item_notification_queue_bs__resize_monitored_item_notification_queue(
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem)
{
    SOPC_InternalMonitoredItem* monitoredItemPointer =
        (SOPC_InternalMonitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem;
    SOPC_ASSERT(monitoredItemPointer->queueSize >= 0);
    SOPC_SLinkedList* notifQueue = monitoredItemPointer->notifQueue;

    /* Discard notifications if more available than new capacity */
    bool discardedNotifs = false;
    while (SOPC_SLinkedList_GetLength(notifQueue) > (uint32_t) monitoredItemPointer->queueSize)
    {
        discardedNotifs = true;
        SOPC_InternalDiscardOneNotification(notifQueue, monitoredItemPointer->discardOldest);
    }
    /* If some notifications were discarded and new capacity is > 1, overflow bit shall be set */
    if (discardedNotifs && monitoredItemPointer->queueSize > 1)
    {
        SOPC_InternalSetOverflowBitAfterDiscard(notifQueue, monitoredItemPointer->discardOldest);
    }

    /* Change notification queue capacity */
    bool capacitySet = SOPC_SLinkedList_SetCapacity(notifQueue, (size_t) monitoredItemPointer->queueSize);
    SOPC_ASSERT(capacitySet);
}
