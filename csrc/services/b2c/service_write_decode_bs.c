/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
void service_write_decode_bs__decode_write_request(
    const constants__t_msg_i service_write_decode_bs__write_msg,
    constants__t_StatusCode_i* const service_write_decode_bs__StatusCode_service)
{
    /* TODO: actually decode something */
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) service_write_decode_bs__write_msg;
    *service_write_decode_bs__StatusCode_service = constants__e_sc_bad_unexpected_error;

    if (encType == &OpcUa_WriteRequest_EncodeableType)
    {
        OpcUa_WriteRequest* req = (OpcUa_WriteRequest*) service_write_decode_bs__write_msg;

        if (0 < req->NoOfNodesToWrite && req->NoOfNodesToWrite <= constants__k_n_WriteResponse_max)
        {
            /* TODO: req shall not be freed before request is null... */
            request = req;
            *service_write_decode_bs__StatusCode_service = constants__e_sc_ok;
        }
        else
        {
            if (req->NoOfNodesToWrite <= 0)
            {
                *service_write_decode_bs__StatusCode_service = constants__e_sc_bad_nothing_to_do;
            }
            else if (req->NoOfNodesToWrite > constants__k_n_WriteResponse_max)
            {
                *service_write_decode_bs__StatusCode_service = constants__e_sc_bad_too_many_ops;
            }
        }
    }
}

void service_write_decode_bs__free_write_request(void)
{
    /* TODO: don't free the request that you did not initialize */
    request = NULL;
}

void service_write_decode_bs__get_nb_WriteValue(t_entier4* const service_write_decode_bs__nb_req)
{
    /* TODO: does B prevent this operation from being called before decode ? */
    if (NULL != request)
        *service_write_decode_bs__nb_req = request->NoOfNodesToWrite;
    else
        *service_write_decode_bs__nb_req = 0;
}

/**
 * Note: When using request as a OpcUa_WriteRequest,
 * \p nid and \p value are borrowed from request,
 * you should not free them.
 */
void service_write_decode_bs__getall_WriteValue(const constants__t_WriteValue_i service_write_decode_bs__wvi,
                                                t_bool* const service_write_decode_bs__isvalid,
                                                constants__t_StatusCode_i* const service_write_decode_bs__status,
                                                constants__t_NodeId_i* const service_write_decode_bs__nid,
                                                constants__t_AttributeId_i* const service_write_decode_bs__aid,
                                                constants__t_Variant_i* const service_write_decode_bs__value)
{
    /* Failure reasons:
       - wvi is too high
       - invalid attribute id
       - TODO: does B prevent this operation from being called before decode ?
    */
    uint32_t aid;
    OpcUa_WriteValue* wv;

    *service_write_decode_bs__isvalid = false;
    *service_write_decode_bs__status = constants__c_StatusCode_indet;

    if (NULL != request && service_write_decode_bs__wvi <= request->NoOfNodesToWrite)
    {
        wv = &request->NodesToWrite[service_write_decode_bs__wvi - 1];
        *service_write_decode_bs__isvalid = true;
        *service_write_decode_bs__nid = &wv->NodeId;
        aid = wv->AttributeId;
        switch (aid)
        {
        case e_aid_NodeId:
            *service_write_decode_bs__aid = constants__e_aid_NodeId;
            break;
        case e_aid_NodeClass:
            *service_write_decode_bs__aid = constants__e_aid_NodeClass;
            break;
        case e_aid_Value:
            *service_write_decode_bs__aid = constants__e_aid_Value;
            break;
        default:
            *service_write_decode_bs__isvalid = false;
            *service_write_decode_bs__status = constants__e_sc_bad_attribute_id_invalid;
            break;
        }
        *service_write_decode_bs__value = &wv->Value.Value;
    }
    else
    {
        *service_write_decode_bs__status = constants__e_sc_bad_internal_error;
    }
}

void service_write_decode_bs__getAndClean_WriteValuePointer(
    const constants__t_WriteValue_i service_write_decode_bs__wvi,
    constants__t_WriteValuePointer_i* const service_write_decode_bs__wvPointer)
{
    OpcUa_WriteValue* wv = NULL;

    if (NULL != request && service_write_decode_bs__wvi <= request->NoOfNodesToWrite)
    {
        wv = malloc(sizeof(OpcUa_WriteValue));
        if (NULL != wv)
        {
            *wv = request->NodesToWrite[service_write_decode_bs__wvi - 1];
            /* Re-Init the WriteValue to avoid deallocation of its content now copied in new WriteValue */
            OpcUa_WriteValue_Initialize(&request->NodesToWrite[service_write_decode_bs__wvi - 1]);
        }
    }
    *service_write_decode_bs__wvPointer = wv;
}
