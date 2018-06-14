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
void msg_subscription_create_bs__get_msg_create_subscription_req_params(
    const constants__t_msg_i msg_subscription_create_bs__p_req_msg,
    constants__t_opcua_duration_i* const msg_subscription_create_bs__reqPublishInterval,
    t_entier4* const msg_subscription_create_bs__reqLifetimeCount,
    t_entier4* const msg_subscription_create_bs__reqMaxKeepAlive,
    t_entier4* const msg_subscription_create_bs__maxNotificationsPerPublish,
    t_bool* const msg_subscription_create_bs__publishEnabled)
{
    OpcUa_CreateSubscriptionRequest* req = (OpcUa_CreateSubscriptionRequest*) msg_subscription_create_bs__p_req_msg;
    *msg_subscription_create_bs__reqPublishInterval = req->RequestedPublishingInterval;

    if (req->RequestedLifetimeCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqLifetimeCount = (int32_t) req->RequestedLifetimeCount;
    }
    else
    {
        *msg_subscription_create_bs__reqLifetimeCount = INT32_MAX;
    }

    if (req->RequestedMaxKeepAliveCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = (int32_t) req->RequestedMaxKeepAliveCount;
    }
    else
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = INT32_MAX;
    }

    if (req->MaxNotificationsPerPublish <= INT32_MAX)
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = (int32_t) req->MaxNotificationsPerPublish;
    }
    else
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = INT32_MAX;
    }

    *msg_subscription_create_bs__publishEnabled = req->PublishingEnabled;
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
    *msg_subscription_create_bs__reqPublishInterval = req->RequestedPublishingInterval;

    *msg_subscription_create_bs__subscription = req->SubscriptionId;

    if (req->RequestedLifetimeCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqLifetimeCount = (int32_t) req->RequestedLifetimeCount;
    }
    else
    {
        *msg_subscription_create_bs__reqLifetimeCount = INT32_MAX;
    }

    if (req->RequestedMaxKeepAliveCount <= INT32_MAX)
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = (int32_t) req->RequestedMaxKeepAliveCount;
    }
    else
    {
        *msg_subscription_create_bs__reqMaxKeepAlive = INT32_MAX;
    }

    if (req->MaxNotificationsPerPublish <= INT32_MAX)
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = (int32_t) req->MaxNotificationsPerPublish;
    }
    else
    {
        *msg_subscription_create_bs__maxNotificationsPerPublish = INT32_MAX;
    }
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
