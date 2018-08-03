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

#include "notification_republish_queue_bs.h"
#include "notification_republish_queue_util.h"

#include <assert.h>

#include "sopc_logger.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void notification_republish_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
SOPC_ReturnStatus SOPC_InternalOpcUa_DataChangeNotification_Copy(OpcUa_DataChangeNotification* dest,
                                                                 OpcUa_DataChangeNotification* src)
{
    assert(dest != NULL);
    assert(src != NULL);

    dest->NoOfDiagnosticInfos = src->NoOfDiagnosticInfos;
    if (dest->NoOfDiagnosticInfos != 0)
    {
        dest->DiagnosticInfos = calloc((size_t) dest->NoOfDiagnosticInfos, sizeof(SOPC_DiagnosticInfo));
        if (dest->DiagnosticInfos == NULL)
        {
            return SOPC_STATUS_NOK;
        }
        for (int32_t i = 0; i < dest->NoOfDiagnosticInfos; i++)
        {
            SOPC_DiagnosticInfo_Initialize(&dest->DiagnosticInfos[i]);
            if (SOPC_DiagnosticInfo_Copy(&dest->DiagnosticInfos[i], &src->DiagnosticInfos[i]) != SOPC_STATUS_OK)
            {
                free(dest->DiagnosticInfos);
                dest->DiagnosticInfos = NULL;
                return SOPC_STATUS_NOK;
            }
        }
    }
    else
    {
        dest->DiagnosticInfos = NULL;
    }

    dest->NoOfMonitoredItems = src->NoOfMonitoredItems;
    if (dest->NoOfMonitoredItems != 0)
    {
        dest->MonitoredItems = calloc((size_t) dest->NoOfMonitoredItems, sizeof(OpcUa_MonitoredItemNotification));
        if (dest->MonitoredItems == NULL)
        {
            return SOPC_STATUS_NOK;
        }
        for (int32_t i = 0; i < dest->NoOfMonitoredItems; i++)
        {
            OpcUa_MonitoredItemNotification_Initialize(&dest->MonitoredItems[i]);
            if (SOPC_DataValue_Copy(&dest->MonitoredItems[i].Value, &src->MonitoredItems[i].Value) != SOPC_STATUS_OK)
            {
                free(dest->DiagnosticInfos);
                dest->DiagnosticInfos = NULL;

                free(dest->MonitoredItems);
                dest->MonitoredItems = NULL;
                return SOPC_STATUS_NOK;
            }
            dest->MonitoredItems[i].ClientHandle = src->MonitoredItems[i].ClientHandle;
        }
    }
    else
    {
        dest->MonitoredItems = NULL;
    }

    return SOPC_STATUS_OK;
}

void notification_republish_queue_bs__add_republish_notif_to_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
    const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
    const constants__t_notif_msg_i notification_republish_queue_bs__p_notif_msg,
    t_bool* const notification_republish_queue_bs__bres)
{
    assert(notification_republish_queue_bs__p_notif_msg->NoOfNotificationData == 1);
    *notification_republish_queue_bs__bres = false;
    OpcUa_NotificationMessage* notifMsgCpy = malloc(sizeof(OpcUa_NotificationMessage));
    if (notifMsgCpy == NULL)
    {
        return;
    }
    *notifMsgCpy = *notification_republish_queue_bs__p_notif_msg;             /* Shallow copy */
    notifMsgCpy->NotificationData = malloc(1 * sizeof(SOPC_ExtensionObject)); /* Deep copy for notification data */
    if (notifMsgCpy->NotificationData == NULL)
    {
        free(notifMsgCpy);
        return;
    }
    SOPC_ExtensionObject_Initialize(notifMsgCpy->NotificationData);
    if (SOPC_ExtensionObject_Copy(notifMsgCpy->NotificationData,
                                  notification_republish_queue_bs__p_notif_msg->NotificationData) != SOPC_STATUS_OK)
    {
        SOPC_Logger_TraceError(
            "notification_republish_queue_bs__add_republish_notif_to_queue: SOPC_ExtensionObject_Copy failure");
        return;
    }
    else
    {
        /* TODO: deep copy not managed by SOPC_ExtensionObject_Copy, do it manually */
        if (SOPC_InternalOpcUa_DataChangeNotification_Copy(
                (OpcUa_DataChangeNotification*) notifMsgCpy->NotificationData->Body.Object.Value,
                (OpcUa_DataChangeNotification*) notification_republish_queue_bs__p_notif_msg->NotificationData->Body
                    .Object.Value) != SOPC_STATUS_OK)
        {
            SOPC_Logger_TraceError(
                "notification_republish_queue_bs__add_republish_notif_to_queue: SOPC_ExtensionObject_Copy failure");
            return;
        }
    }
    void* res = SOPC_SLinkedList_Append(notification_republish_queue_bs__p_queue,
                                        notification_republish_queue_bs__p_seq_num, (void*) notifMsgCpy);
    if (res != notifMsgCpy)
    {
        OpcUa_NotificationMessage_Clear((void*) notifMsgCpy);
        free(notifMsgCpy);
    }
    else
    {
        *notification_republish_queue_bs__bres = true;
    }
}

void notification_republish_queue_bs__allocate_new_republish_queue(
    t_bool* const notification_republish_queue_bs__bres,
    constants__t_notifRepublishQueue_i* const notification_republish_queue_bs__queue)
{
    *notification_republish_queue_bs__bres = false;
    *notification_republish_queue_bs__queue = SOPC_SLinkedList_Create(0);
    if (*notification_republish_queue_bs__queue != NULL)
    {
        *notification_republish_queue_bs__bres = true;
    }
}

static void SOPC_InternalDeallocNotifMsg(uint32_t id, void* val)
{
    (void) id;
    OpcUa_NotificationMessage_Clear(val);
    free(val);
}

void notification_republish_queue_bs__clear_and_deallocate_republish_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(notification_republish_queue_bs__p_queue, SOPC_InternalDeallocNotifMsg);
    SOPC_SLinkedList_Delete(notification_republish_queue_bs__p_queue);
}

void notification_republish_queue_bs__clear_republish_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue)
{
    SOPC_SLinkedList_Apply(notification_republish_queue_bs__p_queue, SOPC_InternalDeallocNotifMsg);
    SOPC_SLinkedList_Clear(notification_republish_queue_bs__p_queue);
}

void notification_republish_queue_bs__discard_oldest_republish_notif(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue)
{
    void* removed = SOPC_SLinkedList_PopLast(notification_republish_queue_bs__p_queue);
    assert(NULL != removed); // Guarantee in precondition of B model
    SOPC_InternalDeallocNotifMsg(0, removed);
}

void notification_republish_queue_bs__get_nb_republish_notifs(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
    t_entier4* const notification_republish_queue_bs__nb_notifs)
{
    *notification_republish_queue_bs__nb_notifs =
        (int32_t) SOPC_SLinkedList_GetLength(notification_republish_queue_bs__p_queue);
}

void notification_republish_queue_bs__get_republish_notif_from_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
    const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
    t_bool* const notification_republish_queue_bs__bres,
    constants__t_notif_msg_i* const notification_republish_queue_bs__p_notif_msg)
{
    *notification_republish_queue_bs__bres = false;
    *notification_republish_queue_bs__p_notif_msg = SOPC_SLinkedList_FindFromId(
        notification_republish_queue_bs__p_queue, notification_republish_queue_bs__p_seq_num);
    if (*notification_republish_queue_bs__p_notif_msg != NULL)
    {
        *notification_republish_queue_bs__bres = true;
    }
}

void notification_republish_queue_bs__remove_republish_notif_from_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
    const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
    t_bool* const notification_republish_queue_bs__bres)
{
    *notification_republish_queue_bs__bres = false;
    OpcUa_NotificationMessage* notifMsg = (OpcUa_NotificationMessage*) SOPC_SLinkedList_RemoveFromId(
        notification_republish_queue_bs__p_queue, notification_republish_queue_bs__p_seq_num);
    if (notifMsg != NULL)
    {
        OpcUa_NotificationMessage_Clear(notifMsg);
        free(notifMsg);
        *notification_republish_queue_bs__bres = true;
    }
}
