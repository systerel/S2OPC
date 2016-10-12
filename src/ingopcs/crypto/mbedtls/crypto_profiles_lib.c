/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#include "crypto_profiles.h"
#include "crypto_functions_lib.h"


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
 * CertificateSignatureAlgorithm – Sha256           OK, not verified
 */


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

