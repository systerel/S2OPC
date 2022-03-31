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
#include <inttypes.h>

#include "app_cb_call_context_internal.h"
#include "call_method_bs.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#include "util_b2c.h"

/*
  Index in C code equals index from B model minus 1
*/

static struct call_method_bs__ExecResult
{
    int32_t nb;
    SOPC_Variant* variants;
} call_method_bs__execResults = {0, NULL};

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void call_method_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void call_method_bs__exec_callMethod(const constants__t_msg_i call_method_bs__p_req_msg,
                                     const constants__t_CallMethod_i call_method_bs__p_callMethod,
                                     const constants__t_endpoint_config_idx_i call_method_bs__p_endpoint_config_idx,
                                     constants__t_RawStatusCode* const call_method_bs__rawStatusCode)
{
    /* Do not call before the memory is freed */
    assert(0 == call_method_bs__execResults.nb && NULL == call_method_bs__execResults.variants);

    /* Get the Call Method Request from the message */
    /* Ensured by B model */
    assert(NULL != call_method_bs__p_req_msg);
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) call_method_bs__p_req_msg;
    assert(encType == &OpcUa_CallRequest_EncodeableType);
    OpcUa_CallRequest* callRequest = (OpcUa_CallRequest*) call_method_bs__p_req_msg;
    assert(0 < call_method_bs__p_callMethod && call_method_bs__p_callMethod <= callRequest->NoOfMethodsToCall);
    OpcUa_CallMethodRequest* methodToCall = &callRequest->MethodsToCall[call_method_bs__p_callMethod - 1];
    assert(NULL != methodToCall);

    /* Get the Method Call Manager from server configuration */
    SOPC_Endpoint_Config* endpoint_config = SOPC_ToolkitServer_GetEndpointConfig(call_method_bs__p_endpoint_config_idx);
    if (NULL == endpoint_config || NULL == endpoint_config->serverConfigPtr)
    {
        *call_method_bs__rawStatusCode = OpcUa_BadInternalError;
        return;
    }
    SOPC_MethodCallManager* mcm = endpoint_config->serverConfigPtr->mcm;
    if (NULL == mcm || NULL == mcm->pFnGetMethod)
    {
        *call_method_bs__rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    /* Get the C function corresponding to the method */
    SOPC_NodeId* methodId = &methodToCall->MethodId;
    SOPC_MethodCallFunc* method_c = mcm->pFnGetMethod(mcm, methodId);
    if (NULL == method_c)
    {
        *call_method_bs__rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    SOPC_NodeId* objectId = &methodToCall->ObjectId;
    uint32_t nbInputArgs = (0 < methodToCall->NoOfInputArguments) ? (uint32_t) methodToCall->NoOfInputArguments
                                                                  : 0; /* convert to avoid compilator error */
    SOPC_Variant* inputArgs = methodToCall->InputArguments;
    uint32_t noOfOutput = 0;
    SOPC_CallContext* cc = SOPC_CallContext_Copy(SOPC_CallContext_GetCurrent());
    *call_method_bs__rawStatusCode = method_c->pMethodFunc(cc, objectId, nbInputArgs, inputArgs, &noOfOutput,
                                                           &call_method_bs__execResults.variants, method_c->pParam);
    SOPC_CallContext_Free(cc);
    if (0 != noOfOutput && NULL == call_method_bs__execResults.variants)
    {
        char* mNodeId = SOPC_NodeId_ToCString(methodId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "MethodCall %s unexpected failure: application variant array result is NULL which is "
                               "not expected when noOfOutputs (%" PRIu32 ") > 0",
                               mNodeId, noOfOutput);
        SOPC_Free(mNodeId);
        *call_method_bs__rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }
    if (noOfOutput <= INT32_MAX)
    {
        call_method_bs__execResults.nb = (int32_t) noOfOutput;
    }
    else
    {
        call_method_bs__execResults.nb = INT32_MAX;
        *call_method_bs__rawStatusCode = OpcUa_BadQueryTooComplex;
    }
}

void call_method_bs__free_exec_result(void)
{
    if (NULL != call_method_bs__execResults.variants)
    {
        SOPC_Free(call_method_bs__execResults.variants);
        call_method_bs__execResults.variants = NULL;
    }
    call_method_bs__execResults.nb = 0;
}

void call_method_bs__read_exec_result(const t_entier4 call_method_bs__index,
                                      constants__t_Variant_i* const call_method_bs__value)
{
    assert(0 < call_method_bs__index && call_method_bs__index <= call_method_bs__execResults.nb);
    assert(NULL != call_method_bs__value);
    assert(NULL != call_method_bs__execResults.variants);
    *call_method_bs__value = &call_method_bs__execResults.variants[call_method_bs__index - 1];
}

void call_method_bs__read_nb_exec_result(t_entier4* const call_method_bs__nb)
{
    *call_method_bs__nb = call_method_bs__execResults.nb;
}
