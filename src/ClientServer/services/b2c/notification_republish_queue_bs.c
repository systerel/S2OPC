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

#include <assert.h>

#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void notification_republish_queue_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static bool notification_message_copy(OpcUa_NotificationMessage* dst, const OpcUa_NotificationMessage* src)
{
    assert(dst != NULL && src != NULL);
    assert(src->NoOfNotificationData == 1); // Restricted support for now

    SOPC_ExtensionObject* notification_data = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));

    if (notification_data == NULL)
    {
        return false;
    }

    SOPC_ExtensionObject_Initialize(notification_data);

    if (SOPC_ExtensionObject_Copy(notification_data, src->NotificationData) != SOPC_STATUS_OK)
    {
        SOPC_ExtensionObject_Clear(notification_data);
        SOPC_Free(notification_data);
        return false;
    }

    OpcUa_NotificationMessage_Initialize(dst);
    dst->NotificationData = notification_data;
    dst->NoOfNotificationData = 1;
    dst->PublishTime = src->PublishTime;
    dst->SequenceNumber = src->SequenceNumber;

    return true;
}

void notification_republish_queue_bs__add_republish_notif_to_queue(
    const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
    const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
    const constants__t_notif_msg_i notification_republish_queue_bs__p_notif_msg,
    t_bool* const notification_republish_queue_bs__bres)
{
    assert(notification_republish_queue_bs__p_notif_msg->NoOfNotificationData == 1);
    *notification_republish_queue_bs__bres = false;
    OpcUa_NotificationMessage* notifMsgCpy = SOPC_Malloc(sizeof(OpcUa_NotificationMessage));
    if (notifMsgCpy == NULL || !notification_message_copy(notifMsgCpy, notification_republish_queue_bs__p_notif_msg))
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "notification_republish_queue_bs__add_republish_notif_to_queue: Error while copying notification message");
        SOPC_Free(notifMsgCpy);
        return;
    }
    void* res = SOPC_SLinkedList_Append(notification_republish_queue_bs__p_queue,
                                        notification_republish_queue_bs__p_seq_num, (void*) notifMsgCpy);
    if (res != notifMsgCpy)
    {
        OpcUa_NotificationMessage_Clear((void*) notifMsgCpy);
        SOPC_Free(notifMsgCpy);
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
    SOPC_UNUSED_ARG(id);
    OpcUa_NotificationMessage_Clear(val);
    SOPC_Free(val);
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
        SOPC_Free(notifMsg);
        *notification_republish_queue_bs__bres = true;
    }
}
