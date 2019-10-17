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
 * \brief           Creates the minimal validation implementation provided by the stack, which fulfills the PKIProvider
 * interface.
 *
 *                  This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *                  with a single certificate authority, and an optional revocation list.
 *                  It requires a certificate authority signed with SHA-256, and an RSA private key which is at least
 *                  2048 bits long.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the Certificate of the certification authority.
 * \param pRevocationList  An optional certificate chain containing the revocation list. If NULL, no revocation list is
 *                  checked.
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
SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               struct SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);

#endif /* SOPC_PKI_STACK_H_ */
