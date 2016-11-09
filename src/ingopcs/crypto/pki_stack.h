/** \file pki_stack.h
 *
 * Defines the minimal PKI implementation provided by the stack.
 *
 * The stack will not to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 *  Created on: 28 oct. 2016
 *      Author: PAB
 */

#ifndef INGOPCS_PKI_STACK_H_
#define INGOPCS_PKI_STACK_H_


#include "sopc_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"
#include "pki.h"


/**
 * \brief           Creates the minimal validation implementation provided by the stack, which fulfills the PKIProvider interface.
 *
 *                  This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *                  with a single certificate authority, and an optional revocation list.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the Certificate of the certification authority.
 * \param pRevocationList  An optional certificate chain containing the revocation list. If NULL, no revocation list is checked.
 * \param ppPKI     A valid pointer to the newly created PKIProvider.
 *                  You should free such provider with PKIProviderStack_Free
 *
 * \note            Content of the pki is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode PKIProviderStack_Create(Certificate *pCertAuth,
                                   CertificateRevList *pRevocationList,
                                   PKIProvider **ppPKI);

/**
 * \brief           Frees a pki created with PKIProviderStack_Create().
 *
 * \param pPKI      A valid pointer to the pki to free.
 */
void PKIProviderStack_Free(PKIProvider *pPKI);


#endif /* INGOPCS_PKI_STACK_H_ */
