/** \file pki.h
 *
 * Defines the common interface that a PKI should provide. This is a minimal interface, as the main
 * API for certificate and key manipulation is provided by the KeyManager.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 *  Created on: 28 oct. 2016
 *      Author: PAB
 */

#ifndef INGOPCS_PKI_H_
#define INGOPCS_PKI_H_


struct PKIProvider;

#include "ua_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"


typedef StatusCode (*FnValidateCertificate) (const struct PKIProvider *pPKI,
                                             const Certificate *pToValidate);


// TODO: find a more appropriate name, such as PublicKeyInfra, CertificateValdiation, CryptoPKI
typedef struct PKIProvider
{
    FnValidateCertificate pFnValidateCertificate; /**> The validation function, which is called by the CryptoProvider API. */

    void *pUserCertAuthList;    /**> Placeholder for certificate authorities. */
    void *pUserCertRevocList;   /**> Placeholder for certificate revocation list. */
    void *pUserData;            /**> Placeholder for more data. */
} PKIProvider;


#endif /* INGOPCS_PKI_H_ */
