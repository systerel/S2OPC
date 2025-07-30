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

/** \file
 *
 * Implements the base machine that response to a HistoryReadRequest.
 */

#include "msg_history_read_response_bs.h"

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_history_read_response_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_history_read_response_bs__alloc_msg_hist_read_resp_results(
    const t_entier4 msg_history_read_response_bs__p_nb_to_read,
    const constants__t_msg_i msg_history_read_response_bs__p_resp_msg,
    t_bool* const msg_history_read_response_bs__p_isvalid)
{
    OpcUa_HistoryReadResponse* msg_hist_read_resp = msg_history_read_response_bs__p_resp_msg;
    *msg_history_read_response_bs__p_isvalid = false;

    if (msg_history_read_response_bs__p_nb_to_read > 0)
    {
        msg_hist_read_resp->Results =
            SOPC_Calloc((size_t) msg_history_read_response_bs__p_nb_to_read, sizeof(OpcUa_HistoryReadResult));
    }
    if (NULL != msg_hist_read_resp->Results)
    {
        *msg_history_read_response_bs__p_isvalid = true;
        for (int32_t i = 0; i < msg_history_read_response_bs__p_nb_to_read; i++)
        {
            OpcUa_HistoryReadResult_Initialize(&msg_hist_read_resp->Results[i]);
        }
        msg_hist_read_resp->NoOfResults = msg_history_read_response_bs__p_nb_to_read;
    }
    else
    {
        msg_hist_read_resp->NoOfResults = 0;
    }

    msg_hist_read_resp->NoOfDiagnosticInfos = 0;
    msg_hist_read_resp->DiagnosticInfos = NULL;
}

void msg_history_read_response_bs__set_msg_hist_read_response(
    const constants__t_msg_i msg_history_read_response_bs__p_resp_msg,
    const t_entier4 msg_history_read_response_bs__p_index,
    const constants_statuscodes_bs__t_StatusCode_i msg_history_read_response_bs__p_sc,
    const constants__t_Nonce_i msg_history_read_response_bs__p_contPoint,
    const t_entier4 msg_history_read_response_bs__p_nbDataValues,
    const constants__t_DataValue_array_i msg_history_read_response_bs__p_DataValues)
{
    OpcUa_HistoryReadResponse* resp = msg_history_read_response_bs__p_resp_msg;
    OpcUa_HistoryReadResult* pResults = NULL;

    SOPC_ASSERT(msg_history_read_response_bs__p_index > 0);

    pResults = &resp->Results[msg_history_read_response_bs__p_index - 1];

    util_status_code__B_to_C(msg_history_read_response_bs__p_sc, &pResults->StatusCode);
    if (NULL == msg_history_read_response_bs__p_contPoint)
    {
        SOPC_ByteString_Initialize(&pResults->ContinuationPoint);
    }
    else
    {
        pResults->ContinuationPoint = *msg_history_read_response_bs__p_contPoint;
        SOPC_Free(msg_history_read_response_bs__p_contPoint);
    }

    OpcUa_HistoryData* historyDataObj = NULL;
    SOPC_ReturnStatus status = SOPC_ExtensionObject_CreateObject(
        &pResults->HistoryData, &OpcUa_HistoryData_EncodeableType, (void**) &historyDataObj);
    OpcUa_HistoryData_Initialize(historyDataObj);

    if (SOPC_STATUS_OK == status)
    {
        historyDataObj->NoOfDataValues = msg_history_read_response_bs__p_nbDataValues;
        historyDataObj->DataValues = msg_history_read_response_bs__p_DataValues;
    }
}
