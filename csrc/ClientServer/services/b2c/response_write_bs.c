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
 * Implements the structures behind the address space.
 */

#include <string.h>

#include "b2c.h"
#include "response_write_bs.h"
#include "sopc_mem_alloc.h"
#include "sopc_services_api.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "util_b2c.h"

/* Globals */
static SOPC_StatusCode* arr_StatusCode; /* Indexed from 1, first element is never used. */
static t_entier4 nb_req;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void response_write_bs__INITIALISATION(void)
{
    arr_StatusCode = NULL;
    nb_req = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void response_write_bs__alloc_write_request_responses_malloc(const t_entier4 response_write_bs__nb_req,
                                                             t_bool* const response_write_bs__ResponseWrite_allocated)
{
    *response_write_bs__ResponseWrite_allocated = false; /* TODO: set a true and false in b2c.h */
    nb_req = 0;

    if (response_write_bs__nb_req >= 0 &&
        (uint64_t)(response_write_bs__nb_req + 1) <= (uint64_t) SIZE_MAX / sizeof(SOPC_StatusCode))
    {
        arr_StatusCode = SOPC_Malloc(sizeof(SOPC_StatusCode) * (size_t)(response_write_bs__nb_req + 1));
    }
    else
    {
        arr_StatusCode = NULL;
    }
    if (NULL != arr_StatusCode)
    {
        for (int32_t i = 0; i <= response_write_bs__nb_req; i++)
        {
            arr_StatusCode[i] = OpcUa_BadInternalError;
        }
        *response_write_bs__ResponseWrite_allocated = true;
        nb_req = response_write_bs__nb_req;
    }
}

void response_write_bs__reset_ResponseWrite(void)
{
    SOPC_Free(arr_StatusCode);
    arr_StatusCode = NULL;
    nb_req = 0;
}

void response_write_bs__set_ResponseWrite_StatusCode(
    const constants__t_WriteValue_i response_write_bs__wvi,
    const constants_statuscodes_bs__t_StatusCode_i response_write_bs__sc)
{
    util_status_code__B_to_C(response_write_bs__sc, &arr_StatusCode[response_write_bs__wvi]);
}

void response_write_bs__write_WriteResponse_msg_out(const constants__t_msg_i response_write_bs__msg_out)
{
    OpcUa_WriteResponse* msg_write_resp = (OpcUa_WriteResponse*) response_write_bs__msg_out;
    SOPC_StatusCode* lsc = NULL;

    if (nb_req > 0 && (uint64_t) SIZE_MAX / sizeof(SOPC_StatusCode) >= (uint64_t) nb_req)
    {
        lsc = SOPC_Malloc(sizeof(SOPC_StatusCode) * (size_t) nb_req);
    }

    if (NULL == lsc)
    {
        // TODO: report memory error
        msg_write_resp->NoOfResults = 0;
    }
    else
    {
        memcpy(lsc, (void*) (arr_StatusCode + 1), sizeof(SOPC_StatusCode) * (size_t) nb_req);

        msg_write_resp->NoOfResults = nb_req;
    }

    msg_write_resp->Results = lsc;
    msg_write_resp->NoOfDiagnosticInfos = 0;
    msg_write_resp->DiagnosticInfos = NULL;
}
