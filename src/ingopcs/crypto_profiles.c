/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#include "crypto_profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Security policy "Basic256Sha256", as of 09/09/2016:
 * SymmetricSignatureAlgorithm – Hmac_Sha256
 * SymmetricEncryptionAlgorithm – Aes256_CBC
 * AsymmetricSignatureAlgorithm – Rsa_Sha256
 * AsymmetricKeyWrapAlgorithm – KwRsaOaep
 * AsymmetricEncryptionAlgorithm – Rsa_Oaep
 * KeyDerivationAlgorithm – PSHA256
 * DerivedSignatureKeyLength – 256
 * MinAsymmetricKeyLength – 2048
 * MaxAsymmetricKeyLength – 4096
 * CertificateSignatureAlgorithm – Sha256
 */

static StatusCode cpSymmEncrypt_AES256(const struct CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenPlainText,
                                       const KeyBuffer *pKey,
                                       const uint8_t *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput)
{
    return STATUS_OK;
}

static StatusCode cpSymmDecrypt_AES256(const struct CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenPlainText,
                                       const KeyBuffer *pKey,
                                       const uint8_t *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput)
{
    return STATUS_OK;
}

static const struct CryptoProfile g_cpBasic256Sha256 = {
        .DerivedSignatureKeyBitLength = 256,
        .MinAsymmetricKeyBitLength = 2048,
        .MaxAsymmetricKeyBitLength = 4096,
        .pFnSymmEncrypt = &cpSymmEncrypt_AES256,
        .pFnSymmDecrypt = &cpSymmDecrypt_AES256,
};


#ifdef __cplusplus
}
#endif
