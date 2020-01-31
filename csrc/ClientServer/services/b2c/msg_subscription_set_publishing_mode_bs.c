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

#include "msg_subscription_set_publishing_mode_bs.h"

#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

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
    if (msg_subscription_set_publishing_mode_bs__l_nb_subs > 0)
    {
        resp->Results =
            SOPC_Calloc((size_t) msg_subscription_set_publishing_mode_bs__l_nb_subs, sizeof(SOPC_StatusCode));
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
    *msg_subscription_set_publishing_mode_bs__p_pub_enabled = util_SOPC_Boolean_to_B(req->PublishingEnabled);
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
