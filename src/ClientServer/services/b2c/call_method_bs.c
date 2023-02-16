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

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void call_method_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void call_method_bs__exec_callMethod(const constants__t_endpoint_config_idx_i call_method_bs__p_endpoint_config_idx,
                                     const constants__t_CallMethodPointer_i call_method_bs__p_call_method_pointer,
                                     constants__t_RawStatusCode* const call_method_bs__p_rawStatusCode,
                                     t_entier4* const call_method_bs__p_nb_out,
                                     constants__t_ArgumentsPointer_i* const call_method_bs__p_out_arguments)
{
    *call_method_bs__p_nb_out = 0;
    *call_method_bs__p_out_arguments = NULL;
    OpcUa_CallMethodRequest* methodToCall = call_method_bs__p_call_method_pointer;
    assert(NULL != methodToCall);

    /* Get the Method Call Manager from server configuration */
    SOPC_Endpoint_Config* endpoint_config = SOPC_ToolkitServer_GetEndpointConfig(call_method_bs__p_endpoint_config_idx);
    if (NULL == endpoint_config || NULL == endpoint_config->serverConfigPtr)
    {
        *call_method_bs__p_rawStatusCode = OpcUa_BadInternalError;
        return;
    }
    SOPC_MethodCallManager* mcm = endpoint_config->serverConfigPtr->mcm;
    if (NULL == mcm || NULL == mcm->pFnGetMethod)
    {
        *call_method_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    /* Get the C function corresponding to the method */
    SOPC_NodeId* methodId = &methodToCall->MethodId;
    SOPC_MethodCallFunc* method_c = mcm->pFnGetMethod(mcm, methodId);
    if (NULL == method_c)
    {
        *call_method_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }

    SOPC_NodeId* objectId = &methodToCall->ObjectId;
    uint32_t nbInputArgs = (0 < methodToCall->NoOfInputArguments) ? (uint32_t) methodToCall->NoOfInputArguments
                                                                  : 0; /* convert to avoid compilator error */
    SOPC_Variant* inputArgs = methodToCall->InputArguments;
    uint32_t noOfOutput = 0;
    SOPC_Variant* outputArgs = NULL;
    SOPC_CallContext* cc = SOPC_CallContext_Copy(SOPC_CallContext_GetCurrent());
    *call_method_bs__p_rawStatusCode =
        method_c->pMethodFunc(cc, objectId, nbInputArgs, inputArgs, &noOfOutput, &outputArgs, method_c->pParam);
    SOPC_CallContext_Free(cc);
    if (0 != noOfOutput && NULL == outputArgs)
    {
        char* mNodeId = SOPC_NodeId_ToCString(methodId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "MethodCall %s unexpected failure: application variant array result is NULL which is "
                               "not expected when noOfOutputs (%" PRIu32 ") > 0",
                               mNodeId, noOfOutput);
        SOPC_Free(mNodeId);
        *call_method_bs__p_rawStatusCode = OpcUa_BadNotImplemented;
        return;
    }
    if (noOfOutput > INT32_MAX)
    {
        noOfOutput = INT32_MAX;
        // Note: normally used for input arguments but it is the better match and should not occur
        *call_method_bs__p_rawStatusCode = OpcUa_BadTooManyArguments;
    }
    if (SOPC_IsGoodStatus(*call_method_bs__p_rawStatusCode))
    {
        *call_method_bs__p_nb_out = (int32_t) noOfOutput;
        *call_method_bs__p_out_arguments = outputArgs;
    }
    else
    {
        int32_t nbElts = (int32_t) noOfOutput;
        SOPC_Clear_Array(&nbElts, (void**) &outputArgs, sizeof(SOPC_Variant), SOPC_Variant_ClearAux);
    }
}
