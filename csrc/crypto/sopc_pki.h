/*
 *  Copyright (C) 2016 Systerel and others.
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

/** \file sopc_pki.h
 *
 * \brief Defines the common interface that a PKI should provide. This is a minimal interface, as the main
 * API for certificate and key manipulation is provided by KeyManager.
 *
 * The stack will not provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 * The stack will not provide any advanced certificate storage.
 * You can use "user-specific" handles in the PKIProvider struct to implement more options.
 *
 * The pFnValidateCertificate function should not be called directly, but you should call
 * CryptoProvider_Certificate_Validate() instead.
 */

#ifndef SOPC_PKI_H_
#define SOPC_PKI_H_

#include "sopc_crypto_decl.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_toolkit_constants.h"

typedef SOPC_ReturnStatus (*SOPC_FnValidateCertificate)(const struct SOPC_PKIProvider* pPKI,
                                                        const SOPC_Certificate* pToValidate);

/**
 * \brief   The PKIProvider object defines the common interface for the Public Key Infrastructure.
 */
struct SOPC_PKIProvider
{
    /**
     *  \brief          The validation function, which is wrapped by CryptoProvider_Certificate_Validate().
     *
     *                  It implements the validation of the certificate. The CryptoProvider_Certificate_Validate()
     * assumes that a SOPC_STATUS_OK from this function means that the certificate can be trusted. Parameters are
     * validated by CryptoProvider_Certificate_Validate().
     *
     *  \param pPKI     A valid pointer to the PKIProvider.
     *  \param pToValidate  A valid pointer to the Certificate to validate.
     *
     *  \return         SOPC_STATUS_OK when the certificate is successfully validated, and
     * SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK.
     */
    const SOPC_FnValidateCertificate pFnValidateCertificate;

    /** \brief PKI implementations can use this placeholder to store handles to certificate authorities. */
    void* pUserCertAuthList;
    /** \brief PKI implementations can use this placeholder to store handles to certificate revocation list(s). */
    void* pUserCertRevocList;
    /** \brief PKI implementations can use this placeholder to store more specific data. */
    void* pUserData;
};

#endif /* SOPC_PKI_H_ */
