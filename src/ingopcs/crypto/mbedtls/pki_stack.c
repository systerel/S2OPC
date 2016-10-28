/** \file pki_stack.c
 *
 * The minimal PKI implementation provided by the stack. It is lib-specific.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 *  Created on: 28 oct. 2016
 *      Author: PAB
 */


#include "ua_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"
#include "pki.h"
#include "pki_stack.h"

#include "mbedtls/x509.h"


const PKIProvider g_pkiStack = {
        .pFnValidateCertificate = PKIProviderStack_ValidateCertificate,
};


StatusCode PKIProviderStack_ValidateCertificate(const PKIProvider *pPKI,
                                                const Certificate *pToValidate,
                                                const Certificate *pCertificateAuthority,
                                                const CertificateRevList *pRevocationList)
{
    (void)(pPKI);
    mbedtls_x509_crl *rev_list = NULL;
    uint32_t failure_reasons = 0;

    if(NULL == pToValidate || NULL == pCertificateAuthority)
        return STATUS_INVALID_PARAMETERS;

    if(NULL != pRevocationList)
        rev_list = (mbedtls_x509_crl *)(&pRevocationList->crl);
    // crt are not const in crt_verify, but this function does not look like to modify them
    if(mbedtls_x509_crt_verify((mbedtls_x509_crt *)(&pToValidate->crt),
                               (mbedtls_x509_crt *)(&pCertificateAuthority->crt),
                               rev_list,
                               NULL, /* You can specify an expected Common Name here */
                               &failure_reasons,
                               NULL, NULL) != 0)
        // TODO: you could further analyze here...
        return STATUS_NOK;

    return STATUS_OK;
}
