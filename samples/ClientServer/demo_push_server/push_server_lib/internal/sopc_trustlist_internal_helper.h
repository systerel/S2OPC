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

#ifndef SOPC_TRUSTLIST_HELPER_H_
#define SOPC_TRUSTLIST_HELPER_H_

#include "sopc_pki_decl.h"
#include "sopc_trustlist.h"
#include "sopc_types.h"

/*---------------------------------------------------------------------------
 *                           TrustList Encode/Decode
 *---------------------------------------------------------------------------*/
/* NOTE: For the moment, these two functions are used for testing purpose only. */

/**
 * @brief Convert a Trustlist in ByteString form into the OPC UA TrustList object.
 *
 * @param[in] trustListData The ByteString TL to convert.
 * @return OpcUa_TrustListDataType* A pointer to the OPC UA TrustList.
 */
OpcUa_TrustListDataType* SOPC_TrustList_DecodeTrustListData(SOPC_ByteString* trustListData);

/**
 * @brief Convert an OPC UA TrustList object into the ByteString form.
 *
 * @param[in] trustList The OPC UA TrustList to convert.
 * @param[out] trustListData A pointer to the converted TrustList.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TrustList_EncodeTrustListData(OpcUa_TrustListDataType* trustList,
                                                     SOPC_ByteString* trustListData);

/*---------------------------------------------------------------------------
 *                           TrustList helpers
 *---------------------------------------------------------------------------*/
SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                        uint32_t length,
                                                        SOPC_CertificateList** ppCert);

SOPC_ReturnStatus trustList_write_bs_array_to_crl_list(SOPC_ByteString* pArray, uint32_t length, SOPC_CRLList** ppCrl);

SOPC_ReturnStatus trustlist_attach_certs_to_raw_arrays(const SOPC_TrLst_Mask specifiedLists,
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

SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pGenArray,
                                                         uint32_t lenArray,
                                                         SOPC_ByteString** pByteStringArray,
                                                         bool bIsCRL,
                                                         uint32_t* pByteLenTot);

SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(const SOPC_SerializedCertificate* pRawTrustedCertArray,
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

#endif /* SOPC_TRUSTLIST_HELPER_H_ */
