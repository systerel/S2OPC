/*
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 *  Created on: Oct. 14 2016
 *      Author: PAB
 */

#ifndef INGOPCS_KEY_MANAGER_H_
#define INGOPCS_KEY_MANAGER_H_


#include <sopc_base_types.h>
#include "crypto_types.h"
#include "key_manager_lib.h"



/* ------------------------------------------------------------------------------------------------
 * KeyManager API is context-less
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t *buffer, uint32_t lenBuf,
                                                     AsymmetricKey **ppKey);
SOPC_StatusCode KeyManager_AsymmetricKey_CreateFromFile(const char *szPath,
                                                   AsymmetricKey **ppKey,
                                                   char *password,
                                                   uint32_t lenPassword);
void KeyManager_AsymmetricKey_Free(AsymmetricKey *pKey);

SOPC_StatusCode KeyManager_AsymmetricKey_ToDER(const AsymmetricKey *pKey,
                                          uint8_t *ppDest, uint32_t lenDest,
                                          uint32_t *pLenWritten);


/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

// Cert length is in CryptoProvider.

SOPC_StatusCode KeyManager_Certificate_CreateFromDER(const uint8_t *bufferDER, uint32_t lenDER,
                                                Certificate **ppCert);
SOPC_StatusCode KeyManager_Certificate_CreateFromFile(const char *szPath,
                                                 Certificate **ppCert);
void KeyManager_Certificate_Free(Certificate *cert);

SOPC_StatusCode KeyManager_Certificate_CopyDER(const Certificate *pCert,
                                          uint8_t **ppDest, uint32_t *lenAllocated);
SOPC_StatusCode KeyManager_Certificate_GetThumbprint(const CryptoProvider *pProvider,
                                                const Certificate *pCert,
                                                uint8_t *pDest, uint32_t lenDest);
SOPC_StatusCode KeyManager_Certificate_GetPublicKey(const Certificate *pCert,
                                               AsymmetricKey *pKey);


#endif /* INGOPCS_KEY_MANAGER_H_ */
