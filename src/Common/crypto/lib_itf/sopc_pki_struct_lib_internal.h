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
 * \brief Defines the PKI structure used internally.
 *
 * \note Those structures are declared as common to all libraries
 *       but it might be changed to be specific in the future (only used by tests).
 */

#ifndef SOPC_PKI_STRUCT_LIB_INTERNAL_H_
#define SOPC_PKI_STRUCT_LIB_INTERNAL_H_

#include "sopc_crypto_decl.h"
#include "sopc_mutexes.h"
#include "sopc_pki_decl.h"

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
struct SOPC_PKIProvider
{
    SOPC_Mutex mutex;                    /*!< The mutex used to have thread-safe accesses to PKI.
                                              IMPORTANT: it shall remain the first field for SOPC_Internal_ReplacePKIAndClear treatment */
    char* directoryStorePath;            /*!< The directory store path of the PKI*/
    SOPC_CertificateList* pTrustedCerts; /*!< Trusted intermediate CA + trusted certificates*/
    SOPC_CertificateList* pTrustedRoots; /*!< Trusted root CA*/
    SOPC_CRLList* pTrustedCrl;           /*!< CRLs of trusted intermediate CA and trusted root CA*/
    SOPC_CertificateList* pIssuerCerts;  /*!< Issuer intermediate CA*/
    SOPC_CertificateList* pIssuerRoots;  /*!< Issuer root CA*/
    SOPC_CRLList* pIssuerCrl;            /*!< CRLs of issuer intermediate CA and issuer root CA*/
    SOPC_CertificateList* pRejectedList; /*!< The list of Certificates that have been rejected */

    SOPC_CertificateList* pAllCerts;   /*!< Issuer certs + trusted certs (root not included)*/
    SOPC_CertificateList* pAllRoots;   /*!< Issuer roots + trusted roots*/
    SOPC_CertificateList* pAllTrusted; /*!< trusted root + trusted intermediate CAs + trusted certs */

    SOPC_CRLList* pAllCrl;                /*!< Issuer CRLs + trusted CRLs */
    SOPC_FnValidateCert* pFnValidateCert; /*!< Pointer to validation function*/
    bool isPermissive;                    /*!< Define whatever the PKI is permissive (without security)*/
};

#endif /* SOPC_PKI_STRUCT_LIB_INTERNAL_H_ */
