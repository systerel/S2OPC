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

/**
 * \brief           Creates the minimal validation implementation provided by the stack,
 *                  which fulfills the SOPC_PKIProvider interface.
 *
 *   This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *   with a single certificate authority, and its revocation list.
 *   Certificate authority requirements depend on the chosen OPC UA security policy.
 *   The CRL must be the CRL of the certificate authority.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the serialized certificate of the certification authority.
 *                  This object is internally copied, and only the internal copy is freed by SOPC_PKIProvider_Free.
 * \param pRevocationList  An certificate chain containing the revocation list of the certificate authority.
 *                  This object is borrowed and is freed by SOPC_PKIProvider_Free.
 * \param ppPKI     A valid pointer to the newly created SOPC_PKIProvider. You should free such provider with
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
                                               SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);

/**
 * \brief   Define whether the PKI is used for user or application certificates,
 *          The extensions verification for users:
 *              - The keyUsage is expected to be filled with digitalSignature.
 *              - The extendedkeyUsage is not checked.
 *              - The CA flag is expected to be FALSE
 *          The extensions verification for applications:
 *              - The keyUsage is expected to be filled with digitalSignature, nonRepudiation, keyEncipherment
 *                and dataEncipherment.
 *              - The extendedkeyUsage is filled with serverAuth and/or clientAuth.
 *
 * \note By default, the PKI is used for application instance certificates.
 *
 * \param pPKI   A valid pointer to the SOPC_PKIProvider.
 * \param bIsUserPki Define whether the PKI is used for user or application certificates.
 *
 * \return  SOPC_STATUS_OK when successful or SOPC_STATUS_INVALID_PARAMETERS when \p pPKI is NULL.
 *
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_SetUserCert(SOPC_PKIProvider* pPKI, bool bIsUserPki);

/**
 * \brief           Creates a SOPC_PKIProviderStack using lists of paths.
 *
 *   This verifies the certificate in the safest manner (whole certificate chain, with date validation,
 *   mandatory certificate revocation lists).
 *   Certificate Authority (CA) requirements (such as the hash algorithm used for the signature)
 *   depend on the chosen OPC UA security policy.
 *
 *   There are 3 types of certificates to provide to the PKI:
 *   - The "trusted issuers" are CAs which issued certificates are also trusted.
 *     All the certificates of the signing chain including the root CA must be provided.
 *   - The "issued certificates" are certificates issued by untrusted CA.
 *     These certificates are considered trustworthy.
 *     The difference between trusted issuers and trusted issued certificates is that issued
 *     certificates are trusted on a one by one basis, whereas the trusted issuer may emit a large
 *     number of trusted certificates.
 *   - The "untrusted issuers" are CAs that are used to verify the signing chain of the
 *     "issued certificates". Each issued certificate must have its whole signing CA chain in the
 *     untrusted issuers or the trusted issuers up to the root CA.
 *
 *   In addition, there are two more concepts:
 *   - A link CA is part of the certificate validation chain.
 *     All links between a certificate and a root certificate must be provided.
 *   - A root CA is all always trusted, even if there are other root CAs that signed it.
 *     Hence the parent of root CAs will never be checked, and the validation stops on root CAs.
 *
 *   The list of Certificate Revocation List (CRL) must contain exactly one list for each CA of the
 *   provided CAs, either link or root, trusted or untrusted.
 *
 *   Issued certificates should not have CRLs, as they cannot be used to trust any other certificate.
 *   When an issued certificate is used to protect a Secure Channel, it's signing chain will be verified.
 *   For instance, if the certificate is not self signed and appears on the CRL of its signing CA,
 *   the connection will fail as the certificate is in fact invalid.
 *
 * \param lPathTrustedIssuerRoots   A pointer to an array of paths to root trusted issuers of the validation chains.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathTrustedIssuerLinks   A pointer to an array of paths to intermediate certificate authorities.
 *                  This list contain only the trusted intermediate issuers.
 *                  This list must be ordered so that certificate signed by a parent must be present in the list
 *                  before its signing parent.
 *                  Each issued certificate must have its signing certificate chain in this list.
 * \param lPathUntrustedIssuerRoots A pointer to an array of paths to root untrusted issuers of the validation chains.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathUntrustedIssuerLinks A pointer to an array of paths to intermediate certificate authorities.
 *                  This list contain only the untrusted intermediate issuers.
 *                  This list must be ordered so that certificate signed by a parent must be present in the list
 *                  before its signing parent.
 *                  Each issued certificate must have its signing certificate chain in this list.
 * \param lPathIssuedCerts  A pointer to an array of paths to issued certificates.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathCRL  A pointer to an array of paths to each certificate revocation list to use.
 *                  Each CA of the trusted issuers list and the untrusted issuers list must have a CRL in the list.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the CRL from the current working directory.
 * \param ppPKI     A valid pointer to the newly created PKIProvider. You should free such provider with
 *                  SOPC_PKIProvider_Free().
 *
 * \warning         The \p lPathTrustedIssuerLinks and \p lPathUntrustedIssuerLinks must be sorted:
 *                  certificates must be provided in the child ->  parent order.
 *                  In other words, there may be several chains provided in the list, but a signed certificate
 *                  must always be provided before the certificate that signed it.
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            The pki is not safe to share across threads.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathTrustedIssuerRoots,
                                                        char** lPathTrustedIssuerLinks,
                                                        char** lPathUntrustedIssuerRoots,
                                                        char** lPathUntrustedIssuerLinks,
                                                        char** lPathIssuedCerts,
                                                        char** lPathCRL,
                                                        SOPC_PKIProvider** ppPKI);

/* ****************************************************************** */
/* ************************* NEW API ******************************** */

/*
 The directory store shall be form as following:
  .
  |
  ---- Directory_store_name
       |
       ---- default
       |    |
       |    ---- trusted
       |    |    |
       |    |    ---- certs (individual certificates with .DER format)
       |    |    ---- crls  (individual CRLs with .DER format)
       |    ---- issuers
       |         |
       |         ---- certs (individual certificates with .DER format)
       |         ---- crls  (individual CRLs with .DER format)
       |
       ---- trustList (Optional)
       |    |
       |    ---- trusted
       |    |    |
       |    |    ---- certs (individual certificates with .DER format)
       |    |    ---- crls  (individual CRLs with .DER format)
       |    ---- issuers
       |         |
       |         ---- certs (individual certificates with .DER format)
       |         ---- crls  (individual CRLs with .DER format)
*/

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
typedef struct SOPC_PKIProviderNew SOPC_PKIProviderNew;

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromBuffer(SOPC_CertificateList* pTrustedCerts,
                                                    SOPC_CRLList* pTrustedCrl,
                                                    SOPC_CertificateList* pIssuerCerts,
                                                    SOPC_CRLList* pIssuerCrl,
                                                    bool bBackwardInteroperability,
                                                    SOPC_PKIProviderNew** ppPKI);

/**
 * \brief Create the PKIProvider from a directory where certificates are store.
 *
 * The directory store shall be form as following:
 *
 * - Directory_store_name/default/trusted/certs (individual cert with .DER format)
 * - Directory_store_name/default/trusted/crls (individual cert with .DER format)
 * - Directory_store_name/default/issuers/certs (individual cert with .DER format)
 * - Directory_store_name/default/issuers/crls (individual cert with .DER format)
 *
 * - Directory_store_name/trustList/trusted/certs (individual cert with .DER format)
 * - Directory_store_name/trustList/trusted/crls (individual cert with .DER format)
 * - Directory_store_name/trustList/issuers/certs (individual cert with .DER format)
 * - Directory_store_name/trustList/issuers/crls (individual cert with .DER format)
 *
 * The trustList could be empty but not the default fodler.
 * Default folder is use when \p bDefaultBuild is set to True.
 * If \p bDefaultBuild is set to False, the function attempts to build the PKI from the trustList folder
 * and in case of error, it switches from the default folder.
 *
 * Notions :
 * - CA is a root CA if it is self-signed.
 * - trusted/certs = trusted root CA + trusted link CA + trusted cert.
 * - trusted/crls = CRLs of the trusted root CA + trusted link CA.
 * - issuer/certs = untrusted root CA + untrusted link CA.
 * - issuer/crls = CRLs of the untrusted root CA + untrusted link CA.
 * - CAs from trusted/certs and issuers/certs allow to verify the signing chain of a cert which is included into
 *   trusted/certs.
 * - CAs from trusted/certs allow to verify the signing chain of a cert which is not included into trusted/certs.
 *
 * This function check :
 * - That the certificate store is not empty.
 * - That at least one cert from trusted/certs is provided.
 * - That each certificate from issuer/certs are CA.
 * - That each CA has exactly one Certificate Revocation List (CRL).
 * - todo: That each CA keyUsage is filed with keyCertSign and keyCrlSign.
 * - todo: That the chain of signatures is rigth for each certificate.
 *
 * \note Content of the pki is NULL when return value is not SOPC_STATUS_OK.
 *
 * \param directoryStorePath The directory path where certificates are store.
 * \param bDefaultBuild Defined if the PKI is build from directory_store_name/default forlder.
 * \param bBackwardInteroperability Defined if self-signed certificates whose basicConstraints CA flag set to True
 *                                 will be marked as root CA and as trusted certificates.
 * \param ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *              SOPC_PKIProviderNew_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromStore(const char* directoryStorePath,
                                                      bool bDefaultBuild,
                                                      bool bBackwardInteroperability,
                                                      SOPC_PKIProviderNew** ppPKI);

/**
 * \brief Create the PKIProvider from list representation.
 *
 * Notions :
 * - CA is a root CA if it is self-signed.
 * - \p pTrustedCerts = trusted root CA + trusted link CA + trusted cert.
 * - \p pTrustedCrl = CRLs of the trusted root CA + trusted link CA.
 * - \p pIssuerCerts = untrusted root CA + untrusted link CA.
 * - \p pIssuerCrl = CRLs of the untrusted root CA + untrusted link CA.
 * - CAs from trusted/certs and issuers/certs allow to verify the signing chain of a cert which is included into
 *   trusted/certs.
 * - CAs from trusted/certs allow to verify the signing chain of a cert which is not included into trusted/certs.
 *
 * This function check :
 * - That at least one cert from \p pTrustedCerts is provided.
 * - That each certificate from \p pIssuerCerts are CA.
 * - That each CA has exactly one Certificate Revocation List (CRL).
 * - todo: That each CA keyUsage is filed with keyCertSign and keyCrlSign.
 * - todo: That the chain of signatures is rigth for each certificate.
 *
 * \param pTrustedCerts A valid pointer to the trusted certificate list. This object is borrowed and is freed by
 *                      SOPC_PKIProvider_Free.
 * \param pTrustedCrl A valid pointer to the trusted CRL list.
 * \param pIssuerCerts A valid pointer to the issuer certificate list. This object is
 *                      borrowed and is freed by SOPC_PKIProvider_Free.
 * \param pIssuerCrl A valid pointer to the issuer CRL list.
 * \param bBackwardInteroperability Defined if self-signed certificates whose basicConstraints CA flag set to
 *                                  True will be marked as root CA and as trusted certificates.
 * \param ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *              SOPC_PKIProviderNew_Free().
 *
 * \note In case of error, all arguments are freed by the fonction.
 *          Content of the pki is NULL when return value is not SOPC_STATUS_OK.
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error. In case of error, all arguments are freed.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                     SOPC_CRLList* pTrustedCrl,
                                                     SOPC_CertificateList* pIssuerCerts,
                                                     SOPC_CRLList* pIssuerCrl,
                                                     bool bBackwardInteroperability,
                                                     SOPC_PKIProviderNew** ppPKI);

/**
 * \brief   Free a PKI provider.
 */
void SOPC_PKIProviderNew_Free(SOPC_PKIProviderNew* pPKI);

#endif /* SOPC_PKI_STACK_H_ */
