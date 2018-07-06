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

#include <assert.h>

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include "util_b2c.h"

typedef struct SOPC_InternalNotificationElement
{
    constants__t_monitoredItemPointer_i monitoredItemPointer;
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
    (void) id;
    SOPC_InternalNotificationElement* notifElt = (SOPC_InternalNotificationElement*) val;
    OpcUa_WriteValue_Clear(notifElt->value);
    free(notifElt->value);
    free(notifElt);
}

void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(monitored_item_notification_queue_bs__p_queue, SOPC_InternalNotificationQueueElement_Free);
    SOPC_SLinkedList_Delete(monitored_item_notification_queue_bs__p_queue);
}

void monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_TimestampsToReturn_i monitored_item_notification_queue_bs__p_timestampToReturn,
    const constants__t_NodeId_i monitored_item_notification_queue_bs__p_nid,
    const constants__t_AttributeId_i monitored_item_notification_queue_bs__p_aid,
    const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer,
    const constants__t_StatusCode_i monitored_item_notification_queue_bs__p_ValueSc,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    *monitored_item_notification_queue_bs__bres = false;
    OpcUa_WriteValue* wv = malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize(wv);
    SOPC_InternalNotificationElement* notifElt = malloc(sizeof(SOPC_InternalNotificationElement));
    SOPC_InternalNotificationElement* checkAdded = NULL;
    if (NULL != wv && NULL != notifElt)
    {
        util_status_code__B_to_C(monitored_item_notification_queue_bs__p_ValueSc, &wv->Value.Status);
        wv->AttributeId = monitored_item_notification_queue_bs__p_aid;
        /* TODO: manage timestamp in address space instead of giving current time */
        wv->Value.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        wv->Value.ServerTimestamp = wv->Value.SourceTimestamp;

        /* Complying with timestamp to return configured */
        switch (monitored_item_notification_queue_bs__p_timestampToReturn)
        {
        case constants__e_ttr_source:
            wv->Value.ServerTimestamp = 0;
            break;
        case constants__e_ttr_server:
            wv->Value.SourceTimestamp = 0;
            break;
        case constants__e_ttr_neither:
            wv->Value.ServerTimestamp = 0;
            wv->Value.SourceTimestamp = 0;
            break;
        default:
            // Keep both in other cases
            break;
        }

        retStatus = SOPC_NodeId_Copy(&wv->NodeId, monitored_item_notification_queue_bs__p_nid);
        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus =
                SOPC_Variant_Copy(&wv->Value.Value, monitored_item_notification_queue_bs__p_VariantValuePointer);
        }
        if (SOPC_STATUS_OK == retStatus)
        {
            notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
            notifElt->value = wv;
            checkAdded = SOPC_SLinkedList_Append(monitored_item_notification_queue_bs__p_queue, 0, notifElt);
            if (checkAdded == notifElt)
            {
                *monitored_item_notification_queue_bs__bres = true;
            }
            else
            {
                retStatus = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK != retStatus)
        {
            free(notifElt);
            OpcUa_WriteValue_Clear(wv);
            free(wv);
        }
    }
    else if (NULL != notifElt)
    {
        free(notifElt);
    }
    else if (NULL != wv)
    {
        free(wv);
    }
    free(monitored_item_notification_queue_bs__p_VariantValuePointer);
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

    SOPC_InternalNotificationElement* notifElt = malloc(sizeof(SOPC_InternalNotificationElement));
    OpcUa_WriteValue* wv = malloc(sizeof(OpcUa_WriteValue));
    SOPC_InternalNotificationElement* checkAdded = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    if (NULL == notifElt || NULL == wv)
    {
        free(notifElt);
        free(wv);
        return;
    }

    OpcUa_WriteValue_Initialize((void*) wv);
    wv->AttributeId = monitored_item_notification_queue_bs__p_writeValuePointer->AttributeId;
    retStatus =
        SOPC_String_Copy(&wv->IndexRange, &monitored_item_notification_queue_bs__p_writeValuePointer->IndexRange);
    if (SOPC_STATUS_OK == retStatus)
    {
        retStatus = SOPC_NodeId_Copy(&wv->NodeId, &monitored_item_notification_queue_bs__p_writeValuePointer->NodeId);
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        retStatus = SOPC_DataValue_Copy(&wv->Value, &monitored_item_notification_queue_bs__p_writeValuePointer->Value);
    }

    if (SOPC_STATUS_OK == retStatus)
    {
        notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
        notifElt->value = wv;
        if (notifElt->value->Value.SourceTimestamp == 0)
        {
            notifElt->value->Value.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        } // else use the source timestamp of the writeValue request
        notifElt->value->Value.ServerTimestamp = SOPC_Time_GetCurrentTimeUTC();

        /* Complying with timestamp to return configured */
        switch (monitored_item_notification_queue_bs__p_timestampToReturn)
        {
        case constants__e_ttr_source:
            wv->Value.ServerTimestamp = 0;
            break;
        case constants__e_ttr_server:
            wv->Value.SourceTimestamp = 0;
            break;
        case constants__e_ttr_neither:
            wv->Value.ServerTimestamp = 0;
            wv->Value.SourceTimestamp = 0;
            break;
        default:
            // Keep both in other cases
            break;
        }

        checkAdded = SOPC_SLinkedList_Append(monitored_item_notification_queue_bs__p_queue, 0, notifElt);
        if (checkAdded == notifElt)
        {
            *monitored_item_notification_queue_bs__bres = true;
        }
        else
        {
            retStatus = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != retStatus)
    {
        free(notifElt);
        free(wv);
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
    if (NULL != notifElt)
    {
        *monitored_item_notification_queue_bs__p_monitoredItem = notifElt->monitoredItemPointer;
        *monitored_item_notification_queue_bs__p_writeValuePointer = notifElt->value;
        free(notifElt);
        *monitored_item_notification_queue_bs__p_continue =
            SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) > 0;
    }
}

void monitored_item_notification_queue_bs__free_first_monitored_item_notification_value(
    const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer)
{
    free(monitored_item_notification_queue_bs__p_VariantValuePointer);
}

void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_entier4* const monitored_item_notification_queue_bs__p_nb_notifications)
{
    *monitored_item_notification_queue_bs__p_nb_notifications =
        (int32_t) SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue);
}
