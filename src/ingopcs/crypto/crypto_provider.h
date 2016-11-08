/** \file crypto_provider.h
 *
 * Defines the cryptographic API: a  data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_PROVIDER_H_
#define INGOPCS_CRYPTO_PROVIDER_H_


#include <sopc_base_types.h>
#include "secret_buffer.h"
#include "crypto_types.h"
#include "key_sets.h"
#include "key_manager.h"
#include "pki.h"


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief       Creates an initialized CryptoProvider context from a string containing the desired
 *              security policy URI.
 *
 *              The CryptoProvider contains the CryptoProfile corresponding to the security policy.
 *              It should never be modified.
 *
 * \param uri   The URI describing the security policy. Should not be NULL.
 *
 * \return      An initialized CryptoProvider* or NULL if the context could not be created.
 */
CryptoProvider *CryptoProvider_Create(const char *uri);

/**
 * \brief       Frees a CryptoProvider created with CryptoProvider_Create().
 *
 * \param pCryptoProvider  The CryptoProvider to free.
 */
void CryptoProvider_Free(CryptoProvider *pCryptoProvider);

/**
 * \brief       Initializes a CryptoProvider context.
 *              Called by CryptoProvider_Create() upon context creation.
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 */
SOPC_StatusCode CryptoProvider_Init(CryptoProvider *pCryptoProvider);

/**
 * \brief       Deinitializes a CryptoProvider context (this process is specific to the chosen cryptographic library).
 *              Called by CryptoProvider_Free() upon context destruction.
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 */
SOPC_StatusCode CryptoProvider_Deinit(CryptoProvider *pCryptoProvider);


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Writes the length in bytes of the key of the symmetric algorithm in \p pLength.
 *
 *                  The length of the key depends on the security policy associated with \p pProvider.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the key. Its content is unspecified
 *                  when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_SymmetricGetLength_Key(const CryptoProvider *pProvider,
                                                     uint32_t *pLength);

/**
 * \brief           Writes the length in bytes in \p pLengthOut of an encrypted message of \p lengthIn bytes.
 *
 * \warning         Does not take padding into account.
 *
 * \param pProvider An initialized cryptographic context.
 * \param lengthIn  The length in bytes of the message to encrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the ciphered message. Its content is
 *                  unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when given pointers are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_SymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut);

/**
 * \brief           Writes the length in bytes in \p pLengthOut of a decrypted message of \p lengthIn bytes.
 *
 * \warning         Does not take padding into account.
 *
 * \param pProvider An initialized cryptographic context.
 * \param lengthIn  The length in bytes of the message to decrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the deciphered message. Its content is
 *                  unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when given pointers are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_SymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut);

/**
 * \brief           Provides the length in bytes of the symmetric signature message.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the signature message.
 *                  Its content is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_SymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                       uint32_t *pLength);

/**
 * \brief           Provides the lengths in bytes of the blocks used in the symmetric encryption process.
 *
 *                  These lengths are useful to predict the padding sizes required by the symmetric
 *                  encryption process.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pCipherTextBlockSize  An optional pointer to the length in bytes of the block size used by the encryption process.
 * \param pPlainTextBlockSize   An optional pointer to the length in bytes of the block size used by the decryption process.
 *
 * \note            The values held by \p pCipherTextBlockSize and \p pPlainTextBlockSize are unspecified
 *                  when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_SymmetricGetLength_Blocks(const CryptoProvider *pProvider,
                                                    uint32_t *pCipherTextBlockSize,
                                                    uint32_t *pPlainTextBlockSize);

/**
 * \brief           Provides the lengths in bytes of the secrets derived from the nonce exchange.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pSymmCryptoKeyLength   A valid pointer to the length in bytes of the symmetric key used for encryption.
 * \param pSymmSignKeyLength     A valid pointer to the length in bytes of the symmetric key used for signing.
 * \param pSymmInitVectorLength  A valid pointer to the length in bytes of the symmetric initialization vector.
 *
 * \note            The values held by \p pSymmCryptoKeyLength, \p pSymmSignKeyLength and \p pSymmInitVectorLength
 *                  are unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_DeriveGetLengths(const CryptoProvider *pProvider,
                                           uint32_t *pSymmCryptoKeyLength,
                                           uint32_t *pSymmSignKeyLength,
                                           uint32_t *pSymmInitVectorLength);

/**
 * \brief           Writes the length in bits in \p pLenKeyBits of the asymmetric key \p pKey.
 *
 *                  The main purpose of this function is to verify the length of the modulus of the
 *                  asymmetric key \p pKey with respect to the security policy.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pLenKeyBits  A valid pointer to the output length in bits. Its content is unspecified when
 *                  return value is not STATUS_OK.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_KeyBits(const CryptoProvider *pProvider,
                                                      const AsymmetricKey *pKey,
                                                      uint32_t *pLenKeyBits);

/**
 * \brief           Writes the length in bytes in \p pLenKeyBytes of the asymmetric key \p pKey.
 *
 *                  The main purpose of this function is to verify the length of the modulus of the
 *                  asymmetric key \p pKey with respect to the security policy.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pLenKeyBits  A valid pointer to the output length in bytes. Its content is unspecified when
 *                  return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_KeyBytes(const CryptoProvider *pProvider,
                                                       const AsymmetricKey *pKey,
                                                       uint32_t *lenKeyBytes);

/**
 * \brief           Provides the length of the hash used for OAEP encryption/decryption.
 *
 * \note            Internal API.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_OAEPHashLength(const CryptoProvider *pProvider,
                                                             uint32_t *length);

/**
 * \brief           Provides the length of the hash used for PSS signature/verification.
 *
 * \note            Internal API.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_PSSHashLength(const CryptoProvider *pProvider,
                                                            uint32_t *length);

/**
 * \brief           Provides the lengths in bytes of the messages used in asymmetric encryption process.
 *
 *                  These lengths are useful to predict the padding sizes required by the asymmetric
 *                  encryption process.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pCipherTextBlockSize  An optional pointer to the maximum length in bytes of the plain text message used by the encryption process.
 * \param pPlainTextBlockSize   An optional pointer to the length in bytes of the ciphered message used by the decryption process.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Msgs(const CryptoProvider *pProvider,
                                                   const AsymmetricKey *pKey,
                                                   uint32_t *pCipherTextBlockSize,
                                                   uint32_t *pPlainTextBlockSize);

/**
 * \brief           Provides the maximum length in bytes of a message to be encrypted with a single asymmetric
 *                  encryption operation.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pLenMsg   A valid pointer to the length in bytes of the maximum length in bytes of the plain text message used by the encryption process.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_MsgPlainText(const CryptoProvider *pProvider,
                                                           const AsymmetricKey *pKey,
                                                           uint32_t *pLenMsg);

/**
 * \brief           Provides the length in bytes of a ciphered message to be decrypted with a single asymmetric
 *                  decryption operation.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pLenMsg   A valid pointer to the length in bytes of the ciphered message used by the decryption process.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_MsgCipherText(const CryptoProvider *pProvider,
                                                            const AsymmetricKey *pKey,
                                                            uint32_t *pLenMsg);

/** \brief          Calculates the size of the required output buffer to cipher lengthIn bytes through
 *                  asymmetric encryption.
 *
 *                  Hence, the computation takes into account the padding, but it does not include any signature length.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param lengthIn  The length in bytes of the payload to encrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the corresponding encrypted payload.
 *                    Its content is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, STATUS_NOK when there was an error.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                         const AsymmetricKey *pKey,
                                                         uint32_t lengthIn,
                                                         uint32_t *pLengthOut);

/** \brief          Calculates the size of the required output buffer to decipher lengthIn bytes through
 *                  asymmetric decryption.
 *
 *                  Hence, the computation takes into account the padding, but it does not include any signature length.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param lengthIn  The length in bytes of the payload to decrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the corresponding decrypted payload.
 *                    Its content is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, STATUS_NOK when there was an error.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                         const AsymmetricKey *pKey,
                                                         uint32_t lengthIn,
                                                         uint32_t *pLengthOut);

/** \brief          Calculates the size of the required output buffer to contain the asymmetric signature.
 *
 *                  It is a single ciphered-message long.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an AsymmetricKey.
 * \param pLength   A valid pointer to the length in bytes of the signature.
 *                  Its content is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, STATUS_NOK when there was an error.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                        const AsymmetricKey *pKey,
                                                        uint32_t *pLength);

/** \brief          Calculates the size of the signature of the certificates.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the signature.
 *                  Its content is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_StatusCode CryptoProvider_CertificateGetLength_Thumbprint(const CryptoProvider *pProvider,
                                                          uint32_t *pLength);


/* ------------------------------------------------------------------------------------------------
 * Symmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_SymmetricEncrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmetricDecrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmetricSign(const CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SecretBuffer *pKey,
                                        uint8_t *pOutput,
                                        uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmetricVerify(const CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const SecretBuffer *pKey,
                                          const uint8_t *pSignature,
                                          uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmetricGenerateKey(const CryptoProvider *pProvider,
                                               SecretBuffer **ppKeyGenerated);


/* ------------------------------------------------------------------------------------------------
 * Key derivation
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_DerivePseudoRandomData(const CryptoProvider *pProvider,
                                                 const ExposedBuffer *pSecret,
                                                 uint32_t lenSecret,
                                                 const ExposedBuffer *pSeed,
                                                 uint32_t lenSeed,
                                                 ExposedBuffer *pOutput,
                                                 uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_DeriveKeySets(const CryptoProvider *pProvider,
                                        const ExposedBuffer *pClientNonce,
                                        uint32_t lenClientNonce,
                                        const ExposedBuffer *pServerNonce,
                                        uint32_t lenServerNonce,
                                        SC_SecurityKeySet *pClientKeySet,
                                        SC_SecurityKeySet *pServerKeySet);
SOPC_StatusCode CryptoProvider_DeriveKeySetsClient(const CryptoProvider *pProvider, // DeriveKeySets
                                              const SecretBuffer *pClientNonce,
                                              const ExposedBuffer *pServerNonce,
                                              uint32_t lenServerNonce,
                                              SC_SecurityKeySet *pClientKeySet,
                                              SC_SecurityKeySet *pServerKeySet);
SOPC_StatusCode CryptoProvider_DeriveKeySetsServer(const CryptoProvider *pProvider,
                                              const ExposedBuffer *pClientNonce,
                                              uint32_t lenClientNonce,
                                              const SecretBuffer *pServerNonce,
                                              SC_SecurityKeySet *pClientKeySet,
                                              SC_SecurityKeySet *pServerKeySet);


/* ------------------------------------------------------------------------------------------------
 * Asymmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_AsymmetricEncrypt(const CryptoProvider *pProvider,
                                            const uint8_t *pInput,
                                            uint32_t lenInput,
                                            const AsymmetricKey *pKey,
                                            uint8_t *pOutput,
                                            uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_AsymmetricDecrypt(const CryptoProvider *pProvider,
                                            const uint8_t *pInput,
                                            uint32_t lenInput,
                                            const AsymmetricKey *pKey,
                                            uint8_t *pOutput,
                                            uint32_t lenOutput,
                                            uint32_t *lenWritten);
SOPC_StatusCode CryptoProvider_AsymmetricSign(const CryptoProvider *pProvider,
                                         const uint8_t *pInput,
                                         uint32_t lenInput,
                                         const AsymmetricKey *pKeyPrivateLocal,
                                         uint8_t *pSignature,
                                         uint32_t lenSignature);
SOPC_StatusCode CryptoProvider_AsymmetricVerify(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenInput,
                                           const AsymmetricKey *pKeyRemotePublic,
                                           const uint8_t *pSignature,
                                           uint32_t lenSignature);


/* ------------------------------------------------------------------------------------------------
 * Certificate validation
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_Certificate_Validate(const CryptoProvider *pCrypto,
                                               const struct PKIProvider *pPKI,
                                               const Certificate *pCert);


#endif  // INGOPCS_CRYPTO_PROVIDER_H_
