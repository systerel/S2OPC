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

#include <check.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "opcua_identifiers.h"
#include "push_client_connection_helper.h"
#include "push_server_methods.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack_lib_itf.h"

/*---------------------------------------------------------------------------
 *                                  Node ids
 *---------------------------------------------------------------------------*/

// TODO: the two nodeIds gServerDefaultApplicationGroupId and gRsaSha256ApplicationCertificateTypeId
// are defined statically both here and in push_server_method.c. Maybe find an alternative?
static SOPC_NodeId gServerDefaultApplicationGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};

static SOPC_NodeId gGroupIdInvalid = {.IdentifierType = SOPC_IdentifierType_Numeric, .Namespace = 0, .Data.Numeric = 0};
static SOPC_NodeId gGroupIdEmpty = {0};

static SOPC_NodeId gRsaSha256ApplicationCertificateTypeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_RsaSha256ApplicationCertificateType};
static SOPC_NodeId gCertificateTypeIdInvalid = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                .Namespace = 0,
                                                .Data.Numeric = 0};

/*---------------------------------------------------------------------------
 *                    Handle disconnection of the client
 *---------------------------------------------------------------------------*/

// Period used to check for catch signal
#define SIG_UPDATE_TIMEOUT_MS 500

// Flag used on sig stop
static bool stop = false;

static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    stop = true;
}

/*---------------------------------------------------------------------------
 *                     Static truslist helpers functions
 *---------------------------------------------------------------------------*/

// TODO: Some of these functions are used by both the push_server and this file (push_client.c).
// Maybe put them in sopc_trustlist_helper.h in the push_server folder of samples/.
static SOPC_ReturnStatus trustList_write_bs_array_to_cert_list(SOPC_ByteString* pArray,
                                                               uint32_t length,
                                                               SOPC_CertificateList** ppCert)
{
    if (NULL == pArray || 0 == length || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pCert = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    uint32_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        if (NULL == pBsCert)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCert);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCert);
        pCert = NULL;
    }
    *ppCert = pCert;
    return status;
}

static SOPC_ReturnStatus trustList_write_bs_array_to_crl_list(SOPC_ByteString* pArray,
                                                              uint32_t length,
                                                              SOPC_CRLList** ppCrl)
{
    if (NULL == pArray || 0 == length || NULL == ppCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CRLList* pCrl = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pBsCert = NULL;
    uint32_t idx = 0;

    for (idx = 0; idx < length && SOPC_STATUS_OK == status; idx++)
    {
        pBsCert = &pArray[idx];
        if (NULL == pBsCert)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &pCrl);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCrl);
        pCrl = NULL;
    }
    *ppCrl = pCrl;
    return status;
}

static SOPC_ReturnStatus trustlist_attach_certs_to_raw_arrays(const SOPC_CertificateList* pTrustedCerts,
                                                              SOPC_SerializedCertificate** pRawTrustedCertArray,
                                                              uint32_t* pLenTrustedCertArray,
                                                              const SOPC_CRLList* pTrustedCrls,
                                                              SOPC_SerializedCRL** pRawTrustedCrlArray,
                                                              uint32_t* pLenTrustedCrlArray,
                                                              const SOPC_CertificateList* pIssuerCerts,
                                                              SOPC_SerializedCertificate** pRawIssuerCertArray,
                                                              uint32_t* pLenIssuerCertArray,
                                                              const SOPC_CRLList* pIssuerCrls,
                                                              SOPC_SerializedCRL** pRawIssuerCrlArray,
                                                              uint32_t* pLenIssuerCrlArray)
{
    SOPC_ASSERT(NULL != pRawTrustedCertArray);
    SOPC_ASSERT(NULL != pRawTrustedCrlArray);
    SOPC_ASSERT(NULL != pRawIssuerCertArray);
    SOPC_ASSERT(NULL != pRawIssuerCrlArray);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Initialize the lengths */
    *pLenTrustedCertArray = 0;
    *pLenTrustedCrlArray = 0;
    *pLenIssuerCertArray = 0;
    *pLenIssuerCrlArray = 0;

    status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pTrustedCerts, pRawTrustedCertArray,
                                                                     pLenTrustedCertArray);
    status = SOPC_KeyManager_CRLList_AttachToSerializedArray(pTrustedCrls, pRawTrustedCrlArray, pLenTrustedCrlArray);
    status =
        SOPC_KeyManager_CertificateList_AttachToSerializedArray(pIssuerCerts, pRawIssuerCertArray, pLenIssuerCertArray);
    status = SOPC_KeyManager_CRLList_AttachToSerializedArray(pIssuerCrls, pRawIssuerCrlArray, pLenIssuerCrlArray);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(*pRawTrustedCertArray);
        SOPC_Free(*pRawTrustedCrlArray);
        SOPC_Free(*pRawIssuerCertArray);
        SOPC_Free(*pRawIssuerCrlArray);
        *pRawTrustedCertArray = NULL;
        *pRawTrustedCrlArray = NULL;
        *pRawIssuerCertArray = NULL;
        *pRawIssuerCrlArray = NULL;
        *pLenTrustedCertArray = 0;
        *pLenTrustedCrlArray = 0;
        *pLenIssuerCertArray = 0;
        *pLenIssuerCrlArray = 0;
    }

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_array_to_bs_array(const void* pGenArray,
                                                                uint32_t lenArray,
                                                                SOPC_ByteString** pByteStringArray,
                                                                bool bIsCRL)
{
    if (NULL == pGenArray || 0 == lenArray || NULL == pByteStringArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ByteString* pBsArray = NULL;
    const SOPC_SerializedCertificate* pRawCertArray = NULL;
    const SOPC_SerializedCRL* pRawCrlArray = NULL;
    const SOPC_Buffer* pRawBuffer = NULL;
    SOPC_ByteString* pByteString = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (bIsCRL)
    {
        pRawCrlArray = (const SOPC_SerializedCRL*) pGenArray;
    }
    else
    {
        pRawCertArray = (const SOPC_SerializedCertificate*) pGenArray;
    }
    pBsArray = SOPC_Calloc(lenArray, sizeof(SOPC_ByteString));
    if (NULL == pBsArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < lenArray && SOPC_STATUS_OK == status; i++)
    {
        if (bIsCRL)
        {
            pRawBuffer = SOPC_KeyManager_SerializedCRL_Data(&pRawCrlArray[i]);
        }
        else
        {
            pRawBuffer = SOPC_KeyManager_SerializedCertificate_Data(&pRawCertArray[i]);
        }
        if (NULL == pRawBuffer)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* Check before casting */
        if (SOPC_STATUS_OK == status)
        {
            if (INT32_MAX < pRawBuffer->length)
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            pByteString = &pBsArray[i];
            SOPC_ByteString_Initialize(pByteString);
            pByteString->Data = pRawBuffer->data; // Attach data
            pByteString->Length = (int32_t) pRawBuffer->length;
            pByteString->DoNotClear = true;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pBsArray);
        pBsArray = NULL;
    }

    *pByteStringArray = pBsArray;

    return status;
}

static SOPC_ReturnStatus trustlist_attach_raw_arrays_to_bs_arrays(
    const SOPC_SerializedCertificate* pRawTrustedCertArray,
    const uint32_t lenTrustedCertArray,
    SOPC_ByteString** pBsTrustedCertArray,
    const SOPC_SerializedCRL* pRawTrustedCrlArray,
    const uint32_t lenTrustedCrlArray,
    SOPC_ByteString** pBsTrustedCrlArray,
    const SOPC_SerializedCertificate* pRawIssuerCertArray,
    const uint32_t lenIssuerCertArray,
    SOPC_ByteString** pBsIssuerCertArray,
    const SOPC_SerializedCRL* pRawIssuerCrlArray,
    const uint32_t lenIssuerCrlArray,
    SOPC_ByteString** pBsIssuerCrlArray)
{
    if ((NULL == pRawTrustedCertArray && 0 < lenTrustedCertArray) ||
        (NULL == pRawTrustedCrlArray && 0 < lenTrustedCrlArray) ||
        (NULL == pRawIssuerCertArray && 0 < lenIssuerCertArray) ||
        (NULL == pRawIssuerCrlArray && 0 < lenIssuerCrlArray))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pBsTrustedCertArray || NULL == pBsTrustedCrlArray || NULL == pBsIssuerCertArray ||
        NULL == pBsIssuerCrlArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 < lenTrustedCertArray)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawTrustedCertArray, lenTrustedCertArray, pBsTrustedCertArray,
                                                        false);
    }
    if (0 < lenTrustedCrlArray && SOPC_STATUS_OK == status)
    {
        status =
            trustlist_attach_raw_array_to_bs_array(pRawTrustedCrlArray, lenTrustedCrlArray, pBsTrustedCrlArray, true);
    }
    if (0 < lenIssuerCertArray && SOPC_STATUS_OK == status)
    {
        status =
            trustlist_attach_raw_array_to_bs_array(pRawIssuerCertArray, lenIssuerCertArray, pBsIssuerCertArray, false);
    }
    if (0 < lenIssuerCrlArray && SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_array_to_bs_array(pRawIssuerCrlArray, lenIssuerCrlArray, pBsIssuerCrlArray, true);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(*pBsTrustedCertArray);
        SOPC_Free(*pBsTrustedCrlArray);
        SOPC_Free(*pBsIssuerCertArray);
        SOPC_Free(*pBsIssuerCrlArray);
        *pBsTrustedCertArray = NULL;
        *pBsTrustedCrlArray = NULL;
        *pBsIssuerCertArray = NULL;
        *pBsIssuerCrlArray = NULL;
    }

    return status;
}

static SOPC_ReturnStatus cert_file_to_bs(const char* cert_path, SOPC_ByteString** out_bs_array)
{
    SOPC_CertificateList* cert = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(cert_path, &cert);

    if (SOPC_STATUS_OK == status)
    {
        uint8_t* der_cert = NULL;
        uint32_t der_len = 0;
        status = SOPC_KeyManager_Certificate_ToDER(cert, &der_cert, &der_len);
        if (SOPC_STATUS_OK == status)
        {
            (*out_bs_array)->Data = der_cert;
            (*out_bs_array)->Length = (int32_t) der_len;
        }
    }

    SOPC_KeyManager_Certificate_Free(cert);
    return status;
}

// For the moment the SOPC_CRLList needs to be handled out of the function
// (unlike its equivalent cert_file_to_bs() function).
static SOPC_ReturnStatus crl_to_bs(SOPC_CRLList* pCRL, SOPC_ByteString** ppOutBS)
{
    SOPC_ByteString* pBsArray = NULL;
    SOPC_ByteString* pByteString = NULL;
    SOPC_SerializedCRL* pRawTrustedCrlArray = NULL;
    uint32_t nbTrustedCrls = 0;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_CRLList_AttachToSerializedArray(pCRL, &pRawTrustedCrlArray, &nbTrustedCrls);

    const SOPC_Buffer* pRawBuffer = NULL;

    pBsArray = SOPC_Calloc(nbTrustedCrls, sizeof(SOPC_ByteString));
    if (NULL == pBsArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < nbTrustedCrls && SOPC_STATUS_OK == status; i++)
    {
        pRawBuffer = SOPC_KeyManager_SerializedCRL_Data(&pRawTrustedCrlArray[i]);

        if (NULL == pRawBuffer)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* Check before casting */
        if (SOPC_STATUS_OK == status)
        {
            if (INT32_MAX < pRawBuffer->length)
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            pByteString = &pBsArray[i];
            SOPC_ByteString_Initialize(pByteString);
            pByteString->Data = pRawBuffer->data; // Attach data
            pByteString->Length = (int32_t) pRawBuffer->length;
            pByteString->DoNotClear = true;
        }
    }

    SOPC_Free(pRawTrustedCrlArray);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pBsArray);
        pBsArray = NULL;
    }

    *ppOutBS = pBsArray;

    return status;
}

/*---------------------------------------------------------------------------
 *                                Main
 *---------------------------------------------------------------------------*/

bool bUpdateCertificateSucceeded = false;

/* Main function that runs a client. Return values:
 * - 0 on success (connection ok, interaction with the server ok) ;
 * - 1 on failure (anywhere) ;
 * - 2 on unexpected disconnection ;
 * - 3 on success at UpdateCertificate followed by disconnection.
 * The first two options of the main are reserved for cient cert path and key path.
 */
int main(int argc, char* const argv[])
{
    /* 1) Connect to the server */
    int ret = EXIT_FAILURE;
    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_SecureConnection_Config* scConfig = NULL;
    // Initialize client
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    char* server_certificate = "server_public/server_4k_cert.der";
    // The server certificate can be precised (optionnal)
    if (3 < argc)
    {
        if (0 == strcmp(argv[3], "server_certificate"))
        {
            server_certificate = argv[4];
        }
    }

    // Create connection and connect the user/client to the server
    status = create_custom_secure_connection(argv[1], argv[2], true, "./S2OPC_Demo_PKI_Client", server_certificate,
                                             &scConfig);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_Connect(scConfig, SOPC_Client_ConnEventCb, &secureConnection);
    }

    /* 2) If connected, interact with the server */
    if (SOPC_STATUS_OK == status && NULL != secureConnection)
    {
        ret = EXIT_SUCCESS;
        printf("Connected.\n");
        // While no error, parse the options one by one.
        for (int i = 3; i < argc && EXIT_SUCCESS == ret; i++)
        {
            // OPTION: add. Must be followed by the path of the certificate to add.
            // Only trusted certificates can be added with this method, and certificate that does not need a CRL (ie not
            // CA or with pathLen 0).
            if (0 == strcmp(argv[i], "add"))
            {
                i++;
                // Create the ByteString certificate
                SOPC_ByteString* certificate_to_add = SOPC_ByteString_Create();
                status = cert_file_to_bs(argv[i], &certificate_to_add);
                // Add it to the server TL.
                status = SOPC_TEST_TrustList_AddCertificate(secureConnection, certificate_to_add, true);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // Clear
                SOPC_ByteString_Delete(certificate_to_add);
            }

            // OPTION: write. Must be followed by the certificate path, its crl if it's a CA, and eventually trusted or
            // untrusted.
            if (0 == strcmp(argv[i], "write"))
            {
                i++;
                // Create the ByteString certificate
                SOPC_ByteString* certificate_to_add = SOPC_ByteString_Create();
                status = cert_file_to_bs(argv[i], &certificate_to_add);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                i++;
                // Create the ByteString crl
                SOPC_ByteString* crl_to_add = NULL;
                SOPC_CRLList* pCRL = NULL;
                status = SOPC_KeyManager_CRL_CreateOrAddFromFile(argv[i], &pCRL);
                status = crl_to_bs(pCRL, &crl_to_add);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                i++;
                OpcUa_TrustListDataType pTrustList = {0};
                SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustList);
                if (0 == strcmp(argv[i], "untrusted"))
                {
                    // Write the new issuer certificate and crl
                    pTrustList.SpecifiedLists =
                        OpcUa_TrustListMasks_IssuerCertificates | OpcUa_TrustListMasks_IssuerCrls;
                    pTrustList.NoOfIssuerCertificates = 1;
                    pTrustList.NoOfIssuerCrls = 1;
                    pTrustList.IssuerCrls = crl_to_add;
                    pTrustList.IssuerCertificates = certificate_to_add;
                }
                else // Trusted
                {
                    // Write the new trusted certificate and crl
                    pTrustList.SpecifiedLists =
                        OpcUa_TrustListMasks_TrustedCertificates | OpcUa_TrustListMasks_TrustedCrls;
                    pTrustList.NoOfTrustedCertificates = 1;
                    pTrustList.NoOfTrustedCrls = 1;
                    pTrustList.TrustedCrls = crl_to_add;
                    pTrustList.TrustedCertificates = certificate_to_add;
                }

                uint32_t fileHandle = 0;
                status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustList);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // Clear
                SOPC_KeyManager_CRL_Free(pCRL);
                OpcUa_TrustListDataType_Clear(&pTrustList);
            }

            // OPTION: write_remove. This option must be followed by the certificate thumbprint to remove, and
            // trusted or untrusted.
            // NOTE: with the actual push_server implementation we need to create a full new TL to do this.
            if (0 == strcmp(argv[i], "write_remove"))
            {
                // 1) Retrieve the TL or the server.
                uint32_t fileHandle = 0;
                OpcUa_TrustListDataType* pTrustList = NULL;
                status = SOPC_TEST_TrustList_OpenWithMasks(secureConnection, OpcUa_TrustListMasks_All, &fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_Read(secureConnection, fileHandle, &pTrustList);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_Close(secureConnection, fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // 2) Create the CertificateList/CRL from old TL and remove the item we want to remove.
                SOPC_CertificateList* pTrustedCerts = NULL;
                SOPC_CertificateList* pIssuerCerts = NULL;
                SOPC_CRLList* pTrustedCrls = NULL;
                SOPC_CRLList* pIssuerCrls = NULL;
                status = trustList_write_bs_array_to_cert_list(
                    pTrustList->TrustedCertificates, (uint32_t) pTrustList->NoOfTrustedCertificates, &pTrustedCerts);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = trustList_write_bs_array_to_cert_list(
                    pTrustList->IssuerCertificates, (uint32_t) pTrustList->NoOfIssuerCertificates, &pIssuerCerts);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = trustList_write_bs_array_to_crl_list(pTrustList->IssuerCrls,
                                                              (uint32_t) pTrustList->NoOfIssuerCrls, &pIssuerCrls);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = trustList_write_bs_array_to_crl_list(pTrustList->TrustedCrls,
                                                              (uint32_t) pTrustList->NoOfTrustedCrls, &pTrustedCrls);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                bool bCertIsRemoved = false;
                bool bIsIssuer = false;
                i++;
                const char* thumbprint = argv[i];
                i++;
                if (0 == strcmp(argv[i], "untrusted"))
                {
                    status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(&pIssuerCerts, &pIssuerCrls, thumbprint,
                                                                                &bCertIsRemoved, &bIsIssuer);
                }
                else
                {
                    status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(
                        &pTrustedCerts, &pTrustedCrls, thumbprint, &bCertIsRemoved, &bIsIssuer);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // 3) Make a new TL with the modified CertificateList/CRL
                OpcUa_TrustListDataType pTrustListNew = {0};
                SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustListNew);
                SOPC_SerializedCertificate* pRawTrustedCertArray = NULL;
                SOPC_SerializedCertificate* pRawIssuerCertArray = NULL;
                SOPC_SerializedCRL* pRawTrustedCrlArray = NULL;
                SOPC_SerializedCRL* pRawIssuerCrlArray = NULL;
                uint32_t nbTrustedCerts = 0;
                uint32_t nbTrustedCrls = 0;
                uint32_t nbIssuerCerts = 0;
                uint32_t nbIssuerCrls = 0;
                SOPC_ByteString* pBsTrustedCertArray = NULL;
                SOPC_ByteString* pBsTrustedCrlArray = NULL;
                SOPC_ByteString* pBsIssuerCertArray = NULL;
                SOPC_ByteString* pBsIssuerCrlArray = NULL;
                status = trustlist_attach_certs_to_raw_arrays(pTrustedCerts, &pRawTrustedCertArray, &nbTrustedCerts,
                                                              pTrustedCrls, &pRawTrustedCrlArray, &nbTrustedCrls,
                                                              pIssuerCerts, &pRawIssuerCertArray, &nbIssuerCerts,
                                                              pIssuerCrls, &pRawIssuerCrlArray, &nbIssuerCrls);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = trustlist_attach_raw_arrays_to_bs_arrays(
                    pRawTrustedCertArray, nbTrustedCerts, &pBsTrustedCertArray, pRawTrustedCrlArray, nbTrustedCrls,
                    &pBsTrustedCrlArray, pRawIssuerCertArray, nbIssuerCerts, &pBsIssuerCertArray, pRawIssuerCrlArray,
                    nbIssuerCrls, &pBsIssuerCrlArray);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                pTrustListNew.SpecifiedLists = OpcUa_TrustListMasks_All;
                pTrustListNew.TrustedCertificates = pBsTrustedCertArray;
                pTrustListNew.NoOfTrustedCertificates = (int32_t) nbTrustedCerts;
                pTrustListNew.TrustedCrls = pBsTrustedCrlArray;
                pTrustListNew.NoOfTrustedCrls = (int32_t) nbTrustedCrls;
                pTrustListNew.IssuerCertificates = pBsIssuerCertArray;
                pTrustListNew.NoOfIssuerCertificates = (int32_t) nbIssuerCerts;
                pTrustListNew.IssuerCrls = pBsIssuerCrlArray;
                pTrustListNew.NoOfIssuerCrls = (int32_t) nbIssuerCrls;

                // 4) Write this new TL in the server
                status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustListNew);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // Clear
                SOPC_KeyManager_Certificate_Free(pTrustedCerts);
                SOPC_KeyManager_Certificate_Free(pIssuerCerts);
                SOPC_KeyManager_CRL_Free(pTrustedCrls);
                SOPC_KeyManager_CRL_Free(pIssuerCrls);
                SOPC_Free(pRawTrustedCertArray);
                SOPC_Free(pRawTrustedCrlArray);
                SOPC_Free(pRawIssuerCertArray);
                SOPC_Free(pRawIssuerCrlArray);
                SOPC_Free(pBsTrustedCertArray);
                SOPC_Free(pBsTrustedCrlArray);
                SOPC_Free(pBsIssuerCertArray);
                SOPC_Free(pBsIssuerCrlArray);
                OpcUa_TrustListDataType_Clear(pTrustList);
                SOPC_Free(pTrustList);
            }

            // OPTION: remove. Must be followed by the string thumbprint of the certificate to remove, and trusted or
            // untrusted.
            if (0 == strcmp(argv[i], "remove"))
            {
                i++;
                SOPC_String thumbprint = {0};
                status = SOPC_String_InitializeFromCString(&thumbprint, argv[i]);
                i++;
                bool bIsTrustedCert = true;
                if (0 == strcmp(argv[i], "untrusted"))
                {
                    bIsTrustedCert = false;
                }
                status = SOPC_TEST_TrustList_RemoveCertificate(secureConnection, thumbprint, bIsTrustedCert);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: revoke. No argument needs to be provided. It adds a new CRL with a revoked cert in it
            // to the trusted CRLs of the server.
            if (0 == strcmp(argv[i], "revoke"))
            {
                // 1) Create the TL with the new CRL
                OpcUa_TrustListDataType pTrustList = {0};
                SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustList);
                SOPC_ByteString* crl_to_add = NULL;
                SOPC_CRLList* pCRL = NULL;
                status = SOPC_KeyManager_CRL_CreateOrAddFromFile("push_data/cacrl_with_revoked_cert.der", &pCRL);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = crl_to_bs(pCRL, &crl_to_add);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                pTrustList.SpecifiedLists = OpcUa_TrustListMasks_TrustedCrls;
                pTrustList.TrustedCrls = crl_to_add;
                pTrustList.NoOfTrustedCrls = 1;

                // 2) Update the server TL
                uint32_t fileHandle = 0;
                status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustList);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                // Clear
                SOPC_KeyManager_CRL_Free(pCRL);
                OpcUa_TrustListDataType_Clear(&pTrustList);
            }

            // OPTION: sleep. The client sleeps as long as it did not receive a sig stop.
            if (0 == strcmp(argv[i], "sleep"))
            {
                while (!stop)
                {
                    SOPC_Sleep(SIG_UPDATE_TIMEOUT_MS);
                }
            }

            /* OPTION: csr. Must be followed by 4 arguments:
               - groupId: groupIdValid, groupIdInvalid, or groupIdEmpty
               - certificateTypeId: certificateTypeIdValid or certificateTypeIdInvalid
               - newKey: newkey or noNewKey
               - nonce: nonce or noNonce
             */
            if (0 == strcmp(argv[i], "csr"))
            {
                SOPC_ByteString* csr = NULL;
                SOPC_String* subjectName = SOPC_String_Create();
                OpcUa_CallResponse* resp = NULL;

                // groupId
                i++;
                SOPC_NodeId* groupId = &gServerDefaultApplicationGroupId;
                if (0 == strcmp(argv[i], "groupIdInvalid"))
                {
                    groupId = &gGroupIdInvalid;
                }
                else if (0 == strcmp(argv[i], "groupIdEmpty"))
                {
                    groupId = &gGroupIdEmpty;
                }

                // certificateTypeId
                i++;
                SOPC_NodeId* certificateTypeId = &gRsaSha256ApplicationCertificateTypeId;
                if (0 == strcmp(argv[i], "certificateTypeIdInvalid"))
                {
                    certificateTypeId = &gCertificateTypeIdInvalid;
                }

                // New key ?
                i++;
                bool newKey = false;
                if (0 == strcmp(argv[i], "newKey"))
                {
                    newKey = true;
                }

                // Nonce
                i++;
                SOPC_ByteString* nonce = SOPC_ByteString_Create();
                if (0 == strcmp(argv[i], "nonce"))
                {
                    // Generate at least 32 bytes of random and put it in the ByteString nonce
                    SOPC_ExposedBuffer* pBuff = NULL;
                    SOPC_CryptoProvider* pCrypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
                    status = SOPC_CryptoProvider_GenerateRandomBytes(pCrypto, 32, &pBuff);
                    if (SOPC_STATUS_OK != status)
                    {
                        ret = EXIT_FAILURE;
                    }
                    SOPC_CryptoProvider_Free(pCrypto);
                    nonce->Data = pBuff;
                    nonce->Length = 32;
                }

                status = SOPC_TEST_ServerConfiguration_CreateSigningRequest_GetResponse(
                    secureConnection, groupId, certificateTypeId, subjectName, newKey, nonce, &resp);
                if (!SOPC_IsGoodStatus(resp->Results[0].StatusCode))
                {
                    ret = EXIT_FAILURE;
                }
                else
                {
                    printf("Response of CSR ok. Filling the CSR...\n");
                    status = SOPC_TEST_ServerConfiguration_CreateSigningRequest_FromResponse(resp, &csr);
                    if (SOPC_STATUS_OK != status)
                    {
                        ret = EXIT_FAILURE;
                    }
                    // Write the csr der in a file.
#if SOPC_HAS_FILESYSTEM
                    FILE* fp = fopen("push_data/input_csr.der", "wb+");
                    size_t nb_written = fwrite(csr->Data, 1, (size_t) csr->Length, fp);
                    if (0 == nb_written)
                    {
                        ret = EXIT_FAILURE;
                    }
                    fclose(fp);
#endif
                    // Alternative way was using MbedTLS on the CSR directly.
                }

                // Free the response and the csr
                SOPC_ReturnStatus localStatus =
                    SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
                SOPC_UNUSED_RESULT(localStatus);
                SOPC_ByteString_Delete(csr);
                SOPC_ByteString_Delete(nonce);
                SOPC_String_Delete(subjectName);
            }

            // OPTION: getRejectedList. No argument needs to be provided for this option. It creates the
            // folder "rejected", if the rejectedList of the server is not empty.
            if (0 == strcmp(argv[i], "getRejectedList"))
            {
                status = SOPC_TEST_ServerConfiguration_GetRejectedList(secureConnection);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: updateCertificate. Updates server certificate. Must be followed by the path of the certificate,
            // the number of issuers (0 if not), and eventually the paths of the issuer certificates.
            if (0 == strcmp(argv[i], "updateCertificate"))
            {
                i++;
                SOPC_ByteString* bs_server_cert = SOPC_ByteString_Create();
                status = cert_file_to_bs(argv[i], &bs_server_cert);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }

                i++;
                int nbOfIssuers = atoi(argv[i]);
                SOPC_ByteString* pBsIssuerCertArray = NULL;
                SOPC_CertificateList* pIssuerCerts = NULL;
                SOPC_SerializedCertificate* pRawIssuerCertArray = NULL;
                // If issuers are provided
                if (0 < nbOfIssuers)
                {
                    for (int j = 0; j < nbOfIssuers; j++)
                    {
                        i++;
                        status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(argv[i], &pIssuerCerts);
                    }
                    uint32_t nbOfIssuersParsed = 0;
                    status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pIssuerCerts, &pRawIssuerCertArray,
                                                                                     &nbOfIssuersParsed);
                    if (SOPC_STATUS_OK != status)
                    {
                        ret = EXIT_FAILURE;
                    }
                    status = trustlist_attach_raw_array_to_bs_array(pRawIssuerCertArray, nbOfIssuersParsed,
                                                                    &pBsIssuerCertArray, false);
                    if (SOPC_STATUS_OK != status)
                    {
                        ret = EXIT_FAILURE;
                    }
                }

                status = SOPC_TEST_ServerConfiguration_UpdateCertificate(secureConnection, bs_server_cert,
                                                                         pBsIssuerCertArray, nbOfIssuers);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                else
                {
                    bUpdateCertificateSucceeded = true;
                }

                // Clear
                SOPC_KeyManager_Certificate_Free(pIssuerCerts);
                SOPC_Free(pRawIssuerCertArray);
                SOPC_ByteString_Delete(bs_server_cert);
                SOPC_ByteString_Delete(pBsIssuerCertArray);
            }
        }
    }

    /* 3) Disconnect then clear the client config */
    status = SOPC_ClientHelperNew_Disconnect(&secureConnection);
    if (SOPC_STATUS_OK != status)
    {
        ret = EXIT_FAILURE;
    }
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (stop) // If the client received a sig stop unexpected event
    {
        if (bUpdateCertificateSucceeded)
        {
            ret = 3;
        }
        else
        {
            ret = 2;
        }
    }
    return ret;
}
