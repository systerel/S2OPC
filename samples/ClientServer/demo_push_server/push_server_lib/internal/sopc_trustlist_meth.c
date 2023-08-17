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
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);

    printf("Method OpenWithMasks Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* cStrId = NULL;
    bool found = false;
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_Variant* pVariant = NULL;

    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:OpenWithMasks: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &found);
    if (NULL == pTrustList && !found)
    {
        return OpcUa_BadNotFound;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* TrustList already open ? */
    bool isOpen = TrustList_CheckIsOpen(pTrustList);
    if (isOpen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMasks: already open", cStrId);
        return OpcUa_BadInvalidState;
    }
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMasks: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Set mode */
    bool isValid = TrustList_SetOpenMasks(pTrustList, inputArgs[0].Value.Uint32);
    if (!isValid)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:OpenWithMask: invalid rcv masks %" PRIu32,
                               cStrId, inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    isValid = TrustList_SetOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ);
    SOPC_ASSERT(isValid);
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
        pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
        pVariant->BuiltInTypeId = SOPC_UInt32_Id;
        SOPC_UInt32_Initialize(&pVariant->Value.Uint32);
        pVariant->Value.Uint32 = TrustList_GetHandle(pTrustList);
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        TrustList_Reset(pTrustList);
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

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* cStrId = NULL;
    bool found = false;
    SOPC_TrustListContext* pTrustList = NULL;
    SOPC_Variant* pVariant = NULL;

    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:Open: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &found);
    if (!found || NULL == pTrustList)
    {
        return OpcUa_BadNotFound;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* TrustList already open ? */
    bool isOpen = TrustList_CheckIsOpen(pTrustList);
    if (isOpen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: already open", cStrId);
        /* Deviation from the OPC UA specification: an opening followed by a closing, otherwise the file is deleted.*/
        return OpcUa_BadInvalidState;
    }
    /* Check type of input arguments */
    if (SOPC_Byte_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Set mode */
    bool isValid = TrustList_SetOpenMode(pTrustList, inputArgs[0].Value.Byte);
    if (!isValid)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Open: invalid rcv masks %" PRIu8, cStrId,
                               inputArgs[0].Value.Byte);
        return OpcUa_BadInvalidArgument;
    }
    isValid = TrustList_SetOpenMasks(pTrustList, SOPC_TL_MASK_ALL);
    SOPC_ASSERT(isValid);
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
        pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
        pVariant->BuiltInTypeId = SOPC_UInt32_Id;
        SOPC_UInt32_Initialize(&pVariant->Value.Uint32);
        pVariant->Value.Uint32 = TrustList_GetHandle(pTrustList);
        *nbOutputArgs = 1;
        *outputArgs = pVariant;
    }
    else
    {
        TrustList_Reset(pTrustList);
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

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    const char* cStrId = NULL;
    bool found = false;
    SOPC_TrustListContext* pTrustList = NULL;

    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:Close: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &found);
    if (NULL == pTrustList && !found)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Close: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    bool match = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!match)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Close: invalid rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Close => Reset the context */
    TrustList_Reset(pTrustList);

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
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method CloseAndUpdate Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:Method_CloseAndUpdate: unable to create a variant");
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

SOPC_StatusCode TrustList_Method_AddCertificate(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    printf("Method AddCertificate Call\n");

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    const char* cStrId = NULL;
    bool found = false;
    SOPC_TrustListContext* pTrustList = NULL;

    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId) || (NULL == callContextPtr))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:AddCertificate: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &found);
    if (!found || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
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
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

    printf("Method RemoveCertificate Call\n");

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:Method_RemoveCertificate: unable to create a variant");
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

SOPC_StatusCode TrustList_Method_Read(const SOPC_CallContext* callContextPtr,
                                      const SOPC_NodeId* objectId,
                                      uint32_t nbInputArgs,
                                      const SOPC_Variant* inputArgs,
                                      uint32_t* nbOutputArgs,
                                      SOPC_Variant** outputArgs,
                                      void* param)
{
    printf("Method Read Call\n");

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool match = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool foundTrustList = false;
    int32_t reqLen = -1;
    SOPC_Variant* pVariant = NULL;

    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:Read: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &foundTrustList);
    if (!foundTrustList || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Int32_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    match = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!match)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Read: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode */
    match = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "Read: File was not opened for read access");
    if (!match)
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

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool match = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool foundTrustList = false;

    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:Write: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &foundTrustList);
    if (!foundTrustList || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_ByteString_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: bad BuiltInTypeId arguments", cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    match = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!match)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Write: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode */
    match = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING,
                                    "Write: File was not opened for write access");
    if (!match)
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

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    bool match = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool foundTrustList = false;
    SOPC_Variant* pVariant = NULL;

    /* Check input arguments */
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:GetPosition: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &foundTrustList);
    if (!foundTrustList || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check type of input arguments */
    if (SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    match = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!match)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:GetPosition: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode (GetPosition for write mode with erase if existing option has no meaning) */
    match =
        TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "GetPosition: File was not opened for read access");
    if (!match)
    {
        return OpcUa_BadInvalidState;
    }
    /* Create the variant */
    pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:GetPosition: unable to create a variant");
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

    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);
    /* The list of output argument shall be empty if the statusCode Severity is Bad
       (Table 65 – Call Service Parameters spec V1.05) */
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    /* Variable initialization */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool match = false;
    const char* cStrId = NULL;
    SOPC_TrustListContext* pTrustList = NULL;
    bool foundTrustList = false;

    /* Check input arguments */
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:SetPosition: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    /* Retrieve the TrustList */
    pTrustList = TrustList_DictGet(objectId, &foundTrustList);
    if (!foundTrustList || NULL == pTrustList)
    {
        return OpcUa_BadUnexpectedError;
    }
    cStrId = TrustList_GetStrNodeId(pTrustList);
    /* Check type of input arguments */
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_UInt64_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: bad BuiltInTypeId arguments",
                               cStrId);
        return OpcUa_BadInvalidArgument;
    }
    /* Check handle */
    match = TrustList_CheckHandle(pTrustList, inputArgs[0].Value.Uint32, NULL);
    if (!match)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SetPosition: rcv handle: %" PRIu32, cStrId,
                               inputArgs[0].Value.Uint32);
        return OpcUa_BadInvalidArgument;
    }
    /* Check mode (SetPosition for write mode with erase if existing option has no meaning) */
    match =
        TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, "SetPosition: File was not opened for read access");
    if (!match)
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
