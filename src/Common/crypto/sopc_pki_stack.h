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

#endif /* SOPC_PKI_STACK_H_ */
