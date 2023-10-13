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

/** \file pki_stack.c
 *
 * The minimal PKI implementation provided by the stack. It is lib-specific.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 */

#include <stdio.h>
#include <string.h>

#include "sopc_common_constants.h"
#include "sopc_crypto_decl.h"
#include "sopc_macros.h"

#include "sopc_pki_stack.h"

SOPC_ReturnStatus SOPC_PKIProvider_CreateLeafProfile(const char* securityPolicyUri, SOPC_PKI_LeafProfile** ppProfile)
{
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetUsageFromType(SOPC_PKI_LeafProfile* pProfile, SOPC_PKI_Type PKIType)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(PKIType);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURI(SOPC_PKI_LeafProfile* pProfile, const char* applicationUri)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(applicationUri);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURL(SOPC_PKI_LeafProfile* pProfile, const char* url)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(url);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_PKIProvider_DeleteLeafProfile(SOPC_PKI_LeafProfile** ppProfile)
{
    SOPC_UNUSED_ARG(ppProfile);
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateProfile(const char* securityPolicyUri, SOPC_PKI_Profile** ppProfile)
{
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetUsageFromType(SOPC_PKI_Profile* pProfile, SOPC_PKI_Type PKIType)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(PKIType);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURI(SOPC_PKI_Profile* pProfile, const char* applicationUri)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(applicationUri);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURL(SOPC_PKI_Profile* pProfile, const char* url)
{
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(url);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_PKIProvider_DeleteProfile(SOPC_PKI_Profile** ppProfile)
{
    SOPC_UNUSED_ARG(ppProfile);
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateMinimalUserProfile(SOPC_PKI_Profile** ppProfile)
{
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_AddCertToRejectedList(SOPC_PKIProvider* pPKI, const SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pCert);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_VerifyEveryCertificate(SOPC_PKIProvider* pPKI,
                                                          const SOPC_PKI_ChainProfile* pProfile,
                                                          uint32_t** pErrors,
                                                          char*** ppThumbprints,
                                                          uint32_t* pLength)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(pErrors);
    SOPC_UNUSED_ARG(ppThumbprints);
    SOPC_UNUSED_ARG(pLength);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                        const SOPC_PKI_LeafProfile* pProfile,
                                                        uint32_t* error)
{
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  SOPC_PKIProvider** ppPKI)
{
    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(ppPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromStore(const char* directoryStorePath, SOPC_PKIProvider** ppPKI)
{
    if (NULL == directoryStorePath || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIPermissive_Create(SOPC_PKIProvider** ppPKI)
{
    SOPC_UNUSED_ARG(ppPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI)
{
    SOPC_UNUSED_ARG(ppPKI);
}

SOPC_ReturnStatus SOPC_PKIProvider_SetStorePath(const char* directoryStorePath, SOPC_PKIProvider* pPKI)
{
    SOPC_UNUSED_ARG(directoryStorePath);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl)
{
    SOPC_UNUSED_ARG(ppTrustedCerts);
    SOPC_UNUSED_ARG(ppTrustedCrl);
    SOPC_UNUSED_ARG(ppIssuerCerts);
    SOPC_UNUSED_ARG(ppIssuerCrl);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteToStore(SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles)
{
    SOPC_UNUSED_ARG(bEraseExistingFiles);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CopyRejectedList(SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARG(ppCert);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteRejectedCertToStore(SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(bEraseExistingFiles);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_UpdateFromList(SOPC_PKIProvider* pPKI,
                                                  const char* securityPolicyUri,
                                                  SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  const bool bIncludeExistingList)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(bIncludeExistingList);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(SOPC_PKIProvider* pPKI,
                                                     const char* pThumbprint,
                                                     const bool bIsTrusted,
                                                     bool* pIsRemoved,
                                                     bool* pIsIssuer)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pThumbprint);
    SOPC_UNUSED_ARG(bIsTrusted);
    SOPC_UNUSED_ARG(pIsRemoved);
    SOPC_UNUSED_ARG(pIsIssuer);
    return SOPC_STATUS_NOT_SUPPORTED;
}
