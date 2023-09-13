/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "check_helpers.h"

#include <check.h>
#include <stdio.h>
#include <string.h>

#include "check_crypto_certificates.h"
#include "hexlify.h"

#include "sopc_key_cert_pair.h"
#include "sopc_key_manager.h"
#include "sopc_mem_alloc.h"

#include "server_static_security_data.h"

#define PASSWORD "password"
#define CSR_KEY_PATH "./server_private/encrypted_server_2k_key.pem"
#define CSR_PATH "./crypto_tools_csr.der"
#define CSR_SUBJECT_NAME "C=FR, ST=France, L=Aix-en-Provence, O=Systerel, CN=S2OPC Demo Certificate for Server Tests"
#define CSR_SAN_URI "URI:urn:S2OPC:localhost"
#define CSR_SAN_DNS "localhost"
#define CSR_MD "SHA256"

#define SRV_CERT_PATH "./server_public/server_2k_cert.der"
#define SRV_KEY_PATH "./server_private/encrypted_server_2k_key.pem"

START_TEST(test_crypto_check_app_uri)
{
    SOPC_CertificateList* crt_uri = SOPC_UnhexlifyCertificate(SRV_CRT);
    ck_assert(SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S2OPC:localhost"));
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S1OPC:localhost"));
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S2OPC:localhost-postfix"));
    SOPC_KeyManager_Certificate_Free(crt_uri);

    SOPC_CertificateList* crt_no_uri = SOPC_UnhexlifyCertificate(CA_CRT);
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_no_uri, "urn:S2OPC:localhost"));
    SOPC_KeyManager_Certificate_Free(crt_no_uri);
}
END_TEST

START_TEST(test_crypto_get_app_uri)
{
    char* appUri = NULL;
    size_t len = 0;

    SOPC_CertificateList* crt_uri = SOPC_UnhexlifyCertificate(SRV_CRT);
    ck_assert(SOPC_STATUS_OK == SOPC_KeyManager_Certificate_GetMaybeApplicationUri(crt_uri, &appUri, &len));
    ck_assert(strcmp(appUri, "urn:S2OPC:localhost") == 0);
    SOPC_Free(appUri);
    SOPC_KeyManager_Certificate_Free(crt_uri);

    SOPC_CertificateList* crt_no_uri = SOPC_UnhexlifyCertificate(CA_CRT);
    ck_assert(SOPC_STATUS_OK != SOPC_KeyManager_Certificate_GetMaybeApplicationUri(crt_no_uri, &appUri, &len));
    SOPC_KeyManager_Certificate_Free(crt_no_uri);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import)
{
    /* Generate a new RSA key*/
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, false, &pSerGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export the new key */
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, false, "./crypto_tools_gen_key.pem", NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import the new key */
    SOPC_SerializedAsymmetricKey* pSerImpKey = NULL;
    SOPC_AsymmetricKey* pImpKey = NULL;
    status = SOPC_KeyManager_AsymmetricKey_CreateFromFile("./crypto_tools_gen_key.pem", &pImpKey, NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pImpKey, false, &pSerImpKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated key with the imported one */
    uint32_t genKeyLen = SOPC_SecretBuffer_GetLength(pSerGenKey);
    uint32_t impKeyLen = SOPC_SecretBuffer_GetLength(pSerImpKey);
    ck_assert(genKeyLen == impKeyLen);
    SOPC_ExposedBuffer* pRawGenKey = SOPC_SecretBuffer_ExposeModify(pSerGenKey);
    SOPC_ExposedBuffer* pRawImpKey = SOPC_SecretBuffer_ExposeModify(pSerImpKey);
    ck_assert_ptr_nonnull(pRawGenKey);
    ck_assert_ptr_nonnull(pRawImpKey);
    int match = memcmp(pRawGenKey, pRawImpKey, genKeyLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_AsymmetricKey_Free(pImpKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpKey);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import_public)
{
    /* Generate a new RSA key */
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenPubKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, true, &pSerGenPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export the new public key */
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, true, "./crypto_tools_gen_public_key.pem", NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import the new public key */
    SOPC_SerializedAsymmetricKey* pSerImpPubKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerImpPubKeyWithHeader = NULL;
    SOPC_ExposedBuffer* pRawImpPubKeyWithHeader = NULL;
    uint32_t impKeyPubWithHeaderLen = 0;
    SOPC_AsymmetricKey* pImpPubKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile("./crypto_tools_gen_public_key.pem",
                                                                    &pSerImpPubKeyWithHeader);
    ck_assert(SOPC_STATUS_OK == status);
    impKeyPubWithHeaderLen = SOPC_SecretBuffer_GetLength(pSerImpPubKeyWithHeader);
    pRawImpPubKeyWithHeader = SOPC_SecretBuffer_ExposeModify(pSerImpPubKeyWithHeader);
    ck_assert_ptr_nonnull(pRawImpPubKeyWithHeader);
    status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(pRawImpPubKeyWithHeader, impKeyPubWithHeaderLen, true,
                                                            &pImpPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pImpPubKey, true, &pSerImpPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated public key with the imported one */
    uint32_t genKeyPubLen = SOPC_SecretBuffer_GetLength(pSerGenPubKey);
    uint32_t impKeyPubLen = SOPC_SecretBuffer_GetLength(pSerImpPubKey);
    ck_assert(genKeyPubLen == impKeyPubLen);
    SOPC_ExposedBuffer* pRawGenPubKey = SOPC_SecretBuffer_ExposeModify(pSerGenPubKey);
    SOPC_ExposedBuffer* pRawImpPubKey = SOPC_SecretBuffer_ExposeModify(pSerImpPubKey);
    ck_assert_ptr_nonnull(pRawGenPubKey);
    ck_assert_ptr_nonnull(pRawImpPubKey);
    int match = memcmp(pRawGenPubKey, pRawImpPubKey, genKeyPubLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_AsymmetricKey_Free(pImpPubKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenPubKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpPubKeyWithHeader);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpPubKey);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import_encrypted)
{
    /* Generate a new RSA key*/
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, false, &pSerGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export and encrypt the new key */
    size_t pwdLen = strlen(PASSWORD);
    ck_assert(pwdLen < UINT32_MAX);
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, false, "./crypto_tools_encrypted_gen_key.pem", PASSWORD,
                                                     (uint32_t) pwdLen);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import and decrypt the new key */
    SOPC_SerializedAsymmetricKey* pSerDecKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd("./crypto_tools_encrypted_gen_key.pem",
                                                                            &pSerDecKey, PASSWORD, (uint32_t) pwdLen);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated key with the imported one */
    uint32_t genKeyLen = SOPC_SecretBuffer_GetLength(pSerGenKey);
    uint32_t decKeyLen = SOPC_SecretBuffer_GetLength(pSerDecKey);
    ck_assert(genKeyLen == decKeyLen);
    SOPC_ExposedBuffer* pRawGenKey = SOPC_SecretBuffer_ExposeModify(pSerGenKey);
    SOPC_ExposedBuffer* pRawDecKey = SOPC_SecretBuffer_ExposeModify(pSerDecKey);
    ck_assert_ptr_nonnull(pRawGenKey);
    ck_assert_ptr_nonnull(pRawDecKey);
    int match = memcmp(pRawGenKey, pRawDecKey, genKeyLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerDecKey);
}
END_TEST

START_TEST(test_gen_csr)
{
    /* The purpose of this test is to generate a CSR file and verify manually
       its content though an external tool like openssl :
       > openssl req -inform der -in <csr_der> -out <csr_pem> -outform pem (convert DER to PEM)
       > openssl req -text -in <csr_pem> -noout -verify (to verify manually the content)
    */
    SOPC_AsymmetricKey* pKey = NULL;
    SOPC_CertificateList* pCert = NULL;
    char** pDNSNames = NULL;
    uint32_t DNSLen = 0;
    char* subjectName = NULL;
    uint32_t subjectNameLen = 0;
    SOPC_CSR* pCSR = NULL;
    uint8_t* pDER = NULL;
    uint32_t pLen = 0;
    size_t pwdLen = strlen(PASSWORD);
    ck_assert_uint_gt(UINT32_MAX, pwdLen);
    SOPC_ReturnStatus status =
        SOPC_KeyManager_AsymmetricKey_CreateFromFile(CSR_KEY_PATH, &pKey, PASSWORD, (uint32_t) pwdLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_GetSanDnsNames(pCert, &pDNSNames, &DNSLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pDNSNames);
    ck_assert_uint_eq(1, DNSLen);
    int match = memcmp(pDNSNames[0], CSR_SAN_DNS, strlen(CSR_SAN_DNS));
    ck_assert_int_eq(0, match);
    status = SOPC_KeyManager_Certificate_GetSubjectName(pCert, &subjectName, &subjectNameLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(subjectName);
    ck_assert_int_eq('\0', subjectName[subjectNameLen]);
    match = memcmp(subjectName, CSR_SUBJECT_NAME, subjectNameLen);
    ck_assert_int_eq(0, match);
    status = SOPC_KeyManager_CSR_Create(subjectName, true, CSR_MD, CSR_SAN_URI, pDNSNames, DNSLen, &pCSR);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CSR_ToDER(pCSR, pKey, &pDER, &pLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    FILE* fp = NULL;
    fp = fopen(CSR_PATH, "wb");
    ck_assert_ptr_nonnull(fp);
    size_t nb_written = fwrite(pDER, 1, pLen, fp);
    fclose(fp);
    ck_assert(pLen == nb_written);

    SOPC_Free(pDNSNames[0]);
    SOPC_Free(pDNSNames);

    SOPC_KeyManager_AsymmetricKey_Free(pKey);
    SOPC_KeyManager_Certificate_Free(pCert);
    SOPC_KeyManager_CSR_Free(pCSR);
    SOPC_Free(subjectName);
    SOPC_Free(pDER);
}

START_TEST(test_crypto_check_cert_list_to_array)
{
    uint32_t lenArray = 0;
    SOPC_SerializedCertificate* pCertArray = NULL;
    SOPC_SerializedCertificate* pCertRef[2];
    SOPC_CertificateList* pCerts = NULL;
    SOPC_Buffer* pBuffer = NULL;
    SOPC_Buffer* pBufferRef = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &pCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status =
        SOPC_KeyManager_SerializedCertificate_CreateFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &pCertRef[0]);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Users_PKI/trusted/certs/user_cacert.der", &pCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./S2OPC_Users_PKI/trusted/certs/user_cacert.der",
                                                                  &pCertRef[1]);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pCerts, &pCertArray, &lenArray);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pCertArray);
    ck_assert_uint_eq(2, lenArray);
    for (uint32_t idx = 0; idx < lenArray; idx++)
    {
        pBufferRef = pCertRef[idx];
        pBuffer = &pCertArray[idx];
        ck_assert_uint_eq(pBufferRef->length, pBuffer->length);
        int cmp = memcmp(pBufferRef->data, pBuffer->data, pBufferRef->length);
        ck_assert_int_eq(0, cmp);

        SOPC_Buffer_Delete(pBufferRef);
    }
    SOPC_KeyManager_Certificate_Free(pCerts);
    SOPC_Free(pCertArray);
}
END_TEST

START_TEST(test_crypto_check_crl_list_to_array)
{
    uint32_t lenArray = 0;
    SOPC_SerializedCRL* pCrlArray = NULL;
    SOPC_SerializedCRL* pCrlRef[2];
    SOPC_CRLList* pCRLs = NULL;
    SOPC_Buffer* pBuffer = NULL;
    SOPC_Buffer* pBufferRef = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/crl/cacrl.der", &pCRLs);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_ReadFile("./S2OPC_Demo_PKI/trusted/crl/cacrl.der", &pCrlRef[0]);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Users_PKI/trusted/crl/user_cacrl.der", &pCRLs);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_ReadFile("./S2OPC_Users_PKI/trusted/crl/user_cacrl.der", &pCrlRef[1]);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRLList_AttachToSerializedArray(pCRLs, &pCrlArray, &lenArray);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pCrlArray);
    ck_assert_uint_eq(2, lenArray);
    for (uint32_t idx = 0; idx < lenArray; idx++)
    {
        pBufferRef = pCrlRef[idx];
        pBuffer = &pCrlArray[idx];
        ck_assert_uint_eq(pBufferRef->length, pBuffer->length);
        int cmp = memcmp(pBufferRef->data, pBuffer->data, pBufferRef->length);
        ck_assert_int_eq(0, cmp);

        SOPC_Buffer_Delete(pBufferRef);
    }
    SOPC_KeyManager_CRL_Free(pCRLs);
    SOPC_Free(pCrlArray);
}
END_TEST

static void SOPC_KeyCertPairUpdateCallback(uintptr_t updateParam)
{
    bool* updatedBool = (bool*) updateParam;
    *updatedBool = true;
}

static void test_key_pair(bool isFiles)
{
    bool updateDone = false;
    SOPC_KeyCertPair* keyCertPair = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_SerializedCertificate* serializedCert = NULL;
    SOPC_CertificateList* cert = NULL;
    SOPC_AsymmetricKey* key = NULL;

    if (isFiles)
    {
        status = SOPC_KeyCertPair_CreateFromPaths(SRV_CERT_PATH, SRV_KEY_PATH, PASSWORD, &keyCertPair);
    }
    else
    {
        status = SOPC_KeyCertPair_CreateFromBytes(sizeof(server_2k_cert), server_2k_cert, sizeof(server_2k_key),
                                                  server_2k_key, &keyCertPair);
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(keyCertPair);
    // Try to update key / cert pair without update CB defined
    status = SOPC_KeyCertPair_UpdateFromBytes(keyCertPair, sizeof(server_2k_cert), server_2k_cert,
                                              sizeof(server_2k_key), server_2k_key);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);

    status = SOPC_KeyCertPair_SetUpdateCb(keyCertPair, &SOPC_KeyCertPairUpdateCallback, (uintptr_t) &updateDone);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Callback redefinition is not valid
    status = SOPC_KeyCertPair_SetUpdateCb(keyCertPair, &SOPC_KeyCertPairUpdateCallback, (uintptr_t) &updateDone);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);

    // Try to update key without certificate: invalid
    status = SOPC_KeyCertPair_UpdateFromBytes(keyCertPair, 0, NULL, sizeof(server_2k_key), server_2k_key);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert(!updateDone);
    // Update cert only: valid
    status = SOPC_KeyCertPair_UpdateFromBytes(keyCertPair, sizeof(server_2k_cert), server_2k_cert, 0, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(updateDone);
    updateDone = false;
    // Update both cert and key: valid
    status = SOPC_KeyCertPair_UpdateFromBytes(keyCertPair, sizeof(server_2k_cert), server_2k_cert,
                                              sizeof(server_2k_key), server_2k_key);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(updateDone);
    updateDone = false;

    status = SOPC_KeyCertPair_GetSerializedCertCopy(keyCertPair, &serializedCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(serializedCert->length, sizeof(server_2k_cert));
    int cmp = memcmp(server_2k_cert, serializedCert->data, serializedCert->length);
    ck_assert_int_eq(0, cmp);

    status = SOPC_KeyCertPair_GetCertCopy(keyCertPair, &cert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(cert);

    // Check cert copy is the expected cert
    uint8_t* pExpSerCert = NULL;
    uint32_t pLen = 0;
    status = SOPC_KeyManager_Certificate_ToDER(cert, &pExpSerCert, &pLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(pLen, sizeof(server_2k_cert));
    cmp = memcmp(server_2k_cert, pExpSerCert, sizeof(server_2k_cert));
    ck_assert_int_eq(0, cmp);
    SOPC_Free(pExpSerCert);

    status = SOPC_KeyCertPair_GetKeyCopy(keyCertPair, &key);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(key);

    // Check the key copy is the expected key
    SOPC_AsymmetricKey* pExpKey = NULL;
    status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(server_2k_key, sizeof(server_2k_key), false, &pExpKey);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pExpKey);
    // Serialize expected key to remove PEM encoding
    SOPC_SerializedAsymmetricKey* pExpSerKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pExpKey, false, &pExpSerKey);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pExpSerKey);
    // Serialize key copy to remove PEM encoding
    SOPC_SerializedAsymmetricKey* pActualSerKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(key, false, &pActualSerKey);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pActualSerKey);
    // Compare both
    ck_assert_uint_eq(SOPC_SecretBuffer_GetLength(pExpSerKey), SOPC_SecretBuffer_GetLength(pActualSerKey));

    const SOPC_ExposedBuffer* pActualSerKeyExposed = SOPC_SecretBuffer_Expose(pActualSerKey);
    const SOPC_ExposedBuffer* pExpSerKeyExposed = SOPC_SecretBuffer_Expose(pExpSerKey);
    cmp = memcmp(pExpSerKeyExposed, pExpSerKeyExposed, SOPC_SecretBuffer_GetLength(pExpSerKey));
    ck_assert_int_eq(0, cmp);
    SOPC_SecretBuffer_Unexpose(pActualSerKeyExposed, pActualSerKey);
    SOPC_SecretBuffer_Unexpose(pExpSerKeyExposed, pExpSerKey);
    SOPC_KeyManager_AsymmetricKey_Free(pExpKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pExpSerKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pActualSerKey);

    SOPC_KeyCertPair_Delete(&keyCertPair);
    ck_assert_ptr_null(keyCertPair);

    SOPC_KeyManager_SerializedCertificate_Delete(serializedCert);
    SOPC_KeyManager_Certificate_Free(cert);
    SOPC_KeyManager_AsymmetricKey_Free(key);
}

START_TEST(test_key_pair_files)
{
    test_key_pair(true);
}

START_TEST(test_key_pair_bytes)
{
    test_key_pair(false);
}

Suite* tests_make_suite_crypto_tools(void)
{
    Suite* s = suite_create("Crypto tools test");
    TCase *tc_check_app_uri = NULL, *tc_gen_rsa = NULL, *tc_gen_csr = NULL,  *tc_check_crypto_list_to_array = NULL, *tc_key_pair = NULL;

    tc_check_app_uri = tcase_create("Check application URI");
    tc_gen_rsa = tcase_create("Generate RSA keys");
    tc_gen_csr = tcase_create("Generate CSR");
    tc_key_pair = tcase_create("Update Key / Cert pair");
    tc_check_crypto_list_to_array = tcase_create("Check crypto list to serialized array");

    suite_add_tcase(s, tc_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_get_app_uri);
    suite_add_tcase(s, tc_gen_rsa);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import_public);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import_encrypted);
    tcase_set_timeout(tc_gen_rsa, 10);
    suite_add_tcase(s, tc_gen_csr);
    tcase_add_test(tc_gen_csr, test_gen_csr);
    suite_add_tcase(s, tc_key_pair);
    tcase_add_test(tc_key_pair, test_key_pair_files);
    tcase_add_test(tc_key_pair, test_key_pair_bytes);
    suite_add_tcase(s, tc_check_crypto_list_to_array);
    tcase_add_test(tc_check_crypto_list_to_array, test_crypto_check_cert_list_to_array);
    tcase_add_test(tc_check_crypto_list_to_array, test_crypto_check_crl_list_to_array);

    return s;
}
