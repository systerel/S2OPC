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

#include "msg_subscription_publish_ack_bs.h"

#include "util_b2c.h"

#include <assert.h>

static const uint64_t SOPC_MILLISECOND_TO_100_NANOSECONDS = 10000; // 10^4

/* TODO: remove copy of this function already defined notification_republish_queue_bs */
static SOPC_ReturnStatus SOPC_InternalOpcUa_DataChangeNotification_Copy(OpcUa_DataChangeNotification* dest,
                                                                        OpcUa_DataChangeNotification* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    *dest = *src;
    if (src->NoOfDiagnosticInfos != 0)
    {
        dest->DiagnosticInfos = calloc((size_t) src->NoOfDiagnosticInfos, sizeof(SOPC_DiagnosticInfo));
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
    dest->NoOfDiagnosticInfos = src->NoOfDiagnosticInfos;
    if (src->NoOfMonitoredItems != 0)
    {
        dest->MonitoredItems = calloc((size_t) src->NoOfMonitoredItems, sizeof(OpcUa_MonitoredItemNotification));
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
    dest->NoOfMonitoredItems = src->NoOfMonitoredItems;

    return SOPC_STATUS_OK;
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_publish_ack_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_publish_ack_bs__allocate_subscription_ack_results(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
    const t_entier4 msg_subscription_publish_ack_bs__p_nb_acks,
    t_bool* const msg_subscription_publish_ack_bs__bres)
{
    OpcUa_PublishResponse* resp = (OpcUa_PublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    resp->NoOfResults = msg_subscription_publish_ack_bs__p_nb_acks;
    resp->Results = calloc((size_t) msg_subscription_publish_ack_bs__p_nb_acks, sizeof(SOPC_StatusCode));
    if (resp->Results != NULL)
    {
        *msg_subscription_publish_ack_bs__bres = true;
    }
    else
    {
        resp->NoOfResults = 0;
        *msg_subscription_publish_ack_bs__bres = false;
    }
}

void msg_subscription_publish_ack_bs__allocate_subscription_available_seq_nums(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
    const t_entier4 msg_subscription_publish_ack_bs__p_nb_seq_num,
    t_bool* const msg_subscription_publish_ack_bs__bres)
{
    OpcUa_PublishResponse* resp = (OpcUa_PublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    resp->NoOfAvailableSequenceNumbers = msg_subscription_publish_ack_bs__p_nb_seq_num;
    resp->AvailableSequenceNumbers = calloc((size_t) msg_subscription_publish_ack_bs__p_nb_seq_num, sizeof(uint32_t));
    if (resp->AvailableSequenceNumbers != NULL)
    {
        *msg_subscription_publish_ack_bs__bres = true;
    }
    else
    {
        resp->NoOfAvailableSequenceNumbers = 0;
        *msg_subscription_publish_ack_bs__bres = false;
    }
}
void msg_subscription_publish_ack_bs__get_msg_header_expiration_time(
    const constants__t_msg_header_i msg_subscription_publish_bs__p_req_header,
    constants__t_timeref_i* const msg_subscription_publish_bs__req_expiration_time)
{
    OpcUa_RequestHeader* pubReqHeader = msg_subscription_publish_bs__p_req_header;
    *msg_subscription_publish_bs__req_expiration_time = SOPC_TimeReference_GetCurrent();

    int64_t dtDelta = SOPC_Time_GetCurrentTimeUTC() - pubReqHeader->Timestamp;
    uint64_t millisecondsToTarget = 0;
    if (dtDelta > 0)
    {
        if (pubReqHeader->TimeoutHint >= (uint64_t) dtDelta / SOPC_MILLISECOND_TO_100_NANOSECONDS)
        {
            millisecondsToTarget = pubReqHeader->TimeoutHint - (uint64_t) dtDelta / SOPC_MILLISECOND_TO_100_NANOSECONDS;
        }
        else
        {
            // Already expired
            millisecondsToTarget = 0;
            // TODO: log ?
        }
    }
    else
    {
        // Keep only timeoutHint from current time
        millisecondsToTarget = pubReqHeader->TimeoutHint;
    }
    *msg_subscription_publish_bs__req_expiration_time =
        SOPC_TimeReference_AddMilliseconds(*msg_subscription_publish_bs__req_expiration_time, millisecondsToTarget);
}

void msg_subscription_publish_ack_bs__get_msg_publish_request_ack_params(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
    t_entier4* const msg_subscription_publish_ack_bs__p_nb_acks)
{
    OpcUa_PublishRequest* req = (OpcUa_PublishRequest*) msg_subscription_publish_ack_bs__p_req_msg;
    *msg_subscription_publish_ack_bs__p_nb_acks = req->NoOfSubscriptionAcknowledgements;
}

void msg_subscription_publish_ack_bs__getall_msg_publish_request_ack(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
    const t_entier4 msg_subscription_publish_ack_bs__p_index,
    constants__t_subscription_i* const msg_subscription_publish_ack_bs__p_sub,
    constants__t_sub_seq_num_i* const msg_subscription_publish_ack_bs__p_seq_num)
{
    OpcUa_PublishRequest* req = (OpcUa_PublishRequest*) msg_subscription_publish_ack_bs__p_req_msg;
    *msg_subscription_publish_ack_bs__p_sub =
        req->SubscriptionAcknowledgements[msg_subscription_publish_ack_bs__p_index - 1].SubscriptionId;
    *msg_subscription_publish_ack_bs__p_seq_num =
        req->SubscriptionAcknowledgements[msg_subscription_publish_ack_bs__p_index - 1].SequenceNumber;
}

void msg_subscription_publish_ack_bs__setall_msg_publish_resp_ack_result(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
    const t_entier4 msg_subscription_publish_ack_bs__p_index,
    const constants__t_StatusCode_i msg_subscription_publish_ack_bs__p_sc)
{
    OpcUa_PublishResponse* resp = (OpcUa_PublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    util_status_code__B_to_C(msg_subscription_publish_ack_bs__p_sc,
                             &resp->Results[msg_subscription_publish_ack_bs__p_index - 1]);
}

void msg_subscription_publish_ack_bs__setall_msg_publish_resp_available_seq_num(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
    const t_entier4 msg_subscription_publish_ack_bs__p_index,
    const constants__t_sub_seq_num_i msg_subscription_publish_ack_bs__p_seq_num)
{
    OpcUa_PublishResponse* resp = (OpcUa_PublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    resp->AvailableSequenceNumbers[msg_subscription_publish_ack_bs__p_index - 1] =
        msg_subscription_publish_ack_bs__p_seq_num;
}

void msg_subscription_publish_ack_bs__getall_msg_republish_request(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_publish_ack_bs__l_sub,
    constants__t_sub_seq_num_i* const msg_subscription_publish_ack_bs__l_seq_num)
{
    OpcUa_RepublishRequest* req = (OpcUa_RepublishRequest*) msg_subscription_publish_ack_bs__p_req_msg;
    *msg_subscription_publish_ack_bs__l_seq_num = req->RetransmitSequenceNumber;
    *msg_subscription_publish_ack_bs__l_sub = req->SubscriptionId;
}

void msg_subscription_publish_ack_bs__setall_msg_republish_response(
    const constants__t_msg_i msg_subscription_publish_ack_bs__p_resp_msg,
    const constants__t_notif_msg_i msg_subscription_publish_ack_bs__l_notifMsg)
{
    OpcUa_RepublishResponse* resp = (OpcUa_RepublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    resp->NotificationMessage = *msg_subscription_publish_ack_bs__l_notifMsg; /* Shallow copy */
    resp->NotificationMessage.NotificationData =
        malloc(1 * sizeof(SOPC_ExtensionObject)); /* Deep copy for notification data */
    if (resp->NotificationMessage.NotificationData == NULL)
    {
        assert(false); /* TODO: change model to return error in this case OR implement reference count to avoid copy */
        return;
    }
    SOPC_ExtensionObject_Initialize(resp->NotificationMessage.NotificationData);
    if (SOPC_ExtensionObject_Copy(resp->NotificationMessage.NotificationData,
                                  msg_subscription_publish_ack_bs__l_notifMsg->NotificationData) != SOPC_STATUS_OK)
    {
        /* TODO: remove assertion */
        assert(false);
        return;
    }
    else
    {
        /* TODO: deep copy not managed by SOPC_ExtensionObject_Copy, do it manually */
        if (SOPC_InternalOpcUa_DataChangeNotification_Copy(
                (OpcUa_DataChangeNotification*) resp->NotificationMessage.NotificationData->Body.Object.Value,
                (OpcUa_DataChangeNotification*)
                    msg_subscription_publish_ack_bs__l_notifMsg->NotificationData->Body.Object.Value) != SOPC_STATUS_OK)
        {
            assert(false);
            return;
        }
    }
}
