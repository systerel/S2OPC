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

#include <stdio.h>
#include <string.h>

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "sopc_key_manager_lib_itf.h"

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t* buffer,
                                                                 uint32_t lenBuf,
                                                                 bool is_public,
                                                                 SOPC_AsymmetricKey** ppKey)
{
    SOPC_UNUSED_ARG(ppKey);
    SOPC_UNUSED_ARG(buffer);
    SOPC_UNUSED_ARG(lenBuf);
    SOPC_UNUSED_ARG(is_public);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromFile(const char* szPath,
                                                               SOPC_AsymmetricKey** ppKey,
                                                               char* password,
                                                               uint32_t lenPassword)
{
    SOPC_UNUSED_ARG(ppKey);
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(password);
    SOPC_UNUSED_ARG(lenPassword);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_GenRSA(uint32_t RSAKeySize, SOPC_AsymmetricKey** ppKey)
{
    SOPC_UNUSED_ARG(ppKey);
    SOPC_UNUSED_ARG(RSAKeySize);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_CertificateList* pCert,
                                                                      SOPC_AsymmetricKey** pKey)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pCert);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey)
{
    SOPC_UNUSED_ARG(pKey);
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToDER(const SOPC_AsymmetricKey* pKey,
                                                      bool is_public,
                                                      uint8_t* pDest,
                                                      uint32_t lenDest,
                                                      uint32_t* pLenWritten)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(is_public);
    SOPC_UNUSED_ARG(pDest);
    SOPC_UNUSED_ARG(lenDest);
    SOPC_UNUSED_ARG(pLenWritten);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToPEMFile(SOPC_AsymmetricKey* pKey,
                                                          const bool bIsPublic,
                                                          const char* filePath,
                                                          const char* pwd,
                                                          const uint32_t pwdLen)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(bIsPublic);
    SOPC_UNUSED_ARG(filePath);
    SOPC_UNUSED_ARG(pwd);
    SOPC_UNUSED_ARG(pwdLen);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(is_public);
    SOPC_UNUSED_ARG(out);
    return SOPC_STATUS_NOT_SUPPORTED;
}

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                                 uint32_t lenDER,
                                                                 SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARG(bufferDER);
    SOPC_UNUSED_ARG(lenDER);
    SOPC_UNUSED_ARG(ppCert);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(ppCert);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCert);
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER(const SOPC_CertificateList* pCert,
                                                    uint8_t** ppDest,
                                                    uint32_t* pLenAllocated)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppDest);
    SOPC_UNUSED_ARG(pLenAllocated);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_CertificateList* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pDest);
    SOPC_UNUSED_ARG(lenDest);
    return SOPC_STATUS_NOT_SUPPORTED;
}

bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_CertificateList* pCert, const char* application_uri)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(application_uri);
    return false;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppApplicationUri);
    SOPC_UNUSED_ARG(pStringLength);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pLength);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSubjectName(const SOPC_CertificateList* pCert,
                                                             char** ppSubjectName,
                                                             uint32_t* pSubjectNameLen)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppSubjectName);
    SOPC_UNUSED_ARG(pSubjectNameLen);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSanDnsNames(const SOPC_CertificateList* pCert,
                                                             char*** ppDnsNameArray,
                                                             uint32_t* pArrayLength)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppDnsNameArray);
    SOPC_UNUSED_ARG(pArrayLength);
    return SOPC_STATUS_NOT_SUPPORTED;
}

char* SOPC_KeyManager_Certificate_GetCstring_SHA1(const SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCert);
    return NULL;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER_Files(SOPC_CertificateList* pCerts, const char* directoryPath)
{
    SOPC_UNUSED_ARG(pCerts);
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_FindCertInList(const SOPC_CertificateList* pList,
                                                                 const SOPC_CertificateList* pCert,
                                                                 bool* pbMatch)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pList);
    SOPC_UNUSED_ARG(pbMatch);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(SOPC_CertificateList** ppCertList,
                                                                     SOPC_CRLList** ppCRLList,
                                                                     const char* pThumbprint,
                                                                     bool* pbMatch,
                                                                     bool* pbIsIssuer)
{
    SOPC_UNUSED_ARG(ppCertList);
    SOPC_UNUSED_ARG(ppCRLList);
    SOPC_UNUSED_ARG(pThumbprint);
    SOPC_UNUSED_ARG(pbIsIssuer);
    SOPC_UNUSED_ARG(pbMatch);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_IsSelfSigned(const SOPC_CertificateList* pCert, bool* pbIsSelfSigned)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pbIsSelfSigned);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_Copy(const SOPC_CertificateList* pCert, SOPC_CertificateList** ppCertCopy)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppCertCopy);
    return SOPC_STATUS_NOT_SUPPORTED;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Revocation List API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                         uint32_t lenDER,
                                                         SOPC_CRLList** ppCRL)
{
    SOPC_UNUSED_ARG(bufferDER);
    SOPC_UNUSED_ARG(lenDER);
    SOPC_UNUSED_ARG(ppCRL);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL)
{
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(ppCRL);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_ToDER_Files(SOPC_CRLList* pCrls, const char* directoryPath)
{
    SOPC_UNUSED_ARG(pCrls);
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_Copy(const SOPC_CRLList* pCrl, SOPC_CRLList** ppCrlCopy)
{
    SOPC_UNUSED_ARG(pCrl);
    SOPC_UNUSED_ARG(ppCrlCopy);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_GetListLength(const SOPC_CRLList* pCrl, size_t* pLength)
{
    SOPC_UNUSED_ARG(pCrl);
    SOPC_UNUSED_ARG(pLength);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_AttachToSerializedArray(const SOPC_CertificateList* pCerts,
                                                                          SOPC_SerializedCertificate** pSerializedArray,
                                                                          uint32_t* pLenArray)
{
    SOPC_UNUSED_ARG(pCerts);
    SOPC_UNUSED_ARG(pSerializedArray);
    SOPC_UNUSED_ARG(pLenArray);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL)
{
    SOPC_UNUSED_ARG(pCRL);
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Signing request API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_KeyManager_CSR_Create(const char* subjectName,
                                             const bool bIsServer,
                                             const char* mdType,
                                             const char* uri,
                                             char** pDnsArray,
                                             uint32_t arrayLength,
                                             SOPC_CSR** ppCSR)
{
    SOPC_UNUSED_ARG(subjectName);
    SOPC_UNUSED_ARG(bIsServer);
    SOPC_UNUSED_ARG(mdType);
    SOPC_UNUSED_ARG(uri);
    SOPC_UNUSED_ARG(pDnsArray);
    SOPC_UNUSED_ARG(arrayLength);
    SOPC_UNUSED_ARG(ppCSR);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CSR_ToDER(SOPC_CSR* pCSR,
                                            SOPC_AsymmetricKey* pKey,
                                            uint8_t** ppDest,
                                            uint32_t* pLenAllocated)
{
    SOPC_UNUSED_ARG(pCSR);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(ppDest);
    SOPC_UNUSED_ARG(pLenAllocated);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_KeyManager_CRLList_AttachToSerializedArray(const SOPC_CRLList* pCRLs,
                                                                  SOPC_SerializedCRL** pSerializedArray,
                                                                  uint32_t* pLenArray)
{
    SOPC_UNUSED_ARG(pCRLs);
    SOPC_UNUSED_ARG(pSerializedArray);
    SOPC_UNUSED_ARG(pLenArray);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_KeyManager_CSR_Free(SOPC_CSR* pCSR)
{
    SOPC_UNUSED_ARG(pCSR);
}
