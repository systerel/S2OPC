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
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "push_client_connection_helper.h"
#include "sopc_assert.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_trustlist_internal_helper.h" // internal include

/*---------------------------------------------------------------------------
 *                                  Node ids
 *---------------------------------------------------------------------------*/

static const SOPC_NodeId gGroupIdInvalid = {0};
static const SOPC_NodeId gGroupIdEmpty = {0};
static const SOPC_NodeId gCertificateTypeIdInvalid = {0};

/*---------------------------------------------------------------------------
 *                    Handle disconnection of the client
 *---------------------------------------------------------------------------*/

// Period used to check for catch signal
#define SIG_UPDATE_TIMEOUT_MS 500

// Flag used on sig stop
static bool connectionEventReceived = false;

static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    connectionEventReceived = true;
}

/*---------------------------------------------------------------------------
 *                     Internal Truslist helpers functions
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus cert_file_to_bs(const char* cert_path, SOPC_ByteString** out_bs_array)
{
    SOPC_ASSERT(NULL != out_bs_array);
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

// For convenience, the SOPC_CRLList needs to be handled out of the function and freed after the operation
static SOPC_ReturnStatus crl_to_bs(SOPC_CRLList* pCRL, SOPC_ByteString** out_bs_array)
{
    SOPC_ASSERT(NULL != out_bs_array && NULL == *out_bs_array);

    SOPC_ByteString* pBsArray = NULL;
    const SOPC_Buffer* pRawBuffer = NULL;
    SOPC_SerializedCRL* pRawTrustedCrlArray = NULL;
    uint32_t nbTrustedCrls = 0;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_CRLList_AttachToSerializedArray(pCRL, &pRawTrustedCrlArray, &nbTrustedCrls);
    SOPC_ASSERT(1 == nbTrustedCrls);

    if (SOPC_STATUS_OK == status)
    {
        pBsArray = SOPC_Calloc(nbTrustedCrls, sizeof(SOPC_ByteString));
        if (NULL == pBsArray)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pRawBuffer = SOPC_KeyManager_SerializedCRL_Data(pRawTrustedCrlArray);
        if (NULL == pRawBuffer)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
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
        SOPC_ByteString_Initialize(pBsArray);
        pBsArray->Data = pRawBuffer->data; // Attach data
        pBsArray->Length = (int32_t) pRawBuffer->length;
        pBsArray->DoNotClear = true;
    }

    SOPC_Free(pRawTrustedCrlArray);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pBsArray);
        pBsArray = NULL;
    }
    *out_bs_array = pBsArray;

    return status;
}

/*---------------------------------------------------------------------------
 *                       Main helpers functions
 *---------------------------------------------------------------------------*/

// Check that the number of arguments of an option does not exceed the total number
// of arguments given to the program.
static SOPC_ReturnStatus check_number_of_arguments(int nbr_of_arguments,
                                                   int option_position,
                                                   int total_nbr_of_arguments)
{
    return (nbr_of_arguments + option_position < total_nbr_of_arguments) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

static SOPC_ReturnStatus Add_Certificate_To_Server_TL(SOPC_ClientConnection* secureConnection, const char* cert_path)
{
    // Create the ByteString certificate
    SOPC_ByteString* certificate_to_add = SOPC_ByteString_Create();
    SOPC_ASSERT(NULL != certificate_to_add);
    SOPC_ReturnStatus status = cert_file_to_bs(cert_path, &certificate_to_add);

    // Add it to the server TL.
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_AddCertificate(secureConnection, certificate_to_add, true);
    }

    // Free
    SOPC_ByteString_Delete(certificate_to_add);

    return status;
}

static SOPC_ReturnStatus Add_Certificate_To_Server_TL_With_Write_Method(SOPC_ClientConnection* secureConnection,
                                                                        const char* cert_path,
                                                                        const char* crl_path,
                                                                        const char* trusted_or_untrusted)
{
    // Create the ByteString certificate
    SOPC_ByteString* certificate_to_add = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = cert_file_to_bs(cert_path, &certificate_to_add);

    // Create the ByteString crl
    SOPC_ByteString* crl_to_add = NULL;
    SOPC_CRLList* pCRL = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromFile(crl_path, &pCRL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = crl_to_bs(pCRL, &crl_to_add);
    }

    // Add the cert and crl to the TrustList and update the server's TrustList
    OpcUa_TrustListDataType pTrustList = {0};
    uint32_t fileHandle = 0;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustList);
        if (0 == strcmp(trusted_or_untrusted, "untrusted"))
        {
            // Write the new issuer certificate and crl
            pTrustList.SpecifiedLists = OpcUa_TrustListMasks_IssuerCertificates | OpcUa_TrustListMasks_IssuerCrls;
            pTrustList.NoOfIssuerCertificates = 1;
            pTrustList.NoOfIssuerCrls = 1;
            pTrustList.IssuerCrls = crl_to_add;
            pTrustList.IssuerCertificates = certificate_to_add;
            // data moved
            certificate_to_add = NULL;
            crl_to_add = NULL;
        }
        else // Trusted
        {
            // Write the new trusted certificate and crl
            pTrustList.SpecifiedLists = OpcUa_TrustListMasks_TrustedCertificates | OpcUa_TrustListMasks_TrustedCrls;
            pTrustList.NoOfTrustedCertificates = 1;
            pTrustList.NoOfTrustedCrls = 1;
            pTrustList.TrustedCrls = crl_to_add;
            pTrustList.TrustedCertificates = certificate_to_add;
            // data moved
            certificate_to_add = NULL;
            crl_to_add = NULL;
        }
        status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustList);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
    }

    // Free
    SOPC_KeyManager_CRL_Free(pCRL);
    SOPC_ByteString_Delete(certificate_to_add);
    SOPC_ByteString_Delete(crl_to_add);
    OpcUa_TrustListDataType_Clear(&pTrustList);

    return status;
}

// NOTE: with the actual push_server implementation we need to create a full new TL in order to remove one certificate
// using the Write method.
static SOPC_ReturnStatus Remove_Certificate_To_Server_TL_With_Write_Method(SOPC_ClientConnection* secureConnection,
                                                                           const char* thumbprint,
                                                                           const char* trusted_or_untrusted)
{
    // 1) Retrieve the TL or the server.
    uint32_t fileHandle = 0;
    OpcUa_TrustListDataType* pTrustList = NULL;
    SOPC_ReturnStatus status =
        SOPC_TEST_TrustList_OpenWithMasks(secureConnection, OpcUa_TrustListMasks_All, &fileHandle);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_Read(secureConnection, fileHandle, &pTrustList);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_Close(secureConnection, fileHandle);
    }

    // 2) Create the CertificateList/CRL from old TL and remove the item we want to remove.
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CertificateList* pIssuerCerts = NULL;
    SOPC_CRLList* pTrustedCrls = NULL;
    SOPC_CRLList* pIssuerCrls = NULL;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != pTrustList);
        SOPC_ASSERT(NULL != pTrustList->TrustedCertificates);
        SOPC_ASSERT(0 != pTrustList->NoOfTrustedCertificates);
        status = trustList_write_bs_array_to_cert_list(pTrustList->TrustedCertificates,
                                                       (uint32_t) pTrustList->NoOfTrustedCertificates, &pTrustedCerts);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != pTrustList->IssuerCertificates);
        SOPC_ASSERT(0 != pTrustList->NoOfIssuerCertificates);
        status = trustList_write_bs_array_to_cert_list(pTrustList->IssuerCertificates,
                                                       (uint32_t) pTrustList->NoOfIssuerCertificates, &pIssuerCerts);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != pTrustList->IssuerCrls);
        SOPC_ASSERT(0 != pTrustList->NoOfIssuerCrls);
        status = trustList_write_bs_array_to_crl_list(pTrustList->IssuerCrls, (uint32_t) pTrustList->NoOfIssuerCrls,
                                                      &pIssuerCrls);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != pTrustList->TrustedCrls);
        SOPC_ASSERT(0 != pTrustList->NoOfTrustedCrls);
        status = trustList_write_bs_array_to_crl_list(pTrustList->TrustedCrls, (uint32_t) pTrustList->NoOfTrustedCrls,
                                                      &pTrustedCrls);
    }
    bool bCertIsRemoved = false;
    bool bIsIssuer = false;
    if (SOPC_STATUS_OK == status)
    {
        if (0 == strcmp(trusted_or_untrusted, "untrusted"))
        {
            status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(&pIssuerCerts, &pIssuerCrls, thumbprint,
                                                                        &bCertIsRemoved, &bIsIssuer);
        }
        else
        {
            status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(&pTrustedCerts, &pTrustedCrls, thumbprint,
                                                                        &bCertIsRemoved, &bIsIssuer);
        }
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
    uint32_t byteLenTot = 0;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TrLst_Mask specifiedList = SOPC_TL_MASK_ALL;
        status = trustlist_attach_certs_to_raw_arrays(specifiedList, pTrustedCerts, &pRawTrustedCertArray,
                                                      &nbTrustedCerts, pTrustedCrls, &pRawTrustedCrlArray,
                                                      &nbTrustedCrls, pIssuerCerts, &pRawIssuerCertArray,
                                                      &nbIssuerCerts, pIssuerCrls, &pRawIssuerCrlArray, &nbIssuerCrls);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = trustlist_attach_raw_arrays_to_bs_arrays(
            pRawTrustedCertArray, nbTrustedCerts, &pBsTrustedCertArray, pRawTrustedCrlArray, nbTrustedCrls,
            &pBsTrustedCrlArray, pRawIssuerCertArray, nbIssuerCerts, &pBsIssuerCertArray, pRawIssuerCrlArray,
            nbIssuerCrls, &pBsIssuerCrlArray, &byteLenTot);
        SOPC_UNUSED_RESULT(byteLenTot);
    }

    // 4) Write this new TL in the server
    if (SOPC_STATUS_OK == status)
    {
        pTrustListNew.SpecifiedLists = OpcUa_TrustListMasks_All;
        pTrustListNew.TrustedCertificates = pBsTrustedCertArray;
        pTrustListNew.NoOfTrustedCertificates = (int32_t) nbTrustedCerts;
        pTrustListNew.TrustedCrls = pBsTrustedCrlArray;
        pTrustListNew.NoOfTrustedCrls = (int32_t) nbTrustedCrls;
        pTrustListNew.IssuerCertificates = pBsIssuerCertArray;
        pTrustListNew.NoOfIssuerCertificates = (int32_t) nbIssuerCerts;
        pTrustListNew.IssuerCrls = pBsIssuerCrlArray;
        pTrustListNew.NoOfIssuerCrls = (int32_t) nbIssuerCrls;
        status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustListNew);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
    }

    // Free
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

    return status;
}

static SOPC_ReturnStatus Remove_Certificate_To_Server_TL(SOPC_ClientConnection* secureConnection,
                                                         const char* thumbprint,
                                                         const char* trusted_or_untrusted)
{
    SOPC_String string_thumbprint = {0};
    SOPC_ReturnStatus status = SOPC_String_InitializeFromCString(&string_thumbprint, thumbprint);
    bool bIsTrustedCert = true;
    if (0 == strcmp(trusted_or_untrusted, "untrusted"))
    {
        bIsTrustedCert = false;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_RemoveCertificate(secureConnection, string_thumbprint, bIsTrustedCert);
    }

    SOPC_String_Clear(&string_thumbprint);

    return status;
}

// Add a new CRL with a revoked cert in it to the trusted CRLs of the server.
static SOPC_ReturnStatus Add_Constant_Crl_To_Server_TL(SOPC_ClientConnection* secureConnection)
{
    // 1) Create the TL with the new CRL
    OpcUa_TrustListDataType pTrustList = {0};
    SOPC_EncodeableObject_Initialize(&OpcUa_TrustListDataType_EncodeableType, (void*) &pTrustList);
    SOPC_ByteString* crl_to_add = NULL;
    SOPC_CRLList* pCRL = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_CRL_CreateOrAddFromFile("push_data/cacrl_with_revoked_cert.der", &pCRL);
    if (SOPC_STATUS_OK == status)
    {
        status = crl_to_bs(pCRL, &crl_to_add);
    }

    // 2) Update the server TL
    uint32_t fileHandle = 0;
    if (SOPC_STATUS_OK == status)
    {
        pTrustList.SpecifiedLists = OpcUa_TrustListMasks_TrustedCrls;
        pTrustList.TrustedCrls = crl_to_add;
        pTrustList.NoOfTrustedCrls = 1;
        status = SOPC_TEST_TrustList_Open(secureConnection, true, &fileHandle);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_Write(secureConnection, fileHandle, &pTrustList);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_TrustList_CloseAndUpdate(secureConnection, fileHandle);
    }

    // Free
    SOPC_KeyManager_CRL_Free(pCRL);
    OpcUa_TrustListDataType_Clear(&pTrustList);

    return status;
}

static SOPC_ReturnStatus Ask_CSR_To_The_Server_And_Write_CSR_File(SOPC_ClientConnection* secureConnection,
                                                                  const char* string_groupID,
                                                                  const char* string_certificateTypeId,
                                                                  const char* string_newKey,
                                                                  const char* string_nonce)
{
    SOPC_ByteString* csr = NULL;
    SOPC_String* subjectName = SOPC_String_Create();
    OpcUa_CallResponse* resp = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // groupId
    SOPC_NodeId groupId = gServerDefaultApplicationGroupId;
    if (0 == strcmp(string_groupID, "groupIdInvalid"))
    {
        groupId = gGroupIdInvalid;
    }
    else if (0 == strcmp(string_groupID, "groupIdEmpty"))
    {
        groupId = gGroupIdEmpty;
    }

    // certificateTypeId
    SOPC_NodeId certificateTypeId = gRsaSha256ApplicationCertificateTypeId;
    if (0 == strcmp(string_certificateTypeId, "certificateTypeIdInvalid"))
    {
        certificateTypeId = gCertificateTypeIdInvalid;
    }

    // New key ?
    bool newKey = false;
    if (0 == strcmp(string_newKey, "newKey"))
    {
        newKey = true;
    }

    // Nonce
    SOPC_ByteString* nonce = SOPC_ByteString_Create();
    if (0 == strcmp(string_nonce, "nonce"))
    {
        // Generate at least 32 bytes of random and put it in the ByteString nonce
        SOPC_ExposedBuffer* pBuff = NULL;
        SOPC_CryptoProvider* pCrypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
        status = SOPC_CryptoProvider_GenerateRandomBytes(pCrypto, 32, &pBuff);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_CryptoProvider_Free(pCrypto);
            nonce->Data = pBuff;
            nonce->Length = 32;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_ServerConfiguration_CreateSigningRequest_GetResponse(
            secureConnection, &groupId, &certificateTypeId, subjectName, newKey, nonce, &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (resp != NULL)
        {
            if (!SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                printf("Response of CSR of the server NOK.\n");
                status = SOPC_STATUS_NOK;
            }
            else
            {
                printf("Response of CSR of the server ok. Filling the CSR...\n");
                status = SOPC_TEST_ServerConfiguration_CreateSigningRequest_FromResponse(resp, &csr);
            }
        }
    }

#if SOPC_HAS_FILESYSTEM
    if (SOPC_STATUS_OK == status && NULL != csr)
    {
        // Write the csr der in a file.
        FILE* fp = fopen("push_data/input_csr.der", "wb+");
        size_t nb_written = fwrite(csr->Data, 1, (size_t) csr->Length, fp);
        if (0 == nb_written)
        {
            printf("Fail at writing the csr into file.\n");
            status = SOPC_STATUS_NOK;
        }
        fclose(fp);
    }
#endif

    // Free the response and the csr
    SOPC_ReturnStatus localStatus = SOPC_EncodeableObject_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    SOPC_ByteString_Delete(csr);
    SOPC_ByteString_Delete(nonce);
    SOPC_String_Delete(subjectName);

    return status;
}

static SOPC_ReturnStatus Update_Server_Certificate(SOPC_ClientConnection* secureConnection,
                                                   const char* cert_path,
                                                   int nbOfIssuers,
                                                   const SOPC_CertificateList* pIssuerCerts)
{
    SOPC_ByteString* bs_server_cert = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = cert_file_to_bs(cert_path, &bs_server_cert);
    SOPC_ByteString* pBsIssuerCertArray = NULL;
    SOPC_SerializedCertificate* pRawIssuerCertArray = NULL;
    uint32_t byteLenTot = 0;
    // If issuers are provided
    if (SOPC_STATUS_OK == status && nbOfIssuers > 0)
    {
        uint32_t nbOfIssuersParsed = 0;
        status = SOPC_KeyManager_CertificateList_AttachToSerializedArray(pIssuerCerts, &pRawIssuerCertArray,
                                                                         &nbOfIssuersParsed);
        if (SOPC_STATUS_OK == status)
        {
            status = trustlist_attach_raw_array_to_bs_array(pRawIssuerCertArray, nbOfIssuersParsed, &pBsIssuerCertArray,
                                                            false, &byteLenTot);
            SOPC_UNUSED_RESULT(byteLenTot);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TEST_ServerConfiguration_UpdateCertificate(secureConnection, bs_server_cert, pBsIssuerCertArray,
                                                                 nbOfIssuers);
    }

    // Free
    SOPC_Free(pRawIssuerCertArray);
    SOPC_ByteString_Delete(bs_server_cert);
    SOPC_ByteString_Delete(pBsIssuerCertArray);

    return status;
}

/*---------------------------------------------------------------------------
 *                                Main
 *---------------------------------------------------------------------------*/

/* Main function that runs a client. Return values:
 * - 0 on success (connection ok, interaction with the server ok) ;
 * - 1 on failure (anywhere) ;
 * - 2 on unexpected disconnection ;
 * - 3 on success at UpdateCertificate followed by disconnection.
 * USAGE:
 * - argv[1] (mandatory) = client certificate path ;
 * - argv[2] (mandatory) = client private key path ;
 * - you may precise the server certificate to use for the connection by putting
 *   argv[3] = "server_certificate". If you do so, argv[4] should be equal to the server certificate path ;
 * - arguments at position different of 1 and 2 are compared to push client commands (add, write, ...).
 */
int main(int argc, char* const argv[])
{
    const char* usage =
        "\n    Binary USAGE:\n"
        "    - argv[1] (mandatory) = client certificate path ;\n"
        "    - argv[2] (mandatory) = client private key path ;\n"
        "    - you may precise the server certificate to use for the connection by putting\n"
        "      argv[3] = 'server_certificate'. If you do so, argv[4] should be equal to the server certificate path ;\n"
        "    - arguments at position different of 1 and 2 are compared to push client commands (add, write, ...).\n\n";

    int ret = EXIT_FAILURE;
    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_SecureConnection_Config* scConfig = NULL;
    const char* clientCertPath = NULL;
    const char* clientKeyPath = NULL;

    /* 1) Connect to the server */
    // Assert the program has at least the two mandatory arguments
    SOPC_ReturnStatus status = (argc > 1) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        clientCertPath = argv[1];
        clientKeyPath = argv[2];

        /* Initialize SOPC_Common */
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./push_client_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    }

    // Initialize client
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    if (SOPC_STATUS_OK == status)
    {
        const char* server_certificate = "server_public/server_4k_cert.der";
        // The server certificate can be precised (optionnal)
        if (3 < argc)
        {
            const char* maybe_server_certificate = argv[3];
            if (0 == strcmp(maybe_server_certificate, "server_certificate"))
            {
                server_certificate = argv[4];
            }
        }

        // Create connection and connect the user/client to the server
        status = SOPC_Create_Custom_Secure_Connection(clientCertPath, clientKeyPath, true, "./S2OPC_Demo_PKI_Client",
                                                      server_certificate, &scConfig);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(scConfig, SOPC_Client_ConnEventCb, &secureConnection);
    }

    // Manage future update of the server certificate
    bool bUpdateCertificateSucceeded = false;

    /* 2) If connected, interact with the server */
    if (SOPC_STATUS_OK == status && NULL != secureConnection)
    {
        ret = EXIT_SUCCESS;
        printf("Connected.\n");
        // While no error, parse the options one by one.
        for (int i = 3; i < argc && EXIT_SUCCESS == ret; i++)
        {
            // OPTION: add. 1 argument:
            // - the path of the certificate to add.
            if (0 == strcmp(argv[i], "add"))
            {
                status = check_number_of_arguments(1, i, argc);
                // Create inputs and call the function
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* cert_path = argv[i];
                    status = Add_Certificate_To_Server_TL(secureConnection, cert_path);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: write. 3 arguments:
            // - the path of the CA certificate to add,
            // - its crl path,
            // - trusted or untrusted, depending on if we want to add the CA to the trusted or untrusted certificates.
            else if (0 == strcmp(argv[i], "write"))
            {
                status = check_number_of_arguments(3, i, argc);
                // Create inputs and call the function
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* cert_path = argv[i];
                    i++;
                    const char* crl_path = argv[i];
                    i++;
                    const char* trusted_or_untrusted = argv[i];
                    status = Add_Certificate_To_Server_TL_With_Write_Method(secureConnection, cert_path, crl_path,
                                                                            trusted_or_untrusted);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: write_remove. 2 arguments must follow this option:
            // - the thumbprint of the certificate to remove,
            // - trusted or untrusted, depending on if the certificate is in trusted or untrusted of the server.
            else if (0 == strcmp(argv[i], "write_remove"))
            {
                status = check_number_of_arguments(2, i, argc);
                // Create inputs and call the function
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* thumbprint = argv[i];
                    i++;
                    const char* trusted_or_untrusted = argv[i];
                    status = Remove_Certificate_To_Server_TL_With_Write_Method(secureConnection, thumbprint,
                                                                               trusted_or_untrusted);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: remove. 2 arguments must follow this option:
            // - the thumbprint of the certificate to remove,
            // - trusted or untrusted, depending on if the certificate is in trusted or untrusted of the server.
            else if (0 == strcmp(argv[i], "remove"))
            {
                status = check_number_of_arguments(2, i, argc);
                // Create inputs and call the function
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* thumbprint = argv[i];
                    i++;
                    const char* trusted_or_untrusted = argv[i];
                    status = Remove_Certificate_To_Server_TL(secureConnection, thumbprint, trusted_or_untrusted);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: revoke. No argument needs to be provided.
            else if (0 == strcmp(argv[i], "revoke"))
            {
                status = Add_Constant_Crl_To_Server_TL(secureConnection);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: sleep. No argument needs to be provided.
            // The client sleeps as long as it did not receive any connection event.
            else if (0 == strcmp(argv[i], "sleep"))
            {
                while (!connectionEventReceived)
                {
                    SOPC_Sleep(SIG_UPDATE_TIMEOUT_MS);
                }
            }

            // OPTION: csr. 4 arguments must follow this option:
            // - groupId: groupIdValid, groupIdInvalid, or groupIdEmpty
            // - certificateTypeId: certificateTypeIdValid or certificateTypeIdInvalid
            // - newKey: newkey or noNewKey
            // - nonce: nonce or noNonce
            else if (0 == strcmp(argv[i], "csr"))
            {
                status = check_number_of_arguments(4, i, argc);
                // Create inputs and call the function
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* groupID = argv[i];
                    i++;
                    const char* certificateTypeId = argv[i];
                    i++;
                    const char* newKey = argv[i];
                    i++;
                    const char* nonce = argv[i];
                    status = Ask_CSR_To_The_Server_And_Write_CSR_File(secureConnection, groupID, certificateTypeId,
                                                                      newKey, nonce);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: getRejectedList. No argument needs to be provided for this option. It creates the
            // folder "rejected", if the rejectedList of the server is not empty.
            else if (0 == strcmp(argv[i], "getRejectedList"))
            {
                status = SOPC_TEST_ServerConfiguration_GetRejectedList(secureConnection);
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
            }

            // OPTION: updateCertificate. 3 arguments must follow this option:
            // - the path of the certificate,
            // - the number of issuers (0 if not),
            // - the paths of the issuer certificates.
            else if (0 == strcmp(argv[i], "updateCertificate"))
            {
                // If there are at least two arguments, calculate the exact number
                // of arguments needed (depends on the number of issuers)
                status = check_number_of_arguments(2, i, argc);
                int nbOfIssuers = 0;
                if (SOPC_STATUS_OK == status)
                {
                    nbOfIssuers = atoi(argv[i + 2]);
                    status = check_number_of_arguments(2 + nbOfIssuers, i, argc);
                }

                // Create inputs and call the function
                SOPC_CertificateList* pIssuerCerts = NULL;
                if (SOPC_STATUS_OK == status)
                {
                    i++;
                    const char* cert_path = argv[i];
                    // If issuers are provided
                    if (nbOfIssuers > 0)
                    {
                        i++; // skip the number of issuers and go to the path of the first issuer
                        for (int j = 0; j < nbOfIssuers && SOPC_STATUS_OK == status; j++)
                        {
                            i++;
                            status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(argv[i], &pIssuerCerts);
                        }
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        status = Update_Server_Certificate(secureConnection, cert_path, nbOfIssuers, pIssuerCerts);
                    }
                }
                if (SOPC_STATUS_OK != status)
                {
                    ret = EXIT_FAILURE;
                }
                else
                {
                    bUpdateCertificateSucceeded = true;
                }

                SOPC_KeyManager_Certificate_Free(pIssuerCerts);
            }
        }
    }

    /* 3) Disconnect then clear the client config */
    status = SOPC_ClientHelper_Disconnect(&secureConnection);
    if (SOPC_STATUS_OK != status)
    {
        printf("%s", usage);
        ret = EXIT_FAILURE;
    }
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (connectionEventReceived)
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
