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

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_trustlist.h"

/*---------------------------------------------------------------------------
 *                             Constants
 *---------------------------------------------------------------------------*/

#define SOPC_EMPTY_TRUSTLIST_ENCODED_BYTE_SIZE 20u
#define SOPC_LENGTH_BSTRING_ENCODED_BYTE_SIZE 4u

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
                                                              size_t* pLenTrustedCertArray,
                                                              const SOPC_CRLList* pTrustedCrls,
                                                              SOPC_SerializedCRL** pRawTrustedCrlArray,
                                                              size_t* pLenTrustedCrlArray,
                                                              const SOPC_CertificateList* pIssuerCerts,
                                                              SOPC_SerializedCertificate** pRawIssuerCertArray,
                                                              size_t* pLenIssuerCertArray,
                                                              const SOPC_CRLList* pIssuerCrls,
                                                              SOPC_SerializedCRL** pRawIssuerCrlArray,
                                                              size_t* pLenIssuerCrlArray);

static SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pArray,
                                                                size_t lenArray,
                                                                SOPC_ByteString** pByteStringArray,
                                                                bool bIsCRL,
                                                                size_t* pByteLenTot);

static SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(
    const SOPC_SerializedCertificate* pRawTrustedCertArray,
    const size_t lenTrustedCertArray,
    SOPC_ByteString** pBsTrustedCertArray,
    const SOPC_SerializedCRL* pRawTrustedCrlArray,
    const size_t lenTrustedCrlArray,
    SOPC_ByteString** pBsTrustedCrlArray,
    const SOPC_SerializedCertificate* pRawIssuerCertArray,
    const size_t lenIssuerCertArray,
    SOPC_ByteString** pBsIssuerCertArray,
    const SOPC_SerializedCRL* pRawIssuerCrlArray,
    const size_t lenIssuerCrlArray,
    SOPC_ByteString** pBsIssuerCrlArray,
    size_t* pByteLenTot);

static SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                               size_t length,
                                                               SOPC_CertificateList** ppCert);
static SOPC_ReturnStatus trustList_write_bs_array_to_crl_list(SOPC_ByteString* pArray,
                                                              size_t length,
                                                              SOPC_CRLList** ppCrl);
static SOPC_ReturnStatus trustlist_write_decoded_data(const OpcUa_TrustListDataType* pDecode,
                                                      SOPC_CertificateList** ppTrustedCerts,
                                                      SOPC_CRLList** ppTrustedCrls,
                                                      SOPC_CertificateList** ppIssuerCerts,
                                                      SOPC_CRLList** ppIssuerCrls);

/*---------------------------------------------------------------------------
 *                       Static functions (implementation)
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus trustlist_attach_certs_to_raw_arrays(const SOPC_TrLst_Mask specifiedLists,
                                                              const SOPC_CertificateList* pTrustedCerts,
                                                              SOPC_SerializedCertificate** pRawTrustedCertArray,
                                                              size_t* pLenTrustedCertArray,
                                                              const SOPC_CRLList* pTrustedCrls,
                                                              SOPC_SerializedCRL** pRawTrustedCrlArray,
                                                              size_t* pLenTrustedCrlArray,
                                                              const SOPC_CertificateList* pIssuerCerts,
                                                              SOPC_SerializedCertificate** pRawIssuerCertArray,
                                                              size_t* pLenIssuerCertArray,
                                                              const SOPC_CRLList* pIssuerCrls,
                                                              SOPC_SerializedCRL** pRawIssuerCrlArray,
                                                              size_t* pLenIssuerCrlArray)
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
        *pLenTrustedCertArray = 0;
        *pLenTrustedCrlArray = 0;
        *pLenIssuerCertArray = 0;
        *pLenIssuerCrlArray = 0;
    }

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pGenArray,
                                                                size_t lenArray,
                                                                SOPC_ByteString** pByteStringArray,
                                                                bool bIsCRL,
                                                                size_t* pByteLenTot)
{
    if (NULL == pGenArray || 0 == lenArray || NULL == pByteStringArray)
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

    for (size_t i = 0; i < lenArray && SOPC_STATUS_OK == status; i++)
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
        *pByteLenTot = 0;
    }

    *pByteStringArray = pBsArray;

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(
    const SOPC_SerializedCertificate* pRawTrustedCertArray,
    const size_t lenTrustedCertArray,
    SOPC_ByteString** pBsTrustedCertArray,
    const SOPC_SerializedCRL* pRawTrustedCrlArray,
    const size_t lenTrustedCrlArray,
    SOPC_ByteString** pBsTrustedCrlArray,
    const SOPC_SerializedCertificate* pRawIssuerCertArray,
    const size_t lenIssuerCertArray,
    SOPC_ByteString** pBsIssuerCertArray,
    const SOPC_SerializedCRL* pRawIssuerCrlArray,
    const size_t lenIssuerCrlArray,
    SOPC_ByteString** pBsIssuerCrlArray,
    size_t* pByteLenTot)
{
    if ((NULL == pRawTrustedCertArray && 0 < lenTrustedCertArray) ||
        (NULL == pRawTrustedCrlArray && 0 < lenTrustedCrlArray) ||
        (NULL == pRawIssuerCertArray && 0 < lenIssuerCertArray) ||
        (NULL == pRawIssuerCrlArray && 0 < lenIssuerCrlArray))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pBsTrustedCertArray || NULL == pBsTrustedCrlArray || NULL == pBsIssuerCertArray ||
        NULL == pBsIssuerCrlArray)
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
        *pByteLenTot = 0;
    }

    return status;
}

static SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                               size_t length,
                                                               SOPC_CertificateList** ppCert)
{
    if (NULL == pArray || 0 == length || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pCert = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    size_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        if (NULL == pBsCert)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCert);
        }
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
                                                              size_t length,
                                                              SOPC_CRLList** ppCrl)
{
    if (NULL == pArray || 0 == length || NULL == ppCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CRLList* pCrl = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    size_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        if (NULL == pBsCert)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCrl);
        }
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
        if (NULL == pDecode->TrustedCertificates || 0 == pDecode->NoOfTrustedCertificates)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        status = trustList_write_bs_array_to_cert_list(pDecode->TrustedCertificates,
                                                       (size_t) pDecode->NoOfTrustedCertificates, &pTrustedCerts);
    }
    /* Trusted CRLs */
    if (SOPC_TL_MASK_TRUSTED_CRLS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (NULL == pDecode->TrustedCrls || 0 == pDecode->NoOfTrustedCrls)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        status = trustList_write_bs_array_to_crl_list(pDecode->TrustedCrls, (size_t) pDecode->NoOfTrustedCrls,
                                                      &pTrustedCrls);
    }
    /* Issuer Certificates  */
    if (SOPC_TL_MASK_ISSUER_CERTS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (NULL == pDecode->IssuerCertificates || 0 == pDecode->NoOfIssuerCertificates)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        status = trustList_write_bs_array_to_cert_list(pDecode->IssuerCertificates,
                                                       (size_t) pDecode->NoOfIssuerCertificates, &pIssuerCerts);
    }
    /* Issuer CRLs */
    if (SOPC_TL_MASK_ISSUER_CRLS & pDecode->SpecifiedLists && SOPC_STATUS_OK == status)
    {
        if (NULL == pDecode->IssuerCrls || 0 == pDecode->NoOfIssuerCrls)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        status =
            trustList_write_bs_array_to_crl_list(pDecode->IssuerCrls, (size_t) pDecode->NoOfIssuerCrls, &pIssuerCrls);
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

/* Generate a random handle */
SOPC_ReturnStatus TrustList_GenRandHandle(SOPC_TrustListContext* pTrustList)
{
    if (NULL == pTrustList)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    pTrustList->handle = 38;
    return SOPC_STATUS_OK;
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
        pTrustList->openingMode = mode;
        break;
    default:
        pTrustList->openingMode = SOPC_TL_OPEN_MODE_UNKNOWN;
        break;
    }
    return res;
}

/* Set the opening masks for the specifiedLists of the TrustList. */
bool TrustList_SetOpenMasks(SOPC_TrustListContext* pTrustList, SOPC_TrLst_Mask masks)
{
    SOPC_ASSERT(NULL != pTrustList);

    if (masks & ~SOPC_TL_MASK_ALL)
    {
        pTrustList->openingMask = SOPC_TL_MASK_NONE;
        return false;
    }
    pTrustList->openingMask = masks;
    return true;
}

/* Set the TrustList position */
SOPC_ReturnStatus TrustList_SetPosition(SOPC_TrustListContext* pTrustList, uint64_t pos)
{
    SOPC_ASSERT(NULL != pTrustList);

    /* TrustList is not yet encode */
    if (NULL == pTrustList->pTrustListEncoded)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* Check before casting */
    uint64_t position = pos;
    if (UINT32_MAX < position)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    uint32_t bufLength = 0;
    uint32_t bufPos = 0;
    uint32_t bufRemaining = SOPC_Buffer_Remaining(pTrustList->pTrustListEncoded);
    SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(pTrustList->pTrustListEncoded, &bufPos);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bufLength = bufPos + bufRemaining;
    /* if the position is higher than the file size the position is set to the end of the file*/
    if (position > bufLength)
    {
        position = bufLength;
    }
    status = SOPC_Buffer_SetPosition(pTrustList->pTrustListEncoded, (uint32_t) position);
    return status;
}

/* Get the TrustList handle */
uint32_t TrustList_GetHandle(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    return (uint32_t) pTrustList->handle;
}

/* Get the TrustList position */
uint64_t TrustList_GetPosition(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);

    uint64_t pos = 0;
    if (NULL != pTrustList->pTrustListEncoded)
    {
        SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(pTrustList->pTrustListEncoded, (uint32_t*) &pos);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    return pos;
}

/* Check the TrustList handle */
bool TrustList_CheckHandle(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_Handle expected, const char* msg)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool match = expected == pTrustList->handle;
    if (!match && NULL != msg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s: %s", pTrustList->cStrObjectId, msg);
    }
    return match;
}

/* Check the TrustList opening mode */
bool TrustList_CheckOpenMode(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_OpenMode expected, const char* msg)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool match = expected == pTrustList->openingMode;
    if (!match && NULL != msg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TrustList:%s:%s", pTrustList->cStrObjectId, msg);
    }
    return match;
}

/* Check if the TrustList is open */
bool TrustList_CheckIsOpen(const SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);
    bool isOpen = SOPC_TL_OPEN_MODE_UNKNOWN != pTrustList->openingMode && SOPC_TL_MASK_NONE != pTrustList->openingMask;
    return isOpen;
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
    SOPC_ASSERT(NULL == pTrustList->pTrustListEncoded);
    SOPC_ASSERT(NULL != pTrustList->pPKI);

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
    size_t nbTrustedCerts = 0;
    size_t nbTrustedCrls = 0;
    size_t nbIssuerCerts = 0;
    size_t nbIssuerCrls = 0;
    /* Instance of TrustListDataType */
    OpcUa_TrustListDataType pTrustListDataType = {0};
    SOPC_ByteString* pBsTrustedCertArray = NULL;
    SOPC_ByteString* pBsTrustedCrlArray = NULL;
    SOPC_ByteString* pBsIssuerCertArray = NULL;
    SOPC_ByteString* pBsIssuerCrlArray = NULL;
    /* UA Binary encoded stream containing an instance of TrustListDataType */
    size_t nbCerts = 0;
    size_t byteLenTot = 0;
    size_t lenBuffer = 0;
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

    if (SOPC_STATUS_OK == status)
    {
        /* Filter with specifiedLists and leave NULL pRawXXXXArray if not required.

           The ownership of the memory area for each data of the array is the CertificateList and CRLList
           copied from the PKI */

        status = trustlist_attach_certs_to_raw_arrays(pTrustList->openingMask, pTrustedCerts, &pRawTrustedCertArray,
                                                      &nbTrustedCerts, pTrustedCrls, &pRawTrustedCrlArray,
                                                      &nbTrustedCrls, pIssuerCerts, &pRawIssuerCertArray,
                                                      &nbIssuerCerts, pIssuerCrls, &pRawIssuerCrlArray, &nbIssuerCrls);
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
        status = SOPC_STATUS_INVALID_STATE;
    }
    /* Check before casting */
    if (UINT32_MAX < lenBuffer)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    /* Fill the instance of the TrustListDataType */
    if (SOPC_STATUS_OK == status)
    {
        pTrustListDataType.SpecifiedLists = pTrustList->openingMask;
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
    pTrustList->pTrustListEncoded = pBufferTrustListDataType;
    return status;
}

/* Read bytes from the current position of the encoded TrustListDataType */
SOPC_ReturnStatus TrustList_Read(SOPC_TrustListContext* pTrustList, int32_t reqLength, SOPC_ByteString* pDest)
{
    if (NULL == pTrustList || NULL == pDest || 0 == reqLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pTrustList->pTrustListEncoded || NULL != pDest->Data || -1 != pDest->Length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t lengthToWrite = (uint32_t) reqLength;
    uint32_t sizeAvailable = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Get the remaining length */
    sizeAvailable = SOPC_Buffer_Remaining(pTrustList->pTrustListEncoded);
    /* Check before casting */
    if (INT32_MAX < sizeAvailable)
    {
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
        status = SOPC_Buffer_Read(pDest->Data, pTrustList->pTrustListEncoded, lengthToWrite);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pDest->Data);
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
    if (NULL == pEncodedTrustListDataType->Data || 0 == pEncodedTrustListDataType->Length)
    {
        return SOPC_STATUS_OK;
    }
    SOPC_Buffer* pToDecode =
        SOPC_Buffer_Attach(pEncodedTrustListDataType->Data, (uint32_t) pEncodedTrustListDataType->Length);
    if (NULL == pToDecode)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    OpcUa_TrustListDataType* pTrustListDataType = NULL;
    SOPC_EncodeableType* type = &OpcUa_TrustListDataType_EncodeableType;
    void* pValue = NULL;
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(pToDecode, 0);
    if (SOPC_STATUS_OK == status)
    {
        pValue = SOPC_Calloc(1, type->AllocationSize);
        if (NULL == pValue)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_EncodeableObject_Initialize(type, pValue);
        status = SOPC_EncodeableObject_Decode(type, pValue, pToDecode, 1);
    }
    if (SOPC_STATUS_OK == status)
    {
        /* TrustList support only the write mode with the EraseExisting option */
        SOPC_KeyManager_Certificate_Free(pTrustList->pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(pTrustList->pIssuerCerts);
        SOPC_KeyManager_CRL_Free(pTrustList->pTrustedCRLs);
        SOPC_KeyManager_CRL_Free(pTrustList->pIssuerCRLs);
        pTrustListDataType = (OpcUa_TrustListDataType*) pValue;
        if (NULL == pTrustListDataType)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_write_decoded_data(pTrustListDataType, &pTrustList->pTrustedCerts, &pTrustList->pTrustedCRLs,
                                              &pTrustList->pIssuerCerts, &pTrustList->pIssuerCRLs);
    }

    /* Clear */
    SOPC_EncodeableObject_Clear(type, pValue);
    SOPC_Free(pValue);
    SOPC_Free(pToDecode);

    return status;
}

/* Reset the TrustList context when close method is call */
void TrustList_Reset(SOPC_TrustListContext* pTrustList)
{
    SOPC_ASSERT(NULL != pTrustList);

    SOPC_Buffer_Delete(pTrustList->pTrustListEncoded);
    SOPC_KeyManager_Certificate_Free(pTrustList->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustList->pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustList->pTrustedCRLs);
    SOPC_KeyManager_CRL_Free(pTrustList->pIssuerCRLs);

    pTrustList->handle = SOPC_TRUSTLIST_INVALID_HANDLE;
    pTrustList->openingMode = SOPC_TL_OPEN_MODE_UNKNOWN;
    pTrustList->openingMask = SOPC_TL_MASK_NONE;
    pTrustList->pTrustListEncoded = NULL;
    pTrustList->pTrustedCerts = NULL;
    pTrustList->pIssuerCerts = NULL;
    pTrustList->pTrustedCRLs = NULL;
    pTrustList->pIssuerCRLs = NULL;
    pTrustList->bDoNotDelete = false;
}
