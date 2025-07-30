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
 * Implements the base machine that treats a HistoryReadRequest.
 */

#include <stdint.h>
#include <stdio.h>

#include "constants_statuscodes_bs.h"
#include "history_read_treatment_bs.h"
#include "libs2opc_server_internal.h"
#include "msg_history_read_request.h"
#include "util_b2c.h"

#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void history_read_treatment_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void history_read_treatment_bs__external_history_raw_read(
    const constants__t_readRawModifiedDetails_i history_read_treatment_bs__p_readRawDetails,
    const t_bool history_read_treatment_bs__p_TSrequired,
    const t_bool history_read_treatment_bs__p_ContinuationPoint,
    const constants__t_historyReadValueId_i history_read_treatment_bs__p_singleValueId,
    constants_statuscodes_bs__t_StatusCode_i* const history_read_treatment_bs__p_StatusCode,
    constants__t_Nonce_i* const history_read_treatment_bs__p_contPoint,
    t_entier4* const history_read_treatment_bs__p_nbDataValues,
    constants__t_DataValue_array_i* const history_read_treatment_bs__p_DataValues)
{
    if (NULL != sopc_server_helper_config.externalHistoryReadCb)
    {
        SOPC_StatusCode status = SOPC_GoodGenericStatus;

        sopc_server_helper_config.externalHistoryReadCb(
            history_read_treatment_bs__p_readRawDetails, history_read_treatment_bs__p_TSrequired,
            history_read_treatment_bs__p_ContinuationPoint, history_read_treatment_bs__p_singleValueId,
            sopc_server_helper_config.externalHistoryReadContext, &status, history_read_treatment_bs__p_contPoint,
            history_read_treatment_bs__p_nbDataValues, history_read_treatment_bs__p_DataValues);

        /* Check the validity of the callback's return */
        if (*history_read_treatment_bs__p_nbDataValues < 0 ||
            (*history_read_treatment_bs__p_nbDataValues == 0 && *history_read_treatment_bs__p_DataValues != NULL) ||
            (*history_read_treatment_bs__p_nbDataValues > 0 && *history_read_treatment_bs__p_DataValues == NULL))
        {
            *history_read_treatment_bs__p_StatusCode = constants_statuscodes_bs__e_sc_bad_internal_error;
            *history_read_treatment_bs__p_contPoint = constants__c_Nonce_indet;
            *history_read_treatment_bs__p_nbDataValues = 0;
            *history_read_treatment_bs__p_DataValues = constants__c_DataValue_array_indet;
        }
        else
        {
            /* Convert status returned from the callback into B status code */
            util_status_code__C_to_B(status, history_read_treatment_bs__p_StatusCode);
        }
    }
    else
    {
        *history_read_treatment_bs__p_StatusCode = constants_statuscodes_bs__e_sc_bad_history_operation_unsupported;
    }
}

void history_read_treatment_bs__set_ts_srv_dataValues(
    const t_entier4 history_read_treatment_bs__p_nbDataValues,
    const constants__t_DataValue_array_i history_read_treatment_bs__p_DataValues,
    const constants__t_Timestamp history_read_treatment_bs__p_currentTs)
{
    for (int i = 0; i < history_read_treatment_bs__p_nbDataValues; i++)
    {
        history_read_treatment_bs__p_DataValues[i].ServerTimestamp = history_read_treatment_bs__p_currentTs.timestamp;
        history_read_treatment_bs__p_DataValues[i].ServerPicoSeconds =
            history_read_treatment_bs__p_currentTs.picoSeconds;
    }
}
