/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _OpcUa_PKIProvider_H_
#define _OpcUa_PKIProvider_H_ 1

#include "sopc_types_wrapper.h"

OPCUA_BEGIN_EXTERN_C

// Add dependencies on opcua_p_pki.h

/**
  @brief The supported PKIs.
*/
typedef enum _OpcUa_P_PKI_Types
{
    OpcUa_Invalid_PKI   = 0,
    OpcUa_NO_PKI        = 1,
    OpcUa_Override      = 2,
    OpcUa_OpenSSL_PKI   = 3
} OpcUa_P_PKI_Types;

/* OPENSSL PKI specific flags */
#define OPCUA_P_PKI_OPENSSL_USE_DEFAULT_CERT_CRL_LOOKUP_METHOD       0x0001
#define OPCUA_P_PKI_OPENSSL_DONT_ADD_TRUST_LIST_TO_ROOT_CERTIFICATES 0x0002
#define OPCUA_P_PKI_OPENSSL_ADD_UNTRUSTED_LIST_TO_ROOT_CERTIFICATES  0x0004
#define OPCUA_P_PKI_OPENSSL_UNTRUSTED_LIST_IS_INDEX                  0x0008
#define OPCUA_P_PKI_OPENSSL_REVOCATION_LIST_IS_INDEX                 0x0010
#define OPCUA_P_PKI_OPENSSL_REVOCATION_LIST_IS_CONCATENATED_PEM_FILE 0x0020
#define OPCUA_P_PKI_OPENSSL_SUPPRESS_CERT_VALIDITY_PERIOD_CHECK      0x0040
#define OPCUA_P_PKI_OPENSSL_SUPPRESS_CRL_VALIDITY_PERIOD_CHECK       0x0080
#define OPCUA_P_PKI_OPENSSL_SUPPRESS_CRL_NOT_FOUND_ERROR             0x0100
#define OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ONLY_LEAF               0x0200
#define OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ALL_EXCEPT_SELF_SIGNED  0x0400
#define OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ALL                     0x0600
#define OPCUA_P_PKI_OPENSSL_CHECK_SELF_SIGNED_SIGNATURE              0x0800
#define OPCUA_P_PKI_OPENSSL_REQUIRE_CHAIN_CERTIFICATE_IN_TRUST_LIST  0x1000
#define OPCUA_P_PKI_OPENSSL_ALLOW_PROXY_CERTIFICATES                 0x2000
#define OPCUA_P_PKI_OPENSSL_OVERRIDE_IS_DHPARAM_FILE                 0x4000

/**
  @brief The openssl pki config.
  */
struct _OpcUa_P_OpenSSL_CertificateStore_Config
{
    /*! @brief used PKI type. */
    OpcUa_P_PKI_Types   PkiType;

    /*! @brief The trusted certificate store location. */
    OpcUa_StringA       CertificateTrustListLocation;

    /*! @brief The certificate revocation list. */
    OpcUa_StringA       CertificateRevocationListLocation;

    /*! @brief The untrusted certificate store location. */
    OpcUa_StringA       CertificateUntrustedListLocation;

    /*! @brief PKI-specific flags. */
    OpcUa_UInt32        Flags;

    /*! @brief External PKIProvider IF to override default implementation. Checked when Configuration name is "Override" */
    OpcUa_Void*         Override;
};
typedef struct _OpcUa_P_OpenSSL_CertificateStore_Config OpcUa_P_OpenSSL_CertificateStore_Config;

/** note on: .PkiType == OpcUa_OpenSSL_PKI:
 *
 * recommended flags:
 * .Flags = OPCUA_P_PKI_OPENSSL_ADD_UNTRUSTED_LIST_TO_ROOT_CERTIFICATES|
 *          OPCUA_P_PKI_OPENSSL_REQUIRE_CHAIN_CERTIFICATE_IN_TRUST_LIST|
 *          OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ALL;
 *
 * .CertificateTrustListLocation = "<trusted-certs-directory>";
 * .CertificateUntrustedListLocation = "<untrusted-certs-directory>";
 * .CertificateRevocationListLocation = "<revoked-certs-directory>";
 *
 * compatibility:
 * .Flags = 0;
 * .CertificateTrustListLocation = "<certs-directory>";
 */


/**
  @brief The certificate und key format enumeration.
*/
typedef enum _OpcUa_P_FileFormat
{
    OpcUa_Crypto_Encoding_Invalid   = 0,
    OpcUa_Crypto_Encoding_DER       = 1,
    OpcUa_Crypto_Encoding_PEM       = 2,
    OpcUa_Crypto_Encoding_PKCS12    = 3
}
OpcUa_P_FileFormat;

// Original opcua_pki.h

struct _OpcUa_PKIProvider;
/** 
  @brief Validates a given X509 certificate object.

   Validation:
   - Subject/Issuer
   - Path
   - Certificate Revocation List (CRL)
   - Certificate Trust List (CTL)

  @param pPKI                     [in]  The pki handle.
  @param pCertificate             [in]  The certificate that should be validated. (DER encoded ByteString)
  @param pCertificateStore        [in]  The certificate store that validates the passed in certificate.

  @param pValidationCode          [out] The validation code, that gives information about the validation result.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnValidateCertificate)(  
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Int*                  pValidationCode); /* Validation return code. */

/** 
  @brief Validates a given X509 certificate object.
 
   Validation:
   - Subject/Issuer
   - Path
   - Certificate Revocation List (CRL)
   - Certificate Trust List (CTL)

  @param pPKI                     [in]  The pki handle.
  @param pCertificate             [in]  The certificate that should be validated.(DER encoded ByteString)
  @param pCertificateStore        [in]  The certificate store that validates the passed in certificate.

  @param pValidationCode          [out] The validation code, that gives information about the validation result.
*/
OpcUa_StatusCode OpcUa_PKIProvider_ValidateCertificate(
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Int*                  pValidationCode); /* Validation return code. */


/** 
  @brief Creates a certificate store object.

  @param pPKI                         [in]  The pki handle.
  
  @param ppCertificateStore           [out] The handle to the certificate store.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnOpenCertificateStore)(  
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void**                ppCertificateStore); /* type depends on store implementation */

/** 
  @brief Creates a certificate store object.

  @param pPKI                         [in]  The PKI handle.
  
  @param ppCertificateStore           [out] The handle to the certificate store.
*/
OpcUa_StatusCode OpcUa_PKIProvider_OpenCertificateStore(
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void**                ppCertificateStore); /* type depends on store implementation */

/** 
  @brief imports a given certificate into given certificate store.

  @param pPKI                     [in]  The pki handle.
  @param pCertificate             [in]  The certificate that should be imported.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.

  @param pCertificateIndex        [int/out] The index that indicates the store location of the certificate within the certificate store.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnSaveCertificate)(  
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Void*                 pSaveHandle);

/** 
  @brief imports a given certificate into given certificate store.
 
  @param pPKI                     [in]  The PKI handle.
  @param pCertificate             [in]  The certificate that should be imported.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.

  @param pCertificateIndex        [in/out] The index that indicates the store location of the certificate within the certificate store.
*/
OpcUa_StatusCode OpcUa_PKIProvider_SaveCertificate(
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Void*                 pSaveHandle);



/** 
  @brief imports a given certificate into given certificate store.
 
  @param pPKI                     [in]  The pki handle.
  @param pCertificate             [in]  The certificate that should be imported.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.

  @param pCertificateIndex        [out] The index that indicates the store location of the certificate within the certificate store.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnLoadCertificate)(  
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void*                 pLoadHandle,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_ByteString*           pCertificate);

/** 
  @brief imports a given certificate into given certificate store.
 
  @param pPKI                     [in]  The PKI handle.
  @param pCertificate             [in]  The certificate that should be imported.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.

  @param pCertificateIndex        [out] The index that indicates the store location of the certificate within the certificate store.
*/
OpcUa_StatusCode OpcUa_PKIProvider_LoadCertificate(
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void*                 pLoadHandle,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_ByteString*           pCertificate);


/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.

  @param pCertificateStore     [out] The certificate store object.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnCloseCertificateStore)(  
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void**                ppCertificateStore); /* type depends on store implementation */


/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.

  @param pCertificateStore     [out] The certificate store object.
*/
OpcUa_StatusCode OpcUa_PKIProvider_CloseCertificateStore(
    struct _OpcUa_PKIProvider*  pPKI,
    OpcUa_Void**                ppCertificateStore); /* type depends on store implementation */


/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.

  @param pCertificateStore     [out] The certificate store object.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnLoadPrivateKeyFromFile)(
    OpcUa_StringA               privateKeyFile,
    OpcUa_P_FileFormat          fileFormat,
    OpcUa_StringA               password,
    OpcUa_ByteString*           pPrivateKey);

/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.

  @param pCertificateStore     [out] The certificate store object.
*/
OpcUa_StatusCode OpcUa_PKIProvider_LoadPrivateKeyFromFile(
    OpcUa_StringA               privateKeyFile,
    OpcUa_P_FileFormat          fileFormat,
    OpcUa_StringA               password,
    OpcUa_ByteString*           pPrivateKey);

/**
  @brief Extracts data from a certificate store object.

  @param pCertificate          [in] The certificate to examine.
  @param pIssuer               [out, optional] The issuer name of the certificate.
  @param pSubject              [out, optional] The subject name of the certificate.
  @param pSubjectUri           [out, optional] The subject's URI of the certificate.
  @param pSubjectIP            [out, optional] The subject's IP of the certificate.
  @param pSubjectDNS           [out, optional] The subject's DNS name of the certificate.
  @param pCertThumbprint       [out, optional] The thumbprint of the certificate.
  @param pSubjectHash          [out, optional] The hash code of the certificate.
  @param pCertRawLength        [out, optional] The length of the DER encoded data.
                               can be smaller than the total length of pCertificate in case of chain certificate or garbage follow.
*/
typedef OpcUa_StatusCode (OpcUa_PKIProvider_PfnExtractCertificateData)(
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pIssuer,
    OpcUa_ByteString*           pSubject,
    OpcUa_ByteString*           pSubjectUri,
    OpcUa_ByteString*           pSubjectIP,
    OpcUa_ByteString*           pSubjectDNS,
    OpcUa_ByteString*           pCertThumbprint,
    OpcUa_UInt32*               pSubjectHash,
    OpcUa_UInt32*               pCertRawLength);

/**
  @brief Extracts data from a certificate store object.

  @param pCertificate          [in] The certificate to examine.
  @param pIssuer               [out, optional] The issuer name of the certificate.
  @param pSubject              [out, optional] The subject name of the certificate.
  @param pSubjectUri           [out, optional] The subject's URI of the certificate.
  @param pSubjectIP            [out, optional] The subject's IP of the certificate.
  @param pSubjectDNS           [out, optional] The subject's DNS name of the certificate.
  @param pCertThumbprint       [out, optional] The thumbprint of the certificate.
  @param pSubjectHash          [out, optional] The hash code of the certificate.
  @param pCertRawLength        [out, optional] The length of the DER encoded data.
                               can be smaller than the total length of pCertificate in case of chain certificate or garbage follow.
*/
OpcUa_StatusCode OpcUa_PKIProvider_ExtractCertificateData(
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pIssuer,
    OpcUa_ByteString*           pSubject,
    OpcUa_ByteString*           pSubjectUri,
    OpcUa_ByteString*           pSubjectIP,
    OpcUa_ByteString*           pSubjectDNS,
    OpcUa_ByteString*           pCertThumbprint,
    OpcUa_UInt32*               pSubjectHash,
    OpcUa_UInt32*               pCertRawLength);

typedef struct _OpcUa_PKIProvider
{
    OpcUa_Handle                                 Handle; /* Certificate Store */
    OpcUa_PKIProvider_PfnValidateCertificate*    ValidateCertificate;
    OpcUa_PKIProvider_PfnLoadPrivateKeyFromFile* LoadPrivateKeyFromFile;
    OpcUa_PKIProvider_PfnOpenCertificateStore*   OpenCertificateStore;
    OpcUa_PKIProvider_PfnSaveCertificate*        SaveCertificate;
    OpcUa_PKIProvider_PfnLoadCertificate*        LoadCertificate;
    OpcUa_PKIProvider_PfnCloseCertificateStore*  CloseCertificateStore;
    OpcUa_PKIProvider_PfnExtractCertificateData* ExtractCertificateData;
}
OpcUa_PKIProvider;

// Add pki provider services:
OpcUa_StatusCode OpcUa_PKIProvider_Create(   OpcUa_Void*         pCertificateStoreConfig,
                                             OpcUa_PKIProvider*  pProvider);

OpcUa_StatusCode OpcUa_PKIProvider_Delete(   OpcUa_PKIProvider*  pProvider);

OPCUA_END_EXTERN_C

#endif
