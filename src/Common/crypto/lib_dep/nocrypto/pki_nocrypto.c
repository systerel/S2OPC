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

SOPC_ReturnStatus SOPC_PKIProvider_CheckHostName(const SOPC_CertificateList* pToValidate, const char* url)
{
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(url);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckCertificateUsage(const SOPC_CertificateList* pToValidate,
                                                         const SOPC_PKI_LeafProfile* pProfile)
{
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckSecurityPolicy(const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_LeafProfile* pConfig)
{
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pConfig);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_PKIProvider_GetListStats(SOPC_CertificateList* pCert,
                                   uint32_t* caCount,
                                   uint32_t* listLength,
                                   uint32_t* rootCount)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(caCount);
    SOPC_UNUSED_ARG(listLength);
    SOPC_UNUSED_ARG(rootCount);
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckCommonName(const SOPC_CertificateList* pToValidate)
{
    SOPC_UNUSED_ARG(pToValidate);
    return SOPC_STATUS_NOT_SUPPORTED;
}