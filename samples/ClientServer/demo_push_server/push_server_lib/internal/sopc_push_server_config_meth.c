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
 * \brief Internal API implementation to manage methods of the ServerConfigurationType according the Push model.
 */

#include <stdio.h>

#include "sopc_push_server_config.h"
#include "sopc_push_server_config_meth.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

SOPC_StatusCode PushSrvCfg_Method_UpdateCertificate(const SOPC_CallContext* callContextPtr,
                                                    const SOPC_NodeId* objectId,
                                                    uint32_t nbInputArgs,
                                                    const SOPC_Variant* inputArgs,
                                                    uint32_t* nbOutputArgs,
                                                    SOPC_Variant** outputArgs,
                                                    void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method UpdateCertificate Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_UpdateCertificate: unable to create a variant");
        return OpcUa_BadUnexpectedError;
    }

    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    v->BuiltInTypeId = SOPC_Boolean_Id;
    SOPC_Boolean_Initialize(&v->Value.Boolean);
    v->Value.Boolean = false;
    *nbOutputArgs = 1;
    *outputArgs = v;

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode PushSrvCfg_Method_ApplyChanges(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method ApplyChanges Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    return OpcUa_BadNothingToDo;
}

SOPC_StatusCode PushSrvCfg_Method_CreateSigningRequest(const SOPC_CallContext* callContextPtr,
                                                       const SOPC_NodeId* objectId,
                                                       uint32_t nbInputArgs,
                                                       const SOPC_Variant* inputArgs,
                                                       uint32_t* nbOutputArgs,
                                                       SOPC_Variant** outputArgs,
                                                       void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method CreateSigningRequest Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest: unable to create a variant");
        return OpcUa_BadUnexpectedError;
    }
    SOPC_ByteString_Initialize(&v->Value.Bstring);
    v->Value.Bstring.Data = NULL;
    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    v->BuiltInTypeId = SOPC_ByteString_Id;
    *nbOutputArgs = 1;
    *outputArgs = v;

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode PushSrvCfg_Method_GetRejectedList(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method GetRejectedList Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    uint32_t lenArray = 0;
    SOPC_ByteString* pBsCertArray = NULL;
    SOPC_Variant* pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "PushSrvCfg:GetRejectedList: unable to create a variant");
        return OpcUa_BadUnexpectedError;
    }
    /* Get the rejected list */
    SOPC_StatusCode stCode = PushServer_GetRejectedList(&pBsCertArray, &lenArray);
    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:GetRejectedList: unable to get the rejected list");
        stCode = OpcUa_BadUnexpectedError;
    }
    else if (INT32_MAX < lenArray)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "PushSrvCfg:GetRejectedList: rejected list si too large");
        stCode = OpcUa_BadUnexpectedError;
    }
    else
    {
        /* Export */
        stCode = PushServer_ExportRejectedList(false);
        if (!SOPC_IsGoodStatus(stCode))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "PushSrvCfg:GetRejectedList: unable to export the rejected list");
        }
    }
    /* Set the output */
    if (SOPC_IsGoodStatus(stCode))
    {
        pVariant->ArrayType = SOPC_VariantArrayType_Array;
        pVariant->BuiltInTypeId = SOPC_ByteString_Id;
        pVariant->Value.Array.Content.BstringArr = pBsCertArray;
        pVariant->Value.Array.Length = (int32_t) lenArray;
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        for (uint32_t idx = 0; idx < lenArray && NULL != pBsCertArray; idx++)
        {
            SOPC_ByteString_Clear(&pBsCertArray[idx]);
        }
        SOPC_Free(pBsCertArray);
        SOPC_Variant_Delete(pVariant);
    }

    return stCode;
}