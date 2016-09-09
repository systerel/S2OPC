/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#include "crypto_profiles.h"

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
static const struct CryptoProfile g_cpBasic256Sha256 = {
        .DerivedSignatureKeyBitLength = 256,
        .MinAsymmetricKeyBitLength = 2048,
        .MaxAsymmetricKeyBitLength = 4096
};

