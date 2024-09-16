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

#include <stddef.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_buffer.h"
#include "sopc_common_constants.h"
#include "sopc_crypto_profiles.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_key_manager_lib_itf.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_pki_stack_lib_internal_itf.h"
#include "sopc_pki_struct_lib_internal.h"

#ifndef STR_TRUSTLIST_NAME
#define STR_TRUSTLIST_NAME "/updatedTrustList"
#endif

#define STR_TRUSTED "/trusted"
#define STR_TRUSTED_CERTS "/trusted/certs"
#define STR_TRUSTED_CRL "/trusted/crl"
#define STR_ISSUERS "/issuers"
#define STR_ISSUERS_CERTS "/issuers/certs"
#define STR_ISSUERS_CRL "/issuers/crl"
#define STR_REJECTED "/rejected"

#define HEX_THUMBPRINT_SIZE 40

#ifndef WITH_NO_CRYPTO

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha256_2048_4096 = {.mdSign = SOPC_PKI_MD_SHA256,
                                                                         .pkAlgo = SOPC_PKI_PK_RSA,
                                                                         .RSAMinimumKeySize = 2048,
                                                                         .RSAMaximumKeySize = 4096,
                                                                         .bApplySecurityPolicy = true,
                                                                         .keyUsage = SOPC_PKI_KU_NONE,
                                                                         .extendedKeyUsage = SOPC_PKI_EKU_NONE,
                                                                         .sanApplicationUri = NULL,
                                                                         .sanURL = NULL};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha256_2048 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                      .mdSign = SOPC_PKI_MD_SHA256_OR_ABOVE,
                                                                      .pkAlgo = SOPC_PKI_PK_RSA,
                                                                      .RSAMinimumKeySize = 2048,
                                                                      .bDisableRevocationCheck = false};

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha1_1024_2048 = {.mdSign = SOPC_PKI_MD_SHA1_AND_SHA256,
                                                                       .pkAlgo = SOPC_PKI_PK_RSA,
                                                                       .RSAMinimumKeySize = 1024,
                                                                       .RSAMaximumKeySize = 2048,
                                                                       .bApplySecurityPolicy = true,
                                                                       .keyUsage = SOPC_PKI_KU_NONE,
                                                                       .extendedKeyUsage = SOPC_PKI_EKU_NONE,
                                                                       .sanApplicationUri = NULL,
                                                                       .sanURL = NULL};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha1_1024 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                    .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                                                    .pkAlgo = SOPC_PKI_PK_RSA,
                                                                    .RSAMinimumKeySize = 1024,
                                                                    .bDisableRevocationCheck = false};

typedef struct Profile_Cfg
{
    const SOPC_PKI_ChainProfile* chain;
    const SOPC_PKI_LeafProfile* leaf;
    const SOPC_SecurityPolicy_ID id;
} Profile_Cfg;

static const Profile_Cfg g_all_profiles[] = {
    {.id = SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Basic256Sha256_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Basic256_ID,
     .leaf = &g_leaf_profile_rsa_sha1_1024_2048,
     .chain = &g_chain_profile_rsa_sha1_1024},
};

static const SOPC_PKI_KeyUsage_Mask g_appKU = SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT |
                                              SOPC_PKI_KU_DIGITAL_SIGNATURE | SOPC_PKI_KU_NON_REPUDIATION;
static const SOPC_PKI_KeyUsage_Mask g_usrKU =
    SOPC_PKI_KU_DIGITAL_SIGNATURE; // it is not part of the OPC UA but it makes sense to keep it
static const SOPC_PKI_ExtendedKeyUsage_Mask g_clientEKU = SOPC_PKI_EKU_SERVER_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_serverEKU = SOPC_PKI_EKU_CLIENT_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_userEKU = SOPC_PKI_EKU_NONE;

/**
 * Static functions declaration
 */

static void sopc_pki_clear(SOPC_PKIProvider* pPKI);

// Copy newPKI content into currentPKI by preserving currentPKI mutex and then clear previous PKI content.
// Then frees the new PKI structure.
static void sopc_internal_replace_pki_and_clear(SOPC_PKIProvider* currentPKI, SOPC_PKIProvider** newPKI);

static SOPC_ReturnStatus sopc_pki_check_application_uri(const SOPC_CertificateList* pToValidate,
                                                        const char* applicationUri);

static SOPC_ReturnStatus sopc_pki_check_security_level_of_the_update(const SOPC_CertificateList* pTrustedCerts,
                                                                     const SOPC_CRLList* pTrustedCrl,
                                                                     const SOPC_CertificateList* pIssuerCerts,
                                                                     const SOPC_CRLList* pIssuerCrl,
                                                                     const char* securityPolicyUri);

static SOPC_ReturnStatus sopc_pki_merge_certificates(SOPC_CertificateList* pLeft,
                                                     SOPC_CertificateList* pRight,
                                                     SOPC_CertificateList** ppRes);

static SOPC_ReturnStatus write_cert_to_der_files(SOPC_CertificateList* pRoots,
                                                 SOPC_CertificateList* pCerts,
                                                 const char* directoryPath,
                                                 const bool bEraseExistingFiles);

static SOPC_ReturnStatus write_crl_to_der_files(SOPC_CRLList* pCrl,
                                                const char* directoryPath,
                                                const bool bEraseExistingFiles);

static SOPC_ReturnStatus may_create_pki_folder(const char* pBasePath, const char* pSubPath, char** ppPath);

static bool ignore_filtered_file(const char* pFilePath);

static SOPC_ReturnStatus load_certificate_or_crl_list(const char* basePath,
                                                      SOPC_CertificateList** ppCerts,
                                                      SOPC_CRLList** ppCrl,
                                                      bool bIscrl,
                                                      bool bDefaultBuild);

static SOPC_ReturnStatus sopc_pki_load_certificate_and_crl_list_from_store(const char* basePath,
                                                                           SOPC_CertificateList** ppTrustedCerts,
                                                                           SOPC_CRLList** ppTrustedCrl,
                                                                           SOPC_CertificateList** ppIssuerCerts,
                                                                           SOPC_CRLList** ppIssuerCrl,
                                                                           bool bDefaultBuild);

static bool pki_updated_trust_list_dir_exists(const char* path);

static SOPC_ReturnStatus pki_create_from_store(
    const char* directoryStorePath,
    bool bDefaultBuild, /* If true load the root PKI directory without trust list update,
                           if false try to load the updated trust list subdirectory.*/
    SOPC_PKIProvider** ppPKI);

static SOPC_ReturnStatus sopc_pki_merge_crls(SOPC_CRLList* pLeft, SOPC_CRLList* pRight, SOPC_CRLList** ppRes);

static SOPC_ReturnStatus sopc_pki_remove_cert_by_thumbprint(SOPC_CertificateList** ppList,
                                                            SOPC_CRLList** ppCRLList,
                                                            const char* pThumbprint,
                                                            const char* listName,
                                                            bool* pbIsRemoved,
                                                            bool* pbIsIssuer);

static SOPC_ReturnStatus sopc_pki_check_lists(SOPC_CertificateList* pTrustedCerts,
                                              SOPC_CertificateList* pIssuerCerts,
                                              SOPC_CRLList* pTrustedCrl,
                                              SOPC_CRLList* pIssuerCrl,
                                              bool* bTrustedCaFound,
                                              bool* bIssuerCaFound);

static SOPC_ReturnStatus sopc_pki_validate_anything(SOPC_PKIProvider* pPKI,
                                                    const SOPC_CertificateList* pToValidate,
                                                    const SOPC_PKI_Profile* pProfile,
                                                    uint32_t* error);

static SOPC_ReturnStatus sopc_pki_check_list_length(SOPC_PKIProvider* pPKI,
                                                    SOPC_CertificateList* pTrustedCerts,
                                                    SOPC_CRLList* pTrustedCrl,
                                                    SOPC_CertificateList* pIssuerCerts,
                                                    SOPC_CRLList* pIssuerCrl,
                                                    const bool bIncludeExistingList);

static SOPC_ReturnStatus sopc_pki_get_list_length(const SOPC_CertificateList* pTrustedCerts,
                                                  const SOPC_CRLList* pTrustedCrl,
                                                  const SOPC_CertificateList* pIssuerCerts,
                                                  const SOPC_CRLList* pIssuerCrl,
                                                  uint32_t* listLength);

static const SOPC_PKI_ChainProfile* sopc_pki_get_chain_profile_from_security_policy(const char* uri);

static const SOPC_PKI_LeafProfile* sopc_pki_get_leaf_profile_from_security_policy(const char* uri);

/**
 * Static functions definition
 */

static void sopc_pki_clear(SOPC_PKIProvider* pPKI)
{
    if (NULL == pPKI)
    {
        return;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllTrusted);
    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllCerts);
    SOPC_KeyManager_CRL_Free(pPKI->pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pIssuerCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pAllCrl);
    SOPC_KeyManager_Certificate_Free(pPKI->pRejectedList);
    SOPC_Free(pPKI->directoryStorePath);
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

// Copy newPKI content into currentPKI by preserving currentPKI mutex and then clear previous PKI content.
// Then frees the new PKI structure.
static void sopc_internal_replace_pki_and_clear(SOPC_PKIProvider* currentPKI, SOPC_PKIProvider** newPKI)
{
    // tmpPKI used for clear
    SOPC_PKIProvider tmpPKI = *currentPKI;
    tmpPKI.mutex = (*newPKI)->mutex;
    // Replace all except mutex which shall remain the same since PKI is already in use
    currentPKI = memcpy(((char*) currentPKI) + sizeof(SOPC_Mutex), ((char*) (*newPKI)) + sizeof(SOPC_Mutex),
                        sizeof(SOPC_PKIProvider) - sizeof(SOPC_Mutex));
    // clear previous PKI data and unused new PKI mutex
    sopc_pki_clear(&tmpPKI);
    // frees unused new PKI structure
    SOPC_Free(*newPKI);
    *newPKI = NULL;
}

static SOPC_ReturnStatus sopc_pki_check_application_uri(const SOPC_CertificateList* pToValidate,
                                                        const char* applicationUri)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != applicationUri);

    bool ok = SOPC_KeyManager_Certificate_CheckApplicationUri(pToValidate, applicationUri);
    if (!ok)
    {
        char* pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
        const char* thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : the application URI %s is not stored in the URI SAN "
                               "extension of certificate thumbprint %s",
                               applicationUri, thumbprint);
        SOPC_Free(pThumbprint);
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus sopc_pki_check_security_level_of_the_update(const SOPC_CertificateList* pTrustedCerts,
                                                                     const SOPC_CRLList* pTrustedCrl,
                                                                     const SOPC_CertificateList* pIssuerCerts,
                                                                     const SOPC_CRLList* pIssuerCrl,
                                                                     const char* securityPolicyUri)
{
    /*
    TODO :

    -1 Add a way to configure the security level for each security policy uri (give them a weight)
    -2 For each certificate, retrieve their security policies from their properties.
       How to do it? The following issue has been SUBMITTED : https://mantis.opcfoundation.org/view.php?id=8976
    */

    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(securityPolicyUri);

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus sopc_pki_merge_certificates(SOPC_CertificateList* pLeft,
                                                     SOPC_CertificateList* pRight,
                                                     SOPC_CertificateList** ppRes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == ppRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateList* pRes = *ppRes;
    /* Left part */
    if (NULL != pLeft)
    {
        status = SOPC_KeyManager_Certificate_Copy(pLeft, &pRes);
    }
    /* Right part */
    if (NULL != pRight && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pRight, &pRes);
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pRes);
        pRes = NULL;
    }
    *ppRes = pRes;
    return status;
}

#if SOPC_HAS_FILESYSTEM
static SOPC_ReturnStatus remove_files(const char* directoryPath)
{
    SOPC_ASSERT(NULL != directoryPath);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Array* pFilePaths = NULL;
    char* pFilePath = NULL;
    int res = -1;
    /* Get the array of file paths from the given directory */
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(directoryPath, &pFilePaths);
    if (SOPC_FileSystem_GetDir_OK != dirRes)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI write to store: failed to open directory <%s>.",
                               directoryPath);
        return SOPC_STATUS_NOK;
    }
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    for (size_t idx = 0; idx < nbFiles && SOPC_STATUS_OK == status; idx++)
    {
        pFilePath = SOPC_Array_Get(pFilePaths, char*, idx);
        res = remove(pFilePath);
        if (0 != res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    SOPC_Array_Delete(pFilePaths);
    return status;
}
#else
SOPC_ReturnStatus remove_files(const char* directoryPath)
{
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM */

static SOPC_ReturnStatus write_cert_to_der_files(SOPC_CertificateList* pRoots,
                                                 SOPC_CertificateList* pCerts,
                                                 const char* directoryPath,
                                                 const bool bEraseExistingFiles)
{
    SOPC_ASSERT(NULL != directoryPath);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bEraseExistingFiles)
    {
        status = remove_files(directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pRoots)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pRoots, directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pCerts)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pCerts, directoryPath);
    }
    return status;
}

static SOPC_ReturnStatus write_crl_to_der_files(SOPC_CRLList* pCrl,
                                                const char* directoryPath,
                                                const bool bEraseExistingFiles)
{
    SOPC_ASSERT(NULL != directoryPath);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bEraseExistingFiles)
    {
        status = remove_files(directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pCrl)
    {
        status = SOPC_KeyManager_CRL_ToDER_Files(pCrl, directoryPath);
    }
    return status;
}

static SOPC_ReturnStatus may_create_pki_folder(const char* pBasePath, const char* pSubPath, char** ppPath)
{
    SOPC_FileSystem_CreationResult mkdir_res = SOPC_FileSystem_Creation_Error_UnknownIssue;
    char* pPath = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(pBasePath, pSubPath, &pPath);
    if (SOPC_STATUS_OK == status)
    {
        mkdir_res = SOPC_FileSystem_mkdir(pPath);
        if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdir_res && SOPC_FileSystem_Creation_OK != mkdir_res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pPath);
        pPath = NULL;
    }
    *ppPath = pPath;
    return status;
}

static bool ignore_filtered_file(const char* pFilePath)
{
    char* lastSep = strrchr(pFilePath, '/');
    if (NULL != lastSep && '.' == lastSep[1])
    {
        // Ignore file if first file character is a dot '.'
        return true;
    }
    return false;
}

static SOPC_ReturnStatus load_certificate_or_crl_list(const char* basePath,
                                                      SOPC_CertificateList** ppCerts,
                                                      SOPC_CRLList** ppCrl,
                                                      bool bIscrl,
                                                      bool bDefaultBuild)
{
    SOPC_ASSERT(NULL != basePath);
    if (bIscrl)
    {
        SOPC_ASSERT(NULL != ppCrl && NULL == ppCerts);
    }
    else
    {
        SOPC_ASSERT(NULL == ppCrl && NULL != ppCerts);
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Array* pFilePaths = NULL;
    SOPC_CertificateList* pCerts = NULL;
    SOPC_CRLList* pCrl = NULL;
    char* pFilePath = NULL;
    /* Get the array of path from basePath */
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(basePath, &pFilePaths);
    if (SOPC_FileSystem_GetDir_OK != dirRes)
    {
        if (!bDefaultBuild)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON, "> PKI creation warning: failed to open directory <%s>.",
                                     basePath);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: failed to open directory <%s>.",
                                   basePath);
        }
        return SOPC_STATUS_NOK;
    }
    /* Get the size and iterate for each item */
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    for (size_t idx = 0; idx < nbFiles && SOPC_STATUS_OK == status; idx++)
    {
        pFilePath = SOPC_Array_Get(pFilePaths, char*, idx);
        if (ignore_filtered_file(pFilePath))
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "> PKI ignoring file <%s>", pFilePath);
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "> PKI loading file <%s>", pFilePath);

            if (bIscrl)
            {
                /* Load CRL */
                status = SOPC_KeyManager_CRL_CreateOrAddFromFile(pFilePath, &pCrl);
            }
            else
            {
                /* Load CERT */
                status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(pFilePath, &pCerts);
            }
        }
    }
    /* Clear in case of error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCerts);
        SOPC_KeyManager_CRL_Free(pCrl);
    }
    else
    {
        if (bIscrl)
        {
            *ppCrl = pCrl;
        }
        else
        {
            *ppCerts = pCerts;
        }
    }
    /* Always clear */
    SOPC_Array_Delete(pFilePaths);
    return status;
}

static SOPC_ReturnStatus sopc_pki_load_certificate_and_crl_list_from_store(const char* basePath,
                                                                           SOPC_CertificateList** ppTrustedCerts,
                                                                           SOPC_CRLList** ppTrustedCrl,
                                                                           SOPC_CertificateList** ppIssuerCerts,
                                                                           SOPC_CRLList** ppIssuerCrl,
                                                                           bool bDefaultBuild)
{
    SOPC_ASSERT(NULL != basePath);
    SOPC_ASSERT(NULL != ppTrustedCerts);
    SOPC_ASSERT(NULL != ppTrustedCrl);
    SOPC_ASSERT(NULL != ppIssuerCerts);
    SOPC_ASSERT(NULL != ppIssuerCrl);
    /* Trusted Certs */
    char* trustedCertsPath = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(basePath, STR_TRUSTED_CERTS, &trustedCertsPath);
    if (SOPC_STATUS_OK == status)
    {
        status = load_certificate_or_crl_list(trustedCertsPath, ppTrustedCerts, NULL, false, bDefaultBuild);
    }
    SOPC_Free(trustedCertsPath);
    /* Trusted Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* trustedCrlPath = NULL;
        status = SOPC_StrConcat(basePath, STR_TRUSTED_CRL, &trustedCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(trustedCrlPath, NULL, ppTrustedCrl, true, bDefaultBuild);
        }
        SOPC_Free(trustedCrlPath);
    }
    /* Issuer Certs */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCertsPath = NULL;
        status = SOPC_StrConcat(basePath, STR_ISSUERS_CERTS, &issuerCertsPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCertsPath, ppIssuerCerts, NULL, false, bDefaultBuild);
        }
        SOPC_Free(issuerCertsPath);
    }
    /* Issuer Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCrlPath = NULL;
        status = SOPC_StrConcat(basePath, STR_ISSUERS_CRL, &issuerCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCrlPath, NULL, ppIssuerCrl, true, bDefaultBuild);
        }
        SOPC_Free(issuerCrlPath);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(*ppTrustedCerts);
        SOPC_KeyManager_Certificate_Free(*ppIssuerCerts);
        SOPC_KeyManager_CRL_Free(*ppTrustedCrl);
        SOPC_KeyManager_CRL_Free(*ppIssuerCrl);
        *ppTrustedCerts = NULL;
        *ppIssuerCerts = NULL;
        *ppTrustedCrl = NULL;
        *ppIssuerCrl = NULL;
    }
    return status;
}

static bool pki_updated_trust_list_dir_exists(const char* path)
{
    SOPC_Array* pFilePaths = NULL;
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(path, &pFilePaths);
    SOPC_Array_Delete(pFilePaths);
    return (SOPC_FileSystem_GetDir_OK == dirRes);
}

static SOPC_ReturnStatus pki_create_from_store(
    const char* directoryStorePath,
    bool bDefaultBuild, /* If true load the root PKI directory without trust list update,
                         if false try to load the updated trust list subdirectory.*/
    SOPC_PKIProvider** ppPKI)
{
    SOPC_ASSERT(NULL != directoryStorePath);
    SOPC_ASSERT(NULL != ppPKI);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CertificateList* pIssuerCerts = NULL;  /* issuer intermediate CA */
    SOPC_CRLList* pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CRLList* pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    const char* basePath = NULL;
    char* path = NULL;
    char* trust_list_name = NULL;

    /* Select the right folder: add updateTrustList subdirectory path containing PKI update*/
    if (!bDefaultBuild)
    {
        status = SOPC_StrConcat(directoryStorePath, STR_TRUSTLIST_NAME, &path);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
        if (pki_updated_trust_list_dir_exists(path))
        {
            basePath = path;
        }
        else
        {
            SOPC_Free(path);
            path = NULL;
            status = SOPC_STATUS_WOULD_BLOCK;
        }
    }
    else
    {
        basePath = directoryStorePath;
    }
    /* Load the files from the directory Store path */
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_load_certificate_and_crl_list_from_store(basePath, &pTrustedCerts, &pTrustedCrl,
                                                                   &pIssuerCerts, &pIssuerCrl, bDefaultBuild);
    }
    /* Check if the trustList is empty */
    if (SOPC_STATUS_OK == status && NULL == pTrustedCerts && NULL == pTrustedCrl && NULL == pIssuerCerts &&
        NULL == pIssuerCrl)
    {
        status = SOPC_STATUS_NOK;
        if (!bDefaultBuild)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON, "> PKI creation warning: certificate store is empty (%s).",
                                     path);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: certificate store is empty (%s).",
                                   path);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, ppPKI);
    }
    /* if error then try with trusted and issuers folder. */
    if (!bDefaultBuild && SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON,
                              "> PKI creation: loading PKI store root directory %s content (default behavior).",
                              directoryStorePath);
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON,
                              "> PKI creation: the updated PKI store subdirectory %s%s is absent"
                              " or its content cannot be loaded (see warnings in this case).",
                              directoryStorePath, STR_TRUSTLIST_NAME);
        status = pki_create_from_store(directoryStorePath, true, ppPKI);
    }
    /* Copy the directoryStorePath */
    if (SOPC_STATUS_OK == status)
    {
        /* Copy only if not done during the recursive call. */
        if (NULL == (*ppPKI)->directoryStorePath)
        {
            (*ppPKI)->directoryStorePath = SOPC_strdup(directoryStorePath);
            if (NULL == (*ppPKI)->directoryStorePath)
            {
                SOPC_PKIProvider_Free(ppPKI);
                *ppPKI = NULL;
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }

    /* Clear */
    SOPC_Free(path);
    SOPC_Free(trust_list_name);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pIssuerCrl);

    return status;
}

static SOPC_ReturnStatus sopc_pki_get_list_length(const SOPC_CertificateList* pTrustedCerts,
                                                  const SOPC_CRLList* pTrustedCrl,
                                                  const SOPC_CertificateList* pIssuerCerts,
                                                  const SOPC_CRLList* pIssuerCrl,
                                                  uint32_t* listLength)
{
    *listLength = 0;
    size_t lenTrustedCerts = 0;
    size_t lenTrustedCrl = 0;
    size_t lenIssuerCerts = 0;
    size_t lenIssuerCrl = 0;
    size_t lenTot = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != pTrustedCerts)
    {
        status = SOPC_KeyManager_Certificate_GetListLength(pTrustedCerts, &lenTrustedCerts);
    }
    if (NULL != pTrustedCrl && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_GetListLength(pTrustedCrl, &lenTrustedCrl);
    }
    if (NULL != pIssuerCerts && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_GetListLength(pIssuerCerts, &lenIssuerCerts);
    }
    if (NULL != pIssuerCrl && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_GetListLength(pIssuerCrl, &lenIssuerCrl);
    }
    if (SOPC_STATUS_OK == status)
    {
        lenTot = lenTrustedCerts + lenTrustedCrl + lenIssuerCerts + lenIssuerCrl;
        if (UINT32_MAX < lenTot)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            *listLength = (uint32_t) lenTot;
        }
    }
    return status;
}

static SOPC_ReturnStatus sopc_pki_merge_crls(SOPC_CRLList* pLeft, SOPC_CRLList* pRight, SOPC_CRLList** ppRes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == ppRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CRLList* pRes = *ppRes;
    /* Left part */
    if (NULL != pLeft)
    {
        status = SOPC_KeyManager_CRL_Copy(pLeft, &pRes);
    }
    /* Right part */
    if (NULL != pRight && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_Copy(pRight, &pRes);
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pRes);
        pRes = NULL;
    }
    *ppRes = pRes;
    return status;
}

static SOPC_ReturnStatus sopc_pki_remove_cert_by_thumbprint(SOPC_CertificateList** ppList,
                                                            SOPC_CRLList** ppCRLList,
                                                            const char* pThumbprint,
                                                            const char* listName,
                                                            bool* pbIsRemoved,
                                                            bool* pbIsIssuer)
{
    SOPC_ASSERT(NULL != ppList);
    SOPC_ASSERT(NULL != ppCRLList);
    SOPC_ASSERT(NULL != pThumbprint);
    SOPC_ASSERT(NULL != pbIsRemoved);
    SOPC_ASSERT(NULL != pbIsIssuer);

    size_t lenThumb = strlen(pThumbprint);
    SOPC_ASSERT(HEX_THUMBPRINT_SIZE == lenThumb);

    /* Initialized the value to return */
    *pbIsRemoved = false;
    *pbIsIssuer = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == *ppList)
    {
        /* the certificate list is empty, do nothing*/
        return SOPC_STATUS_OK;
    }

    uint32_t count = 0;
    bool bIsIssuer = false;
    bool bAtLeastOneIssuer = false;
    bool bAtLeastOne = false;

    bool bCertIsRemoved = true;
    while (bCertIsRemoved && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(ppList, ppCRLList, pThumbprint, &bCertIsRemoved,
                                                                    &bIsIssuer);
        if (bCertIsRemoved)
        {
            if (bIsIssuer)
            {
                bAtLeastOneIssuer = true;
            }
            if (bAtLeastOneIssuer && !bIsIssuer)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                         "> PKI remove: certificate thumbprint <%s> has been found both as CA and as "
                                         "leaf certificate from %s",
                                         pThumbprint, listName);
            }
            bAtLeastOne = true;
            count = count + 1;
        }
    }

    if (bAtLeastOne && NULL != listName)
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON,
                               "> PKI remove: certificate thumbprint <%s> has been removed (%" PRIu32 " times) from %s",
                               pThumbprint, count, listName);
    }

    *pbIsIssuer = bAtLeastOneIssuer;
    *pbIsRemoved = bAtLeastOne;
    return status;
}

static SOPC_ReturnStatus sopc_pki_check_lists(SOPC_CertificateList* pTrustedCerts,
                                              SOPC_CertificateList* pIssuerCerts,
                                              SOPC_CRLList* pTrustedCrl,
                                              SOPC_CRLList* pIssuerCrl,
                                              bool* bTrustedCaFound,
                                              bool* bIssuerCaFound)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Trusted stats */
    uint32_t trusted_ca_count = 0;
    uint32_t trusted_list_length = 0;
    uint32_t issued_cert_count = 0;
    uint32_t trusted_root_count = 0;
    /* Issuer stats */
    uint32_t issuer_ca_count = 0;
    uint32_t issuer_list_length = 0;
    uint32_t issuer_root_count = 0;
    *bTrustedCaFound = false;
    *bIssuerCaFound = false;

    if (NULL == pTrustedCerts)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: no trusted certificate is provided.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_PKIProviderInternal_GetListStats(pTrustedCerts, &trusted_ca_count, &trusted_list_length, &trusted_root_count);
    issued_cert_count = trusted_list_length - trusted_ca_count;
    /* trusted CA => trusted CRL*/
    if (0 != trusted_ca_count && NULL == pTrustedCrl)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: trusted CA certificates are provided but no CRL.");
    }
    SOPC_PKIProviderInternal_GetListStats(pIssuerCerts, &issuer_ca_count, &issuer_list_length, &issuer_root_count);
    /* issuer CA => issuer CRL*/
    if (0 != issuer_ca_count && NULL == pIssuerCrl)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: issuer CA certificates are provided but no CRL.");
    }
    /* Check if issuerCerts list is only filled with CA. */
    if (issuer_list_length != issuer_ca_count)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: not all issuer certificates are CAs.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case no roots is provided wheras at least one trusted leaf certificate is provided. */
    if ((0 == issuer_root_count) && (0 == trusted_root_count) && (0 != issued_cert_count))
    {
        /* In this case, only trusted self-signed leaf certificates will be accepted. */
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: no root (CA) defined: only trusted self-signed leaf "
                                 "certificates will be accepted without possibility to revoke them (no CRL).");
    }
    *bTrustedCaFound = 0 != trusted_ca_count;
    *bIssuerCaFound = 0 != issuer_ca_count;
    return status;
}

static SOPC_ReturnStatus sopc_pki_check_list_length(SOPC_PKIProvider* pPKI,
                                                    SOPC_CertificateList* pTrustedCerts,
                                                    SOPC_CRLList* pTrustedCrl,
                                                    SOPC_CertificateList* pIssuerCerts,
                                                    SOPC_CRLList* pIssuerCrl,
                                                    const bool bIncludeExistingList)
{
    SOPC_ASSERT(NULL != pPKI);
    uint32_t PKILen = 0;
    uint32_t updateLen = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bIncludeExistingList)
    {
        status = sopc_pki_get_list_length(pPKI->pTrustedCerts, pPKI->pTrustedCrl, pPKI->pIssuerCerts, pPKI->pIssuerCrl,
                                          &PKILen);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_get_list_length(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &updateLen);
    }
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (SOPC_PKI_MAX_NB_CERT_AND_CRL < PKILen + updateLen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: too many (%" PRIu32
                               ") certificates and CRLs. The maximum configured is %" PRIu32
                               ", please change SOPC_PKI_MAX_NB_CERT_AND_CRL",
                               PKILen + updateLen, (uint32_t) SOPC_PKI_MAX_NB_CERT_AND_CRL);
    }
    return status;
}

static SOPC_ReturnStatus sopc_pki_validate_anything(SOPC_PKIProvider* pPKI,
                                                    const SOPC_CertificateList* pToValidate,
                                                    const SOPC_PKI_Profile* pProfile,
                                                    uint32_t* error)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_OK;
}

#define NB_PROFILES (sizeof(g_all_profiles) / sizeof(*g_all_profiles))

static const SOPC_PKI_ChainProfile* sopc_pki_get_chain_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < NB_PROFILES; ++i)
    {
        const struct Profile_Cfg* pProfile = &g_all_profiles[i];
        const SOPC_SecurityPolicy_Config* pPolicy = SOPC_SecurityPolicy_Config_Get(pProfile->id);
        const char* pUri = pPolicy->uri;
        if (pUri != NULL && SOPC_strncmp_ignore_case(uri, pUri, len) == 0)
        {
            return pProfile->chain;
        }
    }

    return NULL;
}

static const SOPC_PKI_LeafProfile* sopc_pki_get_leaf_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < NB_PROFILES; ++i)
    {
        const struct Profile_Cfg* pProfile = &g_all_profiles[i];
        const SOPC_SecurityPolicy_Config* pPolicy = SOPC_SecurityPolicy_Config_Get(pProfile->id);
        const char* pUri = pPolicy->uri;
        if (pUri != NULL && SOPC_strncmp_ignore_case(uri, pUri, len) == 0)
        {
            return pProfile->leaf;
        }
    }
    return NULL;
}

#endif /* WITH_NO_CRYPTO */

/**
 * End of static functions definition
 *
 */

SOPC_ReturnStatus SOPC_PKIProvider_CreateLeafProfile(const char* securityPolicyUri, SOPC_PKI_LeafProfile** ppProfile)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == ppProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_PKI_LeafProfile* pProfileRef = NULL;
    if (NULL != securityPolicyUri)
    {
        pProfileRef = sopc_pki_get_leaf_profile_from_security_policy(securityPolicyUri);
        if (NULL == pProfileRef)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    SOPC_PKI_LeafProfile* pProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_LeafProfile));
    if (NULL == pProfile)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (NULL != securityPolicyUri)
    {
        *pProfile = *pProfileRef;
    }

    *ppProfile = pProfile;
    return SOPC_STATUS_OK;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetUsageFromType(SOPC_PKI_LeafProfile* pProfile, SOPC_PKI_Type PKIType)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(PKIType);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (PKIType)
    {
    case SOPC_PKI_TYPE_CLIENT_APP:
        pProfile->keyUsage = g_appKU;
        pProfile->extendedKeyUsage = g_clientEKU;
        break;
    case SOPC_PKI_TYPE_SERVER_APP:
        pProfile->keyUsage = g_appKU;
        pProfile->extendedKeyUsage = g_serverEKU;
        break;
    case SOPC_PKI_TYPE_USER:
        pProfile->keyUsage = g_usrKU;
        pProfile->extendedKeyUsage = g_userEKU;
        break;
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_STATUS_OK;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURI(SOPC_PKI_LeafProfile* pProfile, const char* applicationUri)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(applicationUri);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pProfile || NULL == applicationUri)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pProfile->sanApplicationUri)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    pProfile->sanApplicationUri = SOPC_strdup(applicationUri);
    if (NULL == pProfile->sanApplicationUri)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_STATUS_OK;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURL(SOPC_PKI_LeafProfile* pProfile, const char* url)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(url);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pProfile || NULL == url)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pProfile->sanURL)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    pProfile->sanURL = SOPC_strdup(url);
    if (NULL == pProfile->sanURL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_STATUS_OK;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURI(SOPC_PKI_Profile* pProfile, const char* applicationUri)
{
    return SOPC_PKIProvider_LeafProfileSetURI(pProfile->leafProfile, applicationUri);
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURL(SOPC_PKI_Profile* pProfile, const char* url)
{
    return SOPC_PKIProvider_LeafProfileSetURL(pProfile->leafProfile, url);
}

void SOPC_PKIProvider_DeleteLeafProfile(SOPC_PKI_LeafProfile** ppProfile)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppProfile);
#else
    if (NULL == ppProfile)
    {
        return;
    }
    SOPC_PKI_LeafProfile* pProfile = *ppProfile;
    if (NULL != pProfile)
    {
        SOPC_Free(pProfile->sanApplicationUri);
        SOPC_Free(pProfile->sanURL);
        SOPC_Free(pProfile);
        *ppProfile = NULL;
    }
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateProfile(const char* securityPolicyUri, SOPC_PKI_Profile** ppProfile)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == ppProfile || NULL == securityPolicyUri)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_PKI_ChainProfile* pChainProfileRef = sopc_pki_get_chain_profile_from_security_policy(securityPolicyUri);
    if (NULL == pChainProfileRef)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_PKI_ChainProfile* pChainProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_ChainProfile));
    if (NULL == pChainProfile)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    *pChainProfile = *pChainProfileRef;

    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_ReturnStatus status = SOPC_PKIProvider_CreateLeafProfile(securityPolicyUri, &pLeafProfile);
    if (SOPC_STATUS_OK == status)
    {
        pProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_Profile));
        if (NULL == pProfile)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pProfile->leafProfile = pLeafProfile;
        pProfile->chainProfile = pChainProfile;
        pProfile->bBackwardInteroperability = true;
        pProfile->bApplyLeafProfile = true;
    }
    else
    {
        SOPC_Free(pChainProfile);
        SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    }
    *ppProfile = pProfile;
    return SOPC_STATUS_OK;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetUsageFromType(SOPC_PKI_Profile* pProfile, SOPC_PKI_Type PKIType)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(PKIType);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pProfile->leafProfile, PKIType);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    switch (PKIType)
    {
    case SOPC_PKI_TYPE_CLIENT_APP:
        pProfile->bApplyLeafProfile = true;
        pProfile->bBackwardInteroperability = true;
        break;
    case SOPC_PKI_TYPE_SERVER_APP:
        pProfile->bApplyLeafProfile = true;
        pProfile->bBackwardInteroperability = true;
        break;
    case SOPC_PKI_TYPE_USER:
        pProfile->bApplyLeafProfile = false;
        pProfile->bBackwardInteroperability = false;
        break;
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_STATUS_OK;
#endif
}

void SOPC_PKIProvider_DeleteProfile(SOPC_PKI_Profile** ppProfile)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppProfile);
#else
    if (NULL == ppProfile)
    {
        return;
    }
    SOPC_PKI_Profile* pProfile = *ppProfile;
    if (NULL != pProfile)
    {
        SOPC_Free(pProfile->chainProfile);
        SOPC_PKIProvider_DeleteLeafProfile(&pProfile->leafProfile);
        SOPC_Free(pProfile);
        *ppProfile = NULL;
    }
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateMinimalUserProfile(SOPC_PKI_Profile** ppProfile)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppProfile);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    /* Minimal profile for the chain.
    The leaf profile is not used for users during the validation process but the user certificate properties
    are checked according to the security policy during the activate session */
    if (NULL == ppProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_ReturnStatus status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_USER);
    }
    *ppProfile = pProfile;
    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                        const SOPC_PKI_LeafProfile* pProfile,
                                                        uint32_t* error)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pToValidate || NULL == pProfile || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *error = SOPC_CertificateValidationError_Unknown;
    uint32_t firstError = SOPC_CertificateValidationError_Unknown;
    uint32_t currentError = SOPC_CertificateValidationError_Unknown;
    bool bErrorFound = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_PKIProvider_CheckCommonName(pToValidate);
    if (SOPC_STATUS_OK != status)
    {
        firstError = SOPC_CertificateValidationError_Invalid;
        bErrorFound = true;
    }
    if (pProfile->bApplySecurityPolicy)
    {
        status = SOPC_PKIProvider_CheckSecurityPolicy(pToValidate, pProfile);
        if (SOPC_STATUS_OK != status)
        {
            currentError = SOPC_CertificateValidationError_PolicyCheckFailed;
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }
    if (NULL != pProfile->sanURL)
    {
        status = SOPC_PKIProvider_CheckHostName(pToValidate, pProfile->sanURL);
        if (SOPC_STATUS_OK != status)
        {
            currentError = SOPC_CertificateValidationError_HostNameInvalid;
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }
    if (NULL != pProfile->sanApplicationUri)
    {
        status = sopc_pki_check_application_uri(pToValidate, pProfile->sanApplicationUri);
        if (SOPC_STATUS_OK != status)
        {
            currentError = SOPC_CertificateValidationError_UriInvalid;
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }
    status = SOPC_PKIProvider_CheckCertificateUsage(pToValidate, pProfile);
    if (SOPC_STATUS_OK != status)
    {
        currentError = SOPC_CertificateValidationError_UseNotAllowed;
        if (!bErrorFound)
        {
            firstError = currentError;
            bErrorFound = true;
        }
    }

    if (bErrorFound)
    {
        *error = firstError;
        status = SOPC_STATUS_NOK;
    }

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIPermissive_Create(SOPC_PKIProvider** ppPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    SOPC_PKIProvider* pPKI = NULL;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pPKI = SOPC_Calloc(1, sizeof(SOPC_PKIProvider));

    if (NULL == pPKI)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    pPKI->pTrustedRoots = NULL;
    pPKI->pTrustedCerts = NULL;
    pPKI->pTrustedCrl = NULL;
    pPKI->pIssuerRoots = NULL;
    pPKI->pIssuerCerts = NULL;
    pPKI->pIssuerCrl = NULL;
    pPKI->pAllCerts = NULL;
    pPKI->pAllRoots = NULL;
    pPKI->pAllCrl = NULL;
    pPKI->pRejectedList = NULL;
    pPKI->directoryStorePath = NULL;
    pPKI->pFnValidateCert = &sopc_pki_validate_anything;
    pPKI->isPermissive = true;
    *ppPKI = pPKI;
    return SOPC_STATUS_OK;
#endif
}

void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppPKI);
#else
    if (NULL == ppPKI || NULL == *ppPKI)
    {
        return;
    }
    sopc_pki_clear(*ppPKI);
    SOPC_Free(*ppPKI);
    *ppPKI = NULL;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_SetStorePath(const char* directoryStorePath, SOPC_PKIProvider* pPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(directoryStorePath);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI || NULL == directoryStorePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Create if necessary the store */
    SOPC_FileSystem_CreationResult mkdir_res = SOPC_FileSystem_mkdir(directoryStorePath);
    if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdir_res && SOPC_FileSystem_Creation_OK != mkdir_res)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Copy the directory store path before exchange the data */
        char* pCopyPath = SOPC_strdup(directoryStorePath);
        if (NULL == pCopyPath)
        {
            status = SOPC_STATUS_NOK;
        }
        SOPC_Free(pPKI->directoryStorePath);
        pPKI->directoryStorePath = pCopyPath;
    }

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(ppTrustedCerts);
    SOPC_UNUSED_ARG(ppTrustedCrl);
    SOPC_UNUSED_ARG(ppIssuerCerts);
    SOPC_UNUSED_ARG(ppIssuerCrl);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI || NULL == ppTrustedCerts || NULL == ppTrustedCrl || NULL == ppIssuerCerts || NULL == ppIssuerCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_CertificateList* pTrustedCerts = *ppTrustedCerts;
    SOPC_CRLList* pTrustedCrl = *ppTrustedCrl;
    SOPC_CertificateList* pIssuerCerts = *ppIssuerCerts;
    SOPC_CRLList* pIssuerCrl = *ppIssuerCrl;

    status = sopc_pki_merge_certificates(pPKI->pTrustedRoots, pPKI->pTrustedCerts, &pTrustedCerts);
    if (SOPC_STATUS_OK == status && NULL != pPKI->pTrustedCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pPKI->pTrustedCrl, &pTrustedCrl);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_merge_certificates(pPKI->pIssuerRoots, pPKI->pIssuerCerts, &pIssuerCerts);
    }
    if (SOPC_STATUS_OK == status && NULL != pPKI->pIssuerCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pPKI->pIssuerCrl, &pIssuerCrl);
    }
    /* Clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(pIssuerCerts);
        SOPC_KeyManager_CRL_Free(pTrustedCrl);
        SOPC_KeyManager_CRL_Free(pIssuerCrl);
        pTrustedCerts = NULL;
        pIssuerCerts = NULL;
        pTrustedCrl = NULL;
        pIssuerCrl = NULL;
    }
    *ppTrustedCerts = pTrustedCerts;
    *ppIssuerCerts = pIssuerCerts;
    *ppTrustedCrl = pTrustedCrl;
    *ppIssuerCrl = pIssuerCrl;

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteToStore(SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(bEraseExistingFiles);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* basePath = NULL;
    char* path = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* The case of the PKI is built from buffer (there is no store) */
    if (NULL == pPKI->directoryStorePath)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(pPKI->directoryStorePath, STR_TRUSTLIST_NAME, &basePath);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(basePath, STR_TRUSTED, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CERTS, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Note: might use pPKI->pAllTrusted instead which is equivalent to the merged list
        status = write_cert_to_der_files(pPKI->pTrustedRoots, pPKI->pTrustedCerts, path, bEraseExistingFiles);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CRL, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_crl_to_der_files(pPKI->pTrustedCrl, path, bEraseExistingFiles);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS_CERTS, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_cert_to_der_files(pPKI->pIssuerRoots, pPKI->pIssuerCerts, path, bEraseExistingFiles);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS_CRL, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_crl_to_der_files(pPKI->pIssuerCrl, path, bEraseExistingFiles);
    }

    SOPC_Free(basePath);
    SOPC_Free(path);

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CopyRejectedList(SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCert)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppCert);
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pRejected = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_Copy(pPKI->pRejectedList, &pRejected);
    }
    /* Clear */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pRejected);
        pRejected = NULL;
    }
    *ppCert = pRejected;

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteRejectedCertToStore(SOPC_PKIProvider* pPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* path = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* The case of the PKI is built from buffer (there is no store) */
    if (NULL == pPKI->directoryStorePath)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(pPKI->directoryStorePath, STR_REJECTED, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = remove_files(path);
        }
    }
    if (SOPC_STATUS_OK == status && NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pPKI->pRejectedList, path);
    }
    SOPC_Free(path);

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_UpdateFromList(SOPC_PKIProvider* pPKI,
                                                  const char* securityPolicyUri,
                                                  SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  const bool bIncludeExistingList)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(securityPolicyUri);
    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(bIncludeExistingList);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    /* Check parameters */
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_PKIProvider* pTmpPKI = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Check the number of certificates plus CRLs */
    status =
        sopc_pki_check_list_length(pPKI, pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, bIncludeExistingList);

    /* Handle that the security level of the update isn't higher than the
    security level of the secure channel. (§7.3.4 part 2 v1.05) */
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_check_security_level_of_the_update(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl,
                                                             securityPolicyUri);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Includes the existing TrustList plus any updates */
        if (bIncludeExistingList && !pPKI->isPermissive)
        {
            SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
            SOPC_CertificateList* tmp_pTrustedCertsTmp = NULL;
            SOPC_CRLList* tmp_pTrustedCrl = NULL;          /* CRLs of trusted intermediate CA and trusted root CA */
            SOPC_CertificateList* tmp_pIssuerCerts = NULL; /* issuer intermediate CA + issuer root CA */
            SOPC_CertificateList* tmp_pIssuerCertsTmp = NULL;
            SOPC_CRLList* tmp_pIssuerCrl = NULL; /* CRLs of issuer intermediate CA and issuer root CA */

            /* tmp_pTrustedCerts = pTrustedCerts + pPKI->pTrustedCerts + pPKI->pTrustedRoot */
            status = sopc_pki_merge_certificates(pPKI->pTrustedCerts, pTrustedCerts, &tmp_pTrustedCertsTmp);
            if (SOPC_STATUS_OK == status)
            {
                status = sopc_pki_merge_certificates(pPKI->pTrustedRoots, tmp_pTrustedCertsTmp, &tmp_pTrustedCerts);
            }
            /* tmp_pTrustedCrl = pTrustedCrl + pPKI->pTrustedCrl */
            if (SOPC_STATUS_OK == status)
            {
                status = sopc_pki_merge_crls(pPKI->pTrustedCrl, pTrustedCrl, &tmp_pTrustedCrl);
            }
            /* tmp_pIssuerCerts = pIssuerCerts + pPKI->pIssuerCerts + pPKI->pIssuerRoot */
            if (SOPC_STATUS_OK == status)
            {
                status = sopc_pki_merge_certificates(pPKI->pIssuerCerts, pIssuerCerts, &tmp_pIssuerCertsTmp);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = sopc_pki_merge_certificates(pPKI->pIssuerRoots, tmp_pIssuerCertsTmp, &tmp_pIssuerCerts);
            }
            /* tmp_pIssuerCrl = pIssuerCrl + pPKI->pIssuerCrl */
            if (SOPC_STATUS_OK == status)
            {
                status = sopc_pki_merge_crls(pPKI->pIssuerCrl, pIssuerCrl, &tmp_pIssuerCrl);
            }
            /* Create a new tmp PKI */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_PKIProvider_CreateFromList(tmp_pTrustedCerts, tmp_pTrustedCrl, tmp_pIssuerCerts,
                                                         tmp_pIssuerCrl, &pTmpPKI);
            }

            SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
            SOPC_KeyManager_Certificate_Free(tmp_pTrustedCertsTmp);
            SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
            SOPC_KeyManager_Certificate_Free(tmp_pIssuerCertsTmp);
            SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
            SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);
        }
        else
        {
            /* Create a new tmp PKI without the existing TrustList */
            status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &pTmpPKI);
        }
    }
    /* Copy the rejected list before exchange the data */
    if (SOPC_STATUS_OK == status && NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_Copy(pPKI->pRejectedList, &pTmpPKI->pRejectedList);
    }
    /* Copy the directory store path before exchange the data */
    if (SOPC_STATUS_OK == status && NULL != pPKI->directoryStorePath)
    {
        pTmpPKI->directoryStorePath = SOPC_strdup(pPKI->directoryStorePath);
        if (NULL == pTmpPKI->directoryStorePath)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Exchange the internal data between tmpPKI and PKI, clear previous data and free tmpPKI structure
    // Note: mutex is kept since PKI should already be in use
    if (SOPC_STATUS_OK == status)
    {
        sopc_internal_replace_pki_and_clear(pPKI, &pTmpPKI);
    }

    // In case of failure we need to clear and free the temporary PKI
    if (NULL != pTmpPKI)
    {
        sopc_pki_clear(pTmpPKI);
        SOPC_Free(pTmpPKI);
        pTmpPKI = NULL;
    }

    // Unlock PKI prior to possibly clearing it
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(SOPC_PKIProvider* pPKI,
                                                     const char* pThumbprint,
                                                     const bool bIsTrusted,
                                                     bool* pIsRemoved,
                                                     bool* pIsIssuer)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pThumbprint);
    SOPC_UNUSED_ARG(bIsTrusted);
    SOPC_UNUSED_ARG(pIsRemoved);
    SOPC_UNUSED_ARG(pIsIssuer);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    /* Initialized the value to return */
    *pIsRemoved = false;
    *pIsIssuer = false;
    if (NULL == pPKI || NULL == pThumbprint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t lenThumbprint = strlen(pThumbprint);
    if (HEX_THUMBPRINT_SIZE != lenThumbprint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool bRootIsRemoved = false;
    bool bCertIsRemoved = false;
    bool bCertIsCA = false;
    bool bRootIsCA = false;

    bool bIsIssuer = false;
    bool bIsRemoved = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Remove from trusted certificates */
    if (bIsTrusted)
    {
        if (NULL != pPKI->pTrustedCerts)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pTrustedCerts, &pPKI->pTrustedCrl, pThumbprint,
                                                        "trusted list", &bCertIsRemoved, &bCertIsCA);
        }
        if (NULL != pPKI->pTrustedRoots && SOPC_STATUS_OK == status)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pTrustedRoots, &pPKI->pTrustedCrl, pThumbprint,
                                                        "trusted root list", &bRootIsRemoved, &bRootIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || bRootIsCA == bRootIsRemoved);
        }
        if (NULL != pPKI->pAllTrusted && SOPC_STATUS_OK == status)
        {
            bool bAllTrustedRemoved = false;
            bool bAllTrustedIsCA = false;
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllTrusted, &pPKI->pTrustedCrl, pThumbprint, NULL,
                                                        &bAllTrustedRemoved, &bAllTrustedIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || (bAllTrustedRemoved == (bRootIsRemoved || bCertIsRemoved) &&
                                                     (bAllTrustedIsCA == (bCertIsCA || bRootIsCA))));
        }
    }
    else
    {
        /* Remove from issuer certificates */
        if (NULL != pPKI->pIssuerCerts)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pIssuerCerts, &pPKI->pIssuerCrl, pThumbprint,
                                                        "issuer list", &bCertIsRemoved, &bCertIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || bCertIsCA == bCertIsRemoved);
        }
        if (NULL != pPKI->pIssuerRoots && SOPC_STATUS_OK == status)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pIssuerRoots, &pPKI->pIssuerCrl, pThumbprint,
                                                        "issuer root list", &bRootIsRemoved, &bRootIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || bRootIsCA == bRootIsRemoved);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bCertIsRemoved || bRootIsRemoved)
        {
            bIsIssuer = bCertIsCA || bRootIsCA;
            bIsRemoved = true;
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                     "> PKI remove: certificate thumbprint <%s> has not been found", pThumbprint);
        }
    }
    if (SOPC_STATUS_OK == status && bIsRemoved)
    {
        bool bAllCertIsRemoved = false;
        bool bAllRootIsRemoved = false;
        bool bAllCertIsCA = false;
        bool bAllRootIsCA = false;
        /* Remove from all list */
        if (NULL != pPKI->pAllCerts)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllCerts, &pPKI->pAllCrl, pThumbprint, NULL,
                                                        &bAllCertIsRemoved, &bAllCertIsCA);
        }
        if (NULL != pPKI->pAllRoots && SOPC_STATUS_OK == status)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllRoots, &pPKI->pAllCrl, pThumbprint, NULL,
                                                        &bAllRootIsRemoved, &bAllRootIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || bAllRootIsCA == bAllRootIsRemoved);
        }
        SOPC_ASSERT(SOPC_STATUS_OK != status || (bCertIsRemoved == bAllCertIsRemoved && bCertIsCA == bAllCertIsCA));
        SOPC_ASSERT(SOPC_STATUS_OK != status || (bRootIsRemoved == bAllRootIsRemoved && bRootIsCA == bAllRootIsCA));
    }

    *pIsIssuer = bIsIssuer;
    *pIsRemoved = bIsRemoved;

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromStore(const char* directoryStorePath, SOPC_PKIProvider** ppPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(ppPKI);
    SOPC_UNUSED_ARG(directoryStorePath);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == directoryStorePath || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = pki_create_from_store(directoryStorePath, false, ppPKI);
    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != pPKI->pFnValidateCert)
    {
        status = pPKI->pFnValidateCert(pPKI, pToValidate, pProfile, error);
    }
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
#endif
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  SOPC_PKIProvider** ppPKI)
{
#ifdef WITH_NO_CRYPTO
    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(ppPKI);
    return SOPC_STATUS_NOT_SUPPORTED;
#else
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* tmp_pTrustedRoots = NULL; /* trusted root CA */
    SOPC_CertificateList* tmp_pIssuerRoots = NULL;  /* issuer root CA */
    SOPC_CertificateList* tmp_pAllRoots = NULL;     /* issuer + trusted roots */
    SOPC_CertificateList* tmp_pAllCerts = NULL;     /* issuer + trusted certs */
    SOPC_CertificateList* tmp_pAllTrusted = NULL;   /* trusted CAs and certs */

    SOPC_CRLList* tmp_pAllCrl = NULL; /* issuer crl + trusted crl  */

    SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CRLList* tmp_pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CertificateList* tmp_pIssuerCerts = NULL;  /* issuer intermediate CA + issuer root CA */
    SOPC_CRLList* tmp_pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    bool bTrustedCaFound = false;
    bool bIssuerCaFound = false;
    uint32_t listLength = 0;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check the number of certificates plus CRLs of the PKI */
    status = sopc_pki_get_list_length(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &listLength);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (SOPC_PKI_MAX_NB_CERT_AND_CRL < listLength)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: too many (%" PRIu32
                               ") certificates and CRLs. The maximum configured is %" PRIu32
                               ", please change SOPC_PKI_MAX_NB_CERT_AND_CRL",
                               listLength, (uint32_t) SOPC_PKI_MAX_NB_CERT_AND_CRL);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /*
       - Check that pTrustedCerts is not empty.
       - Check if there are CAs but no CRLs.
       - Check if issuerCerts list is only filled with CA.
       - Check and warn if issuerCerts is not empty but pTrustedCerts is only filed with CA.
         In this case, if there is no root into pTrustedCerts then
         no certificates will be accepted during validation process.
       - Check and warn in case no root defined but trusted certificates defined.
         In this case, only trusted self-signed issued certificates will be accepted.
    */
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_check_lists(pTrustedCerts, pIssuerCerts, pTrustedCrl, pIssuerCrl, &bTrustedCaFound,
                                      &bIssuerCaFound);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
    }

    /* Copy the lists */
    status = SOPC_KeyManager_Certificate_Copy(pTrustedCerts, &tmp_pTrustedCerts);
    if (SOPC_STATUS_OK == status && NULL != pTrustedCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pTrustedCrl, &tmp_pTrustedCrl);
    }
    if (SOPC_STATUS_OK == status && NULL != pIssuerCerts)
    {
        status = SOPC_KeyManager_Certificate_Copy(pIssuerCerts, &tmp_pIssuerCerts);
    }
    if (SOPC_STATUS_OK == status && NULL != pIssuerCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pIssuerCrl, &tmp_pIssuerCrl);
    }

    /* Check the CRL-CA association before creating the PKI. */
    bool bTrustedCRL = false;
    bool bIssuerCRL = false;
    if (SOPC_STATUS_OK == status)
    {
        if (bTrustedCaFound && NULL != tmp_pTrustedCrl)
        {
            status = SOPC_KeyManager_CertificateList_CheckCRL(tmp_pTrustedCerts, tmp_pTrustedCrl, &bTrustedCRL);
        }
        else
        {
            bTrustedCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bIssuerCaFound && NULL != tmp_pIssuerCrl)
        {
            status = SOPC_KeyManager_CertificateList_CheckCRL(tmp_pIssuerCerts, tmp_pIssuerCrl, &bIssuerCRL);
        }
        else
        {
            bIssuerCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status && (!bTrustedCRL || !bIssuerCRL))
    {
        if (!bTrustedCRL)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_COMMON,
                "> PKI creation warning: Not all certificate authorities in given trusted certificates have at least "
                "one certificate revocation list! Certificates issued by these CAs will be refused.");
        }
        if (!bIssuerCRL)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_COMMON,
                "> PKI creation warning: Not all certificate authorities in given issuer certificates have at least "
                "one certificate revocation list! Certificates issued by these CAs will be refused.");
        }
    }
    /* Retrieve the root from list */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderInternal_SplitRootFromCertList(&tmp_pTrustedCerts, &tmp_pTrustedRoots);
    }
    if (SOPC_STATUS_OK == status && NULL != tmp_pIssuerCerts)
    {
        status = SOPC_PKIProviderInternal_SplitRootFromCertList(&tmp_pIssuerCerts, &tmp_pIssuerRoots);
    }
    /* Merge trusted and issuer list */
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_merge_certificates(tmp_pIssuerCerts, tmp_pTrustedCerts, &tmp_pAllCerts);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_merge_certificates(tmp_pIssuerRoots, tmp_pTrustedRoots, &tmp_pAllRoots);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = sopc_pki_merge_crls(tmp_pIssuerCrl, tmp_pTrustedCrl, &tmp_pAllCrl);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pTrustedCerts, &tmp_pAllTrusted);
    }
    /* Create the PKI */
    if (SOPC_STATUS_OK == status)
    {
        pPKI = SOPC_Calloc(1, sizeof(SOPC_PKIProvider));
        if (NULL == pPKI)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&pPKI->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        pPKI->pTrustedRoots = tmp_pTrustedRoots;
        pPKI->pTrustedCerts = tmp_pTrustedCerts;
        pPKI->pTrustedCrl = tmp_pTrustedCrl;
        pPKI->pIssuerRoots = tmp_pIssuerRoots;
        pPKI->pIssuerCerts = tmp_pIssuerCerts;
        pPKI->pIssuerCrl = tmp_pIssuerCrl;
        pPKI->pAllCerts = tmp_pAllCerts;
        pPKI->pAllRoots = tmp_pAllRoots;
        pPKI->pAllTrusted = tmp_pAllTrusted;
        pPKI->pAllCrl = tmp_pAllCrl;
        pPKI->pRejectedList = NULL;
        pPKI->directoryStorePath = NULL;
        pPKI->pFnValidateCert = &SOPC_PKIProviderInternal_ValidateProfileAndCertificate;
        pPKI->isPermissive = false;
        *ppPKI = pPKI;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pAllRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pAllTrusted);
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pAllCerts);
        SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
        SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);
        SOPC_KeyManager_CRL_Free(tmp_pAllCrl);
    }

    return status;
#endif
}
