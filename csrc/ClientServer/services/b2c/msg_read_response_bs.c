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

#include <string.h>
#include <time.h>

#include "msg_read_response_bs.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_types.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_response_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_response_bs__alloc_read_response(const t_entier4 msg_read_response_bs__p_nb_resps,
                                               const constants__t_msg_i msg_read_response_bs__p_resp_msg,
                                               t_bool* const msg_read_response_bs__p_isvalid)
{
    OpcUa_ReadResponse* msg_read_resp = (OpcUa_ReadResponse*) msg_read_response_bs__p_resp_msg;

    msg_read_resp->NoOfResults = msg_read_response_bs__p_nb_resps;
    if (msg_read_response_bs__p_nb_resps > 0)
    {
        msg_read_resp->Results = SOPC_Calloc((size_t) msg_read_response_bs__p_nb_resps, sizeof(SOPC_DataValue));
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
}

void msg_read_response_bs__set_read_response(const constants__t_msg_i msg_read_response_bs__p_resp_msg,
                                             const constants__t_ReadValue_i msg_read_response_bs__p_rvi,
                                             const constants__t_Variant_i msg_read_response_bs__p_value,
                                             const constants__t_RawStatusCode msg_read_response_bs__p_raw_sc,
                                             const constants__t_Timestamp msg_read_response_bs__p_ts_src,
                                             const constants__t_Timestamp msg_read_response_bs__p_ts_srv)
{
    OpcUa_ReadResponse* resp = (OpcUa_ReadResponse*) msg_read_response_bs__p_resp_msg;
    SOPC_DataValue* pDataValue = NULL;

    assert(msg_read_response_bs__p_rvi > 0);

    /* rvi is castable, it's one of its properties, but it starts at 1 */
    pDataValue = &resp->Results[msg_read_response_bs__p_rvi - 1];

    SOPC_Variant_Initialize(&pDataValue->Value);

    if (constants__c_Variant_indet != msg_read_response_bs__p_value)
    {
        /* Note: the following only copies the context of the Variant, not the entire Variant */
        SOPC_Variant_Move(&pDataValue->Value, msg_read_response_bs__p_value);
    }

    pDataValue->Status = msg_read_response_bs__p_raw_sc;
    pDataValue->SourceTimestamp = msg_read_response_bs__p_ts_src.timestamp;
    pDataValue->SourcePicoSeconds = msg_read_response_bs__p_ts_src.picoSeconds;
    pDataValue->ServerTimestamp = msg_read_response_bs__p_ts_srv.timestamp;
    pDataValue->ServerPicoSeconds = msg_read_response_bs__p_ts_srv.picoSeconds;
}
