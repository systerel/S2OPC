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

#include "msg_subscription_create_monitored_item_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

#include "util_b2c.h"

#include "sopc_types.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_create_monitored_item_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_create_monitored_item_bs__alloc_msg_create_monitored_items_resp_results(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_create_monitored_item_bs__p_nb_results,
    t_bool* const msg_subscription_create_monitored_item_bs__bres)
{
    *msg_subscription_create_monitored_item_bs__bres = false;
    OpcUa_CreateMonitoredItemsResponse* createResp =
        (OpcUa_CreateMonitoredItemsResponse*) msg_subscription_create_monitored_item_bs__p_resp_msg;
    if (msg_subscription_create_monitored_item_bs__p_nb_results > 0)
    {
        if (SIZE_MAX / (uint32_t) msg_subscription_create_monitored_item_bs__p_nb_results >
            sizeof(OpcUa_MonitoredItemCreateResult))
        {
            createResp->NoOfResults = msg_subscription_create_monitored_item_bs__p_nb_results;
            createResp->Results = calloc((size_t) msg_subscription_create_monitored_item_bs__p_nb_results,
                                         sizeof(OpcUa_MonitoredItemCreateResult));
            if (NULL != createResp->Results)
            {
                *msg_subscription_create_monitored_item_bs__bres = true;
            }
        }
    }
    else
    {
        createResp->NoOfResults = 0;
        *msg_subscription_create_monitored_item_bs__bres = true;
    }
}

void msg_subscription_create_monitored_item_bs__check_msg_create_monitored_items_req_not_null(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
    t_bool* const msg_subscription_create_monitored_item_bs__l_monitored_item_not_null)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_create_monitored_item_bs__p_req_msg;
    *msg_subscription_create_monitored_item_bs__l_monitored_item_not_null = createReq->ItemsToCreate != NULL;
}

void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_nb_monitored_items(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
    t_entier4* const msg_subscription_create_monitored_item_bs__p_nb_monitored_items)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_create_monitored_item_bs__p_req_msg;
    *msg_subscription_create_monitored_item_bs__p_nb_monitored_items = createReq->NoOfItemsToCreate;
}

void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_subscription(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_create_monitored_item_bs__p_subscription)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_create_monitored_item_bs__p_req_msg;
    if (createReq->SubscriptionId > 0 && createReq->SubscriptionId <= INT32_MAX)
    {
        *msg_subscription_create_monitored_item_bs__p_subscription = (int32_t) createReq->SubscriptionId;
    }
    else
    {
        *msg_subscription_create_monitored_item_bs__p_subscription = constants__c_subscription_indet;
    }
}

void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_timestamp_to_ret(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
    constants__t_TimestampsToReturn_i* const msg_subscription_create_monitored_item_bs__p_timestampToRet)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_create_monitored_item_bs__p_req_msg;
    bool res = util_TimestampsToReturn__C_to_B(createReq->TimestampsToReturn,
                                               msg_subscription_create_monitored_item_bs__p_timestampToRet);
    if (false == res)
    {
        *msg_subscription_create_monitored_item_bs__p_timestampToRet = constants__c_TimestampsToReturn_indet;
    }
}

void msg_subscription_create_monitored_item_bs__getall_monitored_item_req_params(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
    const constants__t_monitoredItemReqIndex_i msg_subscription_create_monitored_item_bs__p_index,
    t_bool* const msg_subscription_create_monitored_item_bs__p_bres,
    constants__t_NodeId_i* const msg_subscription_create_monitored_item_bs__p_nid,
    constants__t_AttributeId_i* const msg_subscription_create_monitored_item_bs__p_aid,
    constants__t_monitoringMode_i* const msg_subscription_create_monitored_item_bs__p_monitMode,
    constants__t_client_handle_i* const msg_subscription_create_monitored_item_bs__p_clientHandle,
    constants__t_opcua_duration_i* const msg_subscription_create_monitored_item_bs__p_samplingItv,
    t_entier4* const msg_subscription_create_monitored_item_bs__p_queueSize)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_create_monitored_item_bs__p_req_msg;
    OpcUa_MonitoredItemCreateRequest* monitReq =
        &createReq->ItemsToCreate[msg_subscription_create_monitored_item_bs__p_index];
    *msg_subscription_create_monitored_item_bs__p_nid = &monitReq->ItemToMonitor.NodeId;

    *msg_subscription_create_monitored_item_bs__p_bres =
        util_AttributeId__C_to_B(monitReq->ItemToMonitor.AttributeId, msg_subscription_create_monitored_item_bs__p_aid);
    if (false == *msg_subscription_create_monitored_item_bs__p_bres)
    {
        *msg_subscription_create_monitored_item_bs__p_aid = constants__c_AttributeId_indet;
    }

    switch (monitReq->MonitoringMode)
    {
    case OpcUa_MonitoringMode_Disabled:
        *msg_subscription_create_monitored_item_bs__p_monitMode = constants__e_monitoringMode_disabled;
        break;
    case OpcUa_MonitoringMode_Sampling:
        *msg_subscription_create_monitored_item_bs__p_monitMode = constants__e_monitoringMode_sampling;
        break;
    case OpcUa_MonitoringMode_Reporting:
        *msg_subscription_create_monitored_item_bs__p_monitMode = constants__e_monitoringMode_reporting;
        break;
    default:
        *msg_subscription_create_monitored_item_bs__p_bres = false;
        *msg_subscription_create_monitored_item_bs__p_monitMode = constants__c_monitoringMode_indet;
    }
    *msg_subscription_create_monitored_item_bs__p_clientHandle = monitReq->RequestedParameters.ClientHandle;
    *msg_subscription_create_monitored_item_bs__p_samplingItv = monitReq->RequestedParameters.SamplingInterval;
    if (monitReq->RequestedParameters.QueueSize <= INT32_MAX)
    {
        *msg_subscription_create_monitored_item_bs__p_queueSize = (int32_t) monitReq->RequestedParameters.QueueSize;
    }
    else
    {
        *msg_subscription_create_monitored_item_bs__p_queueSize = INT32_MAX;
    }
}

void msg_subscription_create_monitored_item_bs__setall_msg_monitored_item_resp_params(
    const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_resp_msg,
    const constants__t_monitoredItemReqIndex_i msg_subscription_create_monitored_item_bs__p_index,
    const constants__t_StatusCode_i msg_subscription_create_monitored_item_bs__p_sc,
    const constants__t_monitoredItem_i msg_subscription_create_monitored_item_bs__p_monitored_item,
    const constants__t_opcua_duration_i msg_subscription_create_monitored_item_bs__p_revSamplingItv,
    const t_entier4 msg_subscription_create_monitored_item_bs__p_revQueueSize)
{
    OpcUa_CreateMonitoredItemsResponse* createResp =
        (OpcUa_CreateMonitoredItemsResponse*) msg_subscription_create_monitored_item_bs__p_resp_msg;
    OpcUa_MonitoredItemCreateResult* monitResp =
        &createResp->Results[msg_subscription_create_monitored_item_bs__p_index];
    util_status_code__B_to_C(msg_subscription_create_monitored_item_bs__p_sc, &monitResp->StatusCode);
    monitResp->MonitoredItemId = (uint32_t) msg_subscription_create_monitored_item_bs__p_monitored_item;
    monitResp->RevisedSamplingInterval = msg_subscription_create_monitored_item_bs__p_revSamplingItv;
    monitResp->RevisedQueueSize = (uint32_t) msg_subscription_create_monitored_item_bs__p_revQueueSize;
}
