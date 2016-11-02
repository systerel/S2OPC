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


#include "ua_base_types.h"
#include "crypto_types.h"
#include "key_manager_lib.h"



typedef struct KeyManager {
    CryptoProvider *pProvider; /**< Crypto provider associated to the key manager. Should not be modified nor replaced outside KeyManager API */
    AsymmetricKey *pkPriv; /**< The current private key associated to the KeyManager. */
    int8_t *pathTrusted; // TODO: remove that, now it's in PKI
    int8_t *pathRevoked;
} KeyManager;

/* ------------------------------------------------------------------------------------------------
 * KeyManager API
 * ------------------------------------------------------------------------------------------------
 */
KeyManager *KeyManager_Create(CryptoProvider *pProvider,
                              const int8_t *pathTrusted, uint32_t lenPathTrusted, // Copied, will be \0 terminated
                              const int8_t *pathRevoked, uint32_t lenPathRevoked);
void KeyManager_Delete(KeyManager *pManager);


/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */
/*StatusCode KeyManager_AsymmetricKey_ToNewDER(const KeyManager *pManager,
                                            const AsymmetricKey *pKey,
                                            uint8_t **ppDest, uint32_t *lenAllocated);*/
StatusCode KeyManager_AsymmetricKey_CreateFromBuffer(const KeyManager *pManager,
                                                     const uint8_t *buffer, uint32_t lenBuf,
                                                     AsymmetricKey **ppKey);
StatusCode KeyManager_AsymmetricKey_CreateFromFile(const KeyManager *pManager,
                                                   const char *szPath,
                                                   AsymmetricKey **ppKey);
void KeyManager_AsymmetricKey_Free(AsymmetricKey *pKey);


/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_CertificateGetLength_Thumbprint(const KeyManager *,
                                                      uint32_t *);


StatusCode KeyManager_Certificate_CreateFromDER(const KeyManager *pManager,
                                                const uint8_t *bufferDER, uint32_t lenDER,
                                                Certificate **ppCert);
StatusCode KeyManager_Certificate_CreateFromFile(const KeyManager *pManager,
                                                 const int8_t *szPath,
                                                 Certificate **ppCert);
void KeyManager_Certificate_Free(Certificate *cert);

StatusCode KeyManager_Certificate_CopyDER(const KeyManager *pManager,
                                          const Certificate *pCert,
                                          uint8_t **ppDest, uint32_t *lenAllocated);
StatusCode KeyManager_Certificate_GetThumbprint(const KeyManager *pManager,
                                                const Certificate *pCert,
                                                uint8_t *pDest, uint32_t lenDest);
StatusCode KeyManager_Certificate_GetPublicKey(const KeyManager *pManager,
                                               const Certificate *pCert,
                                               AsymmetricKey *pKey);


#endif /* INGOPCS_KEY_MANAGER_H_ */
