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

/* own */
#include "opcua_pki.h"

#include <string.h>

#include "key_manager.h"
#include "sopc_base_types.h"


OpcUa_StatusCode OpcUa_PKIProvider_Create(    OpcUa_Void*         a_pCertificateStoreConfig,
                                              OpcUa_PKIProvider*  a_pProvider)
{
    a_pProvider->Handle = a_pCertificateStoreConfig;
    a_pProvider->CloseCertificateStore = OpcUa_PKIProvider_CloseCertificateStore;
    a_pProvider->ExtractCertificateData = OpcUa_PKIProvider_ExtractCertificateData;
    a_pProvider->LoadCertificate = OpcUa_PKIProvider_LoadCertificate;
    a_pProvider->LoadPrivateKeyFromFile = OpcUa_PKIProvider_LoadPrivateKeyFromFile;
    a_pProvider->OpenCertificateStore = OpcUa_PKIProvider_OpenCertificateStore;
    a_pProvider->SaveCertificate = OpcUa_PKIProvider_SaveCertificate;
    a_pProvider->ValidateCertificate = OpcUa_PKIProvider_ValidateCertificate;
    return STATUS_OK;
}

OpcUa_StatusCode OpcUa_PKIProvider_Delete(    OpcUa_PKIProvider*  a_pProvider)
{
    memset(a_pProvider, 0, sizeof(OpcUa_PKIProvider));
    return STATUS_OK;
}

/*============================================================================
 * OpcUa_PKIProvider_ValidateCertificate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_PKIProvider_ValidateCertificate(
    struct _OpcUa_PKIProvider*  a_pPKI,
    OpcUa_ByteString*           a_pCertificate,
    OpcUa_Void*                 a_pCertificateStore,
    OpcUa_Int*                  a_pValidationCode) /* Validation return codes from OpenSSL */
{
    (void) a_pPKI;
    (void) a_pCertificate;
    (void) a_pCertificateStore;
    (void) a_pValidationCode;
    return STATUS_NOK;
}

/*============================================================================
 * OpcUa_PKIProvider_OpenCertificateStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_PKIProvider_OpenCertificateStore(
    struct _OpcUa_PKIProvider*  a_pPKI,
    OpcUa_Void**                a_ppCertificateStore)        /* type depends on store implementation */
{
    (void) a_pPKI;
    (void) a_ppCertificateStore;
    return STATUS_OK;
}

/*============================================================================
 * OpcUa_PKIProvider_SaveCertificate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_PKIProvider_LoadCertificate(
    struct _OpcUa_PKIProvider*  a_pPKI,
    OpcUa_Void*                 a_pLoadHandle,
    OpcUa_Void*                 a_pCertificateStore,
    OpcUa_ByteString*           a_pCertificate)
{
    (void) a_pPKI;
    (void) a_pCertificateStore;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Certificate *cert = NULL;
    if(a_pLoadHandle != NULL && a_pCertificate != NULL){
        status = KeyManager_Certificate_CreateFromFile(a_pLoadHandle, &cert);
    }
    if(STATUS_OK == status){
        uint32_t lenAllocated = 0;
        status = KeyManager_Certificate_CopyDER(cert, &a_pCertificate->Data, &lenAllocated);
        if(lenAllocated > INT32_MAX){
            SOPC_ByteString_Clear(a_pCertificate);
        }else{
            a_pCertificate->Length = (int32_t) lenAllocated;
        }
    }
    KeyManager_Certificate_Free(cert);
    return status;
}

/*============================================================================
 * OpcUa_PKIProvider_SaveCertificate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_PKIProvider_SaveCertificate(
    struct _OpcUa_PKIProvider*  a_pPKI,
    OpcUa_ByteString*           a_pCertificate,
    OpcUa_Void*                 a_pCertificateStore,
    OpcUa_Void*                 a_pSaveHandle)
{
    (void) a_pPKI;
    (void) a_pCertificate;
    (void) a_pCertificateStore;
    (void) a_pSaveHandle;
    return STATUS_NOK;
}
/*============================================================================
 * OpcUa_PKIProvider_CloseCertificateStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_PKIProvider_CloseCertificateStore(
    struct _OpcUa_PKIProvider*   a_pPKI,
    OpcUa_Void**                 a_ppCertificateStore) /* type depends on store implementation */
{
    (void) a_pPKI;
    (void) a_ppCertificateStore;

    return STATUS_OK;
}

OpcUa_StatusCode OpcUa_PKIProvider_LoadPrivateKeyFromFile(
    OpcUa_StringA               privateKeyFile,
    OpcUa_P_FileFormat          fileFormat,
    OpcUa_StringA               password,
    OpcUa_ByteString*           pPrivateKey){
    (void) password;
    AsymmetricKey* key;
    if(fileFormat != OpcUa_Crypto_Encoding_PEM)
        return STATUS_NOK;
    if (STATUS_OK == KeyManager_AsymmetricKey_CreateFromFile(privateKeyFile, &key, password, (NULL == password) ? 0:strlen(password))){
        pPrivateKey->Data = (uint8_t*) key;
        pPrivateKey->Length = 1;
        return STATUS_OK;
    }
    return STATUS_NOK;
}

OpcUa_StatusCode OpcUa_PKIProvider_ExtractCertificateData(
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pIssuer,
    OpcUa_ByteString*           pSubject,
    OpcUa_ByteString*           pSubjectUri,
    OpcUa_ByteString*           pSubjectIP,
    OpcUa_ByteString*           pSubjectDNS,
    OpcUa_ByteString*           pCertThumbprint,
    OpcUa_UInt32*               pSubjectHash,
    OpcUa_UInt32*               pCertRawLength){
    (void) pCertificate;
    (void) pIssuer;
    (void) pSubject;
    (void) pSubjectUri;
    (void) pSubjectIP;
    (void) pSubjectDNS;
    (void) pCertThumbprint;
    (void) pSubjectHash;
    (void) pCertRawLength;
    return STATUS_NOK;
}
