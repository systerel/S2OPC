/** \file pki.h
 *
 * Defines the common interface that a PKI should provide. This is a minimal interface, as the main
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
 *
 *  Created on: 28 oct. 2016
 *      Author: PAB
 */

#ifndef INGOPCS_PKI_H_
#define INGOPCS_PKI_H_


struct PKIProvider;

#include "sopc_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"


typedef SOPC_StatusCode (*FnValidateCertificate) (const struct PKIProvider *pPKI,
                                             const Certificate *pToValidate);


// TODO: find a more appropriate name, such as PublicKeyInfra, CertificateValdiation, CryptoPKI
/**
 * \brief   The PKIProvider object defines the common interface for the Public Key Infrastructure.
 */
typedef struct PKIProvider
{
    /**
     *  \brief          The validation function, which is wrapped by CryptoProvider_Certificate_Validate().
     *
     *                  It implements the validation of the certificate. The CryptoProvider_Certificate_Validate() assumes
     *                  that a STATUS_OK from this function means that the certificate can be trusted.
     *                  Parameters are validated by CryptoProvider_Certificate_Validate().
     *
     *  \param pPKI     A valid pointer to the PKIProvider.
     *  \param pToValidate  A valid pointer to the Certificate to validate.
     *
     *  \return         STATUS_OK when the certificate is successfully validated, and STATUS_INVALID_PARAMETERS or STATUS_NOK.
     */
    const FnValidateCertificate pFnValidateCertificate;

    /** \brief PKI implementations can use this placeholder to store handles to certificate authorities. */
    void *pUserCertAuthList;
    /** \brief PKI implementations can use this placeholder to store handles to certificate revocation list(s). */
    void *pUserCertRevocList;
    /** \brief PKI implementations can use this placeholder to store more specific data. */
    void *pUserData;
} PKIProvider;


#endif /* INGOPCS_PKI_H_ */
