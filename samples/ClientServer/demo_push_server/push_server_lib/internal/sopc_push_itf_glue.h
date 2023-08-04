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
 * \brief ITF glue to be deleted after rebase.
 */

#ifndef SOPC_PUSH_ITF_GLUE_
#define SOPC_PUSH_ITF_GLUE_

#include "sopc_enums.h"
#include "sopc_pki.h"

typedef enum
{
    SOPC_PKI_MD_SHA1,
    SOPC_PKI_MD_SHA256,
    SOPC_PKI_MD_SHA1_AND_SHA256,
    SOPC_PKI_MD_SHA1_OR_ABOVE,
    SOPC_PKI_MD_SHA256_OR_ABOVE,
} SOPC_PKI_MdSign;

typedef enum
{
    SOPC_PKI_PK_ANY,
    SOPC_PKI_PK_RSA
} SOPC_PKI_PkAlgo;

typedef enum
{
    SOPC_PKI_CURVES_ANY,
} SOPC_PKI_EllipticCurves;

typedef enum
{
    SOPC_PKI_KU_DISABLE_CHECK = 0x0000,
    SOPC_PKI_KU_NON_REPUDIATION = 0x0001,
    SOPC_PKI_KU_DIGITAL_SIGNATURE = 0x0002,
    SOPC_PKI_KU_KEY_ENCIPHERMENT = 0x0004,
    SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT = 0x0008,
    SOPC_PKI_KU_KEY_CERT_SIGN = 0x0010,
    SOPC_PKI_KU_KEY_CRL_SIGN = 0x00100
} SOPC_PKI_KeyUsage_Mask;

typedef enum
{
    SOPC_PKI_EKU_DISABLE_CHECK,
    SOPC_PKI_EKU_CLIENT_AUTH,
    SOPC_PKI_EKU_SERVER_AUTH,
} SOPC_PKI_ExtendedKeyUsage_Mask;

typedef struct SOPC_PKI_LeafProfile
{
    SOPC_PKI_MdSign mdSign;
    SOPC_PKI_PkAlgo pkAlgo;
    uint32_t RSAMinimumKeySize;
    uint32_t RSAMaximumKeySize;
    bool bApplySecurityPolicy;
    SOPC_PKI_KeyUsage_Mask keyUsage;
    SOPC_PKI_ExtendedKeyUsage_Mask extendedKeyUsage;
    char* sanApplicationUri;
    char* sanURL;
} SOPC_PKI_LeafProfile;

typedef struct SOPC_PKI_ChainProfile
{
    SOPC_PKI_MdSign mdSign;
    SOPC_PKI_PkAlgo pkAlgo;
    SOPC_PKI_EllipticCurves curves;
    uint32_t RSAMinimumKeySize;
} SOPC_PKI_ChainProfile;

typedef struct SOPC_PKI_Profile
{
    SOPC_PKI_LeafProfile* leafProfile;
    SOPC_PKI_ChainProfile* chainProfile;
    bool bBackwardInteroperability;
    bool bApplyLeafProfile;
} SOPC_PKI_Profile;

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error);

SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(const SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl);

SOPC_ReturnStatus SOPC_PKIProvider_GetTrustedCertificates(const SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCerts);
SOPC_ReturnStatus SOPC_PKIProvider_GetTrustedCRLs(const SOPC_PKIProvider* pPKI, SOPC_CRLList** ppCrls);
SOPC_ReturnStatus SOPC_PKIProvider_GetIssuerCertificates(const SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCerts);
SOPC_ReturnStatus SOPC_PKIProvider_GetIssuerCRLs(const SOPC_PKIProvider* pPKI, SOPC_CRLList** ppCrls);
SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(const char* thumbprint, SOPC_PKIProvider* pPKI);

SOPC_ReturnStatus SOPC_KeyManager_Certificate_RemoveInList(const char* thumbprint, SOPC_CertificateList* pCerts);

#endif /* SOPC_PUSH_ITF_GLUE_ */
