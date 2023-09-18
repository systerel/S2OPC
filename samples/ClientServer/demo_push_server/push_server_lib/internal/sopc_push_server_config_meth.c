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

#include "sopc_certificate_group.h"
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
    SOPC_UNUSED_ARG(param);

    printf("Method CreateSigningRequest Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* cStrId = NULL;
    bool bIsValidType = false;
    bool bIsValidSubject = false;
    bool bFound = false;
    SOPC_CertGroupContext* pGroupCtx = NULL;
    SOPC_Variant* pVariant = NULL;
    /* Input variables */
    const SOPC_NodeId* pCertificateGroupId = NULL;
    const SOPC_NodeId* pCertificateTypeId = NULL;
    const SOPC_String* pSubjectName = NULL;
    SOPC_Boolean bRegeneratePrivateKey = false;
    /* Check input arguments */
    if ((5 != nbInputArgs) || (NULL == inputArgs))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_NodeId_Id != inputArgs[0].BuiltInTypeId || SOPC_NodeId_Id != inputArgs[1].BuiltInTypeId ||
        SOPC_String_Id != inputArgs[2].BuiltInTypeId || SOPC_Boolean_Id != inputArgs[3].BuiltInTypeId ||
        SOPC_ByteString_Id != inputArgs[4].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest: bad BuiltInTypeId arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Set input values */
    pCertificateGroupId = inputArgs[0].Value.NodeId;
    pCertificateTypeId = inputArgs[1].Value.NodeId;
    pSubjectName = &inputArgs[2].Value.String;
    bRegeneratePrivateKey = inputArgs[3].Value.Boolean;
    /* Retrieve the Certificate Group  */
    pGroupCtx = CertificateGroup_DictGet(pCertificateGroupId, &bFound);
    if (NULL == pGroupCtx && !bFound)
    {
        return OpcUa_BadInvalidArgument;
    }
    cStrId = CertificateGroup_GetStrNodeId(pGroupCtx);
    /* Check if the given certificate type belongs to the group */
    bIsValidType = CertificateGroup_CheckType(pGroupCtx, pCertificateTypeId);
    if (!bIsValidType)
    {
        char* cStrTypeId = SOPC_NodeId_ToCString(pCertificateTypeId);
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "PushSrvCfg:Method_CreateSigningRequest:CertGroup:%s: rcv invalid certificateTypeId : %s", cStrId,
            cStrTypeId);
        SOPC_Free(cStrTypeId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check the given subjectName */
    bIsValidSubject = CertificateGroup_CheckSubjectName(pGroupCtx, pSubjectName);
    if (!bIsValidSubject)
    {
        const char* toPrint = SOPC_String_GetRawCString(pSubjectName);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest:CertGroup:%s: rcv invalid subjectName : %s",
                               cStrId, toPrint);
        return OpcUa_BadInvalidArgument;
    }
    /* Create the output variant */
    pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest: unable to create a variant");
        return OpcUa_BadUnexpectedError;
    }
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_ByteString_Id;
    SOPC_ByteString_Initialize(&pVariant->Value.Bstring);
    /* Discard the previous new key */
    CertificateGroup_DiscardNewKey(pGroupCtx);
    /* Create the signing request */
    status =
        CertificateGroup_CreateSigningRequest(pGroupCtx, pSubjectName, bRegeneratePrivateKey, &pVariant->Value.Bstring);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "PushSrvCfg:Method_CreateSigningRequest: failed to set up the CSR");
        statusCode = OpcUa_BadUnexpectedError;
    }
    if (SOPC_IsGoodStatus(statusCode))
    {
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        SOPC_Variant_Delete(pVariant);
    }

    return statusCode;
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
