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

#include "msg_subscription_publish_bs.h"

#include <inttypes.h>

#include "sopc_date_time.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_services_api_internal.h"
#include "sopc_types.h"
#include "util_b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_publish_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_publish_bs__alloc_notification_message_items(
    const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
    const t_entier4 msg_subscription_publish_bs__p_nb_data_notifications,
    const t_entier4 msg_subscription_publish_bs__p_nb_event_notifications,
    t_bool* const msg_subscription_publish_bs__bres,
    constants__t_notif_msg_i* const msg_subscription_publish_bs__p_notifMsg)
{
    *msg_subscription_publish_bs__bres = false;
    OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_publish_resp_msg;
    OpcUa_NotificationMessage* notifMsg = &pubResp->NotificationMessage;
    OpcUa_DataChangeNotification* dataChangeNotif = NULL;
    OpcUa_EventNotificationList* eventNotifList = NULL;

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if ((uint64_t) msg_subscription_publish_bs__p_nb_data_notifications <
            SIZE_MAX / sizeof(OpcUa_MonitoredItemNotification) &&
        (uint64_t) msg_subscription_publish_bs__p_nb_event_notifications < SIZE_MAX / sizeof(OpcUa_EventFieldList))
    {
        notifMsg->PublishTime = SOPC_Time_GetCurrentTimeUTC();
        notifMsg->NoOfNotificationData = 1;
        bool hasData = msg_subscription_publish_bs__p_nb_data_notifications > 0;
        bool hasEvent = msg_subscription_publish_bs__p_nb_event_notifications > 0;
        if (hasData && hasEvent)
        {
            // 1 for each type
            notifMsg->NoOfNotificationData = 2;
        }

        // Create the extension object
        notifMsg->NotificationData = SOPC_Calloc((size_t) notifMsg->NoOfNotificationData, sizeof(SOPC_ExtensionObject));

        bool dataToSet = hasData;
        bool eventToSet = hasEvent;

        if (NULL != notifMsg->NotificationData)
        {
            for (int32_t i = 0; i < notifMsg->NoOfNotificationData; i++)
            {
                SOPC_ExtensionObject* notifData = &notifMsg->NotificationData[i];

                SOPC_ExtensionObject_Initialize(notifData);

                if (dataToSet)
                {
                    // Create the notification data extension object
                    status = SOPC_ExtensionObject_CreateObject(notifData, &OpcUa_DataChangeNotification_EncodeableType,
                                                               (void**) &dataChangeNotif);
                    dataToSet = false;
                }
                else if (eventToSet)
                {
                    // Create the notification data extension object
                    status = SOPC_ExtensionObject_CreateObject(notifData, &OpcUa_EventNotificationList_EncodeableType,
                                                               (void**) &eventNotifList);
                    eventToSet = false;
                }
            }
        }
        else
        {
            notifMsg->NoOfNotificationData = 0;
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == status && hasData)
        {
            dataChangeNotif->NoOfMonitoredItems = msg_subscription_publish_bs__p_nb_data_notifications;
            dataChangeNotif->MonitoredItems =
                SOPC_Malloc((size_t) dataChangeNotif->NoOfMonitoredItems * sizeof(OpcUa_MonitoredItemNotification));
            if (NULL != dataChangeNotif->MonitoredItems)
            {
                for (int32_t i = 0; i < dataChangeNotif->NoOfMonitoredItems; i++)
                {
                    OpcUa_MonitoredItemNotification_Initialize(&dataChangeNotif->MonitoredItems[i]);
                }
            }
        }
        if (SOPC_STATUS_OK == status && hasEvent)
        {
            eventNotifList->NoOfEvents = msg_subscription_publish_bs__p_nb_event_notifications;
            eventNotifList->Events = SOPC_Malloc((size_t) eventNotifList->NoOfEvents * sizeof(OpcUa_EventFieldList));
            if (NULL != eventNotifList->Events)
            {
                for (int32_t i = 0; i < eventNotifList->NoOfEvents; i++)
                {
                    OpcUa_EventFieldList_Initialize(&eventNotifList->Events[i]);
                }
                *msg_subscription_publish_bs__p_notifMsg = notifMsg;
                *msg_subscription_publish_bs__bres = true;
            }
            else
            {
                SOPC_ExtensionObject_Clear(notifMsg->NotificationData);
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            *msg_subscription_publish_bs__p_notifMsg = notifMsg;
            *msg_subscription_publish_bs__bres = true;
        }
        else
        {
            if (SOPC_STATUS_OK != status)
            {
                for (int32_t i = 0; i < notifMsg->NoOfNotificationData; i++)
                {
                    SOPC_ExtensionObject_Clear(&notifMsg->NotificationData[i]);
                }
                SOPC_Free(notifMsg->NotificationData);
                notifMsg->NotificationData = NULL;
            }
        }
    }
}

void msg_subscription_publish_bs__get_notification_message_no_items(
    const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
    constants__t_notif_msg_i* const msg_subscription_publish_bs__p_notifMsg)
{
    OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_publish_resp_msg;
    *msg_subscription_publish_bs__p_notifMsg = &pubResp->NotificationMessage;
}

void msg_subscription_publish_bs__generate_internal_send_publish_response_event(
    const constants__t_session_i msg_subscription_publish_bs__p_session,
    const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
    const constants__t_server_request_handle_i msg_subscription_publish_bs__p_req_handle,
    const constants__t_request_context_i msg_subscription_publish_bs__p_req_context,
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_publish_bs__p_statusCode)
{
    SOPC_Internal_AsyncSendMsgData* eventData = SOPC_Malloc(sizeof(SOPC_Internal_AsyncSendMsgData));
    if (NULL != eventData)
    {
        eventData->msgToSend = msg_subscription_publish_bs__p_publish_resp_msg;
        eventData->requestHandle = msg_subscription_publish_bs__p_req_handle;
        eventData->requestId = msg_subscription_publish_bs__p_req_context;

        SOPC_EventHandler_PostAsNext(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO,
                                     (uint32_t) msg_subscription_publish_bs__p_session, (uintptr_t) eventData,
                                     (uintptr_t) msg_subscription_publish_bs__p_statusCode);
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "generate_internal_send_publish_response_event: out of memory error sending publish response "
            "session=%" PRIu32 ", requestId/Handle=%" PRIu32 "/%" PRIu32 "",
            msg_subscription_publish_bs__p_session, msg_subscription_publish_bs__p_req_context,
            msg_subscription_publish_bs__p_req_handle);
    }
}

void msg_subscription_publish_bs__set_msg_publish_resp_notificationMsg(
    const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
    const t_bool msg_subscription_publish_bs__p_moreNotifs)
{
    OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_resp_msg;
    pubResp->MoreNotifications = msg_subscription_publish_bs__p_moreNotifs;
}

void msg_subscription_publish_bs__set_msg_publish_resp_subscription(
    const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
    const constants__t_subscription_i msg_subscription_publish_bs__p_subscription)
{
    OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_resp_msg;
    pubResp->SubscriptionId = (uint32_t) msg_subscription_publish_bs__p_subscription;
}

void msg_subscription_publish_bs__set_notification_message_sequence_number(
    const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg,
    const constants__t_sub_seq_num_i msg_subscription_publish_bs__p_seq_num)
{
    msg_subscription_publish_bs__p_notifMsg->SequenceNumber = msg_subscription_publish_bs__p_seq_num;
}

void msg_subscription_publish_bs__set_publish_response_msg(
    const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg)
{
    /* Only to set variable in the B model */
    SOPC_UNUSED_ARG(msg_subscription_publish_bs__p_publish_resp_msg);
}

void msg_subscription_publish_bs__setall_notification_msg_monitored_item_data_notif(
    const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg,
    const t_entier4 msg_subscription_publish_bs__p_index,
    const constants__t_monitoredItemId_i msg_subscription_publish_bs__p_monitored_item_id,
    const constants__t_client_handle_i msg_subscription_publish_bs__p_clientHandle,
    const constants__t_WriteValuePointer_i msg_subscription_publish_bs__p_wv_pointer)
{
    SOPC_UNUSED_ARG(msg_subscription_publish_bs__p_monitored_item_id);
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == msg_subscription_publish_bs__p_notifMsg->NotificationData->Encoding);
    // DataChangeNotif is always first if present: see alloc_notification_message_items
    SOPC_ASSERT(&OpcUa_DataChangeNotification_EncodeableType ==
                msg_subscription_publish_bs__p_notifMsg->NotificationData[0].Body.Object.ObjType);

    OpcUa_DataChangeNotification* dataChangeNotif =
        (OpcUa_DataChangeNotification*) msg_subscription_publish_bs__p_notifMsg->NotificationData->Body.Object.Value;
    dataChangeNotif->MonitoredItems[msg_subscription_publish_bs__p_index - 1].ClientHandle =
        msg_subscription_publish_bs__p_clientHandle;
    SOPC_DataValue_Copy(&dataChangeNotif->MonitoredItems[msg_subscription_publish_bs__p_index - 1].Value,
                        &msg_subscription_publish_bs__p_wv_pointer->Value);

    OpcUa_WriteValue_Clear(msg_subscription_publish_bs__p_wv_pointer);
    SOPC_Free(msg_subscription_publish_bs__p_wv_pointer);
}

void msg_subscription_publish_bs__setall_notification_msg_monitored_item_event_notif(
    const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg,
    const t_entier4 msg_subscription_publish_bs__p_index,
    const constants__t_monitoredItemId_i msg_subscription_publish_bs__p_monitored_item_id,
    const constants__t_eventFieldList_i msg_subscription_publish_bs__p_event_field_list)
{
    SOPC_UNUSED_ARG(msg_subscription_publish_bs__p_monitored_item_id);
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == msg_subscription_publish_bs__p_notifMsg->NotificationData->Encoding);
    OpcUa_EventNotificationList* eventNotifList = NULL;
    if (&OpcUa_EventNotificationList_EncodeableType ==
        msg_subscription_publish_bs__p_notifMsg->NotificationData[0].Body.Object.ObjType)
    {
        eventNotifList = (OpcUa_EventNotificationList*) msg_subscription_publish_bs__p_notifMsg->NotificationData[0]
                             .Body.Object.Value;
    }
    else if (2 == msg_subscription_publish_bs__p_notifMsg->NoOfNotificationData &&
             &OpcUa_EventNotificationList_EncodeableType ==
                 msg_subscription_publish_bs__p_notifMsg->NotificationData[1].Body.Object.ObjType)
    {
        eventNotifList = (OpcUa_EventNotificationList*) msg_subscription_publish_bs__p_notifMsg->NotificationData[1]
                             .Body.Object.Value;
    }
    else
    {
        SOPC_ASSERT(false);
    }
    eventNotifList->Events[msg_subscription_publish_bs__p_index - 1] = *msg_subscription_publish_bs__p_event_field_list;
    SOPC_Free(msg_subscription_publish_bs__p_event_field_list);
}
