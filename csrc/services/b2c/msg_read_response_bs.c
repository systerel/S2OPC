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
        msg_read_resp->Results = (SOPC_DataValue*) calloc(msg_read_response_bs__p_nb_resps, sizeof(SOPC_DataValue));
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
                                             const constants__t_StatusCode_i msg_read_response_bs__sc,
                                             const constants__t_AttributeId_i msg_read_response_bs__aid)
{
    OpcUa_ReadResponse* pMsgReadResp = (OpcUa_ReadResponse*) msg_read_response_bs__resp_msg;
    SOPC_DataValue* pDataValue = NULL;
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;

    if (msg_read_response_bs__rvi > 0)
    {
        /* rvi is castable, it's one of its properties, but it starts at 1 */
        pDataValue = &pMsgReadResp->Results[msg_read_response_bs__rvi - 1];

        SOPC_Variant_Initialize(&pDataValue->Value);

        if (constants__c_Variant_indet != msg_read_response_bs__val)
        {
            /* Note: the following only copies the context of the Variant, not the entire Variant */
            retStatus = SOPC_Variant_ShallowCopy(&pDataValue->Value, (SOPC_Variant*) msg_read_response_bs__val);
        }

        if (retStatus == SOPC_STATUS_OK)
        {
            util_status_code__B_to_C(msg_read_response_bs__sc, &pDataValue->Status);
        }
        else
        {
            util_status_code__B_to_C(constants__e_sc_bad_unexpected_error, &pDataValue->Status);
        }

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
