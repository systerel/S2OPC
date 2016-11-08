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


#include "../core_types/sopc_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"
#include "pki.h"


/**
 * \brief   The minimal validation implementation, with a single certificate authority.
 *
 *          This verifies the certificate in the safest manner (whole certificate chain, date validation).
 *
 * \warning Certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param [in, optional] pRevocationList    A certificate chain containing the revocation list. If NULL, no revocation list is checked.
 */
SOPC_StatusCode PKIProviderStack_Create(Certificate *pCertAuth,
                                   CertificateRevList *pRevocationList,
                                   PKIProvider **ppPKI);
void PKIProviderStack_Free(PKIProvider *pPKI);


#endif /* INGOPCS_PKI_STACK_H_ */
