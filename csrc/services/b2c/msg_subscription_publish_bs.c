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

#include "msg_subscription_publish_bs.h"

#include "util_b2c.h"

#include "sopc_services_api_internal.h"
#include "sopc_types.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

static const uint64_t SOPC_MILLISECOND_TO_100_NANOSECONDS = 10000; // 10^4

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_publish_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_publish_bs__generate_internal_send_publish_response_event(
    const constants__t_session_i msg_subscription_publish_bs__p_session,
    const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
    const constants__t_server_request_handle_i msg_subscription_publish_bs__p_req_handle,
    const constants__t_request_context_i msg_subscription_publish_bs__p_req_context,
    const constants__t_StatusCode_i msg_subscription_publish_bs__p_statusCode)
{
    SOPC_Internal_AsyncSendMsgData* eventData = malloc(sizeof(SOPC_Internal_AsyncSendMsgData));
    if (NULL != eventData)
    {
        eventData->msgToSend = msg_subscription_publish_bs__p_publish_resp_msg;
        eventData->requestHandle = msg_subscription_publish_bs__p_req_handle;
        eventData->requestId = msg_subscription_publish_bs__p_req_context;

        SOPC_Services_InternalEnqueuePrioEvent(SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO,
                                               (uint32_t) msg_subscription_publish_bs__p_session, eventData,
                                               (uintptr_t) msg_subscription_publish_bs__p_statusCode);
    }
    else
    {
        // TODO: log ?
    }
}

void msg_subscription_publish_bs__get_msg_header_expiration_time(
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

void msg_subscription_publish_bs__set_msg_publish_resp_notificationMsg(
    const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
    const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg)
{
    if (msg_subscription_publish_bs__p_notifMsg != constants__c_notif_msg_indet)
    {
        OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_resp_msg;
        pubResp->NotificationMessage = *msg_subscription_publish_bs__p_notifMsg;
        free(msg_subscription_publish_bs__p_notifMsg);
    }
}

void msg_subscription_publish_bs__set_msg_publish_resp_subscription(
    const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
    const constants__t_subscription_i msg_subscription_publish_bs__p_subscription)
{
    OpcUa_PublishResponse* pubResp = (OpcUa_PublishResponse*) msg_subscription_publish_bs__p_resp_msg;
    pubResp->SubscriptionId = (uint32_t) msg_subscription_publish_bs__p_subscription;
}
