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
 * Implements the base machine that "sends" the ReadResponse.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "msg_read_response_bs.h"
#include "util_b2c.h"

#include "sopc_time.h"
#include "sopc_types.h"

/*----------
   Globals
  ----------*/
static constants__t_TimestampsToReturn_i ttrRequested = constants__e_ttr_neither;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_response_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_response_bs__alloc_read_response(
    const t_entier4 msg_read_response_bs__p_nb_resps,
    const constants__t_TimestampsToReturn_i msg_read_response_bs__p_TimestampsToReturn,
    const constants__t_msg_i msg_read_response_bs__p_resp_msg,
    t_bool* const msg_read_response_bs__p_isvalid)
{
    OpcUa_ReadResponse* msg_read_resp = (OpcUa_ReadResponse*) msg_read_response_bs__p_resp_msg;

    msg_read_resp->NoOfResults = msg_read_response_bs__p_nb_resps;
    if (msg_read_response_bs__p_nb_resps > 0)
    {
        msg_read_resp->Results =
            (SOPC_DataValue*) calloc((size_t) msg_read_response_bs__p_nb_resps, sizeof(SOPC_DataValue));
    }
    else
    {
        msg_read_resp->Results = NULL;
    }
    if (NULL == msg_read_resp->Results)
    {
        *msg_read_response_bs__p_isvalid = false;
        return;
    }
    else
        *msg_read_response_bs__p_isvalid = true;

    for (int32_t i = 0; i < msg_read_resp->NoOfResults; i++)
        SOPC_DataValue_Initialize(&msg_read_resp->Results[i]);

    msg_read_resp->NoOfDiagnosticInfos = 0;
    msg_read_resp->DiagnosticInfos = NULL;

    ttrRequested = msg_read_response_bs__p_TimestampsToReturn;
}

void msg_read_response_bs__set_read_response(const constants__t_msg_i msg_read_response_bs__resp_msg,
                                             const constants__t_ReadValue_i msg_read_response_bs__rvi,
                                             const constants__t_Variant_i msg_read_response_bs__val,
                                             const constants_statuscodes_bs__t_StatusCode_i msg_read_response_bs__sc,
                                             const constants__t_AttributeId_i msg_read_response_bs__aid)
{
    OpcUa_ReadResponse* pMsgReadResp = (OpcUa_ReadResponse*) msg_read_response_bs__resp_msg;
    SOPC_DataValue* pDataValue = NULL;

    if (msg_read_response_bs__rvi > 0)
    {
        /* rvi is castable, it's one of its properties, but it starts at 1 */
        pDataValue = &pMsgReadResp->Results[msg_read_response_bs__rvi - 1];

        SOPC_Variant_Initialize(&pDataValue->Value);

        if (constants__c_Variant_indet != msg_read_response_bs__val)
        {
            /* Note: the following only copies the context of the Variant, not the entire Variant */
            SOPC_Variant_Move(&pDataValue->Value, msg_read_response_bs__val);
        }

        util_status_code__B_to_C(msg_read_response_bs__sc, &pDataValue->Status);

        if (msg_read_response_bs__aid == constants__e_aid_Value &&
            (constants__e_ttr_both == ttrRequested || constants__e_ttr_source == ttrRequested))
        {
            pDataValue->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
        }
        if (constants__e_ttr_both == ttrRequested || constants__e_ttr_server == ttrRequested)
        {
            pDataValue->ServerTimestamp = SOPC_Time_GetCurrentTimeUTC();
        }
    }
}
