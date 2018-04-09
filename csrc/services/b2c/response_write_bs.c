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
 * Implements the structures behind the address space.
 */

#include <stdlib.h>
#include <string.h>

#include "b2c.h"
#include "response_write_bs.h"
#include "util_b2c.h"

#include "sopc_services_api.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"

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
        arr_StatusCode = (SOPC_StatusCode*) malloc(sizeof(SOPC_StatusCode) * (size_t)(response_write_bs__nb_req + 1));
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
    free(arr_StatusCode);
    arr_StatusCode = NULL;
    nb_req = 0;
}

void response_write_bs__getall_ResponseWrite_StatusCode(const constants__t_WriteValue_i response_write_bs__wvi,
                                                        t_bool* const response_write_bs__isvalid,
                                                        constants__t_StatusCode_i* const response_write_bs__sc)
{
    *response_write_bs__isvalid = false;

    if (response_write_bs__wvi <= nb_req) /* It is not necessary to test arr_StatusCode */
    {
        *response_write_bs__isvalid = true;
        util_status_code__C_to_B(arr_StatusCode[response_write_bs__wvi], response_write_bs__sc);
    }
}

void response_write_bs__set_ResponseWrite_StatusCode(const constants__t_WriteValue_i response_write_bs__wvi,
                                                     const constants__t_StatusCode_i response_write_bs__sc)
{
    util_status_code__B_to_C(response_write_bs__sc, &arr_StatusCode[response_write_bs__wvi]);
}

void response_write_bs__write_WriteResponse_msg_out(const constants__t_msg_i response_write_bs__msg_out)
{
    OpcUa_WriteResponse* msg_write_resp = (OpcUa_WriteResponse*) response_write_bs__msg_out;
    SOPC_StatusCode* lsc = NULL;

    if (nb_req > 0 && (uint64_t) SIZE_MAX / sizeof(SOPC_StatusCode) >= (uint64_t) nb_req)
    {
        lsc = (SOPC_StatusCode*) malloc(sizeof(SOPC_StatusCode) * (size_t) nb_req);

        memcpy((void*) lsc, (void*) (arr_StatusCode + 1), sizeof(SOPC_StatusCode) * (size_t) nb_req);
    }

    if (NULL == lsc)
    {
        // TODO: report memory error
        msg_write_resp->NoOfResults = 0;
    }
    else
    {
        msg_write_resp->NoOfResults = nb_req;
    }

    msg_write_resp->Results = lsc;
    msg_write_resp->NoOfDiagnosticInfos = 0;
    msg_write_resp->DiagnosticInfos = NULL;
}
