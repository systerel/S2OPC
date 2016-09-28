/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#include <stdlib.h>
#include <string.h>

#include "crypto_profiles.h"
#include "mbedtls/aes.h"


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


// TODO: it is called CryptoProvider, why would it be in a crypto_profiles_*.c ?
static StatusCode CryptoProvider_SymmEncrypt_AES256(const struct CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenPlainText,
                                                    const KeyBuffer *pKey,
                                                    const uint8_t *pIV,
                                                    uint8_t *pOutput,
                                                    uint32_t lenOutput)
{
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation initialize a new context.
    unsigned char iv_cpy[16]; // Performance note: IV is modified during the operation, so it should be copied first

    if(lenOutput < lenPlainText)
        return STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, 16);

    if(mbedtls_aes_setkey_enc(&aes, (unsigned char *)pKey, 256) != 0)
        return STATUS_INVALID_PARAMETERS;
    if(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, lenPlainText, iv_cpy, (unsigned char *)pInput, (unsigned char *)pOutput) != 0)
        return STATUS_INVALID_PARAMETERS;

    return STATUS_OK;
}


static StatusCode CryptoProvider_SymmDecrypt_AES256(const struct CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenCipherText,
                                       const KeyBuffer *pKey,
                                       const uint8_t *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput)
{
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation initialize a new context.
    unsigned char iv_cpy[16]; // Performance note: IV is modified during the operation, so it should be copied first

    if(lenOutput < lenCipherText)
        return STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, 16);

    if(mbedtls_aes_setkey_dec(&aes, (unsigned char *)pKey, 256) != 0)
        return STATUS_INVALID_PARAMETERS;
    if(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, lenCipherText, iv_cpy, (unsigned char *)pInput, (unsigned char *)pOutput) != 0)
        return STATUS_INVALID_PARAMETERS;

    return STATUS_OK;
}


const CryptoProfile g_cpBasic256Sha256 = {
        .SecurityPolicyID = SecurityPolicy_Basic256Sha256_ID,
        .DerivedSignatureKeyBitLength = 256,
        .MinAsymmetricKeyBitLength = 2048,
        .MaxAsymmetricKeyBitLength = 4096,
        .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
        .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
};


#ifdef __cplusplus
}
#endif
