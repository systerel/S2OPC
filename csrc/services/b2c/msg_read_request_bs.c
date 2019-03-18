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
 * Implements the base machine that reads a ReadRequest.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "msg_read_request_bs.h"
#include "util_b2c.h"

#include "address_space_impl.h"
#include "sopc_logger.h"
#include "sopc_types.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request_bs__getall_req_ReadValue_AttributeId(const constants__t_msg_i msg_read_request_bs__msg,
                                                           const constants__t_ReadValue_i msg_read_request_bs__rvi,
                                                           constants__t_StatusCode_i* const msg_read_request_bs__sc,
                                                           constants__t_AttributeId_i* const msg_read_request_bs__aid)
{
    /* TODO: is message type checked at this point? */
    OpcUa_ReadRequest* msg_read_req = (OpcUa_ReadRequest*) msg_read_request_bs__msg;
    static bool bWarned = false;
    *msg_read_request_bs__sc = constants__e_sc_ok;

    *msg_read_request_bs__aid =
        util_AttributeId__C_to_B(msg_read_req->NodesToRead[msg_read_request_bs__rvi - 1].AttributeId);

    if (*msg_read_request_bs__aid == constants__c_AttributeId_indet)
    {
        *msg_read_request_bs__sc = constants__e_sc_bad_attribute_id_invalid;
        if (!bWarned)
        {
            SOPC_Logger_TraceWarning("msg_read_request_bs__getall_req_ReadValue_AttributeId: unsupported attribute id");
            bWarned = true;
        }
    }
}

void msg_read_request_bs__getall_req_ReadValue_IndexRange(
    const constants__t_msg_i msg_read_request_bs__msg,
    const constants__t_ReadValue_i msg_read_request_bs__rvi,
    constants__t_IndexRange_i* const msg_read_request_bs__index_range)
{
    assert(msg_read_request_bs__rvi >= 0);

    OpcUa_ReadRequest* request = msg_read_request_bs__msg;
    size_t node_index = (size_t) msg_read_request_bs__rvi - 1;
    assert(request->NoOfNodesToRead >= 0 && node_index < ((size_t) request->NoOfNodesToRead));
    *msg_read_request_bs__index_range = &request->NodesToRead[node_index].IndexRange;
}

void msg_read_request_bs__getall_req_ReadValue_NodeId(const constants__t_msg_i msg_read_request_bs__msg,
                                                      const constants__t_ReadValue_i msg_read_request_bs__rvi,
                                                      constants__t_NodeId_i* const msg_read_request_bs__nid)
{
    OpcUa_ReadRequest* msg_read_req = (OpcUa_ReadRequest*) msg_read_request_bs__msg;
    *msg_read_request_bs__nid = &msg_read_req->NodesToRead[msg_read_request_bs__rvi - 1].NodeId;
}

void msg_read_request_bs__read_req_TimestampsToReturn(
    const constants__t_msg_i msg_read_request_bs__p_msg,
    constants__t_TimestampsToReturn_i* const msg_read_request_bs__p_tsToReturn)
{
    OpcUa_ReadRequest* msg_read_req = (OpcUa_ReadRequest*) msg_read_request_bs__p_msg;
    *msg_read_request_bs__p_tsToReturn = util_TimestampsToReturn__C_to_B(msg_read_req->TimestampsToReturn);
}

void msg_read_request_bs__read_req_MaxAge(const constants__t_msg_i msg_read_request_bs__p_msg,
                                          t_bool* const msg_read_request_bs__p_maxAge_valid)
{
    OpcUa_ReadRequest* msg_read_req = (OpcUa_ReadRequest*) msg_read_request_bs__p_msg;
    *msg_read_request_bs__p_maxAge_valid = msg_read_req->MaxAge >= 0;
}

void msg_read_request_bs__read_req_nb_ReadValue(const constants__t_msg_i msg_read_request_bs__msg,
                                                t_entier4* const msg_read_request_bs__nb)
{
    OpcUa_ReadRequest* msg_read_req = (OpcUa_ReadRequest*) msg_read_request_bs__msg;
    *msg_read_request_bs__nb = msg_read_req->NoOfNodesToRead;
}
