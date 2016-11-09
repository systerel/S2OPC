/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "crypto_profiles.h"
#include "crypto_functions_lib.h"


/* Security policy "Basic256Sha256", as of 09/09/2016:
 * SymmetricSignatureAlgorithm – Hmac_Sha256        OK
 * SymmetricEncryptionAlgorithm – Aes256_CBC        OK
 * AsymmetricSignatureAlgorithm – Rsa_Sha256        OK
 * AsymmetricKeyWrapAlgorithm – KwRsaOaep           unused
 * AsymmetricEncryptionAlgorithm – Rsa_Oaep         OK
 * KeyDerivationAlgorithm – PSHA256                 OK
 * DerivedSignatureKeyLength – 256                  ??? AES256 implies this
 * MinAsymmetricKeyLength – 2048                    OK
 * MaxAsymmetricKeyLength – 4096                    OK
 * CertificateSignatureAlgorithm – Sha256
 */


const CryptoProfile g_cpBasic256Sha256 = {
        .SecurityPolicyID = SecurityPolicy_Basic256Sha256_ID,
        .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
        .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
        .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
        .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
        .pFnSymmGenKey = &CryptoProvider_SymmGenKey_AES256,
        .pFnDeriveData = &CryptoProvider_DeriveData_PRF_SHA256,
        .pFnAsymEncrypt = &CryptoProvider_AsymEncrypt_RSA_OAEP,
        .pFnAsymDecrypt = &CryptoProvider_AsymDecrypt_RSA_OAEP,
        .pFnAsymSign = &CryptoProvider_AsymSign_RSASSA_PKCS1_v15,
        .pFnAsymVerify = &CryptoProvider_AsymVerify_RSASSA_PKCS1_v15,
        .pFnCertVerify = &CryptoProvider_CertVerify_RSA_SHA256_2048_4096,
};

