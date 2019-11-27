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

/** \file sopc_pki_stack.h
 *
 * \brief Defines the minimal PKI implementation provided by the stack.
 *
 * The stack will not to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 */

#ifndef SOPC_PKI_STACK_H_
#define SOPC_PKI_STACK_H_

#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_pki.h"

struct SOPC_CRLList;

/**
 * \brief           Creates the minimal validation implementation provided by the stack,
 *                  which fulfills the SOPC_PKIProvider interface.
 *
 *                  This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *                  with a single certificate authority, and its revocation list.
 *                  Certificate authority requirements depend on the chosen OPC UA security policy.
 *                  The CRL must be the CRL of the certificate authority.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the serialized certificate of the certification authority.
 *                  This object is internally copied, and only the internal copy is freed by SOPC_PKIProvider_Free.
 * \param pRevocationList  An certificate chain containing the revocation list of the certificate authority.
 *                  This object is borrowed and is freed by SOPC_PKIProvider_Free.
 * \param ppPKI     A valid pointer to the newly created SOPC_PKIProvider. You should free such provider with
 *                  SOPC_PKIProvider_Free().
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            The pki is not safe to share across threads.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               struct SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);

/**
 * \brief           Creates a SOPC_PKIProviderStack using lists of paths.
 *
 *                  This verifies the certificate in the safest manner (whole certificate chain, with date validation,
 *                  mandatory certificate revocation lists).
 *                  Certificate Authority (CA) requirements depend on the chosen OPC UA security policy.
 *                  The list of Certificate Revocation List (CRL) must contain exactly one list for each CA of the
 *                  CA list.
 *
 *                  Issued certificates are also accepted (certificates whose chain is not trusted)
 *                  and must be added to the certificate authority list.
 *                  Issued certificates are always trusted, they cannot be revoked,
 *                  as they cannot appear on a CRL of a trusted CA.
 *                  They have no CRL, as they don't sign other certificates (otherwise, they are regular CAs).
 *
 * \param lPathCA   A pointer to an array of paths to each certificate (or trusted certificate) to use
 *                  in the validation chain.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathCRL  A pointer to an array of paths to each certificate revocation list to use.
 *                  Each certificate of the CA list must have a valid CRL in the CRL list.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the CRL from the current working directory.
 * \param ppPKI     A valid pointer to the newly created PKIProvider. You should free such provider with
 *                  SOPC_PKIProvider_Free().
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            The pki is not safe to share across threads.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathCA, char** lPathCRL, SOPC_PKIProvider** ppPKI);

#endif /* SOPC_PKI_STACK_H_ */
