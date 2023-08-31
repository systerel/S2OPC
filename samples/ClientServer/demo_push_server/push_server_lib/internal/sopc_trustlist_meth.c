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
 * \brief Internal API implementation to manage methods of the TrustListType according the Push model.
 */

#include <inttypes.h>
#include <stdio.h>

#include "sopc_trustlist.h"
#include "sopc_trustlist_meth.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

SOPC_StatusCode TrustList_Method_OpenWithMasks(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param)
{
    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(param);

    printf("Method OpenWithMasks Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* cStrId = NULL;
    bool bFound = false;
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_Variant* pVariant = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (NULL == pTrustList && !bFound)
    {
        return OpcUa_BadNotFound;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* TrustList already open ? */
    bool bIsOpen = TrustList_CheckIsOpen(pTrustList);
    if (bIsOpen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMasks: already open", cStrId);
        return OpcUa_BadInvalidState;
    }
    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMasks: bad inputs arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMasks: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Set mode */
    bool bIsValid = TrustList_SetOpenMasks(pTrustList, inputArgs[0].Value.Uint32);
    if (!bIsValid)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMask: invalid rcv masks %" PRIu32,
                               cStrId, inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    bIsValid = TrustList_SetOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ);
    SOPC_ASSERT(bIsValid);
    /* Encode the TrustList */
    status = TrustList_Encode(pTrustList);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:OpenWithMask: failed to encode the trustList", cStrId);
        statusCode = OpcUa_BadUnexpectedError;
    }
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        /* Create the output variant */
        pVariant = SOPC_Variant_Create();
        if (NULL == pVariant)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:OpenWithMask: unable to create a variant", cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Generate and set the handle */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        status = TrustList_GenRandHandle(pTrustList);
        statusCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    }
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        SOPC_AddressSpaceAccess* pAddSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
        Trustlist_WriteVarSize(pTrustList, pAddSpAccess);
        Trustlist_WriteVarOpenCount(pTrustList, pAddSpAccess);
        pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
        pVariant->BuiltInTypeId = SOPC_UInt32_Id;
        SOPC_UInt32_Initialize(&pVariant->Value.Uint32);
        pVariant->Value.Uint32 = TrustList_GetHandle(pTrustList);
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        TrustList_Reset(pTrustList, NULL);
        SOPC_Variant_Delete(pVariant);
    }

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode TrustList_Method_Open(const SOPC_CallContext* callContextPtr,
                                      const SOPC_NodeId* objectId,
                                      uint32_t nbInputArgs,
                                      const SOPC_Variant* inputArgs,
                                      uint32_t* nbOutputArgs,
                                      SOPC_Variant** outputArgs,
                                      void* param)
{
    printf("Method Open Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* cStrId = NULL;
    bool bFound = false;
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_Variant* pVariant = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadNotFound;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* TrustList already open ? */
    bool bIsOpen = TrustList_CheckIsOpen(pTrustList);
    if (bIsOpen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: already open", cStrId);
        /* Deviation from the OPC UA specification: an opening followed by a closing, otherwise the file is deleted.*/
        return OpcUa_BadInvalidState;
    }
    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_Byte_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Set mode */
    bool bIsValid = TrustList_SetOpenMode(pTrustList, inputArgs[0].Value.Byte);
    if (!bIsValid)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: invalid rcv mode %" PRIu8, cStrId,
                               inputArgs[0].Value.Byte);
        return OpcUa_BadInvalidArgument;
    }
    bIsValid = TrustList_SetOpenMasks(pTrustList, SOPC_TL_MASK_ALL);
    SOPC_ASSERT(bIsValid);
    /* Encode the TrustList */
    status = TrustList_Encode(pTrustList);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: failed to encode the trustList",
                               cStrId);
        statusCode = OpcUa_BadUnexpectedError;
    }
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        /* Create the output variant */
        pVariant = SOPC_Variant_Create();
        if (NULL == pVariant)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: unable to create a variant",
                                   cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Generate and set the random handle */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        status = TrustList_GenRandHandle(pTrustList);
        statusCode = SOPC_STATUS_OK == status ? SOPC_GoodGenericStatus : OpcUa_BadUnexpectedError;
    }
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        SOPC_AddressSpaceAccess* pAddSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
        Trustlist_WriteVarSize(pTrustList, pAddSpAccess);
        Trustlist_WriteVarOpenCount(pTrustList, pAddSpAccess);
        pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
        pVariant->BuiltInTypeId = SOPC_UInt32_Id;
        SOPC_UInt32_Initialize(&pVariant->Value.Uint32);
        pVariant->Value.Uint32 = TrustList_GetHandle(pTrustList);
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        TrustList_Reset(pTrustList, NULL);
        SOPC_Variant_Delete(pVariant);
    }
    return statusCode;
}

SOPC_StatusCode TrustList_Method_Close(const SOPC_CallContext* callContextPtr,
                                       const SOPC_NodeId* objectId,
                                       uint32_t nbInputArgs,
                                       const SOPC_Variant* inputArgs,
                                       uint32_t* nbOutputArgs,
                                       SOPC_Variant** outputArgs,
                                       void* param)
{
    printf("Method Close Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    const char* cStrId = NULL;
    bool bFound = false;
    SOPC_TrustListContext* pTrustList = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (NULL == pTrustList && !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Close: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Close: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bool bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Close: invalid rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Close => Reset the context */
    SOPC_AddressSpaceAccess* pAddSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    TrustList_Reset(pTrustList, pAddSpAccess);

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode TrustList_Method_CloseAndUpdate(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    printf("Method CloseAndUpdate Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    const char* cStrId = NULL;
    bool bFound = false;
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_Variant* pVariant = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (NULL == pTrustList && !bFound)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: bad inputs arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bool bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:CloseAndUpdate: invalid rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Create the output variant */
    pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: unable to create a variant",
                               cStrId);
        return OpcUa_BadUnexpectedError;
    }
    /* The Server does not support transactions it applies the changes immediately and sets
       applyChangesRequired to FALSE.*/
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_Boolean_Id;
    SOPC_Boolean_Initialize(&pVariant->Value.Boolean);
    pVariant->Value.Boolean = false;

    /* Verify and Update the TrustList */
    const char* secPolUri = SOPC_CallContext_GetSecurityPolicy(callContextPtr);
    statusCode = TrustList_WriteUpdate(pTrustList, secPolUri);
    /* Export the update (certificate files) */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        status = TrustList_Export(pTrustList, false, false);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:CloseAndUpdate: failed to export the new TrustList", cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    /* Raise an event to re-evaluate the certificate  */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        status = TrustList_RaiseEvent(pTrustList);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: event failed", cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    if (0 == (statusCode & SOPC_GoodGenericStatus))
    {
        /* The variant is deleted by the method call manager */
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        SOPC_Variant_Delete(pVariant);
    }
    /* Close => Reset the context */
    SOPC_AddressSpaceAccess* pAddSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    TrustList_Reset(pTrustList, pAddSpAccess);

    return statusCode;
}

SOPC_StatusCode TrustList_Method_AddCertificate(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    printf("Method AddCertificate Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    const char* cStrId = NULL;
    bool bFound = false;
    SOPC_TrustListContext* pTrustList = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:AddCertificate: bad inputs arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* This Method returns Bad_NotWritable if the TrustList Object is read only */
    bool isReadMode = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, NULL);
    if (isReadMode)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:AddCertificate: is open in read mode",
                               cStrId);
        return OpcUa_BadNotWritable;
    }
    /* The Open Method was called with write access and the CloseAndUpdate Method has not been called */
    bool isWriteMode = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING, NULL);
    if (isWriteMode)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "TrustList:%s:AddCertificate: is open in write mode and the CloseAndUpdate has not been called", cStrId);
        return OpcUa_BadInvalidState;
    }
    /* Check type of input arguments */
    if (SOPC_ByteString_Id != inputArgs[0].BuiltInTypeId || SOPC_Boolean_Id != inputArgs[1].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:AddCertificate: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* This Method cannot provide CRLs so issuer Certificates cannot be added with this Method */
    if (false == inputArgs[1].Value.Boolean)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:AddCertificate: issuer certificates cannot be added", cStrId);
        return OpcUa_BadCertificateInvalid;
    }
    /* Validate the certificate and update the PKI that belongs to the TrustList */
    const char* secPolUri = SOPC_CallContext_GetSecurityPolicy(callContextPtr);
    statusCode = TrustList_AddUpdate(pTrustList, &inputArgs[0].Value.Bstring, secPolUri);
    /* Export the update (certificate files) */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        SOPC_ReturnStatus status = TrustList_Export(pTrustList, false, true);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:AddCertificate: failed to export the new TrustList", cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }

    return statusCode;
}

SOPC_StatusCode TrustList_Method_RemoveCertificate(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param)
{
    printf("Method RemoveCertificate Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool bFound = false;
    bool bIsRemove = false;
    bool bIsIssuer = false;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:RemoveCertificate: bad inputs arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if ((SOPC_String_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Boolean_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:RemoveCertificate: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* This Method returns Bad_NotWritable if the TrustList Object is read only */
    bool isReadMode = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, NULL);
    if (isReadMode)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:RemoveCertificate: is open in read mode",
                               cStrId);
        return OpcUa_BadNotWritable;
    }
    /* The Open Method was called with write access and the CloseAndUpdate Method has not been called */
    bool isWriteMode = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING, NULL);
    if (isWriteMode)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "TrustList:%s:RemoveCertificate: is open in write mode and the CloseAndUpdate has not been called", cStrId);
        return OpcUa_BadInvalidState;
    }
    /* Finally remove the certificate with its thumbprint */
    const char* secPolUri = SOPC_CallContext_GetSecurityPolicy(callContextPtr);
    statusCode = TrustList_RemoveCert(pTrustList, (const SOPC_String*) &inputArgs[0].Value.Bstring,
                                      inputArgs[1].Value.Boolean, secPolUri, &bIsRemove, &bIsIssuer);
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        if (!bIsRemove)
        {
            statusCode = OpcUa_BadInvalidArgument;
        }
    }
    /* Export the update (certificate files) */
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        status = TrustList_Export(pTrustList, true, true);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:RemoveCertificate: failed to export the new TrustList", cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    if (bIsIssuer && 0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        /* Raise an event to re-evaluate the certificate  */
        status = TrustList_RaiseEvent(pTrustList);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:RemoveCertificate: event failed",
                                   cStrId);
            statusCode = OpcUa_BadUnexpectedError;
        }
    }
    return statusCode;
}

SOPC_StatusCode TrustList_Method_Read(const SOPC_CallContext* callContextPtr,
                                      const SOPC_NodeId* objectId,
                                      uint32_t nbInputArgs,
                                      const SOPC_Variant* inputArgs,
                                      uint32_t* nbOutputArgs,
                                      SOPC_Variant** outputArgs,
                                      void* param)
{
    printf("Method Read Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bMatch = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool bFound = false;
    int32_t reqLen = -1;
    SOPC_Variant* pVariant = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Int32_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode */
    bMatch = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "Read: File was not opened for read access");
    if (!bMatch)
    {
        return OpcUa_BadInvalidState;
    }
    /* Check length (only positive values are allowed) */
    reqLen = inputArgs[1].Value.Int32;
    if (0 >= reqLen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: Invalid rcv length: %" PRId32, cStrId,
                               reqLen);
        return OpcUa_BadInvalidArgument;
    }
    /* Create the output variant */
    pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: unable to create a variant", cStrId);
        return OpcUa_BadUnexpectedError;
    }
    /* Initialize the byte string to return */
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_ByteString_Id;
    SOPC_ByteString_Initialize(&pVariant->Value.Bstring);
    /* Read the TrustList */
    status = TrustList_Read(pTrustList, reqLen, &pVariant->Value.Bstring);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:Read: failed to read the Ua Binary encoded trustlist", cStrId);
        statusCode = OpcUa_BadUnexpectedError;
    }
    if (0 == (statusCode & SOPC_GoodStatusOppositeMask))
    {
        /* The variant is deleted by the method call manager */
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        /* Clear */
        pVariant->Value.Bstring.Data = NULL;
        pVariant->Value.Bstring.Length = -1;
        pVariant->Value.Bstring.DoNotClear = false;
        SOPC_Variant_Delete(pVariant);
    }
    return statusCode;
}

SOPC_StatusCode TrustList_Method_Write(const SOPC_CallContext* callContextPtr,
                                       const SOPC_NodeId* objectId,
                                       uint32_t nbInputArgs,
                                       const SOPC_Variant* inputArgs,
                                       uint32_t* nbOutputArgs,
                                       SOPC_Variant** outputArgs,
                                       void* param)
{
    printf("Method Write Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bMatch = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool bFound = false;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_ByteString_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode */
    bMatch = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING,
                                     "Write: File was not opened for write access");
    if (!bMatch)
    {
        return OpcUa_BadInvalidState;
    }
    /* Finally decode the TrustList */
    status = TrustList_Decode(pTrustList, (const SOPC_ByteString*) &inputArgs[1].Value.Bstring);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: failed to decode the trustList",
                               cStrId);
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode TrustList_Method_GetPosition(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param)
{
    printf("Method GetPosition Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    bool bMatch = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool bFound = false;
    SOPC_Variant* pVariant = NULL;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode (GetPosition for write mode with erase if existing option has no meaning) */
    bMatch =
        TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "GetPosition: File was not opened for read access");
    if (!bMatch)
    {
        return OpcUa_BadInvalidState;
    }
    /* Create the variant */
    pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: unable to create a variant",
                               cStrId);
        return OpcUa_BadUnexpectedError;
    }
    /* Finally get the position */
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_UInt64_Id;
    SOPC_UInt64_Initialize(&pVariant->Value.Uint64);
    pVariant->Value.Uint64 = TrustList_GetPosition(pTrustList);
    *nbOutputArgs = 1;
    *outputArgs = pVariant;

    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode TrustList_Method_SetPosition(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param)
{
    printf("Method SetPosition Call\n");

    SOPC_ASSERT(NULL != objectId);
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bMatch = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool bFound = false;

    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, true, callContextPtr, &bFound);
    if (!bFound || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: bad inputs arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_UInt64_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bMatch = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!bMatch)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode (SetPosition for write mode with erase if existing option has no meaning) */
    bMatch =
        TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "SetPosition: File was not opened for read access");
    if (!bMatch)
    {
        return OpcUa_BadInvalidState;
    }
    /* Finally set the position */
    status = TrustList_SetPosition(pTrustList, inputArgs[1].Value.Uint64);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: failed to set position",
                               cStrId);
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}
