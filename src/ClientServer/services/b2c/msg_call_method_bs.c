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

#include <assert.h>

#include "msg_call_method_bs.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

/*
  Index in C code equals index from B model minus 1
*/

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_call_method_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static OpcUa_CallResponse* msg_call_method_bs__getCallResponse(const constants__t_msg_i msg_call_method_bs__p_res_msg)
{
    assert(NULL != msg_call_method_bs__p_res_msg);
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) msg_call_method_bs__p_res_msg;
    assert(encType == &OpcUa_CallResponse_EncodeableType);
    return (OpcUa_CallResponse*) msg_call_method_bs__p_res_msg;
}

static OpcUa_CallMethodResult* msg_call_method_bs__getCallResult(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod)
{
    OpcUa_CallResponse* response = msg_call_method_bs__getCallResponse(msg_call_method_bs__p_res_msg);
    /* ensured by B model */
    assert(0 < msg_call_method_bs__callMethod && msg_call_method_bs__callMethod <= response->NoOfResults);

    OpcUa_CallMethodResult* result = &response->Results[msg_call_method_bs__callMethod - 1];
    assert(NULL != result);
    return result;
}

static OpcUa_CallRequest* msg_call_method_bs__getCallRequest(const constants__t_msg_i msg_call_method_bs__p_req_msg)
{
    assert(NULL != msg_call_method_bs__p_req_msg);
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) msg_call_method_bs__p_req_msg;
    assert(encType == &OpcUa_CallRequest_EncodeableType);
    return (OpcUa_CallRequest*) msg_call_method_bs__p_req_msg;
}

static OpcUa_CallMethodRequest* msg_call_method_bs__getCallMethod(
    const constants__t_msg_i msg_call_method_bs__p_req_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod)
{
    OpcUa_CallRequest* request = msg_call_method_bs__getCallRequest(msg_call_method_bs__p_req_msg);

    /* ensured by B model */
    assert(0 < msg_call_method_bs__callMethod && msg_call_method_bs__callMethod <= request->NoOfMethodsToCall);

    OpcUa_CallMethodRequest* method = &request->MethodsToCall[msg_call_method_bs__callMethod - 1];
    assert(NULL != method);
    return method;
}

void msg_call_method_bs__alloc_CallMethod_Res_InputArgumentResult(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod,
    const t_entier4 msg_call_method_bs__nb,
    constants_statuscodes_bs__t_StatusCode_i* const msg_call_method_bs__statusCode)
{
    assert(NULL != msg_call_method_bs__statusCode);

    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);

    /* ensured by B model */
    assert(msg_call_method_bs__nb > 0);

    result->InputArgumentResults = SOPC_Calloc((size_t) msg_call_method_bs__nb, sizeof(SOPC_StatusCode));

    if (NULL == result->InputArgumentResults)
    {
        *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        result->NoOfInputArgumentResults = 0;
        return;
    }
    result->NoOfInputArgumentResults = msg_call_method_bs__nb;
    *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
}

void msg_call_method_bs__alloc_CallMethod_Res_OutputArgument(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod,
    const t_entier4 msg_call_method_bs__nb,
    constants_statuscodes_bs__t_StatusCode_i* const msg_call_method_bs__statusCode)
{
    assert(NULL != msg_call_method_bs__statusCode);

    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);

    /* ensured by B model */
    assert(msg_call_method_bs__nb > 0);

    result->OutputArguments = SOPC_Calloc((size_t) msg_call_method_bs__nb, sizeof(SOPC_Variant));

    if (NULL == result->OutputArguments)
    {
        *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        result->NoOfOutputArguments = 0;
        return;
    }
    result->NoOfOutputArguments = 0; /* number of copied elements */
    *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
}

void msg_call_method_bs__alloc_CallMethod_Result(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const t_entier4 msg_call_method_bs__nb,
    constants_statuscodes_bs__t_StatusCode_i* const msg_call_method_bs__statusCode)
{
    assert(NULL != msg_call_method_bs__statusCode);
    assert(msg_call_method_bs__nb > 0);

    OpcUa_CallResponse* response = msg_call_method_bs__getCallResponse(msg_call_method_bs__p_res_msg);

    response->Results = SOPC_Calloc((size_t) msg_call_method_bs__nb, sizeof(OpcUa_CallMethodResult));

    if (NULL == response->Results)
    {
        *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        response->NoOfResults = 0;
        return;
    }

    *msg_call_method_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
    response->NoOfResults = msg_call_method_bs__nb;
    for (int i = 0; i < msg_call_method_bs__nb; i++)
    {
        OpcUa_CallMethodResult_Initialize(&response->Results[i]);
    }
}

void msg_call_method_bs__free_CallMethod_Res_InputArgument(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod)
{
    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);

    SOPC_Free(result->InputArgumentResults);
    result->InputArgumentResults = NULL;
    result->NoOfInputArgumentResults = 0;
}

void msg_call_method_bs__free_CallMethod_Res_OutputArgument(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod)
{
    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);
    for (int i = 0; i < result->NoOfOutputArguments; i++)
    {
        SOPC_Variant_Clear(&result->OutputArguments[i]);
    }
    SOPC_Free(result->OutputArguments);
    result->OutputArguments = NULL;
    result->NoOfOutputArguments = 0;
}

void msg_call_method_bs__read_CallMethod_InputArguments(
    const constants__t_msg_i msg_call_method_bs__p_req_msg,
    const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
    const t_entier4 msg_call_method_bs__p_index_arg,
    constants__t_Variant_i* const msg_call_method_bs__p_arg)
{
    assert(NULL != msg_call_method_bs__p_arg);

    OpcUa_CallMethodRequest* method =
        msg_call_method_bs__getCallMethod(msg_call_method_bs__p_req_msg, msg_call_method_bs__p_callMethod);
    /* ensured by B model */
    assert(0 < msg_call_method_bs__p_index_arg && msg_call_method_bs__p_index_arg <= method->NoOfInputArguments);

    *msg_call_method_bs__p_arg = &method->InputArguments[msg_call_method_bs__p_index_arg - 1];
}

void msg_call_method_bs__read_CallMethod_MethodId(const constants__t_msg_i msg_call_method_bs__p_req_msg,
                                                  const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
                                                  constants__t_NodeId_i* const msg_call_method_bs__p_methodid)
{
    assert(NULL != msg_call_method_bs__p_methodid);
    OpcUa_CallMethodRequest* method =
        msg_call_method_bs__getCallMethod(msg_call_method_bs__p_req_msg, msg_call_method_bs__p_callMethod);
    *msg_call_method_bs__p_methodid = &method->MethodId;
}

void msg_call_method_bs__read_CallMethod_Nb_InputArguments(
    const constants__t_msg_i msg_call_method_bs__p_req_msg,
    const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
    t_entier4* const msg_call_method_bs__p_nb)
{
    assert(NULL != msg_call_method_bs__p_nb);

    OpcUa_CallMethodRequest* method =
        msg_call_method_bs__getCallMethod(msg_call_method_bs__p_req_msg, msg_call_method_bs__p_callMethod);
    *msg_call_method_bs__p_nb = method->NoOfInputArguments;
}
void msg_call_method_bs__read_CallMethod_Objectid(const constants__t_msg_i msg_call_method_bs__p_req_msg,
                                                  const constants__t_CallMethod_i msg_call_method_bs__p_callMethod,
                                                  constants__t_NodeId_i* const msg_call_method_bs__p_objectid)
{
    assert(NULL != msg_call_method_bs__p_objectid);
    OpcUa_CallMethodRequest* method =
        msg_call_method_bs__getCallMethod(msg_call_method_bs__p_req_msg, msg_call_method_bs__p_callMethod);
    *msg_call_method_bs__p_objectid = &method->ObjectId;
}

void msg_call_method_bs__read_call_method_request(
    const constants__t_msg_i msg_call_method_bs__p_req_msg,
    constants_statuscodes_bs__t_StatusCode_i* const msg_call_method_bs__Status,
    t_entier4* const msg_call_method_bs__p_nb)
{
    *msg_call_method_bs__Status = constants_statuscodes_bs__e_sc_ok;
    msg_call_method_bs__read_nb_CallMethods(msg_call_method_bs__p_req_msg, msg_call_method_bs__p_nb);
}

void msg_call_method_bs__read_nb_CallMethods(const constants__t_msg_i msg_call_method_bs__p_req_msg,
                                             t_entier4* const msg_call_method_bs__p_nb)
{
    assert(NULL != msg_call_method_bs__p_req_msg);
    assert(NULL != msg_call_method_bs__p_nb);
    OpcUa_CallRequest* request = msg_call_method_bs__getCallRequest(msg_call_method_bs__p_req_msg);
    *msg_call_method_bs__p_nb = request->NoOfMethodsToCall;
}

void msg_call_method_bs__write_CallMethod_Res_InputArgumentResult(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod,
    const t_entier4 msg_call_method_bs__index,
    const constants_statuscodes_bs__t_StatusCode_i msg_call_method_bs__statusCode)
{
    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);
    /* ensured by B model */
    assert(0 < msg_call_method_bs__index && msg_call_method_bs__index <= result->NoOfInputArgumentResults);
    util_status_code__B_to_C(msg_call_method_bs__statusCode,
                             &result->InputArgumentResults[msg_call_method_bs__index - 1]);
}

void msg_call_method_bs__write_CallMethod_Res_OutputArgument(
    const constants__t_msg_i msg_call_method_bs__p_res_msg,
    const constants__t_CallMethod_i msg_call_method_bs__callMethod,
    const t_entier4 msg_call_method_bs__index,
    const constants__t_Variant_i msg_call_method_bs__value)
{
    assert(NULL != msg_call_method_bs__value);
    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);
    /* ensured by B model */
    assert(0 < msg_call_method_bs__index && msg_call_method_bs__index == result->NoOfOutputArguments + 1);
    SOPC_Variant_Move(&result->OutputArguments[msg_call_method_bs__index - 1], msg_call_method_bs__value);
    result->NoOfOutputArguments++;
}

void msg_call_method_bs__write_CallMethod_Res_Status(const constants__t_msg_i msg_call_method_bs__p_res_msg,
                                                     const constants__t_CallMethod_i msg_call_method_bs__callMethod,
                                                     const constants__t_RawStatusCode msg_call_method_bs__rawStatusCode)
{
    OpcUa_CallMethodResult* result =
        msg_call_method_bs__getCallResult(msg_call_method_bs__p_res_msg, msg_call_method_bs__callMethod);

    result->StatusCode = msg_call_method_bs__rawStatusCode;
}
