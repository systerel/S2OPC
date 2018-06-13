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

#include "msg_subscription_set_publishing_mode_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

#include "opcua_statuscodes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_set_publishing_mode_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_set_publishing_mode_bs__allocate_msg_subscription_set_publishing_mode_resp_results_array(
    const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_resp_msg,
    const t_entier4 msg_subscription_set_publishing_mode_bs__l_nb_subs,
    t_bool* const msg_subscription_set_publishing_mode_bs__bres)
{
    OpcUa_SetPublishingModeResponse* resp =
        (OpcUa_SetPublishingModeResponse*) msg_subscription_set_publishing_mode_bs__p_resp_msg;
    if (msg_subscription_set_publishing_mode_bs__l_nb_subs > 0 &&
        (uint64_t) msg_subscription_set_publishing_mode_bs__l_nb_subs <= SIZE_MAX)
    {
        resp->Results = calloc((size_t) msg_subscription_set_publishing_mode_bs__l_nb_subs, sizeof(SOPC_StatusCode));
    }
    if (NULL != resp->Results)
    {
        resp->NoOfResults = msg_subscription_set_publishing_mode_bs__l_nb_subs;
        *msg_subscription_set_publishing_mode_bs__bres = true;
    }
    else
    {
        *msg_subscription_set_publishing_mode_bs__bres = false;
    }
}

void msg_subscription_set_publishing_mode_bs__getall_msg_set_publishing_mode_at_index(
    const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_req_msg,
    const t_entier4 msg_subscription_set_publishing_mode_bs__p_index,
    constants__t_subscription_i* const msg_subscription_set_publishing_mode_bs__p_sub_id)
{
    OpcUa_SetPublishingModeRequest* req =
        (OpcUa_SetPublishingModeRequest*) msg_subscription_set_publishing_mode_bs__p_req_msg;
    *msg_subscription_set_publishing_mode_bs__p_sub_id =
        req->SubscriptionIds[msg_subscription_set_publishing_mode_bs__p_index - 1];
}

void msg_subscription_set_publishing_mode_bs__getall_msg_subscription_set_publishing_mode_params(
    const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_req_msg,
    t_entier4* const msg_subscription_set_publishing_mode_bs__p_nb_reqs,
    t_bool* const msg_subscription_set_publishing_mode_bs__p_pub_enabled)
{
    OpcUa_SetPublishingModeRequest* req =
        (OpcUa_SetPublishingModeRequest*) msg_subscription_set_publishing_mode_bs__p_req_msg;
    if (req->NoOfSubscriptionIds > 0)
    {
        *msg_subscription_set_publishing_mode_bs__p_nb_reqs = req->NoOfSubscriptionIds;
    }
    else
    {
        *msg_subscription_set_publishing_mode_bs__p_nb_reqs = 0;
    }
    *msg_subscription_set_publishing_mode_bs__p_pub_enabled = req->PublishingEnabled;
}

void msg_subscription_set_publishing_mode_bs__setall_msg_subscription_set_publishing_mode_resp_at_index(
    const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_resp_msg,
    const t_entier4 msg_subscription_set_publishing_mode_bs__p_index,
    const t_bool msg_subscription_set_publishing_mode_bs__p_valid_sub)
{
    OpcUa_SetPublishingModeResponse* resp =
        (OpcUa_SetPublishingModeResponse*) msg_subscription_set_publishing_mode_bs__p_resp_msg;
    if (msg_subscription_set_publishing_mode_bs__p_valid_sub)
    {
        resp->Results[msg_subscription_set_publishing_mode_bs__p_index - 1] = SOPC_GoodGenericStatus;
    }
    else
    {
        resp->Results[msg_subscription_set_publishing_mode_bs__p_index - 1] = OpcUa_BadSubscriptionIdInvalid;
    }
}

void msg_subscription_set_publishing_mode_bs__get_msg_subscription_set_publishing_mode_req_first_subscription(
    const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_req_msg,
    t_entier4* const msg_subscription_set_publishing_mode_bs__l_nb_subs,
    constants__t_subscription_i* const msg_subscription_set_publishing_mode_bs__p_first_sub_in_request,
    t_bool* const msg_subscription_set_publishing_mode_bs__p_pub_enabled)
{
    OpcUa_SetPublishingModeRequest* req =
        (OpcUa_SetPublishingModeRequest*) msg_subscription_set_publishing_mode_bs__p_req_msg;
    *msg_subscription_set_publishing_mode_bs__l_nb_subs = req->NoOfSubscriptionIds;
    if (req->NoOfSubscriptionIds > 0)
    {
        *msg_subscription_set_publishing_mode_bs__p_first_sub_in_request = req->SubscriptionIds[0];
        *msg_subscription_set_publishing_mode_bs__p_pub_enabled = req->PublishingEnabled;
    }
}
