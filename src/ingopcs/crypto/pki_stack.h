/** \file pki_stack.h
 *
 * Defines the minimal PKI implementation provided by the stack.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 *  Created on: 28 oct. 2016
 *      Author: PAB
 */

#ifndef INGOPCS_PKI_STACK_H_
#define INGOPCS_PKI_STACK_H_


#include "ua_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"
#include "pki.h"


extern const PKIProvider g_pkiStack; /**> The PKI instance */


/**
 * \brief   The minimal validation implementation.
 *
 *          This verifies the certificate in the safest manner (whole certificate chain, date validation).
 *
 * \param [in, optional] pRevocationList    A certificate chain containing the revocation list. If NULL, no revocation list is checked.
 */
StatusCode PKIProviderStack_ValidateCertificate(const PKIProvider *pPKI,
                                                const Certificate *pToValidate,
                                                const Certificate *pCertificateAuthority,
                                                const CertificateRevList *pRevocationList);


#endif /* INGOPCS_PKI_STACK_H_ */
