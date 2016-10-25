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
    int8_t *pathTrusted;
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
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_AsymetricKey_ToNewDER(const KeyManager *pManager,
                                            const AsymmetricKey *pKey,
                                            uint8_t **ppDest, uint32_t *lenAllocated);


/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_CertificateGetLength_Thumbprint(const KeyManager *,
                                                      uint32_t *);


StatusCode KeyManager_Certificate_Load(const KeyManager *pManager,
                                       const uint8_t *bufferDER, uint32_t lenDER,
                                       Certificate *pCert);
StatusCode KeyManager_Certificate_LoadFromFile(const KeyManager *pManager,
                                               const int8_t *szPath,
                                               Certificate *cert);
StatusCode KeyManager_Certificate_ToNewDER(const KeyManager *pManager,
                                           const Certificate *pCert,
                                           uint8_t **ppDest, uint32_t *lenAllocated);
StatusCode KeyManager_Certificate_GetThumbprint(const KeyManager *pManager,
                                                const Certificate *pCert,
                                                uint8_t *pDest, uint32_t lenDest);
/*StatusCode KeyManager_Certificate_GetPublicKey(const KeyManager *pManager,
                                               const Certificate *pCert,
                                               AsymetricKey *pKey);*/


#endif /* INGOPCS_KEY_MANAGER_H_ */
