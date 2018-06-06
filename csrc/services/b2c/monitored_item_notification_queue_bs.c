/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue)
{
    SOPC_SLinkedList_Delete(monitored_item_notification_queue_bs__p_queue);
}

void monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
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
        wv->Value.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        wv->Value.ServerTimestamp = SOPC_Time_GetCurrentTimeUTC();
        retStatus = SOPC_NodeId_Copy(&wv->NodeId, monitored_item_notification_queue_bs__p_nid);
        if (SOPC_STATUS_OK == retStatus)
        {
            wv->Value.Value = *monitored_item_notification_queue_bs__p_VariantValuePointer;
            free(monitored_item_notification_queue_bs__p_VariantValuePointer);
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
}

void monitored_item_notification_queue_bs__add_monitored_item_notification_to_queue(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
    const constants__t_WriteValuePointer_i monitored_item_notification_queue_bs__p_writeValuePointer,
    t_bool* const monitored_item_notification_queue_bs__bres)
{
    *monitored_item_notification_queue_bs__bres = false;
    if (SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue) <
        INT32_MAX) // number of notifications returned in B model as a int32
    {
        SOPC_InternalNotificationElement* notifElt = malloc(sizeof(SOPC_InternalNotificationElement));
        SOPC_InternalNotificationElement* checkAdded = NULL;
        if (NULL != notifElt)
        {
            notifElt->monitoredItemPointer = monitored_item_notification_queue_bs__p_monitoredItem;
            notifElt->value = monitored_item_notification_queue_bs__p_writeValuePointer;
            if (notifElt->value->Value.SourceTimestamp == 0)
            {
                notifElt->value->Value.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
            }
            notifElt->value->Value.ServerTimestamp = SOPC_Time_GetCurrentTimeUTC();

            checkAdded = SOPC_SLinkedList_Append(monitored_item_notification_queue_bs__p_queue, 0, notifElt);
            if (checkAdded == notifElt)
            {
                *monitored_item_notification_queue_bs__bres = true;
            }
            else
            {
                free(notifElt);
                /* TODO: valid write values not represented in B model, if it is the case late deallocate in B model*/
                OpcUa_WriteValue_Clear(monitored_item_notification_queue_bs__p_writeValuePointer);
                free(monitored_item_notification_queue_bs__p_writeValuePointer);
            }
        }
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

void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
    const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
    t_entier4* const monitored_item_notification_queue_bs__p_nb_notifications)
{
    *monitored_item_notification_queue_bs__p_nb_notifications =
        (int32_t) SOPC_SLinkedList_GetLength(monitored_item_notification_queue_bs__p_queue);
}
