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

#include "opcua_identifiers.h"
#include "push_server_methods.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_trustlist_internal_helper.h" // common helper to the demo push server lib

/* Definition of the NodeIds and the variants related to the push server */
const SOPC_NodeId gServerDefaultApplicationGroupId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup};

const SOPC_NodeId gRsaSha256ApplicationCertificateTypeId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_RsaSha256ApplicationCertificateType};

static const SOPC_NodeId gServerConfiguration = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                 .Namespace = 0,
                                                 .Data.Numeric = OpcUaId_ServerConfiguration};

static const SOPC_NodeId gServerConfiguration_CreateSigningRequestId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfigurationType_CreateSigningRequest};

static const SOPC_NodeId gServerConfiguration_UpdateCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfigurationType_UpdateCertificate};

static const SOPC_NodeId gServerConfiguration_GetRejectedListId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfigurationType_GetRejectedList};

static const SOPC_NodeId gServerTrustList = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_ServerConfiguration_CertificateGroups_DefaultApplicationGroup_TrustList};

static const SOPC_NodeId gServerTrustList_OpenWithMasksId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                             .Namespace = 0,
                                                             .Data.Numeric = OpcUaId_TrustListType_OpenWithMasks};

static const SOPC_NodeId gServerTrustList_CloseAndUpdateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                              .Namespace = 0,
                                                              .Data.Numeric = OpcUaId_TrustListType_CloseAndUpdate};

static const SOPC_NodeId gServerTrustList_AddCertificateId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                              .Namespace = 0,
                                                              .Data.Numeric = OpcUaId_TrustListType_AddCertificate};

static const SOPC_NodeId gServerTrustList_RemoveCertificateId = {
    .IdentifierType = SOPC_IdentifierType_Numeric,
    .Namespace = 0,
    .Data.Numeric = OpcUaId_TrustListType_RemoveCertificate};

static const SOPC_NodeId gServerFileType_OpenId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                                   .Namespace = 0,
                                                   .Data.Numeric = OpcUaId_FileType_Open};

static const SOPC_NodeId gFileType_ReadId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                             .Namespace = 0,
                                             .Data.Numeric = OpcUaId_FileType_Read};

static const SOPC_NodeId gFileType_WriteId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                              .Namespace = 0,
                                              .Data.Numeric = OpcUaId_FileType_Write};

static const SOPC_NodeId gFileType_CloseId = {.IdentifierType = SOPC_IdentifierType_Numeric,
                                              .Namespace = 0,
                                              .Data.Numeric = OpcUaId_FileType_Close};

static const SOPC_Variant CSRinputDefaultArguments[5] = {
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {0}}};

static const SOPC_Variant UpdateCertificateDefaultArguments[6] = {
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId = NULL}},
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId = NULL}},
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_Array, {.Array = {0}}},
    {true, SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {0}}};

static const SOPC_Variant OpenWithMasksDefaultArguments = {true,
                                                           SOPC_UInt32_Id,
                                                           SOPC_VariantArrayType_SingleValue,
                                                           {.Uint32 = OpcUa_TrustListMasks_All}};

static const SOPC_Variant RemoveCertificateDefaultArguments[2] = {
    {true, SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {0}}};

static const SOPC_Variant AddCertificateDefaultArguments[2] = {
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {0}},
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {0}}};

static const SOPC_Variant OpenDefaultArguments = {true,
                                                  SOPC_Byte_Id,
                                                  SOPC_VariantArrayType_SingleValue,
                                                  {.Byte = OpcUa_OpenFileMode_Read}};

static const SOPC_Variant ReadDefaultArguments[2] = {
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 0}},
    {true,
     SOPC_Int32_Id,
     SOPC_VariantArrayType_SingleValue,
     {.Int32 = SOPC_DEFAULT_MAX_STRING_LENGTH}}}; // Max length to read
static const SOPC_Variant WriteDefaultArguments[2] = {
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 0}},
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {0}}};

static const SOPC_Variant CloseDefaultArguments = {true,
                                                   SOPC_UInt32_Id,
                                                   SOPC_VariantArrayType_SingleValue,
                                                   {.Uint32 = 0}};

/* Definition of the methods */
SOPC_ReturnStatus SOPC_TEST_TrustList_Read(SOPC_ClientConnection* secureConnection,
                                           uint32_t fileHandle,
                                           OpcUa_TrustListDataType** ppTrustList)
{
    // Check parameters
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ASSERT(NULL != ppTrustList);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant Read[2] = {ReadDefaultArguments[0], ReadDefaultArguments[1]};
    Read[0].Value.Uint32 = fileHandle;
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList, &gFileType_ReadId, 2, Read);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
                SOPC_Variant* trustListVariant = resp->Results[0].OutputArguments;
                SOPC_ASSERT(SOPC_ByteString_Id == trustListVariant->BuiltInTypeId);
                SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == trustListVariant->ArrayType);

                if (trustListVariant->Value.Bstring.Length < Read[1].Value.Int32)
                {
                    *ppTrustList = SOPC_TrustList_DecodeTrustListData(&trustListVariant->Value.Bstring);
                    status = (NULL != *ppTrustList ? SOPC_STATUS_OK : SOPC_STATUS_ENCODING_ERROR);
                }
                else
                {
                    printf("TrustList length is greater than the maximum value configured: %" PRIi32 "\n",
                           Read[1].Value.Int32);
                }
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_TrustList_Close(SOPC_ClientConnection* secureConnection, uint32_t fileHandle)
{
    // Check parameter
    SOPC_ASSERT(NULL != secureConnection);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant Close = CloseDefaultArguments;
    Close.Value.Uint32 = fileHandle;
    SOPC_ReturnStatus status =
        SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList, &gFileType_CloseId, 1, &Close);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (!SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            printf(
                "Warning: TrustList.Close operation failed ! Next operation might require timeout to occur before "
                "being accepted.\n");
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

// Implementation choice: Open only for future writing. For future reading use OpenWithMasks().
SOPC_ReturnStatus SOPC_TEST_TrustList_Open(SOPC_ClientConnection* secureConnection, bool write, uint32_t* pFileHandle)
{
    // Check parameters
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ASSERT(NULL != pFileHandle);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant Open = OpenDefaultArguments;
    if (write)
    {
        Open.Value.Byte = OpcUa_OpenFileMode_Write | OpcUa_OpenFileMode_EraseExisting;
    }

    SOPC_ReturnStatus status =
        SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList, &gServerFileType_OpenId, 1, &Open);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
                SOPC_Variant* fileHandlerVariant = resp->Results[0].OutputArguments;
                SOPC_ASSERT(SOPC_UInt32_Id == fileHandlerVariant->BuiltInTypeId);
                SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == fileHandlerVariant->ArrayType);
                *pFileHandle = fileHandlerVariant->Value.Uint32;
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_TrustList_Write(SOPC_ClientConnection* secureConnection,
                                            uint32_t fileHandle,
                                            OpcUa_TrustListDataType* pTrustList)
{
    // Check parameters
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ASSERT(NULL != pTrustList);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant Write[2] = {WriteDefaultArguments[0], WriteDefaultArguments[1]};
    Write[0].Value.Uint32 = fileHandle;
    SOPC_ReturnStatus status = SOPC_TrustList_EncodeTrustListData(pTrustList, &Write[1].Value.Bstring);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList, &gFileType_WriteId, 2, Write);
        SOPC_ByteString_Clear(&Write[1].Value.Bstring);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (!SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_TrustList_CloseAndUpdate(SOPC_ClientConnection* secureConnection, uint32_t fileHandle)
{
    // Check parameter
    SOPC_ASSERT(NULL != secureConnection);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant Close = CloseDefaultArguments;
    Close.Value.Uint32 = fileHandle;
    SOPC_ReturnStatus status =
        SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList, &gServerTrustList_CloseAndUpdateId, 1, &Close);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(1 == resp->NoOfResults);
        if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
        {
            SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
            SOPC_Variant* applyChangedRequiredVariant = resp->Results[0].OutputArguments;
            SOPC_ASSERT(SOPC_Boolean_Id == applyChangedRequiredVariant->BuiltInTypeId);
            SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == applyChangedRequiredVariant->ArrayType);
            /* TODO: call apply changes method if needed */
            SOPC_ASSERT(applyChangedRequiredVariant->Value.Boolean == false &&
                        "TODO: ApplyChange call not implemented !!!");
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_TrustList_AddCertificate(SOPC_ClientConnection* secureConnection,
                                                     SOPC_ByteString* certificate,
                                                     bool isTrustedCertificate)
{
    // Check parameters
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ASSERT(NULL != certificate);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant AddCertificate[2] = {AddCertificateDefaultArguments[0], AddCertificateDefaultArguments[1]};
    AddCertificate[0].Value.Bstring = *certificate;
    AddCertificate[1].Value.Boolean = isTrustedCertificate;
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList,
                                                                &gServerTrustList_AddCertificateId, 2, AddCertificate);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (!SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_TrustList_RemoveCertificate(SOPC_ClientConnection* secureConnection,
                                                        SOPC_String pThumbprint,
                                                        bool isTrustedCertificate)
{
    // Check parameter
    SOPC_ASSERT(NULL != secureConnection);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant RemoveCertificate[2] = {RemoveCertificateDefaultArguments[0], RemoveCertificateDefaultArguments[1]};
    RemoveCertificate[0].Value.String = pThumbprint;
    RemoveCertificate[1].Value.Boolean = isTrustedCertificate;
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(
        req, 0, &gServerTrustList, &gServerTrustList_RemoveCertificateId, 2, RemoveCertificate);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (!SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_GetResponse(
    SOPC_ClientConnection* secureConnection,
    SOPC_NodeId* groupId,
    SOPC_NodeId* certificateTypeId,
    SOPC_String* subjectName,
    bool renewKey,
    SOPC_ByteString* nonce,
    OpcUa_CallResponse** resp)
{
    // Check parameters
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ASSERT(NULL != groupId);
    SOPC_ASSERT(NULL != certificateTypeId);
    SOPC_ASSERT(NULL != subjectName);
    SOPC_ASSERT(NULL != nonce);
    SOPC_ASSERT(NULL != resp && NULL == *resp);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);

    SOPC_Variant CSRinputArguments[5] = {CSRinputDefaultArguments[0], CSRinputDefaultArguments[1],
                                         CSRinputDefaultArguments[2], CSRinputDefaultArguments[3],
                                         CSRinputDefaultArguments[4]};

    CSRinputArguments[0].Value.NodeId = groupId;
    CSRinputArguments[1].Value.NodeId = certificateTypeId;
    CSRinputArguments[2].Value.String = *subjectName;
    CSRinputArguments[3].Value.Boolean = renewKey;
    CSRinputArguments[4].Value.Bstring = *nonce;
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(
        req, 0, &gServerConfiguration, &gServerConfiguration_CreateSigningRequestId, 5, CSRinputArguments);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) resp);
    }

    if (status != SOPC_STATUS_OK)
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_FromResponse(OpcUa_CallResponse* resp,
                                                                                  SOPC_ByteString** csr)
{
    // Check parameters
    SOPC_ASSERT(NULL != csr && NULL == *csr);
    SOPC_ASSERT(NULL != resp);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
    {
        SOPC_ASSERT(1 == resp->NoOfResults);
        if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
        {
            SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
            SOPC_Variant* csrVariant = resp->Results[0].OutputArguments;
            SOPC_ASSERT(SOPC_ByteString_Id == csrVariant->BuiltInTypeId);
            SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == csrVariant->ArrayType);

            *csr = SOPC_Calloc(1, sizeof(**csr));
            if (NULL == *csr)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else if (csrVariant->Value.Bstring.Length > 0)
            {
                (*csr)->Data = csrVariant->Value.Bstring.Data;
                (*csr)->Length = csrVariant->Value.Bstring.Length;
                SOPC_Variant_Initialize(csrVariant); // Data moved from variant
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return status;
}

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_UpdateCertificate(SOPC_ClientConnection* secureConnection,
                                                                  SOPC_ByteString* certificate,
                                                                  SOPC_ByteString* issuersArray,
                                                                  int32_t nbOfIssuers)
{
    // Check parameter
    SOPC_ASSERT(certificate != NULL);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant UpdateCertificateInputArguments[6] = {
        UpdateCertificateDefaultArguments[0], UpdateCertificateDefaultArguments[1],
        UpdateCertificateDefaultArguments[2], UpdateCertificateDefaultArguments[3],
        UpdateCertificateDefaultArguments[4], UpdateCertificateDefaultArguments[5]};
    SOPC_NodeId UpdateCertificateInputArguments_ApplicationGroupId = gServerDefaultApplicationGroupId;
    UpdateCertificateInputArguments[0].Value.NodeId = &UpdateCertificateInputArguments_ApplicationGroupId;
    SOPC_NodeId UpdateCertificateInputArguments_ApplicationCertificateTypeId = gRsaSha256ApplicationCertificateTypeId;
    UpdateCertificateInputArguments[1].Value.NodeId = &UpdateCertificateInputArguments_ApplicationCertificateTypeId;

    // Copy certificate data into input argument
    SOPC_ByteString_Clear(&UpdateCertificateInputArguments[2].Value.Bstring);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&UpdateCertificateInputArguments[2].Value.Bstring,
                                                             certificate->Data, certificate->Length);
    // If deep copy succeeded, will need to free the variant
    if (SOPC_STATUS_OK == status)
    {
        UpdateCertificateInputArguments[2].DoNotClear = false;
    }

    // If issuers are provided.
    if (SOPC_STATUS_OK == status && 0 < nbOfIssuers)
    {
        // Check if issuers were effectively provided
        SOPC_ASSERT(issuersArray != NULL);

        UpdateCertificateInputArguments[3].Value.Array.Length = nbOfIssuers;
        UpdateCertificateInputArguments[3].Value.Array.Content.BstringArr =
            SOPC_Calloc((size_t) nbOfIssuers, sizeof(SOPC_ByteString));
        SOPC_ASSERT(NULL != UpdateCertificateInputArguments[3].Value.Array.Content.BstringArr);
        // Alloc has been made, will need to free the variant
        UpdateCertificateInputArguments[3].DoNotClear = false;
        for (int32_t i = 0; i < nbOfIssuers && SOPC_STATUS_OK == status; i++)
        {
            // Copy issuers data into input argument
            SOPC_ByteString_Clear(&UpdateCertificateInputArguments[3].Value.Array.Content.BstringArr[i]);
            status =
                SOPC_ByteString_CopyFromBytes(&UpdateCertificateInputArguments[3].Value.Array.Content.BstringArr[i],
                                              issuersArray[i].Data, issuersArray[i].Length);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_CallRequest_SetMethodToCall(req, 0, &gServerConfiguration, &gServerConfiguration_UpdateCertificateId,
                                             6, UpdateCertificateInputArguments);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
                SOPC_Variant* applyChangedRequiredVariant = resp->Results[0].OutputArguments;
                SOPC_ASSERT(SOPC_Boolean_Id == applyChangedRequiredVariant->BuiltInTypeId);
                SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == applyChangedRequiredVariant->ArrayType);
                /* TODO: call apply changes method if needed */
                SOPC_ASSERT(applyChangedRequiredVariant->Value.Boolean == false &&
                            "TODO: ApplyChange call not implemented !!!");
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);

    SOPC_Variant_Clear(&UpdateCertificateInputArguments[2]);
    SOPC_Variant_Clear(&UpdateCertificateInputArguments[3]);

    return status;
}

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_GetRejectedList(SOPC_ClientConnection* secureConnection)
{
    // Check parameter
    SOPC_ASSERT(secureConnection != NULL);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(req != NULL);
    OpcUa_CallResponse* resp = NULL;

    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(req, 0, &gServerConfiguration,
                                                                &gServerConfiguration_GetRejectedListId, 0, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
                SOPC_Variant* getRejectedListVariant = resp->Results[0].OutputArguments;
                SOPC_UNUSED_ARG(getRejectedListVariant);
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);
    return status;
}

// OPC UA: OpenWithMasks methdo can only be used to read the TL.
SOPC_ReturnStatus SOPC_TEST_TrustList_OpenWithMasks(SOPC_ClientConnection* secureConnection,
                                                    OpcUa_TrustListMasks mask,
                                                    uint32_t* pFileHandle)
{
    // Check parameters
    SOPC_ASSERT(secureConnection != NULL);
    SOPC_ASSERT(pFileHandle != NULL);

    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ASSERT(NULL != req);
    OpcUa_CallResponse* resp = NULL;

    SOPC_Variant OpenWithMasks = OpenWithMasksDefaultArguments;
    OpenWithMasks.Value.Uint32 = (uint32_t) mask;
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCall(req, 0, &gServerTrustList,
                                                                &gServerTrustList_OpenWithMasksId, 1, &OpenWithMasks);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) req, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            if (SOPC_IsGoodStatus(resp->Results[0].StatusCode))
            {
                SOPC_ASSERT(1 == resp->Results[0].NoOfOutputArguments);
                SOPC_Variant* fileHandlerVariant = resp->Results[0].OutputArguments;
                SOPC_ASSERT(SOPC_UInt32_Id == fileHandlerVariant->BuiltInTypeId);
                SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == fileHandlerVariant->ArrayType);
                *pFileHandle = fileHandlerVariant->Value.Uint32;
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (req != NULL)
        {
            SOPC_Encodeable_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &req);
        }
    }

    SOPC_ReturnStatus localStatus = SOPC_Encodeable_Delete(&OpcUa_CallResponse_EncodeableType, (void**) &resp);
    SOPC_UNUSED_RESULT(localStatus);

    return status;
}
