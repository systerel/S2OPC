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
 * Implements the getters for the write request.
 */

#include "service_write_decode_bs.h"
#include "b2c.h"
#include "util_b2c.h"

#include "address_space_impl.h" /* e_aid_* */
#include "sopc_types.h"

/* Globals */
static OpcUa_WriteRequest* request;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_write_decode_bs__INITIALISATION(void)
{
    request = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
  /*@ requires \valid(service_write_decode_bs__StatusCode_service);
    @ requires \separated(service_write_decode_bs__StatusCode_service, request);
    @ assigns request;
    @ assigns *service_write_decode_bs__StatusCode_service;
    @
    @ behavior e_sc_ok:
    @     assumes 0 < ((OpcUa_WriteRequest*) service_write_decode_bs__write_msg)->NoOfNodesToWrite
    @     && ((OpcUa_WriteRequest*) service_write_decode_bs__write_msg)->NoOfNodesToWrite <= constants__k_n_WriteResponse_max;
    @     ensures request == (OpcUa_WriteRequest*) service_write_decode_bs__write_msg;
    @     ensures *service_write_decode_bs__StatusCode_service == constants_statuscodes_bs__e_sc_ok;
    @
    @ behavior e_sc_bad_nothing_to_do:
    @     assumes ((OpcUa_WriteRequest*) service_write_decode_bs__write_msg)->NoOfNodesToWrite <= 0;
    @     ensures request == \old(request);
    @     ensures *service_write_decode_bs__StatusCode_service == constants_statuscodes_bs__e_sc_bad_nothing_to_do;
    @
    @ behavior e_sc_bad_too_many_ops:
    @     assumes ((OpcUa_WriteRequest*) service_write_decode_bs__write_msg)->NoOfNodesToWrite > constants__k_n_WriteResponse_max;
    @     ensures request == \old(request);
    @     ensures *service_write_decode_bs__StatusCode_service == constants_statuscodes_bs__e_sc_bad_too_many_ops;
    @
    @ disjoint behaviors;
    @ complete behaviors;
 */
void service_write_decode_bs__decode_write_request(
    const constants__t_msg_i service_write_decode_bs__write_msg,
    constants_statuscodes_bs__t_StatusCode_i* const service_write_decode_bs__StatusCode_service)
{
    OpcUa_WriteRequest* req = (OpcUa_WriteRequest*) service_write_decode_bs__write_msg;

    if (0 < req->NoOfNodesToWrite && req->NoOfNodesToWrite <= constants__k_n_WriteResponse_max)
    {
        /* TODO: req shall not be freed before request is null... */
        request = req;
        *service_write_decode_bs__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
    }
    else
    {
        if (req->NoOfNodesToWrite <= 0)
        {
            *service_write_decode_bs__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
        }
        else if (req->NoOfNodesToWrite > constants__k_n_WriteResponse_max)
        {
            *service_write_decode_bs__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
        }
    }
}

/*@ requires \true;
  @ assigns request;
  @ ensures request == \null;
 */
void service_write_decode_bs__free_write_request(void)
{
    request = NULL;
}

/*@ requires \valid(service_write_decode_bs__nb_req);
  @ requires \valid(request);
  @ assigns *service_write_decode_bs__nb_req;
  @ ensures *service_write_decode_bs__nb_req == request->NoOfNodesToWrite;
 */
void service_write_decode_bs__get_nb_WriteValue(t_entier4* const service_write_decode_bs__nb_req)
{
    assert (NULL != request);
    *service_write_decode_bs__nb_req = request->NoOfNodesToWrite;
}

/**
 * Note: When using request as a OpcUa_WriteRequest,
 * \p nid and \p value are borrowed from request,
 * you should not free them.
 */
 /*@ requires 1 <= service_write_decode_bs__wvi <= request->NoOfNodesToWrite;
  @ requires \valid(service_write_decode_bs__isvalid);
  @ requires \valid(service_write_decode_bs__status);
  @ requires \valid(service_write_decode_bs__nid);
  @ requires \valid(service_write_decode_bs__aid);
  @ requires \valid(service_write_decode_bs__dataValue);
  @ requires \valid(request);
  @ requires \valid(request->NodesToWrite+(service_write_decode_bs__wvi - 1));
  @ requires \separated(service_write_decode_bs__nid, service_write_decode_bs__dataValue,
    service_write_decode_bs__status, service_write_decode_bs__aid, service_write_decode_bs__isvalid, request,
    request->NodesToWrite+(service_write_decode_bs__wvi - 1));
  @ assigns *service_write_decode_bs__isvalid;
  @ assigns *service_write_decode_bs__status;
  @ assigns *service_write_decode_bs__aid;
  @ assigns *service_write_decode_bs__nid;
  @ assigns *service_write_decode_bs__dataValue;
  @
  @ behavior invalid_attribute:
  @     assumes *service_write_decode_bs__aid == constants__c_AttributeId_indet;
  @     ensures *service_write_decode_bs__isvalid == false;
  @     ensures *service_write_decode_bs__status == constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
  @
  @ behavior valid_attribute:
  @     assumes *service_write_decode_bs__aid == constants__c_AttributeId_indet;
  @     ensures *service_write_decode_bs__nid == &(&request->NodesToWrite[service_write_decode_bs__wvi - 1])->NodeId;
  @     ensures *service_write_decode_bs__dataValue == &(&request->NodesToWrite[service_write_decode_bs__wvi - 1])->Value;
  @     ensures *service_write_decode_bs__index_range == &(&request->NodesToWrite[service_write_decode_bs__wvi - 1])->IndexRange;
  @     ensures *service_write_decode_bs__isvalid == true;
  @     ensures *service_write_decode_bs__status == constants_statuscodes_bs__e_sc_ok;
  @ complete behaviors;
  @ disjoint behaviors;
 */
void service_write_decode_bs__getall_WriteValue(
    const constants__t_WriteValue_i service_write_decode_bs__wvi,
    t_bool* const service_write_decode_bs__isvalid,
    constants_statuscodes_bs__t_StatusCode_i* const service_write_decode_bs__status,
    constants__t_NodeId_i* const service_write_decode_bs__nid,
    constants__t_AttributeId_i* const service_write_decode_bs__aid,
    constants__t_DataValue_i* const service_write_decode_bs__dataValue,
    constants__t_IndexRange_i* const service_write_decode_bs__index_range)
{
    *service_write_decode_bs__nid = constants__c_NodeId_indet;
    *service_write_decode_bs__dataValue = constants__c_DataValue_indet;

    OpcUa_WriteValue* wv = &request->NodesToWrite[service_write_decode_bs__wvi - 1];
    *service_write_decode_bs__aid = util_AttributeId__C_to_B(wv->AttributeId);
    if (*service_write_decode_bs__aid == constants__c_AttributeId_indet)
    {
        *service_write_decode_bs__isvalid = false;
        *service_write_decode_bs__status = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
        return;
    }

    *service_write_decode_bs__nid = &wv->NodeId;
    *service_write_decode_bs__dataValue = &wv->Value;
    *service_write_decode_bs__index_range = &wv->IndexRange;
    *service_write_decode_bs__isvalid = true;
    *service_write_decode_bs__status = constants_statuscodes_bs__e_sc_ok;
}

/*@ requires \valid(service_write_decode_bs__wvPointer);
  @ requires 1 <= service_write_decode_bs__wvi <= request->NoOfNodesToWrite;
  @ requires \valid(request);
  @ requires \valid(request->NodesToWrite+(service_write_decode_bs__wvi - 1));
  @ requires \separated(service_write_decode_bs__wvPointer, request);
  @ assigns *service_write_decode_bs__wvPointer;
  @ ensures *service_write_decode_bs__wvPointer == &request->NodesToWrite[service_write_decode_bs__wvi - 1];
*/
void service_write_decode_bs__getall_WriteValuePointer(
    const constants__t_WriteValue_i service_write_decode_bs__wvi,
    constants__t_WriteValuePointer_i* const service_write_decode_bs__wvPointer)
{
    *service_write_decode_bs__wvPointer = &request->NodesToWrite[service_write_decode_bs__wvi - 1];
}
