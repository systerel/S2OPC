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


#include <stdlib.h>

#include "ua_base_types.h"
#include "crypto_provider.h"
#include "key_manager.h"
#include "pki.h"
#include "pki_stack.h"

#include "mbedtls/x509.h"


static StatusCode PKIProviderStack_ValidateCertificate(const PKIProvider *pPKI,
                                                       const Certificate *pToValidate)
{
    (void)(pPKI);
    Certificate *cert_ca = NULL;
    CertificateRevList *cert_rev_list = NULL;
    mbedtls_x509_crl *rev_list = NULL;
    uint32_t failure_reasons = 0;

    if(NULL == pPKI || NULL == pToValidate)
        return STATUS_INVALID_PARAMETERS;
    if(NULL == pPKI->pFnValidateCertificate || NULL == pPKI->pUserCertAuthList) // TODO: useful pFn verification?
        return STATUS_INVALID_PARAMETERS;

    // Gathers certificates from pki structure
    if(NULL != pPKI->pUserCertRevocList)
    {
        cert_rev_list = (CertificateRevList *)(pPKI->pUserCertRevocList);
        rev_list = (mbedtls_x509_crl *)(&cert_rev_list->crl);
    }
    cert_ca = (Certificate *)(pPKI->pUserCertAuthList);

    // Now, verifies the certificate
    // crt are not const in crt_verify, but this function does not look like to modify them
    if(mbedtls_x509_crt_verify((mbedtls_x509_crt *)(&pToValidate->crt),
                               (mbedtls_x509_crt *)(&cert_ca->crt),
                               rev_list,
                               NULL, /* You can specify an expected Common Name here */
                               &failure_reasons,
                               NULL, NULL) != 0)
        // TODO: you could further analyze here...
        return STATUS_NOK;

    return STATUS_OK;
}


StatusCode PKIProviderStack_New(Certificate *pCertAuth,
                                CertificateRevList *pRevocationList,
                                PKIProvider **ppPKI)
{
    PKIProvider *pki = NULL;

    if(NULL == pCertAuth || NULL == ppPKI)
        return STATUS_INVALID_PARAMETERS;

    pki = (PKIProvider *)malloc(sizeof(PKIProvider));
    if(NULL == pki)
        return STATUS_NOK;

    pki->pFnValidateCertificate = PKIProviderStack_ValidateCertificate;
    pki->pUserCertAuthList = pCertAuth;
    pki->pUserCertRevocList = pRevocationList; // Can be NULL
    pki->pUserData = NULL;
    *ppPKI = pki;

    return STATUS_OK;
}


void PKIProviderStack_Free(PKIProvider *pPKI)
{
    free((void *)pPKI);
}

