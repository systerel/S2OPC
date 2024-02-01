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
 * \brief Interface implementation to manage the TrustListType.
 */
#include <inttypes.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_date_time.h"
#include "sopc_helper_encode.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_pki_struct_lib_internal.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_trustlist.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

#define SOPC_EMPTY_TRUSTLIST_ENCODED_BYTE_SIZE 20u
#define SOPC_LENGTH_BSTRING_ENCODED_BYTE_SIZE 4u
#define THUMBPRINT_LENGTH 40u

/*---------------------------------------------------------------------------
 *                             Internal types
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                             Global variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *                      Prototype of static functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_attach_certs_to_raw_arrays(const SOPC_TrLst_Mask specifiedLists,
                                                              const SOPC_CertificateList* pTrustedCerts,
                                                              SOPC_SerializedCertificate** pRawTrustedCertArray,
                                                              uint32_t* pLenTrustedCertArray,
                                                              const SOPC_CRLList* pTrustedCrls,
                                                              SOPC_SerializedCRL** pRawTrustedCrlArray,
                                                              uint32_t* pLenTrustedCrlArray,
                                                              const SOPC_CertificateList* pIssuerCerts,
                                                              SOPC_SerializedCertificate** pRawIssuerCertArray,
                                                              uint32_t* pLenIssuerCertArray,
                                                              const SOPC_CRLList* pIssuerCrls,
                                                              SOPC_SerializedCRL** pRawIssuerCrlArray,
                                                              uint32_t* pLenIssuerCrlArray);

static SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pArray,
                                                                uint32_t lenArray,
                                                                SOPC_ByteString** pByteStringArray,
                                                                bool bIsCRL,
                                                                uint32_t* pByteLenTot);

static SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(
    const SOPC_SerializedCertificate* pRawTrustedCertArray,
    const uint32_t lenTrustedCertArray,
    SOPC_ByteString** pBsTrustedCertArray,
    const SOPC_SerializedCRL* pRawTrustedCrlArray,
    const uint32_t lenTrustedCrlArray,
    SOPC_ByteString** pBsTrustedCrlArray,
    const SOPC_SerializedCertificate* pRawIssuerCertArray,
    const uint32_t lenIssuerCertArray,
    SOPC_ByteString** pBsIssuerCertArray,
    const SOPC_SerializedCRL* pRawIssuerCrlArray,
    const uint32_t lenIssuerCrlArray,
    SOPC_ByteString** pBsIssuerCrlArray,
    uint32_t* pByteLenTot);

static SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                               uint32_t length,
                                                               SOPC_CertificateList** ppCert);
static SOPC_ReturnStatus trustList_write_bs_array_to_crl_list(SOPC_ByteString* pArray,
                                                              uint32_t length,
                                                              SOPC_CRLList** ppCrl);
static SOPC_ReturnStatus trustlist_write_decoded_data(const OpcUa_TrustListDataType* pDecode,
                                                      SOPC_CertificateList** ppTrustedCerts,
                                                      SOPC_CRLList** ppTrustedCrls,
                                                      SOPC_CertificateList** ppIssuerCerts,
                                                      SOPC_CRLList** ppIssuerCrls);

static bool trustlist_is_valid_masks(SOPC_TrLst_Mask mask);
static SOPC_ReturnStatus gen_handle_with_retries(SOPC_TrustListContext* pTrustList, uint8_t depth);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_attach_certs_to_raw_arrays(const SOPC_TrLst_Mask specifiedLists,
                                                              const SOPC_CertificateList* pTrustedCerts,
                                                              SOPC_SerializedCertificate** pRawTrustedCertArray,
                                                              uint32_t* pLenTrustedCertArray,
                                                              const SOPC_CRLList* pTrustedCrls,
                                                              SOPC_SerializedCRL** pRawTrustedCrlArray,
                                                              uint32_t* pLenTrustedCrlArray,
                                                              const SOPC_CertificateList* pIssuerCerts,
                                                              SOPC_SerializedCertificate** pRawIssuerCertArray,
                                                              uint32_t* pLenIssuerCertArray,
                                                              const SOPC_CRLList* pIssuerCrls,
                                                              SOPC_SerializedCRL** pRawIssuerCrlArray,
                                                              uint32_t* pLenIssuerCrlArray)
{
    SOPC_ASSERT(NULL != pRawTrustedCertArray);
    SOPC_ASSERT(NULL != pRawTrustedCrlArray);
    SOPC_ASSERT(NULL != pRawIssuerCertArray);
    SOPC_ASSERT(NULL != pRawIssuerCrlArray);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Initialize the lengths */
    *pLenTrustedCertArray = 0;
    *pLenTrustedCrlArray = 0;
    *pLenIssuerCertArray = 0;
    *pLenIssuerCrlArray = 0;

    if (specifiedLists & SOPC_TL_MASK_TRUSTED_CERTS)
    {
        if (NULL != pTrustedCerts)
        {
            status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pTrustedCerts, pRawTrustedCertArray,
                                                                             pLenTrustedCertArray);
        }
    }
    if ((SOPC_STATUS_OK == status) && (specifiedLists & SOPC_TL_MASK_TRUSTED_CRLS))
    {
        if (NULL != pTrustedCrls)
        {
            status =
                SOPC_KeyManager_CRLList_AttachToSerializedArray(pTrustedCrls, pRawTrustedCrlArray, pLenTrustedCrlArray);
        }
    }
    if ((SOPC_STATUS_OK == status) && (specifiedLists & SOPC_TL_MASK_ISSUER_CERTS))
    {
        if (NULL != pIssuerCerts)
        {
            status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pIssuerCerts, pRawIssuerCertArray,
                                                                             pLenIssuerCertArray);
        }
    }
    if ((SOPC_STATUS_OK == status) && (specifiedLists & SOPC_TL_MASK_ISSUER_CRLS))
    {
        if (NULL != pIssuerCrls)
        {
            status =
                SOPC_KeyManager_CRLList_AttachToSerializedArray(pIssuerCrls, pRawIssuerCrlArray, pLenIssuerCrlArray);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(*pRawTrustedCertArray);
        SOPC_Free(*pRawTrustedCrlArray);
        SOPC_Free(*pRawIssuerCertArray);
        SOPC_Free(*pRawIssuerCrlArray);
        *pRawTrustedCertArray = NULL;
        *pRawTrustedCrlArray = NULL;
        *pRawIssuerCertArray = NULL;
        *pRawIssuerCrlArray = NULL;
        *pLenTrustedCertArray = 0;
        *pLenTrustedCrlArray = 0;
        *pLenIssuerCertArray = 0;
        *pLenIssuerCrlArray = 0;
    }

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pGenArray,
                                                                uint32_t lenArray,
                                                                SOPC_ByteString** pByteStringArray,
                                                                bool bIsCRL,
                                                                uint32_t* pByteLenTot)
{
    if (NULL == pGenArray || 0 == lenArray || NULL == pByteStringArray || NULL == pByteLenTot)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ByteString* pBsArray = NULL;
    const SOPC_SerializedCertificate* pRawCertArray = NULL;
    const SOPC_SerializedCRL* pRawCrlArray = NULL;
    const SOPC_Buffer* pRawBuffer = NULL;
    SOPC_ByteString* pByteString = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (bIsCRL)
    {
        pRawCrlArray = (const SOPC_SerializedCRL*) pGenArray;
    }
    else
    {
        pRawCertArray = (const SOPC_SerializedCertificate*) pGenArray;
    }
    pBsArray = SOPC_Calloc(lenArray, sizeof(SOPC_ByteString));
    if (NULL == pBsArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < lenArray && SOPC_STATUS_OK == status; i++)
    {
        if (bIsCRL)
        {
            pRawBuffer = SOPC_KeyManager_SerializedCRL_Data(&pRawCrlArray[i]);
        }
        else
        {
            pRawBuffer = SOPC_KeyManager_SerializedCertificate_Data(&pRawCertArray[i]);
        }
        if (NULL == pRawBuffer)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* Check before casting */
        if (SOPC_STATUS_OK == status)
        {
            if (INT32_MAX < pRawBuffer->length)
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            pByteString = &pBsArray[i];
            SOPC_ByteString_Initialize(pByteString);
            pByteString->Data = pRawBuffer->data; // Attach data
            pByteString->Length = (int32_t) pRawBuffer->length;
            pByteString->DoNotClear = true;
            *pByteLenTot = *pByteLenTot + pRawBuffer->length;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pBsArray);
        pBsArray = NULL;
        *pByteLenTot = 0;
    }

    *pByteStringArray = pBsArray;

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(
    const SOPC_SerializedCertificate* pRawTrustedCertArray,
    const uint32_t lenTrustedCertArray,
    SOPC_ByteString** pBsTrustedCertArray,
    const SOPC_SerializedCRL* pRawTrustedCrlArray,
    const uint32_t lenTrustedCrlArray,
    SOPC_ByteString** pBsTrustedCrlArray,
    const SOPC_SerializedCertificate* pRawIssuerCertArray,
    const uint32_t lenIssuerCertArray,
    SOPC_ByteString** pBsIssuerCertArray,
    const SOPC_SerializedCRL* pRawIssuerCrlArray,
    const uint32_t lenIssuerCrlArray,
    SOPC_ByteString** pBsIssuerCrlArray,
    uint32_t* pByteLenTot)
{
    if ((NULL == pRawTrustedCertArray && 0 < lenTrustedCertArray) ||
        (NULL == pRawTrustedCrlArray && 0 < lenTrustedCrlArray) ||
        (NULL == pRawIssuerCertArray && 0 < lenIssuerCertArray) ||
        (NULL == pRawIssuerCrlArray && 0 < lenIssuerCrlArray))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pBsTrustedCertArray || NULL == pBsTrustedCrlArray || NULL == pBsIssuerCertArray ||
        NULL == pBsIssuerCrlArray || NULL == pByteLenTot)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pByteLenTot = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 < lenTrustedCertArray)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawTrustedCertArray, lenTrustedCertArray, pBsTrustedCertArray,
                                                        false, pByteLenTot);
    }
    if (0 < lenTrustedCrlArray && SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawTrustedCrlArray, lenTrustedCrlArray, pBsTrustedCrlArray,
                                                        true, pByteLenTot);
    }
    if (0 < lenIssuerCertArray && SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawIssuerCertArray, lenIssuerCertArray, pBsIssuerCertArray,
                                                        false, pByteLenTot);
    }
    if (0 < lenIssuerCrlArray && SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawIssuerCrlArray, lenIssuerCrlArray, pBsIssuerCrlArray, true,
                                                        pByteLenTot);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(*pBsTrustedCertArray);
        SOPC_Free(*pBsTrustedCrlArray);
        SOPC_Free(*pBsIssuerCertArray);
        SOPC_Free(*pBsIssuerCrlArray);
        *pBsTrustedCertArray = NULL;
        *pBsTrustedCrlArray = NULL;
        *pBsIssuerCertArray = NULL;
        *pBsIssuerCrlArray = NULL;
        *pByteLenTot = 0;
    }

    return status;
}

static SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                               uint32_t length,
                                                               SOPC_CertificateList** ppCert)
{
    if (NULL == pArray || 0 == length || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pCert = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    uint32_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCert);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCert);
        pCert = NULL;
    }
    *ppCert = pCert;
    return status;
}

static SOPC_ReturnStatus trustList_write_bs_array_to_crl_list(SOPC_ByteString* pArray,
                                                              uint32_t length,
                                                              SOPC_CRLList** ppCrl)
{
    if (NULL == pArray || 0 == length || NULL == ppCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CRLList* pCrl = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    uint32_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCrl);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCrl);
        pCrl = NULL;
    }
    *ppCrl = pCrl;
    return status;
}

static SOPC_ReturnStatus trustlist_write_decoded_data(const OpcUa_TrustListDataType* pDecode,
                                                      SOPC_CertificateList** ppTrustedCerts,
                                                      SOPC_CRLList** ppTrustedCrls,
                                                      SOPC_CertificateList** ppIssuerCerts,
                                                      SOPC_CRLList** ppIssuerCrls)
{
    if (NULL == pDecode || NULL == ppTrustedCerts || NULL == ppTrustedCrls || NULL == ppIssuerCerts ||
        NULL == ppIssuerCrls)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrls = NULL;
    SOPC_CertificateList* pIssuerCerts = NULL;
    SOPC_CRLList* pIssuerCrls = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Trusted Certificates  */
    if (SOPC_TL_MASK_TRUSTED_CERTS & pDecode->SpecifiedLists)
    {
        if (0 < pDecode->NoOfTrustedCertificates)
        {
            status = trustList_write_bs_array_to_cert_list(pDecode->TrustedCertificates,
                                                           (uint32_t) pDecode->NoOfTrustedCertificates, &pTrustedCerts);
        }
    }
    /* Trusted CRLs */
    if (SOPC_TL_MASK_TRUSTED_CRLS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (0 < pDecode->NoOfTrustedCrls)
        {
            status = trustList_write_bs_array_to_crl_list(pDecode->TrustedCrls, (uint32_t) pDecode->NoOfTrustedCrls,
                                                          &pTrustedCrls);
        }
    }
    /* Issuer Certificates  */
    if (SOPC_TL_MASK_ISSUER_CERTS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (0 < pDecode->NoOfIssuerCertificates)
        {
            status = trustList_write_bs_array_to_cert_list(pDecode->IssuerCertificates,
                                                           (uint32_t) pDecode->NoOfIssuerCertificates, &pIssuerCerts);
        }
    }
    /* Issuer CRLs */
    if (SOPC_TL_MASK_ISSUER_CRLS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (0 < pDecode->NoOfIssuerCrls)
        {
            status = trustList_write_bs_array_to_crl_list(pDecode->IssuerCrls, (uint32_t) pDecode->NoOfIssuerCrls,
                                                          &pIssuerCrls);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pTrustedCerts);
        SOPC_KeyManager_CRL_Free(pTrustedCrls);
        SOPC_KeyManager_Certificate_Free(pIssuerCerts);
        SOPC_KeyManager_CRL_Free(pIssuerCrls);
        pTrustedCerts = NULL;
        pTrustedCrls = NULL;
        pIssuerCerts = NULL;
        pIssuerCrls = NULL;
    }
    *ppTrustedCerts = pTrustedCerts;
    *ppTrustedCrls = pTrustedCrls;
    *ppIssuerCerts = pIssuerCerts;
    *ppIssuerCrls = pIssuerCrls;
    return status;
}

static bool trustlist_is_valid_masks(SOPC_TrLst_Mask masks)
{
    return 0 == (masks & ~SOPC_TL_MASK_ALL);
}

static SOPC_ReturnStatus gen_handle_with_retries(SOPC_TrustListContext* pTrustList, uint8_t depth)
{
    SOPC_ASSERT(NULL != pTrustList);

    if (0 == depth)
    {
        return SOPC_STATUS_NOK;
    }
    else
    {
        depth = (uint8_t)(depth - 1);
    }
    SOPC_TrLst_Handle genHandle = SOPC_TRUSTLIST_INVALID_HANDLE;
    SOPC_ExposedBuffer* pBuff = NULL;
    SOPC_CryptoProvider* pCrypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
    SOPC_ReturnStatus status = SOPC_CryptoProvider_GenerateRandomBytes(pCrypto, 2, &pBuff);
    if (SOPC_STATUS_OK == status)
    {
        genHandle = (uint32_t) pBuff[0] + (((uint32_t) pBuff[1]) << 8);
        if (SOPC_TRUSTLIST_INVALID_HANDLE == genHandle)
        {
            status = gen_handle_with_retries(pTrustList, depth);
        }
        else
        {
            pTrustList->opnCtx.handle = genHandle;
        }
    }
    SOPC_Free(pBuff);
    SOPC_CryptoProvider_Free(pCrypto);
    return status;
}

/* Start the activity timeout */
void TrustList_StartActivityTimeout(SOPC_TrustListContext* pContext)
{
    SOPC_ASSERT(NULL != pContext);
    SOPC_ASSERT(0 == pContext->eventMgr.activityTimeoutTimId);
    SOPC_LooperEvent activityTimeoutEvent = {0};
    activityTimeoutEvent.params = (uintptr_t) pContext;
    pContext->eventMgr.activityTimeoutTimId =
        SOPC_EventTimer_Create(pContext->eventMgr.pHandler, activityTimeoutEvent, SOPC_TRUSTLIST_ACTIVITY_TIMEOUT_MS);
    if (0 == pContext->eventMgr.activityTimeoutTimId)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s: failed to start the activity timeout",
                                 pContext->cStrObjectId);
    }
}

/* Reset the activity timeout */
void TrustList_ResetActivityTimeout(SOPC_TrustListContext* pContext)
{
    SOPC_ASSERT(NULL != pContext);
    SOPC_EventTimer_Cancel(pContext->eventMgr.activityTimeoutTimId);
    pContext->eventMgr.activityTimeoutTimId = 0;
    SOPC_Atomic_Int_Set(&pContext->eventMgr.timeoutElapsed, 0);
    TrustList_StartActivityTimeout(pContext);
}

/* Generate a random handle */
SOPC_ReturnStatus TrustList_GenRandHandle(SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);

    uint8_t retry = 2;
    SOPC_ReturnStatus status = gen_handle_with_retries(pTrustList, retry);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s: Internal error, failed to generate a random handle",
                               pTrustList->cStrObjectId);
    }
    return status;
}

/* Set the opening mode of the TrustList */
bool TrustList_SetOpenMode(SOPC_TrustListContext* pTrustList, SOPC_TrLst_OpenMode mode)
{
    SOPC_ASSERT(NULL != pTrustList);

    bool res = false;
    switch (mode)
    {
    case SOPC_TL_OPEN_MODE_READ:
    case SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING:
        res = true;
        pTrustList->opnCtx.openingMode = mode;
        break;
    default:
        pTrustList->opnCtx.openingMode = SOPC_TL_OPEN_MODE_UNKNOWN;
        break;
    }
    return res;
}

/* Set the opening masks for the specifiedLists of the TrustList. */
bool TrustList_SetOpenMasks(SOPC_TrustListContext* pTrustList, SOPC_TrLst_Mask masks)
{
    SOPC_ASSERT(NULL != pTrustList);

    pTrustList->opnCtx.openingMask = SOPC_TL_MASK_NONE;
    bool isValid = trustlist_is_valid_masks(masks);
    if (isValid)
    {
        pTrustList->opnCtx.openingMask = masks;
    }
    return isValid;
}

/* Set the TrustList position */
SOPC_ReturnStatus TrustList_SetPosition(SOPC_TrustListContext* pTrustList, uint64_t pos)
{
    SOPC_ASSERT(NULL != pTrustList);

    /* TrustList is not yet encode */
    if (NULL == pTrustList->opnCtx.pTrustListEncoded)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* Check before casting */
    if (UINT32_MAX < pos)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:SetPosition: position (%" PRIu64 ") is too large for buffer",
                               pTrustList->cStrObjectId, pos);
        return SOPC_STATUS_INVALID_STATE;
    }
    uint32_t position = (uint32_t) pos;
    uint32_t bufLength = TrustList_GetLength(pTrustList);
    /* if the position is higher than the file size the position is set to the end of the file*/
    if (position > bufLength)
    {
        position = bufLength;
    }
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(pTrustList->opnCtx.pTrustListEncoded, position);
    return status;
}

/* Get the TrustList handle */
uint32_t TrustList_GetHandle(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    return (uint32_t) pTrustList->opnCtx.handle;
}

/* Get the TrustList position */
uint64_t TrustList_GetPosition(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);

    uint64_t pos = 0;
    if (NULL != pTrustList->opnCtx.pTrustListEncoded)
    {
        SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(pTrustList->opnCtx.pTrustListEncoded, (uint32_t*) &pos);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    return pos;
}

/* Get the TrustList length  */
uint32_t TrustList_GetLength(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    uint32_t bufLength = 0;
    if (NULL != pTrustList->opnCtx.pTrustListEncoded)
    {
        bufLength = pTrustList->opnCtx.pTrustListEncoded->length;
    }
    return bufLength;
}

/* Get the specifiedLists mask */
SOPC_TrLst_Mask TrustList_GetSpecifiedListsMask(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    return pTrustList->opnCtx.specifiedLists;
}

/* Get the TOFU state status */
bool TrustList_isInTOFUSate(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    return NULL != pTrustList->pFnUpdateCompleted;
}

/* Check the TrustList handle */
bool TrustList_CheckHandle(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_Handle expected, const char* msg)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool match = expected == pTrustList->opnCtx.handle;
    if (!match && NULL != msg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:(handle):%s", pTrustList->cStrObjectId, msg);
    }
    return match;
}

/* Check the TrustList opening mode */
bool TrustList_CheckOpenMode(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_OpenMode expected, const char* msg)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool match = expected == pTrustList->opnCtx.openingMode;
    if (!match && NULL != msg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:(mode):%s", pTrustList->cStrObjectId, msg);
    }
    return match;
}

/* Check if the TrustList is opened */
bool TrustList_CheckIsOpened(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool isOpened = SOPC_TL_OPEN_MODE_UNKNOWN != pTrustList->opnCtx.openingMode &&
                    SOPC_TL_MASK_NONE != pTrustList->opnCtx.openingMask;
    return isOpened;
}

/* Get the TrustList nodeId */
const char* TrustList_GetStrNodeId(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    return (const char*) pTrustList->cStrObjectId;
}

/* Read the PKI and encode the trustList in a UA Binary encoded stream containing an instance of TrustListDataType */
SOPC_ReturnStatus TrustList_Encode(SOPC_TrustListContext* pTrustList)
{
    /* Check parameters.
       The PKI, the encoded result buffer and the opening mask are consider valid here. */
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL == pTrustList->opnCtx.pTrustListEncoded);
    SOPC_ASSERT(NULL != pTrustList->pPKI);

    bool bIsRead = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, NULL);
    if (!bIsRead)
    {
        /* Do not encode the TrustList in write mode */
        return SOPC_STATUS_OK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Extracted data from the PKI */
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CertificateList* pIssuerCerts = NULL;
    SOPC_CRLList* pTrustedCrls = NULL;
    SOPC_CRLList* pIssuerCrls = NULL;
    /* Converted data from the PKI to raw DER array */
    SOPC_SerializedCertificate* pRawTrustedCertArray = NULL;
    SOPC_SerializedCertificate* pRawIssuerCertArray = NULL;
    SOPC_SerializedCRL* pRawTrustedCrlArray = NULL;
    SOPC_SerializedCRL* pRawIssuerCrlArray = NULL;
    uint32_t nbTrustedCerts = 0;
    uint32_t nbTrustedCrls = 0;
    uint32_t nbIssuerCerts = 0;
    uint32_t nbIssuerCrls = 0;
    /* Instance of TrustListDataType */
    OpcUa_TrustListDataType pTrustListDataType = {0};
    SOPC_ByteString* pBsTrustedCertArray = NULL;
    SOPC_ByteString* pBsTrustedCrlArray = NULL;
    SOPC_ByteString* pBsIssuerCertArray = NULL;
    SOPC_ByteString* pBsIssuerCrlArray = NULL;
    /* UA Binary encoded stream containing an instance of TrustListDataType */
    uint32_t nbCerts = 0;
    uint32_t byteLenTot = 0;
    uint32_t lenBuffer = 0;
    SOPC_Buffer* pBufferTrustListDataType = NULL;

    SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustListDataType);

    /* For embedded platform, it is advisable not to make copies of the PKI data and filter the list
       with specifiedLists eg:
            SOPC_PKIProvider_AttachToList_TrustedCertificates()
            SOPC_PKIProvider_AttachToList_TrustedCRLs()
            SOPC_PKIProvider_AttachToList_IssuerCertificates()
            SOPC_PKIProvider_AttachToList_IssuerCRL()
        Another solution is to keep SOPC_PKIProvider_WriteOrAppendToList and append a boolean argument such as
       "bToAttach".
            */
    status = SOPC_PKIProvider_WriteOrAppendToList(pTrustList->pPKI, &pTrustedCerts, &pTrustedCrls, &pIssuerCerts,
                                                  &pIssuerCrls);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:Encode: PKI WriteOrAppendToList function failed",
                               pTrustList->cStrObjectId);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Filter with specifiedLists and leave NULL pRawXXXXArray if not required.

           The ownership of the memory area for each data of the array is the CertificateList and CRLList
           copied from the PKI */

        status = trustlist_attach_certs_to_raw_arrays(
            pTrustList->opnCtx.openingMask, pTrustedCerts, &pRawTrustedCertArray, &nbTrustedCerts, pTrustedCrls,
            &pRawTrustedCrlArray, &nbTrustedCrls, pIssuerCerts, &pRawIssuerCertArray, &nbIssuerCerts, pIssuerCrls,
            &pRawIssuerCrlArray, &nbIssuerCrls);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_arrays_to_bs_arrays(
            pRawTrustedCertArray, nbTrustedCerts, &pBsTrustedCertArray, pRawTrustedCrlArray, nbTrustedCrls,
            &pBsTrustedCrlArray, pRawIssuerCertArray, nbIssuerCerts, &pBsIssuerCertArray, pRawIssuerCrlArray,
            nbIssuerCrls, &pBsIssuerCrlArray, &byteLenTot);
    }
    /* Get the UA Binary encoded stream size */
    nbCerts = nbTrustedCerts + nbTrustedCrls + nbIssuerCerts + nbIssuerCrls;
    if (SOPC_STATUS_OK == status)
    {
        lenBuffer =
            SOPC_EMPTY_TRUSTLIST_ENCODED_BYTE_SIZE + byteLenTot + nbCerts * SOPC_LENGTH_BSTRING_ENCODED_BYTE_SIZE;
    }
    if (pTrustList->maxTrustListSize < lenBuffer)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:Encode: buffer size which holding the TrustList is too large (%" PRIu32
                               ")",
                               pTrustList->cStrObjectId, lenBuffer);
        status = SOPC_STATUS_INVALID_STATE;
    }
    /* Check before casting */
    if (SOPC_STATUS_OK == status)
    {
        if (INT32_MAX < nbTrustedCerts)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:Encode: trusted certificates are too many (%" PRIu32 ")",
                                   pTrustList->cStrObjectId, nbTrustedCerts);
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (INT32_MAX < nbTrustedCrls)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:Encode: trusted CRLs are too many (%" PRIu32 ")",
                                   pTrustList->cStrObjectId, nbTrustedCrls);
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (INT32_MAX < nbIssuerCerts)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:Encode: issuer certificates are too many (%" PRIu32 ")",
                                   pTrustList->cStrObjectId, nbIssuerCerts);
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (INT32_MAX < nbIssuerCrls)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:Encode: issuer CRLs are too many (%" PRIu32 ")",
                                   pTrustList->cStrObjectId, nbIssuerCrls);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    /* Fill the instance of the TrustListDataType */
    if (SOPC_STATUS_OK == status)
    {
        pTrustListDataType.SpecifiedLists = pTrustList->opnCtx.openingMask;
        pTrustListDataType.TrustedCertificates = pBsTrustedCertArray;
        pTrustListDataType.NoOfTrustedCertificates = (int32_t) nbTrustedCerts;
        pTrustListDataType.TrustedCrls = pBsTrustedCrlArray;
        pTrustListDataType.NoOfTrustedCrls = (int32_t) nbTrustedCrls;
        pTrustListDataType.IssuerCertificates = pBsIssuerCertArray;
        pTrustListDataType.NoOfIssuerCertificates = (int32_t) nbIssuerCerts;
        pTrustListDataType.IssuerCrls = pBsIssuerCrlArray;
        pTrustListDataType.NoOfIssuerCrls = (int32_t) nbIssuerCrls;

        /* Create the buffer which holding the Ua Binary encoded stream containing the TrustListDataType instance */
        pBufferTrustListDataType = SOPC_Buffer_Create((uint32_t) lenBuffer);
        if (NULL == pBufferTrustListDataType)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Encode the instance of the TrustListDataType to an Ua binary stream */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Encode(&OpcUa_TrustListDataType_EncodeableType,
                                              (const void*) &pTrustListDataType, pBufferTrustListDataType, 1);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:Encode: EncodeableObject function failed", pTrustList->cStrObjectId);
        }
    }
    /* Reset the position of the buffer */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_SetPosition(pBufferTrustListDataType, 0);
    }
    /* Alway clear at the end */
    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrls);
    SOPC_KeyManager_CRL_Free(pIssuerCrls);
    SOPC_Free(pRawTrustedCertArray);
    SOPC_Free(pRawTrustedCrlArray);
    SOPC_Free(pRawIssuerCertArray);
    SOPC_Free(pRawIssuerCrlArray);
    SOPC_Free(pBsTrustedCertArray);
    SOPC_Free(pBsTrustedCrlArray);
    SOPC_Free(pBsIssuerCertArray);
    SOPC_Free(pBsIssuerCrlArray);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(pBufferTrustListDataType);
        pBufferTrustListDataType = NULL;
    }
    /* Return Result */
    pTrustList->opnCtx.pTrustListEncoded = pBufferTrustListDataType;
    return status;
}

/* Read bytes from the current position of the encoded TrustListDataType */
SOPC_ReturnStatus TrustList_Read(SOPC_TrustListContext* pTrustList, int32_t reqLength, SOPC_ByteString* pDest)
{
    if (NULL == pTrustList || NULL == pDest || reqLength <= 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pTrustList->opnCtx.pTrustListEncoded || NULL != pDest->Data || pDest->Length > 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t lengthToWrite = (uint32_t) reqLength;
    uint32_t sizeAvailable = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Get the remaining length */
    sizeAvailable = SOPC_Buffer_Remaining(pTrustList->opnCtx.pTrustListEncoded);
    /* Check before casting */
    if (INT32_MAX < sizeAvailable)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:Read: data size (%" PRIu32 ") is too large for byteString",
                               pTrustList->cStrObjectId, sizeAvailable);
        return SOPC_STATUS_INVALID_STATE;
    }
    if (lengthToWrite > sizeAvailable)
    {
        lengthToWrite = sizeAvailable;
    }
    if (lengthToWrite)
    {
        pDest->Length = (int32_t) lengthToWrite;
        pDest->Data = SOPC_Malloc(lengthToWrite * sizeof(SOPC_Byte));
        status = SOPC_Buffer_Read(pDest->Data, pTrustList->opnCtx.pTrustListEncoded, lengthToWrite);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pDest->Data);
        SOPC_ByteString_Initialize(pDest);
    }
    return status;
}

/* Decode the trustList UA Binary stream to a TrustListDataType */
SOPC_ReturnStatus TrustList_Decode(SOPC_TrustListContext* pTrustList, const SOPC_ByteString* pEncodedTrustListDataType)
{
    if (NULL == pTrustList || NULL == pEncodedTrustListDataType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Writing an empty or null ByteString returns a Good result code without any affect on the TrustList. */
    if (NULL == pEncodedTrustListDataType->Data || pEncodedTrustListDataType->Length <= 0)
    {
        return SOPC_STATUS_OK;
    }
    SOPC_Buffer* pToDecode =
        SOPC_Buffer_Attach(pEncodedTrustListDataType->Data, (uint32_t) pEncodedTrustListDataType->Length);
    if (NULL == pToDecode)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    OpcUa_TrustListDataType trustListDataType = {0};
    OpcUa_TrustListDataType_Initialize(&trustListDataType);
    SOPC_EncodeableType* type = &OpcUa_TrustListDataType_EncodeableType;
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(pToDecode, 0);

    /* Decode the byteString */
    status = SOPC_EncodeableObject_Decode(type, (void*) &trustListDataType, pToDecode, 0);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Decode: EncodeableObject function failed",
                               pTrustList->cStrObjectId);
    }

    /* check the mask (SpecifiedLists) */
    if (SOPC_STATUS_OK == status)
    {
        bool isValid = trustlist_is_valid_masks(trustListDataType.SpecifiedLists);
        if (!isValid)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:Decode: invalid SpecifiedLists:%" PRIX32,
                                   pTrustList->cStrObjectId, trustListDataType.SpecifiedLists);
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        /* TrustList support only the write mode with the EraseExisting option */
        SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pIssuerCerts);
        SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pTrustedCRLs);
        SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pIssuerCRLs);

        status = trustlist_write_decoded_data(&trustListDataType, &pTrustList->opnCtx.pTrustedCerts,
                                              &pTrustList->opnCtx.pTrustedCRLs, &pTrustList->opnCtx.pIssuerCerts,
                                              &pTrustList->opnCtx.pIssuerCRLs);
    }
    /* Register the part of the TrustList to update with CloseAndUpdate */
    if (SOPC_STATUS_OK == status)
    {
        pTrustList->opnCtx.specifiedLists = trustListDataType.SpecifiedLists;
    }

    /* Clear */
    SOPC_EncodeableObject_Clear(type, &trustListDataType);
    SOPC_Free(pToDecode);

    return status;
}

/* Remove a single Certificate */
SOPC_StatusCode TrustList_RemoveCert(SOPC_TrustListContext* pTrustList,
                                     const SOPC_String* pThumbprint,
                                     const bool bIsTrustedCert,
                                     bool* pbIsRemove,
                                     bool* pbIsIssuer)
{
    *pbIsRemove = false;
    *pbIsIssuer = false;

    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->pPKI);
    SOPC_ASSERT(NULL != pbIsRemove);
    SOPC_ASSERT(NULL != pbIsIssuer);

    if (NULL == pThumbprint)
    {
        return OpcUa_BadUnexpectedError;
    }
    if (NULL == pThumbprint->Data || pThumbprint->Length <= 0)
    {
        /* The certificate to remove will not be found */
        return OpcUa_BadInvalidArgument;
    }

    uint32_t lenThumb = (uint32_t) pThumbprint->Length;

    if (THUMBPRINT_LENGTH != lenThumb)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:RemoveCertificate: rcv invalid thumbprint length : %" PRId32
                               "(expected 40 bytes for hexadecimal SHA1) ",
                               pTrustList->cStrObjectId, pThumbprint->Length);
        return OpcUa_BadInvalidArgument;
    }

    const char* cStrThumb = SOPC_String_GetRawCString(pThumbprint);
    if (NULL == cStrThumb)
    {
        return OpcUa_BadUnexpectedError;
    }

    bool bIsRemove = false;
    bool bIsIssuer = false;
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status =
        SOPC_PKIProvider_RemoveCertificate(pTrustList->pPKI, cStrThumb, bIsTrustedCert, &bIsRemove, &bIsIssuer);

    if (bIsRemove && SOPC_STATUS_OK == status)
    {
        if (!bIsTrustedCert && !bIsIssuer)
        {
            statusCode = OpcUa_BadUnexpectedError;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:RemoveCertificate: failed to remove certificate thumbprint <%s>",
                               pTrustList->cStrObjectId, cStrThumb);
        statusCode = OpcUa_BadUnexpectedError;
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:RemoveCertificate: certificate thumbprint <%s> has been removed",
                               pTrustList->cStrObjectId, cStrThumb);
    }

    *pbIsRemove = bIsRemove;
    *pbIsIssuer = bIsIssuer;
    return statusCode;
}

/* Validate the certificate and update the PKI that belongs to the TrustList */
SOPC_StatusCode TrustList_UpdateWithAddCertificateMethod(SOPC_TrustListContext* pTrustList,
                                                         const SOPC_ByteString* pBsCert,
                                                         const char* secPolUri)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->pPKI);
    if (NULL == pBsCert)
    {
        return OpcUa_BadCertificateInvalid;
    }
    if (NULL == pBsCert->Data || 0 >= pBsCert->Length)
    {
        return OpcUa_BadCertificateInvalid;
    }
    if (NULL == secPolUri)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_StatusCode statusCode = SOPC_GoodGenericStatus;
    SOPC_StatusCode validationError = SOPC_GoodGenericStatus;
    SOPC_CertificateList* pCert = NULL;
    char* pThumb = NULL;
    const char* thumb = NULL;
    SOPC_PKIProvider* pTmpPKI = NULL;
    SOPC_CertificateList* pToUpdateTrustedCerts = NULL;
    SOPC_CRLList* pToUpdateTrustedCRLs = NULL;
    SOPC_CertificateList* pToUpdateIssuerCerts = NULL;
    SOPC_CRLList* pToUpdateIssuerCRLs = NULL;

    /* TODO: we should rework SOPC_PKIProvider_UpdateFromList to handle both CloseAndUpdate and AddCertificate (avoid
     * copies and tmp PKI, cf public issue #1262) */

    /* Create the certificate */
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length,
                                                                              &pToUpdateTrustedCerts);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pToUpdateTrustedCerts, &pCert);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:AddCertificate: certificate parse failed",
                               pTrustList->cStrObjectId);
        statusCode = OpcUa_BadCertificateInvalid;
    }
    else
    {
        pThumb = SOPC_KeyManager_Certificate_GetCstring_SHA1(pCert);
        thumb = NULL == pThumb ? "NULL" : pThumb;
    }
    /* Create a temporary PKI with the current certificates plus the trusted leaf certificate to be added  */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_WriteOrAppendToList(pTrustList->pPKI, &pToUpdateTrustedCerts, &pToUpdateTrustedCRLs,
                                                      &pToUpdateIssuerCerts, &pToUpdateIssuerCRLs);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:AddCertificate: PKI WriteOrAppendToList function failed",
                                   pTrustList->cStrObjectId);
            statusCode = OpcUa_BadUnexpectedError;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromList(pToUpdateTrustedCerts, pToUpdateTrustedCRLs, pToUpdateIssuerCerts,
                                                     pToUpdateIssuerCRLs, &pTmpPKI);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:AddCertificate: PKI creation failed",
                                       pTrustList->cStrObjectId);
                statusCode = OpcUa_BadUnexpectedError;
            }
        }
    }
    /* Create the PKI profile and validate the certificate */
    if (SOPC_STATUS_OK == status)
    {
        /* Minimum profile (we do not know for which security policy this will be used) */
        SOPC_PKI_ChainProfile chainProfile = {.curves = SOPC_PKI_CURVES_ANY,
                                              .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                              .pkAlgo = SOPC_PKI_PK_RSA,
                                              .RSAMinimumKeySize = 1024};
        SOPC_PKI_Profile profile = {.leafProfile = NULL,
                                    .chainProfile = &chainProfile,
                                    .bApplyLeafProfile = true,
                                    .bBackwardInteroperability = true};
        status = SOPC_PKIProvider_CreateLeafProfile(NULL, &profile.leafProfile);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_PKI_Type pki_type =
                SOPC_TRUSTLIST_GROUP_APP == pTrustList->groupType ? SOPC_PKI_TYPE_SERVER_APP : SOPC_PKI_TYPE_USER;
            status = SOPC_PKIProvider_ProfileSetUsageFromType(&profile, pki_type);
        }
        if (SOPC_STATUS_OK == status)
        {
            profile.bApplyLeafProfile = true;
            /* Validate the certificate */
            status = SOPC_PKIProvider_ValidateCertificate(pTmpPKI, pCert, &profile, &validationError);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "TrustList:%s:AddCertificate: certificate validation failed with error code %" PRIX32
                    " for thumbprint <%s>",
                    pTrustList->cStrObjectId, validationError, thumb);
                statusCode = validationError;
            }
        }
        else
        {
            statusCode = OpcUa_BadUnexpectedError;
        }
        SOPC_PKIProvider_DeleteLeafProfile(&profile.leafProfile);
    }
    /* Update the PKI with the new certificate */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_UpdateFromList(pTrustList->pPKI, secPolUri, pCert, NULL, NULL, NULL, true);
        if (SOPC_STATUS_OK != status)
        {
            /* The security level of the update is probably higher than the security level of the secure channel */
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "TrustList:%s:AddCertificate: trustList update failed for certificate thumbprint <%s>",
                pTrustList->cStrObjectId, thumb);
            statusCode = OpcUa_BadCertificateInvalid;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "TrustList:%s:AddCertificate: certificate thumbprint <%s> has been added",
                               pTrustList->cStrObjectId, thumb);
    }
    /* Clear */
    SOPC_Free(pThumb);
    SOPC_PKIProvider_Free(&pTmpPKI);
    SOPC_KeyManager_Certificate_Free(pToUpdateTrustedCerts);
    SOPC_KeyManager_CRL_Free(pToUpdateTrustedCRLs);
    SOPC_KeyManager_Certificate_Free(pToUpdateIssuerCerts);
    SOPC_KeyManager_CRL_Free(pToUpdateIssuerCRLs);
    SOPC_KeyManager_Certificate_Free(pCert);
    return statusCode;
}

/* Validate the written TrustList and update the PKI.*/
SOPC_StatusCode TrustList_UpdateWithWriteMethod(SOPC_TrustListContext* pTrustList, const char* secPolUri)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->pPKI);
    if (NULL == secPolUri)
    {
        return OpcUa_BadUnexpectedError;
    }
    /* No fields are provided or the write method has not yet been called */
    if (SOPC_TL_MASK_NONE == pTrustList->opnCtx.specifiedLists ||
        (NULL == pTrustList->opnCtx.pTrustedCerts && NULL == pTrustList->opnCtx.pTrustedCRLs &&
         NULL == pTrustList->opnCtx.pIssuerCerts && NULL == pTrustList->opnCtx.pIssuerCRLs))
    {
        return SOPC_GoodGenericStatus;
    }
    uint32_t* pErrors = NULL;
    char** pThumbprints = NULL;
    uint32_t length = 0;
    SOPC_PKIProvider* pTmpPKI = NULL;
    SOPC_ReturnStatus statusCode = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bIncludeExistingList = false;

    SOPC_CertificateList* pToUpdateTrustedCerts = NULL;
    SOPC_CRLList* pToUpdateTrustedCRLs = NULL;
    SOPC_CertificateList* pToUpdateIssuerCerts = NULL;
    SOPC_CRLList* pToUpdateIssuerCRLs = NULL;

    /* TODO: we should optimize by adding VerifyEveryCertificate inside the UpdateFromList (avoid copies and tmp PKI, cf
     * public issue #1262) */
    if (NULL != pTrustList->opnCtx.pTrustedCerts)
    {
        status = SOPC_KeyManager_Certificate_Copy(pTrustList->opnCtx.pTrustedCerts, &pToUpdateTrustedCerts);
    }
    if (NULL != pTrustList->opnCtx.pTrustedCRLs && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_Copy(pTrustList->opnCtx.pTrustedCRLs, &pToUpdateTrustedCRLs);
    }
    if (NULL != pTrustList->opnCtx.pIssuerCerts && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pTrustList->opnCtx.pIssuerCerts, &pToUpdateIssuerCerts);
    }
    if (NULL != pTrustList->opnCtx.pIssuerCRLs && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_Copy(pTrustList->opnCtx.pIssuerCRLs, &pToUpdateIssuerCRLs);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: certificate(s) copy failed",
                               pTrustList->cStrObjectId);
        statusCode = OpcUa_BadCertificateInvalid;
    }

    /* If only part of the TrustList is being updated the Server creates a new TrustList that includes
       the existing TrustList plus any updates and validates the new TrustList. */
    if (SOPC_TL_MASK_ALL != pTrustList->opnCtx.specifiedLists && SOPC_STATUS_OK == status)
    {
        bIncludeExistingList = true;

        status = SOPC_PKIProvider_WriteOrAppendToList(
            pTrustList->pPKI, &pTrustList->opnCtx.pTrustedCerts, &pTrustList->opnCtx.pTrustedCRLs,
            &pTrustList->opnCtx.pIssuerCerts, &pTrustList->opnCtx.pIssuerCRLs);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "TrustList:%s:CloseAndUpdate: PKI WriteOrAppendToList function failed",
                                   pTrustList->cStrObjectId);
            statusCode = OpcUa_BadCertificateInvalid;
        }
    }
    /* Create a temporary PKI to verify every certificate */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_PKIProvider_CreateFromList(pTrustList->opnCtx.pTrustedCerts, pTrustList->opnCtx.pTrustedCRLs,
                                            pTrustList->opnCtx.pIssuerCerts, pTrustList->opnCtx.pIssuerCRLs, &pTmpPKI);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: PKI creation failed",
                                   pTrustList->cStrObjectId);
            statusCode = OpcUa_BadCertificateInvalid;
        }
    }
    /* Set a "permissive" profile, indeed the certificate properties are verified by UpdateFromList function.
       UpdateFromList handles that the security level of the update isn't higher than the security level of the secure
       channel.*/
    const SOPC_PKI_ChainProfile profile = {.curves = SOPC_PKI_CURVES_ANY,
                                           .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                           .pkAlgo = SOPC_PKI_PK_RSA,
                                           .RSAMinimumKeySize = 1024};
    /* Verify every certificate with a "permissive" profile => check only include signature and validity */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_VerifyEveryCertificate(pTmpPKI, &profile, &pErrors, &pThumbprints, &length);
        if (SOPC_STATUS_OK != status)
        {
            for (uint32_t i = 0; i < length; i++)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "TrustList:%s:CloseAndUpdate: certificate thumbprint %s is invalid with error %" PRIX32,
                    pTrustList->cStrObjectId, pThumbprints[i], pErrors[i]);
                SOPC_Free(pThumbprints[i]);
            }
            SOPC_Free(pThumbprints);
            SOPC_Free(pErrors);
            statusCode = OpcUa_BadCertificateInvalid;
        }
    }
    /* Verify the security level of the new certificates and update the PKI.
       If errors occur, the new PKI is discarded. */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_PKIProvider_UpdateFromList(pTrustList->pPKI, secPolUri, pToUpdateTrustedCerts, pToUpdateTrustedCRLs,
                                            pToUpdateIssuerCerts, pToUpdateIssuerCRLs, bIncludeExistingList);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:CloseAndUpdate: PKI update failed",
                                   pTrustList->cStrObjectId);
            statusCode = OpcUa_BadCertificateInvalid;
        }
    }
    /* Clear */
    SOPC_PKIProvider_Free(&pTmpPKI);
    SOPC_KeyManager_Certificate_Free(pToUpdateTrustedCerts);
    SOPC_KeyManager_CRL_Free(pToUpdateTrustedCRLs);
    SOPC_KeyManager_Certificate_Free(pToUpdateIssuerCerts);
    SOPC_KeyManager_CRL_Free(pToUpdateIssuerCRLs);
    return statusCode;
}

/* Export the update (certificate files) */
SOPC_ReturnStatus TrustList_Export(const SOPC_TrustListContext* pTrustList,
                                   const bool bEraseExiting,
                                   const bool bForcePush)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->pPKI);

    if (!bForcePush)
    {
        /* No fields are provided => Do nothing */
        if (SOPC_TL_MASK_NONE == pTrustList->opnCtx.specifiedLists)
        {
            return SOPC_STATUS_OK;
        }
    }
    bool bNotIncludeExitingFile = SOPC_TL_MASK_ALL == pTrustList->opnCtx.specifiedLists || bEraseExiting;
    SOPC_ReturnStatus status = SOPC_PKIProvider_WriteToStore(pTrustList->pPKI, bNotIncludeExitingFile);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "TrustList:%s:CloseAndUpdate: PKI WriteToStore function failed",
                                 pTrustList->cStrObjectId);
    }
    return status;
}

/* Raise an event to re-evaluate the certificates. */
SOPC_ReturnStatus TrustList_RaiseEvent(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_TRUSTLIST_GROUP_APP == pTrustList->groupType)
    {
        SOPC_ToolkitServer_AsyncReEvalSecureChannels(false);
    }
    else if (SOPC_TRUSTLIST_GROUP_USR == pTrustList->groupType)
    {
        SOPC_ToolkitServer_AsyncReEvalUserCertSessions();
    }
    else
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "TrustList:%s:RaiseEvent: unexpected certificate group type : %d",
                                 pTrustList->cStrObjectId, (const int) pTrustList->groupType);
        status = SOPC_STATUS_NOK;
    }
    return status;
}

/* Executes user callback to indicate the end of a valid update */
void TrustList_UpdateCompleted(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    if (TrustList_isInTOFUSate(pTrustList))
    {
        pTrustList->pFnUpdateCompleted();
    }
}

/* Write the value of the Size variable */
void Trustlist_WriteVarSize(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->varIds.pSizeId);
    if (NULL == pAddSpAccess)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:SizeVariable: bad address space access",
                                 pTrustList->cStrObjectId);
        return;
    }

    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    SOPC_StatusCode varSizeStCode = SOPC_GoodGenericStatus;
    bool bIsOpened = TrustList_CheckIsOpened(pTrustList);
    if (bIsOpened)
    {
        bool bIsRead = TrustList_CheckOpenMode(pTrustList, SOPC_TL_OPEN_MODE_READ, NULL);
        if (!bIsRead)
        {
            varSizeStCode = OpcUa_BadNotSupported;
        }
    }
    dv.Value.BuiltInTypeId = SOPC_UInt64_Id;
    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.Value.Uint64 = TrustList_GetLength(pTrustList);
    SOPC_DateTime ts = 0; // will set current time as source TS
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_WriteValue(pAddSpAccess, pTrustList->varIds.pSizeId, NULL,
                                                                &dv.Value, &varSizeStCode, &ts, NULL);
    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "TrustList:%s:SizeVariable: unable to write the size %" PRId64,
                                 pTrustList->cStrObjectId, dv.Value.Value.Uint64);
    }
}

/* Write the value of the OpenCount variable */
void Trustlist_WriteVarOpenCount(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->varIds.pOpenCountId);
    if (NULL == pAddSpAccess)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "TrustList:%s:OpenCountVariable: bad address space access", pTrustList->cStrObjectId);
        return;
    }

    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    uint16_t openCount = 0;
    bool bIsOpened = TrustList_CheckIsOpened(pTrustList);
    if (bIsOpened)
    {
        openCount = 1;
    }
    dv.Value.BuiltInTypeId = SOPC_UInt16_Id;
    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.Value.Uint16 = openCount;
    SOPC_DateTime ts = 0; // will set current time as source TS
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_WriteValue(pAddSpAccess, pTrustList->varIds.pOpenCountId, NULL,
                                                                &dv.Value, NULL, &ts, NULL);
    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "TrustList:%s:OpenCountVariable: unable to write the opencount %" PRId16,
                                 pTrustList->cStrObjectId, dv.Value.Value.Uint16);
    }
}

/* Write the value of the LastUpdateTime variable */
void Trustlist_WriteVarLastUpdateTime(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess)
{
    SOPC_ASSERT(NULL != pTrustList);
    SOPC_ASSERT(NULL != pTrustList->varIds.pLastUpdateTimeId);
    if (NULL == pAddSpAccess)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:LastUpdateTime: bad address space access",
                                 pTrustList->cStrObjectId);
        return;
    }

    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    dv.Value.BuiltInTypeId = SOPC_DateTime_Id;
    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.Value.Date = SOPC_Time_GetCurrentTimeUTC();
    SOPC_DateTime ts = 0; // will set current time as source TS
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_WriteValue(pAddSpAccess, pTrustList->varIds.pLastUpdateTimeId,
                                                                NULL, &dv.Value, NULL, &ts, NULL);
    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "TrustList:%s:LastUpdateTime: unable to write the current time to the LastUpdateTime variable",
            pTrustList->cStrObjectId);
    }
}

/* Reset the TrustList context when close method is call */
void TrustList_Reset(SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess)
{
    SOPC_ASSERT(NULL != pTrustList);

    SOPC_EventTimer_Cancel(pTrustList->eventMgr.activityTimeoutTimId);
    pTrustList->eventMgr.activityTimeoutTimId = 0;
    SOPC_Atomic_Int_Set(&pTrustList->eventMgr.timeoutElapsed, 0);
    SOPC_Buffer_Delete(pTrustList->opnCtx.pTrustListEncoded);
    SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustList->opnCtx.pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pTrustedCRLs);
    SOPC_KeyManager_CRL_Free(pTrustList->opnCtx.pIssuerCRLs);

    memset(&pTrustList->opnCtx, 0, sizeof(SOPC_TrLst_OpeningCtx));

    if (NULL != pAddSpAccess)
    {
        Trustlist_WriteVarSize(pTrustList, pAddSpAccess);
        Trustlist_WriteVarOpenCount(pTrustList, pAddSpAccess);
    }
}
