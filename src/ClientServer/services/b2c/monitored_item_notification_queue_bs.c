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

#include <assert.h>

#include "monitored_item_notification_queue_bs.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include "monitored_item_pointer_impl.h"
#include "util_b2c.h"

typedef struct SOPC_InternalNotificationElement
{
    SOPC_InternalMontitoredItem* monitoredItemPointer;
    OpcUa_WriteValue* value;
} SOPC_InternalNotificationElement;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_notification_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_notification_queue_bs__allocate_new_monitored_item_notification_queue(
    t_bool* const monitored_item_notification_queue_bs__bres,
    constants__t_notificationQueue_i* const monitored_item_notification_queue_bs__queue)
{
    *monitored_item_notification_queue_bs__queue = SOPC_SLinkedList_Create(0);
    if (*monitored_item_notification_queue_bs__queue == NULL)
    {
        *monitored_item_notification_queue_bs__bres = false;
    }
    else
    {
        *monitored_item_notification_queue_bs__bres = true;
    }
}

static void SOPC_InternalNotificationQueueElement_Free(uint32_t id, void* val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_InternalNotificationElement* notifElt = (SOPC_InternalNotificationElement*) val;
    OpcUa_WriteValue_Clear(notifElt->value);
    SOPC_Free(notifElt->value);
    SOPC_Free(notifElt);
}

void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(monitored_item_notification_queue_bs__p_queue, SOPC_InternalNotificationQueueElement_Free);
    SOPC_SLinkedList_Delete(monitored_item_notification_queue_bs__p_queue);
}

static SOPC_ReturnStatus SOPC_InternalAddCommonFinishAddNotifElt(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    SOPC_InternalNotificationElement* notifElt,
    const SOPC_String* indexRange,
    const SOPC_StatusCode valueStatus,
    const SOPC_Value_Timestamp monitored_item_notification_queue_bs__p_val_ts_src,
    const SOPC_Value_Timestamp monitored_item_notification_queue_bs__p_val_ts_srv,
    const SOPC_NodeId* monitored_item_notification_queue_bs__p_nid,
    const uint32_t attributeId)
{
    assert(notifElt != NULL);
    SOPC_InternalNotificationElement* checkAdded = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;

    retStatus = SOPC_String_Copy(&notifElt->value->IndexRange, indexRange);
    if (SOPC_STATUS_OK == retStatus)
    {
        retStatus = SOPC_NodeId_Copy(&notifElt->value->NodeId, monitored_item_notification_queue_bs__p_nid);
    }

    if (SOPC_STATUS_OK != retStatus)
    {
        return retStatus;
    }

    notifElt->value->AttributeId = attributeId;
    notifElt->value->Value.Status = valueStatus;
    notifElt->value->Value.SourceTimestamp = monitored_item_notification_queue_bs__p_val_ts_src.timestamp;
    notifElt->value->Value.SourcePicoSeconds = monitored_item_notification_queue_bs__p_val_ts_src.picoSeconds;
    notifElt->value->Value.ServerTimestamp = monitored_item_notification_queue_bs__p_val_ts_srv.timestamp;
    notifElt->value->Value.ServerPicoSeconds = monitored_item_notification_queue_bs__p_val_ts_srv.picoSeconds;

    checkAdded = SOPC_SLinkedList_Append(monitored_item_notification_queue_bs__p_queue, 0, notifElt);
    if (checkAdded == notifElt)
    {
        return SOPC_STATUS_OK;
    }
    else
    {
        return SOPC_STATUS_NOK;
    }
}

void monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
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
    SOPC_InternalNotificationElement* notifElt = SOPC_Malloc(sizeof(SOPC_InternalNotificationElement));
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
        retStatus = SOPC_InternalAddCommonFinishAddNotifElt(
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

void monitored_item_notification_queue_bs__add_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_TimestampsToReturn_i monitored_item_notification_queue_bs__p_timestampToReturn,
    const constants__t_WriteValuePointer_i monitored_item_notification_queue_bs__p_writeValuePointer,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    *monitored_item_notification_queue_bs__bres = false;
    if (SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) >=
        INT32_MAX) // number of notifications returned in B model as a int32
    {
        return;
    }

    SOPC_ReturnStatus retStatus = SOPC_STATUS_NOK;
    SOPC_InternalNotificationElement* notifElt = SOPC_Malloc(sizeof(SOPC_InternalNotificationElement));
    OpcUa_WriteValue* pNewWriteValue = SOPC_Malloc(sizeof(OpcUa_WriteValue));

    if (NULL == notifElt || NULL == pNewWriteValue)
    {
        SOPC_Free(notifElt);
        SOPC_Free(pNewWriteValue);
        return;
    }

    OpcUa_WriteValue_Initialize((void*) pNewWriteValue);
    notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
    notifElt->value = pNewWriteValue;

    /* IMPORTANT NOTE: indexRange filtering on value shall be done here ! */
    SOPC_NumericRange* indexRange =
        ((SOPC_InternalMontitoredItem*) monitored_item_notification_queue_bs__p_monitoredItem)->indexRange;
    if (NULL != indexRange)
    {
        retStatus = constants_statuscodes_bs__e_sc_ok ==
                    util_read_value_indexed_helper(
                        &pNewWriteValue->Value.Value,
                        &monitored_item_notification_queue_bs__p_writeValuePointer->Value.Value, indexRange);
    }
    else
    {
        retStatus = SOPC_Variant_Copy(&pNewWriteValue->Value.Value,
                                      &monitored_item_notification_queue_bs__p_writeValuePointer->Value.Value);
    }

    if (retStatus == SOPC_STATUS_OK)
    {
        SOPC_Value_Timestamp srcTs =
            (SOPC_Value_Timestamp){monitored_item_notification_queue_bs__p_writeValuePointer->Value.SourceTimestamp,
                                   monitored_item_notification_queue_bs__p_writeValuePointer->Value.SourcePicoSeconds};
        SOPC_Value_Timestamp srvTs =
            (SOPC_Value_Timestamp){monitored_item_notification_queue_bs__p_writeValuePointer->Value.ServerTimestamp,
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

        retStatus = SOPC_InternalAddCommonFinishAddNotifElt(
            monitored_item_notification_queue_bs__p_queue, notifElt,
            &monitored_item_notification_queue_bs__p_writeValuePointer->IndexRange,
            monitored_item_notification_queue_bs__p_writeValuePointer->Value.Status, srcTs, srvTs,
            &monitored_item_notification_queue_bs__p_writeValuePointer->NodeId,
            monitored_item_notification_queue_bs__p_writeValuePointer->AttributeId);
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

void monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_notification(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_bool* const monitored_item_notification_queue_bs__p_continue,
    constants__t_monitoredItemPointer_i* const monitored_item_notification_queue_bs__p_monitoredItem,
    constants__t_WriteValuePointer_i* const monitored_item_notification_queue_bs__p_writeValuePointer)
{
    *monitored_item_notification_queue_bs__p_continue = false;
    SOPC_InternalNotificationElement* notifElt =
        SOPC_SLinkedList_PopHead(monitored_item_notification_queue_bs__p_queue);

    assert(notifElt != NULL);

    *monitored_item_notification_queue_bs__p_monitoredItem = notifElt->monitoredItemPointer;
    *monitored_item_notification_queue_bs__p_writeValuePointer = notifElt->value;
    SOPC_Free(notifElt);
    *monitored_item_notification_queue_bs__p_continue =
        SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) > 0;
}

void monitored_item_notification_queue_bs__free_first_monitored_item_notification_value(
    const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer)
{
    SOPC_Free(monitored_item_notification_queue_bs__p_VariantValuePointer);
}

void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_entier4* const monitored_item_notification_queue_bs__p_nb_notifications)
{
    *monitored_item_notification_queue_bs__p_nb_notifications =
        (int32_t) SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue);
}
