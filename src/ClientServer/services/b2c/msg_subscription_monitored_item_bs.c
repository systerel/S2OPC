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

#include "msg_subscription_monitored_item_bs.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

#include "util_b2c.h"

#include "sopc_logger.h"
#include "sopc_types.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_monitored_item_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_monitored_item_bs__alloc_msg_create_monitored_items_resp_results(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_nb_results,
    t_bool* const msg_subscription_monitored_item_bs__bres)
{
    *msg_subscription_monitored_item_bs__bres = false;
    OpcUa_CreateMonitoredItemsResponse* createResp =
        (OpcUa_CreateMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    if (msg_subscription_monitored_item_bs__p_nb_results > 0)
    {
        if (SIZE_MAX / (uint32_t) msg_subscription_monitored_item_bs__p_nb_results >
            sizeof(OpcUa_MonitoredItemCreateResult))
        {
            createResp->NoOfResults = msg_subscription_monitored_item_bs__p_nb_results;
            createResp->Results = SOPC_Calloc((size_t) msg_subscription_monitored_item_bs__p_nb_results,
                                              sizeof(OpcUa_MonitoredItemCreateResult));
            if (NULL != createResp->Results)
            {
                for (int32_t i = 0; i < createResp->NoOfResults; i++)
                {
                    OpcUa_MonitoredItemCreateResult_Initialize(&createResp->Results[i]);
                }
                *msg_subscription_monitored_item_bs__bres = true;
            }
        }
    }
    else
    {
        createResp->NoOfResults = 0;
        *msg_subscription_monitored_item_bs__bres = true;
    }
}

void msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_nb_monitored_items(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    t_entier4* const msg_subscription_monitored_item_bs__p_nb_monitored_items)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_nb_monitored_items = createReq->NoOfItemsToCreate;
}

void msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_subscription(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_monitored_item_bs__p_subscription)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    if (createReq->SubscriptionId > 0 && createReq->SubscriptionId <= INT32_MAX)
    {
        *msg_subscription_monitored_item_bs__p_subscription = createReq->SubscriptionId;
    }
    else
    {
        *msg_subscription_monitored_item_bs__p_subscription = constants__c_subscription_indet;
    }
}

void msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_timestamp_to_ret(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_TimestampsToReturn_i* const msg_subscription_monitored_item_bs__p_timestampToRet)
{
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_timestampToRet =
        util_TimestampsToReturn__C_to_B(createReq->TimestampsToReturn);
}

static bool check_monitored_item_datachange_filter_param(SOPC_ExtensionObject* filter,
                                                         SOPC_AttributeId attributeId,
                                                         constants_statuscodes_bs__t_StatusCode_i* sc)
{
    assert(NULL != filter);
    assert(NULL != sc);

    if (filter->Length > 0)
    {
        if (attributeId != SOPC_AttributeId_Value)
        {
            *sc = constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
        }
        else if (SOPC_ExtObjBodyEncoding_Object != filter->Encoding)
        {
            // Filter was not decoded as an object => unknown type
            *sc = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
        }
        else if (&OpcUa_AggregateFilter_EncodeableType == filter->Body.Object.ObjType ||
                 &OpcUa_EventFilter_EncodeableType == filter->Body.Object.ObjType)
        {
            // AggregateFilter or EventFilter are not supported
            *sc = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_unsupported;
        }
        else if (&OpcUa_DataChangeFilter_EncodeableType != filter->Body.Object.ObjType)
        {
            // Filter type is unexpected
            *sc = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
        }
        else
        {
            // Note: DataChangeFilter does not have result and thus does not provide revised value in response
            OpcUa_DataChangeFilter* dcf = filter->Body.Object.Value;
            bool validFilter = true;
            bool validPercent = true;
            bool supportedFilter = true;
            switch (dcf->Trigger)
            {
            case OpcUa_DataChangeTrigger_Status:
            case OpcUa_DataChangeTrigger_StatusValue:
            case OpcUa_DataChangeTrigger_StatusValueTimestamp:
                validFilter = true;
                break;
            default:
                validFilter = false;
                break;
            }
            switch (dcf->DeadbandType)
            {
            case OpcUa_DeadbandType_None:
            case OpcUa_DeadbandType_Absolute:
                // Check deadband value is positive (absolute value expected)
                if (dcf->DeadbandValue < 0.0)
                {
                    validFilter = false;
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Failed to create a MI filter with an absolute deadband value negative: %lf",
                                           dcf->DeadbandValue);
                }
                break;
            case OpcUa_DeadbandType_Percent: // Not supported for now
                // Check deadband value is between 0.0 and 100.0 (percent expected)
                if (dcf->DeadbandValue < 0.0 || dcf->DeadbandValue > 100.0)
                {
                    validPercent = false;
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "Failed to create a MI filter with an percent deadband value out of range [0.0, 100.0] : %lf",
                        dcf->DeadbandValue);
                }
                break;
            default:
                supportedFilter = false;
                break;
            }
            if (!validPercent)
            {
                *sc = constants_statuscodes_bs__e_sc_bad_deadband_filter_invalid;
            }
            else if (!validFilter)
            {
                *sc = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
            }
            else if (!supportedFilter)
            {
                *sc = constants_statuscodes_bs__e_sc_bad_monitored_item_filter_unsupported;
            }
            else
            {
                return true;
            }
        }
    }
    else
    {
        return true;
    }

    return false;
}

void msg_subscription_monitored_item_bs__getall_create_monitored_item_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    t_bool* const msg_subscription_monitored_item_bs__p_bres,
    constants_statuscodes_bs__t_StatusCode_i* const msg_subscription_monitored_item_bs__p_sc,
    constants__t_NodeId_i* const msg_subscription_monitored_item_bs__p_nid,
    constants__t_AttributeId_i* const msg_subscription_monitored_item_bs__p_aid,
    constants__t_monitoringMode_i* const msg_subscription_monitored_item_bs__p_monitMode,
    constants__t_client_handle_i* const msg_subscription_monitored_item_bs__p_clientHandle,
    constants__t_opcua_duration_i* const msg_subscription_monitored_item_bs__p_samplingItv,
    constants__t_monitoringFilter_i* const msg_subscription_monitored_item_bs__p_filter,
    t_bool* const msg_subscription_monitored_item_bs__p_discardOldest,
    t_entier4* const msg_subscription_monitored_item_bs__p_queueSize,
    constants__t_IndexRange_i* const msg_subscription_monitored_item_bs__p_indexRange)
{
    *msg_subscription_monitored_item_bs__p_aid = constants__c_AttributeId_indet;
    *msg_subscription_monitored_item_bs__p_monitMode = constants__c_monitoringMode_indet;
    *msg_subscription_monitored_item_bs__p_clientHandle = 0;
    *msg_subscription_monitored_item_bs__p_filter = constants__c_monitoringFilter_indet;
    *msg_subscription_monitored_item_bs__p_samplingItv = 0;
    *msg_subscription_monitored_item_bs__p_queueSize = 0;
    OpcUa_CreateMonitoredItemsRequest* createReq =
        (OpcUa_CreateMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    OpcUa_MonitoredItemCreateRequest* monitReq =
        &createReq->ItemsToCreate[msg_subscription_monitored_item_bs__p_index - 1];
    *msg_subscription_monitored_item_bs__p_nid = &monitReq->ItemToMonitor.NodeId;

    *msg_subscription_monitored_item_bs__p_aid = util_AttributeId__C_to_B(monitReq->ItemToMonitor.AttributeId);
    *msg_subscription_monitored_item_bs__p_bres =
        *msg_subscription_monitored_item_bs__p_aid != constants__c_AttributeId_indet;

    *msg_subscription_monitored_item_bs__p_sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;

    if (*msg_subscription_monitored_item_bs__p_bres)
    {
        if (monitReq->ItemToMonitor.IndexRange.Length > 0)
        {
            *msg_subscription_monitored_item_bs__p_indexRange = &monitReq->ItemToMonitor.IndexRange;
        }
        else
        {
            *msg_subscription_monitored_item_bs__p_indexRange = NULL;
        }
    }

    if (*msg_subscription_monitored_item_bs__p_bres)
    {
        switch (monitReq->MonitoringMode)
        {
        case OpcUa_MonitoringMode_Disabled:
            *msg_subscription_monitored_item_bs__p_monitMode = constants__e_monitoringMode_disabled;
            break;
        case OpcUa_MonitoringMode_Sampling:
            *msg_subscription_monitored_item_bs__p_monitMode = constants__e_monitoringMode_sampling;
            break;
        case OpcUa_MonitoringMode_Reporting:
            *msg_subscription_monitored_item_bs__p_monitMode = constants__e_monitoringMode_reporting;
            break;
        default:
            *msg_subscription_monitored_item_bs__p_bres = false;
            *msg_subscription_monitored_item_bs__p_monitMode = constants__c_monitoringMode_indet;
            *msg_subscription_monitored_item_bs__p_sc = constants_statuscodes_bs__e_sc_bad_monitoring_mode_invalid;
        }
    }

    if (*msg_subscription_monitored_item_bs__p_bres)
    {
        *msg_subscription_monitored_item_bs__p_clientHandle = monitReq->RequestedParameters.ClientHandle;
        *msg_subscription_monitored_item_bs__p_samplingItv = monitReq->RequestedParameters.SamplingInterval;
        *msg_subscription_monitored_item_bs__p_discardOldest = monitReq->RequestedParameters.DiscardOldest;

        if (monitReq->RequestedParameters.QueueSize <= INT32_MAX)
        {
            *msg_subscription_monitored_item_bs__p_queueSize = (int32_t) monitReq->RequestedParameters.QueueSize;
        }
        else
        {
            *msg_subscription_monitored_item_bs__p_queueSize = INT32_MAX;
        }

        *msg_subscription_monitored_item_bs__p_bres = check_monitored_item_datachange_filter_param(
            &monitReq->RequestedParameters.Filter, monitReq->ItemToMonitor.AttributeId,
            msg_subscription_monitored_item_bs__p_sc);

        if (*msg_subscription_monitored_item_bs__p_bres)
        {
            *msg_subscription_monitored_item_bs__p_filter =
                (OpcUa_DataChangeFilter*) monitReq->RequestedParameters.Filter.Body.Object.Value;
        }
    }

    if (*msg_subscription_monitored_item_bs__p_bres)
    {
        *msg_subscription_monitored_item_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
    }
}

void msg_subscription_monitored_item_bs__setall_msg_create_monitored_item_resp_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_monitored_item_bs__p_sc,
    const constants__t_monitoredItemId_i msg_subscription_monitored_item_bs__p_monitored_item_id,
    const constants__t_opcua_duration_i msg_subscription_monitored_item_bs__p_revSamplingItv,
    const t_entier4 msg_subscription_monitored_item_bs__p_revQueueSize)
{
    OpcUa_CreateMonitoredItemsResponse* createResp =
        (OpcUa_CreateMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    OpcUa_MonitoredItemCreateResult* monitResp = &createResp->Results[msg_subscription_monitored_item_bs__p_index - 1];
    util_status_code__B_to_C(msg_subscription_monitored_item_bs__p_sc, &monitResp->StatusCode);
    monitResp->MonitoredItemId = msg_subscription_monitored_item_bs__p_monitored_item_id;
    monitResp->RevisedSamplingInterval = msg_subscription_monitored_item_bs__p_revSamplingItv;
    monitResp->RevisedQueueSize = (uint32_t) msg_subscription_monitored_item_bs__p_revQueueSize;
}

void msg_subscription_monitored_item_bs__alloc_msg_modify_monitored_items_resp_results(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_nb_results,
    t_bool* const msg_subscription_monitored_item_bs__bres)
{
    *msg_subscription_monitored_item_bs__bres = false;
    OpcUa_ModifyMonitoredItemsResponse* modifyResp =
        (OpcUa_ModifyMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    if (msg_subscription_monitored_item_bs__p_nb_results > 0)
    {
        if (SIZE_MAX / (uint32_t) msg_subscription_monitored_item_bs__p_nb_results > sizeof(*modifyResp->Results))
        {
            modifyResp->NoOfResults = msg_subscription_monitored_item_bs__p_nb_results;
            modifyResp->Results =
                SOPC_Calloc((size_t) msg_subscription_monitored_item_bs__p_nb_results, sizeof(*modifyResp->Results));
            if (NULL != modifyResp->Results)
            {
                for (int32_t i = 0; i < modifyResp->NoOfResults; i++)
                {
                    OpcUa_MonitoredItemModifyResult_Initialize(&modifyResp->Results[i]);
                }
                *msg_subscription_monitored_item_bs__bres = true;
            }
        }
    }
    else
    {
        modifyResp->NoOfResults = 0;
        *msg_subscription_monitored_item_bs__bres = true;
    }
}

void msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_nb_monitored_items(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    t_entier4* const msg_subscription_monitored_item_bs__p_nb_monitored_items)
{
    OpcUa_ModifyMonitoredItemsRequest* modifyReq =
        (OpcUa_ModifyMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_nb_monitored_items = modifyReq->NoOfItemsToModify;
}

void msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_subscription(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_monitored_item_bs__p_subscription)
{
    OpcUa_ModifyMonitoredItemsRequest* modifyReq =
        (OpcUa_ModifyMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    if (modifyReq->SubscriptionId > 0 && modifyReq->SubscriptionId <= INT32_MAX)
    {
        *msg_subscription_monitored_item_bs__p_subscription = modifyReq->SubscriptionId;
    }
    else
    {
        *msg_subscription_monitored_item_bs__p_subscription = constants__c_subscription_indet;
    }
}

void msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_timestamp_to_ret(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_TimestampsToReturn_i* const msg_subscription_monitored_item_bs__p_timestampToRet)
{
    OpcUa_ModifyMonitoredItemsRequest* modifyReq =
        (OpcUa_ModifyMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_timestampToRet =
        util_TimestampsToReturn__C_to_B(modifyReq->TimestampsToReturn);
}

void msg_subscription_monitored_item_bs__getall_modify_monitored_item_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    t_bool* const msg_subscription_monitored_item_bs__p_bres,
    constants_statuscodes_bs__t_StatusCode_i* const msg_subscription_monitored_item_bs__p_sc,
    constants__t_monitoredItemId_i* const msg_subscription_monitored_item_bs__p_monitored_item_id,
    constants__t_client_handle_i* const msg_subscription_monitored_item_bs__p_clientHandle,
    constants__t_opcua_duration_i* const msg_subscription_monitored_item_bs__p_samplingItv,
    constants__t_monitoringFilter_i* const msg_subscription_monitored_item_bs__p_filter,
    t_bool* const msg_subscription_monitored_item_bs__p_discardOldest,
    t_entier4* const msg_subscription_monitored_item_bs__p_queueSize)
{
    *msg_subscription_monitored_item_bs__p_clientHandle = 0;
    *msg_subscription_monitored_item_bs__p_samplingItv = 0;
    *msg_subscription_monitored_item_bs__p_queueSize = 0;
    OpcUa_ModifyMonitoredItemsRequest* modifyReq =
        (OpcUa_ModifyMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    OpcUa_MonitoredItemModifyRequest* monitReq =
        &modifyReq->ItemsToModify[msg_subscription_monitored_item_bs__p_index - 1];

    // Note: we have to consider attribute is a Value attribute since we have no access to information here.
    //       Its validity is checked later.
    *msg_subscription_monitored_item_bs__p_bres = check_monitored_item_datachange_filter_param(
        &monitReq->RequestedParameters.Filter, SOPC_AttributeId_Value, msg_subscription_monitored_item_bs__p_sc);

    // Check active filter is valid type
    if (*msg_subscription_monitored_item_bs__p_bres)
    {
        *msg_subscription_monitored_item_bs__p_sc = constants_statuscodes_bs__e_sc_ok;

        *msg_subscription_monitored_item_bs__p_filter =
            (OpcUa_DataChangeFilter*) monitReq->RequestedParameters.Filter.Body.Object.Value;

        *msg_subscription_monitored_item_bs__p_monitored_item_id = monitReq->MonitoredItemId;
        *msg_subscription_monitored_item_bs__p_clientHandle = monitReq->RequestedParameters.ClientHandle;
        *msg_subscription_monitored_item_bs__p_samplingItv = monitReq->RequestedParameters.SamplingInterval;
        *msg_subscription_monitored_item_bs__p_discardOldest = monitReq->RequestedParameters.DiscardOldest;

        if (monitReq->RequestedParameters.QueueSize <= INT32_MAX)
        {
            *msg_subscription_monitored_item_bs__p_queueSize = (int32_t) monitReq->RequestedParameters.QueueSize;
        }
        else
        {
            *msg_subscription_monitored_item_bs__p_queueSize = INT32_MAX;
        }
    }
}

void msg_subscription_monitored_item_bs__setall_msg_modify_monitored_item_resp_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_monitored_item_bs__p_sc,
    const constants__t_opcua_duration_i msg_subscription_monitored_item_bs__p_revSamplingItv,
    const t_entier4 msg_subscription_monitored_item_bs__p_revQueueSize)
{
    OpcUa_ModifyMonitoredItemsResponse* modifyResp =
        (OpcUa_ModifyMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    OpcUa_MonitoredItemModifyResult* monitResp = &modifyResp->Results[msg_subscription_monitored_item_bs__p_index - 1];
    util_status_code__B_to_C(msg_subscription_monitored_item_bs__p_sc, &monitResp->StatusCode);
    monitResp->RevisedSamplingInterval = msg_subscription_monitored_item_bs__p_revSamplingItv;
    monitResp->RevisedQueueSize = (uint32_t) msg_subscription_monitored_item_bs__p_revQueueSize;
}

void msg_subscription_monitored_item_bs__alloc_msg_delete_monitored_items_resp_results(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_nb_results,
    t_bool* const msg_subscription_monitored_item_bs__bres)
{
    assert(msg_subscription_monitored_item_bs__p_nb_results > 0);
    *msg_subscription_monitored_item_bs__bres = false;
    OpcUa_DeleteMonitoredItemsResponse* deleteResp =
        (OpcUa_DeleteMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    if (SIZE_MAX / (uint32_t) msg_subscription_monitored_item_bs__p_nb_results > sizeof(*deleteResp->Results))
    {
        deleteResp->Results =
            SOPC_Calloc((size_t) msg_subscription_monitored_item_bs__p_nb_results, sizeof(*deleteResp->Results));
        if (NULL != deleteResp->Results)
        {
            deleteResp->NoOfResults = msg_subscription_monitored_item_bs__p_nb_results;
            for (int32_t i = 0; i < deleteResp->NoOfResults; i++)
            {
                SOPC_StatusCode_Initialize(&deleteResp->Results[i]);
            }
            *msg_subscription_monitored_item_bs__bres = true;
        }
    }
}

void msg_subscription_monitored_item_bs__get_msg_delete_monitored_items_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_monitored_item_bs__p_subscription,
    t_entier4* const msg_subscription_monitored_item_bs__p_nb_monitored_items)
{
    OpcUa_DeleteMonitoredItemsRequest* deleteReq =
        (OpcUa_DeleteMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    if (deleteReq->SubscriptionId > 0 && deleteReq->SubscriptionId <= INT32_MAX)
    {
        *msg_subscription_monitored_item_bs__p_subscription = deleteReq->SubscriptionId;
    }
    else
    {
        *msg_subscription_monitored_item_bs__p_subscription = constants__c_subscription_indet;
    }
    *msg_subscription_monitored_item_bs__p_nb_monitored_items = deleteReq->NoOfMonitoredItemIds;
}

void msg_subscription_monitored_item_bs__getall_delete_monitored_item_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    constants__t_monitoredItemId_i* const msg_subscription_monitored_item_bs__p_monitored_item_id)
{
    OpcUa_DeleteMonitoredItemsRequest* deleteReq =
        (OpcUa_DeleteMonitoredItemsRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_monitored_item_id =
        deleteReq->MonitoredItemIds[msg_subscription_monitored_item_bs__p_index - 1];
}

void msg_subscription_monitored_item_bs__setall_msg_delete_monitored_item_resp_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_monitored_item_bs__p_sc)
{
    OpcUa_DeleteMonitoredItemsResponse* deleteResp =
        (OpcUa_DeleteMonitoredItemsResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    util_status_code__B_to_C(msg_subscription_monitored_item_bs__p_sc,
                             &deleteResp->Results[msg_subscription_monitored_item_bs__p_index - 1]);
}

void msg_subscription_monitored_item_bs__alloc_msg_set_monit_mode_monitored_items_resp_results(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_nb_results,
    t_bool* const msg_subscription_monitored_item_bs__bres)
{
    assert(msg_subscription_monitored_item_bs__p_nb_results > 0);
    *msg_subscription_monitored_item_bs__bres = false;
    OpcUa_SetMonitoringModeResponse* setMonitModeResp =
        (OpcUa_SetMonitoringModeResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    if (SIZE_MAX / (uint32_t) msg_subscription_monitored_item_bs__p_nb_results > sizeof(*setMonitModeResp->Results))
    {
        setMonitModeResp->Results =
            SOPC_Calloc((size_t) msg_subscription_monitored_item_bs__p_nb_results, sizeof(*setMonitModeResp->Results));
        if (NULL != setMonitModeResp->Results)
        {
            setMonitModeResp->NoOfResults = msg_subscription_monitored_item_bs__p_nb_results;
            for (int32_t i = 0; i < setMonitModeResp->NoOfResults; i++)
            {
                SOPC_StatusCode_Initialize(&setMonitModeResp->Results[i]);
            }
            *msg_subscription_monitored_item_bs__bres = true;
        }
    }
}

void msg_subscription_monitored_item_bs__get_msg_set_monit_mode_monitored_items_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    constants__t_subscription_i* const msg_subscription_monitored_item_bs__p_subscription,
    constants__t_monitoringMode_i* const msg_subscription_monitored_item_bs__p_monitoring_mode,
    t_entier4* const msg_subscription_monitored_item_bs__p_nb_monitored_items)
{
    OpcUa_SetMonitoringModeRequest* setMonitModReq =
        (OpcUa_SetMonitoringModeRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    if (setMonitModReq->SubscriptionId > 0 && setMonitModReq->SubscriptionId <= INT32_MAX)
    {
        *msg_subscription_monitored_item_bs__p_subscription = setMonitModReq->SubscriptionId;
    }
    else
    {
        *msg_subscription_monitored_item_bs__p_subscription = constants__c_subscription_indet;
    }
    switch (setMonitModReq->MonitoringMode)
    {
    case OpcUa_MonitoringMode_Disabled:
        *msg_subscription_monitored_item_bs__p_monitoring_mode = constants__e_monitoringMode_disabled;
        break;
    case OpcUa_MonitoringMode_Sampling:
        *msg_subscription_monitored_item_bs__p_monitoring_mode = constants__e_monitoringMode_sampling;
        break;
    case OpcUa_MonitoringMode_Reporting:
        *msg_subscription_monitored_item_bs__p_monitoring_mode = constants__e_monitoringMode_reporting;
        break;
    default:
        *msg_subscription_monitored_item_bs__p_monitoring_mode = constants__c_monitoringMode_indet;
    }
    *msg_subscription_monitored_item_bs__p_nb_monitored_items = setMonitModReq->NoOfMonitoredItemIds;
}

void msg_subscription_monitored_item_bs__getall_set_monit_mode_monitored_item_req_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_req_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    constants__t_monitoredItemId_i* const msg_subscription_monitored_item_bs__p_monitored_item_id)
{
    OpcUa_SetMonitoringModeRequest* setMonitModReq =
        (OpcUa_SetMonitoringModeRequest*) msg_subscription_monitored_item_bs__p_req_msg;
    *msg_subscription_monitored_item_bs__p_monitored_item_id =
        setMonitModReq->MonitoredItemIds[msg_subscription_monitored_item_bs__p_index - 1];
}

void msg_subscription_monitored_item_bs__setall_msg_set_monit_mode_monitored_item_resp_params(
    const constants__t_msg_i msg_subscription_monitored_item_bs__p_resp_msg,
    const t_entier4 msg_subscription_monitored_item_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_subscription_monitored_item_bs__p_sc)
{
    OpcUa_SetMonitoringModeResponse* setMonitModeResp =
        (OpcUa_SetMonitoringModeResponse*) msg_subscription_monitored_item_bs__p_resp_msg;
    util_status_code__B_to_C(msg_subscription_monitored_item_bs__p_sc,
                             &setMonitModeResp->Results[msg_subscription_monitored_item_bs__p_index - 1]);
}
