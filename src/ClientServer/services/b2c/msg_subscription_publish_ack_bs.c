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

#include "msg_subscription_publish_ack_bs.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

static const uint64_t SOPC_YEAR_TO_MILLISECONDS = 31536000000; // 365 * 24 * 60 * 60 * 1000

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
    resp->Results = SOPC_Calloc((size_t) msg_subscription_publish_ack_bs__p_nb_acks, sizeof(SOPC_StatusCode));
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
    resp->AvailableSequenceNumbers =
        SOPC_Calloc((size_t) msg_subscription_publish_ack_bs__p_nb_seq_num, sizeof(uint32_t));
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

    uint64_t millisecondsToTarget = 0;
    if (0 == pubReqHeader->TimeoutHint)
    {
        // Request does not expire: set 1 year validity
        millisecondsToTarget = SOPC_YEAR_TO_MILLISECONDS;
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
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_publish_ack_bs__p_sc)
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
    const constants__t_notif_msg_i msg_subscription_publish_ack_bs__p_notifMsg,
    constants_statuscodes_bs__t_StatusCode_i* const msg_subscription_publish_ack_bs__sc)
{
    *msg_subscription_publish_ack_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    OpcUa_RepublishResponse* resp = (OpcUa_RepublishResponse*) msg_subscription_publish_ack_bs__p_resp_msg;
    resp->NotificationMessage = *msg_subscription_publish_ack_bs__p_notifMsg; /* Shallow copy */
    resp->NotificationMessage.NotificationData =
        SOPC_Malloc(1 * sizeof(SOPC_ExtensionObject)); /* Deep copy for notification data */
    if (resp->NotificationMessage.NotificationData == NULL)
    {
        return;
    }
    SOPC_ExtensionObject_Initialize(resp->NotificationMessage.NotificationData);
    if (SOPC_ExtensionObject_Copy(resp->NotificationMessage.NotificationData,
                                  msg_subscription_publish_ack_bs__p_notifMsg->NotificationData) != SOPC_STATUS_OK)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "msg_subscription_publish_ack_bs__setall_msg_republish_response: SOPC_ExtensionObject_Copy failure");
        return;
    }
    *msg_subscription_publish_ack_bs__sc = constants_statuscodes_bs__e_sc_ok;
}
