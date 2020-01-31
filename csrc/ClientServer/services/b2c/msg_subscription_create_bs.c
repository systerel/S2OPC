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

#include "msg_subscription_create_bs.h"

#include "util_b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_create_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static void SOPC_InternalCommonCreateModifySubscription(
    double RequestedPublishingInterval,
    uint32_t RequestedLifetimeCount,
    uint32_t RequestedMaxKeepAliveCount,
    uint32_t MaxNotificationsPerPublish,
    constants__t_opcua_duration_i* const msg_subscription_create_bs__reqPublishInterval,
    t_entier4* const msg_subscription_create_bs__reqLifetimeCount,
    t_entier4* const msg_subscription_create_bs__reqMaxKeepAlive,
    t_entier4* const msg_subscription_create_bs__maxNotificationsPerPublish)
{
    *msg_subscription_create_bs__reqPublishInterval = RequestedPublishingInterval;

    if (RequestedLifetimeCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqLifetimeCount = (int32_t) RequestedLifetimeCount;
    }
    else
    {
        *msg_subscription_create_bs__reqLifetimeCount = INT32_MAX;
    }

    if (RequestedMaxKeepAliveCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = (int32_t) RequestedMaxKeepAliveCount;
    }
    else
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = INT32_MAX;
    }

    if (MaxNotificationsPerPublish <= INT32_MAX)
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = (int32_t) MaxNotificationsPerPublish;
    }
    else
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = INT32_MAX;
    }
}

void msg_subscription_create_bs__get_msg_create_subscription_req_params(
    const constants__t_msg_i msg_subscription_create_bs__p_req_msg,
    constants__t_opcua_duration_i* const msg_subscription_create_bs__reqPublishInterval,
    t_entier4* const msg_subscription_create_bs__reqLifetimeCount,
    t_entier4* const msg_subscription_create_bs__reqMaxKeepAlive,
    t_entier4* const msg_subscription_create_bs__maxNotificationsPerPublish,
    t_bool* const msg_subscription_create_bs__publishEnabled)
{
    OpcUa_CreateSubscriptionRequest* req = (OpcUa_CreateSubscriptionRequest*) msg_subscription_create_bs__p_req_msg;
    *msg_subscription_create_bs__publishEnabled = util_SOPC_Boolean_to_B(req->PublishingEnabled);

    SOPC_InternalCommonCreateModifySubscription(
        req->RequestedPublishingInterval, req->RequestedLifetimeCount, req->RequestedMaxKeepAliveCount,
        req->MaxNotificationsPerPublish, msg_subscription_create_bs__reqPublishInterval,
        msg_subscription_create_bs__reqLifetimeCount, msg_subscription_create_bs__reqMaxKeepAlive,
        msg_subscription_create_bs__maxNotificationsPerPublish);
}

void msg_subscription_create_bs__get_msg_modify_subscription_req_params(
    const constants__t_msg_i msg_subscription_create_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_create_bs__subscription,
    constants__t_opcua_duration_i* const msg_subscription_create_bs__reqPublishInterval,
    t_entier4* const msg_subscription_create_bs__reqLifetimeCount,
    t_entier4* const msg_subscription_create_bs__reqMaxKeepAlive,
    t_entier4* const msg_subscription_create_bs__maxNotificationsPerPublish)
{
    OpcUa_ModifySubscriptionRequest* req = (OpcUa_ModifySubscriptionRequest*) msg_subscription_create_bs__p_req_msg;
    *msg_subscription_create_bs__subscription = req->SubscriptionId;

    SOPC_InternalCommonCreateModifySubscription(
        req->RequestedPublishingInterval, req->RequestedLifetimeCount, req->RequestedMaxKeepAliveCount,
        req->MaxNotificationsPerPublish, msg_subscription_create_bs__reqPublishInterval,
        msg_subscription_create_bs__reqLifetimeCount, msg_subscription_create_bs__reqMaxKeepAlive,
        msg_subscription_create_bs__maxNotificationsPerPublish);
}

void msg_subscription_create_bs__set_msg_create_subscription_resp_params(
    const constants__t_msg_i msg_subscription_create_bs__p_resp_msg,
    const constants__t_subscription_i msg_subscription_create_bs__p_subscription,
    const constants__t_opcua_duration_i msg_subscription_create_bs__p_revisedPublishInterval,
    const t_entier4 msg_subscription_create_bs__p_revisedLifetimeCount,
    const t_entier4 msg_subscription_create_bs__p_revisedMaxKeepAlive)
{
    OpcUa_CreateSubscriptionResponse* resp = (OpcUa_CreateSubscriptionResponse*) msg_subscription_create_bs__p_resp_msg;
    resp->SubscriptionId = (uint32_t) msg_subscription_create_bs__p_subscription;
    resp->RevisedPublishingInterval = msg_subscription_create_bs__p_revisedPublishInterval;
    resp->RevisedLifetimeCount = (uint32_t) msg_subscription_create_bs__p_revisedLifetimeCount;
    resp->RevisedMaxKeepAliveCount = (uint32_t) msg_subscription_create_bs__p_revisedMaxKeepAlive;
}

void msg_subscription_create_bs__set_msg_modify_subscription_resp_params(
    const constants__t_msg_i msg_subscription_create_bs__p_resp_msg,
    const constants__t_opcua_duration_i msg_subscription_create_bs__p_revisedPublishInterval,
    const t_entier4 msg_subscription_create_bs__p_revisedLifetimeCount,
    const t_entier4 msg_subscription_create_bs__p_revisedMaxKeepAlive)
{
    OpcUa_ModifySubscriptionResponse* resp = (OpcUa_ModifySubscriptionResponse*) msg_subscription_create_bs__p_resp_msg;
    resp->RevisedPublishingInterval = msg_subscription_create_bs__p_revisedPublishInterval;
    resp->RevisedLifetimeCount = (uint32_t) msg_subscription_create_bs__p_revisedLifetimeCount;
    resp->RevisedMaxKeepAliveCount = (uint32_t) msg_subscription_create_bs__p_revisedMaxKeepAlive;
}
