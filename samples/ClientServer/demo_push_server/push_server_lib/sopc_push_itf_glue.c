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

#include "sopc_push_itf_glue.h"
#include "sopc_macros.h"

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_GetTrustedCertificates(const SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCerts)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(ppCerts);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_PKIProvider_GetTrustedCRLs(const SOPC_PKIProvider* pPKI, SOPC_CRLList** ppCrls)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(ppCrls);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_PKIProvider_GetIssuerCertificates(const SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCerts)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(ppCerts);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_PKIProvider_GetIssuerCRLs(const SOPC_PKIProvider* pPKI, SOPC_CRLList** ppCrls)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(ppCrls);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(const char* thumbprint, SOPC_PKIProvider* pPKI)
{
    SOPC_UNUSED_ARG(thumbprint);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_RemoveInList(const char* thumbprint, SOPC_CertificateList* pCerts)
{
    SOPC_UNUSED_ARG(thumbprint);
    SOPC_UNUSED_ARG(pCerts);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDerArray(const SOPC_CertificateList* pCerts, uint8_t** pDerArray)
{
    SOPC_UNUSED_ARG(pCerts);
    SOPC_UNUSED_ARG(pDerArray);
    return SOPC_STATUS_OK;
}
SOPC_ReturnStatus SOPC_KeyManager_CRL_ToDerArray(const SOPC_CRLList* pCrls, uint8_t** pDerArray)
{
    SOPC_UNUSED_ARG(pCrls);
    SOPC_UNUSED_ARG(pDerArray);
    return SOPC_STATUS_OK;
}
