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

#include "secret_buffer.h"
#include "crypto_profiles.h"
#include "crypto_provider.h"
#include "crypto_provider_lib.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


/* Security policy "Basic256Sha256", as of 09/09/2016:
 * SymmetricSignatureAlgorithm – Hmac_Sha256        OK
 * SymmetricEncryptionAlgorithm – Aes256_CBC        OK
 * AsymmetricSignatureAlgorithm – Rsa_Sha256
 * AsymmetricKeyWrapAlgorithm – KwRsaOaep
 * AsymmetricEncryptionAlgorithm – Rsa_Oaep
 * KeyDerivationAlgorithm – PSHA256
 * DerivedSignatureKeyLength – 256
 * MinAsymmetricKeyLength – 2048                    OK
 * MaxAsymmetricKeyLength – 4096                    OK
 * CertificateSignatureAlgorithm – Sha256           OK
 */


// TODO: it is called CryptoProvider, why would it be in a crypto_profiles_*.c ?
static StatusCode CryptoProvider_SymmEncrypt_AES256(const CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenPlainText,
                                                    const ExposedBuffer *pKey,
                                                    const ExposedBuffer *pIV,
                                                    uint8_t *pOutput,
                                                    uint32_t lenOutput)
{
    (void) pProvider;
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


static StatusCode CryptoProvider_SymmDecrypt_AES256(const CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenCipherText,
                                       const ExposedBuffer *pKey,
                                       const ExposedBuffer *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput)
{
    (void) pProvider;
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation initialize a new context.
    unsigned char iv_cpy[16]; // Performance note: IV is modified during the operation, so it should be copied first

    if(lenOutput < lenCipherText)
        return STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, 16);

    if(mbedtls_aes_setkey_dec(&aes, (unsigned char *)pKey, SecurityPolicy_Basic256Sha256_SymmLen_Key*8) != 0)
        return STATUS_INVALID_PARAMETERS;
    if(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, lenCipherText, iv_cpy, (unsigned char *)pInput, (unsigned char *)pOutput) != 0)
        return STATUS_INVALID_PARAMETERS;

    return STATUS_OK;
}


static StatusCode CryptoProvider_SymmSign_HMAC_SHA256(const CryptoProvider *pProvider,
                                                      const uint8_t *pInput,
                                                      uint32_t lenInput,
                                                      const ExposedBuffer *pKey,
                                                      uint8_t *pOutput)
{
    const mbedtls_md_info_t *pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint32_t lenKey;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKey) != STATUS_OK)
        return STATUS_NOK;

    if(mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pOutput) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


static StatusCode CryptoProvider_SymmVerify_HMAC_SHA256(const CryptoProvider *pProvider,
                                                        const uint8_t *pInput,
                                                        uint32_t lenInput,
                                                        const ExposedBuffer *pKey,
                                                        const uint8_t *pSignature)
{
    const mbedtls_md_info_t *pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint32_t lenKey, lenSig;
    uint8_t *pCalcSig;
    StatusCode status = STATUS_OK;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKey) != STATUS_OK)
        return STATUS_NOK;

    if(CryptoProvider_SymmetricGetLength_Signature(pProvider, &lenSig) != STATUS_OK)
        return STATUS_NOK;

    pCalcSig = malloc(lenSig);
    if(NULL == pCalcSig)
        return STATUS_NOK;

    status = mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pCalcSig) != 0 ? STATUS_NOK : STATUS_OK;
    if(STATUS_OK == status)
        status = memcmp(pSignature, pCalcSig, lenSig) != 0 ? STATUS_NOK : STATUS_OK;

    free(pCalcSig);

    return status;
}


// Fills pKey with SecurityPolicy_Basic256Sha256_Symm_KeyLength bytes of random data
StatusCode CryptoProvider_SymmGenKey_AES256(const CryptoProvider *pProvider,
                                            ExposedBuffer *pKey)
{
    CryptolibContext *pCtx = NULL;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pProvider->pCryptolibContext || NULL == pKey)
        return STATUS_INVALID_PARAMETERS;

    pCtx = pProvider->pCryptolibContext;
    if(mbedtls_ctr_drbg_random(&(pCtx->ctxDrbg), pKey, SecurityPolicy_Basic256Sha256_SymmLen_Key) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


const CryptoProfile g_cpBasic256Sha256 = {
        .SecurityPolicyID = SecurityPolicy_Basic256Sha256_ID,
        .DerivedSignatureKeyBitLength = 256,
        .MinAsymmetricKeyBitLength = 2048,
        .MaxAsymmetricKeyBitLength = 4096,
        .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
        .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
        .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
        .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
        .pFnSymmGenKey = &CryptoProvider_SymmGenKey_AES256,
};

