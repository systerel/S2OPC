/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

struct SOPC_CertificateRevList;

/**
 * \brief           Creates the minimal validation implementation provided by the stack, which fulfills the PKIProvider
 * interface.
 *
 *                  This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *                  with a single certificate authority, and an optional revocation list.
 *                  It requires a certificate authority signed with SHA-256, and an RSA private key which is at least
 * 2048 bits long.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the Certificate of the certification authority.
 * \param pRevocationList  An optional certificate chain containing the revocation list. If NULL, no revocation list is
 * checked. \param ppPKI     A valid pointer to the newly created PKIProvider. You should free such provider with
 * PKIProviderStack_Free
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_Certificate* pCertAuth,
                                               struct SOPC_CertificateRevList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);

/**
 * \brief           Frees a pki created with PKIProviderStack_Create().
 *
 * \param pPKI      A valid pointer to the pki to free.
 */
void SOPC_PKIProviderStack_Free(SOPC_PKIProvider* pPKI);

#endif /* SOPC_PKI_STACK_H_ */
