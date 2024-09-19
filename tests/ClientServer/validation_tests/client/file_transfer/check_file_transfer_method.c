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

#include <string.h>

#include "check_file_transfer_method.h"
#include "libs2opc_request_builder.h"

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

static void initialize_variant_call_method_request(SOPC_Variant* pVariant,
                                                   SOPC_BuiltinId BuiltInTypeId,
                                                   SOPC_VariantValue Value)
{
    memset(pVariant, 0, sizeof(SOPC_Variant));
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->DoNotClear = false;
    pVariant->BuiltInTypeId = BuiltInTypeId;
    pVariant->Value = Value;
}

uint32_t SOPC_TEST_FileTransfer_OpenMethod(SOPC_ClientConnection* scConnection,
                                           const bool bIsItem1,
                                           OpcUa_CallResponse** callResponse,
                                           SOPC_Byte mode)
{
    SOPC_ASSERT(NULL != callResponse);
    SOPC_ASSERT(NULL == *callResponse);
    OpcUa_CallRequest* callRequest = NULL;

    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;
    char* methodId = bIsItem1 ? NODEID_MET_OPEN_ITEM1 : NODEID_MET_OPEN_ITEM2;

    const int32_t nbOfInputParams = 1;
    uint32_t fileHandle = INVALID_FILE_HANDLE;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Byte = mode;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_Byte_Id, valueCallRequest);

    callRequest = SOPC_CallRequest_Create(1);

    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    OpcUa_CallResponse* resp = *callResponse;
    if (SOPC_STATUS_OK == status && NULL != resp && 1 == resp->NoOfResults)
    {
        OpcUa_CallMethodResult* result = &resp->Results[0];
        if (NULL != result->OutputArguments)
        {
            SOPC_Variant* variantResult = &result->OutputArguments[0];
            if (SOPC_UInt32_Id == variantResult->BuiltInTypeId)
            {
                fileHandle = variantResult->Value.Uint32;
            }
        }
    }

    SOPC_Variant_Delete(pVariantCallRequest);
    return fileHandle;
}

void SOPC_TEST_FileTransfer_CloseMethod(SOPC_ClientConnection* scConnection,
                                        const bool bIsItem1,
                                        OpcUa_CallResponse** callResponse,
                                        uint32_t fileHandle)
{
    SOPC_ASSERT(NULL != callResponse);
    SOPC_ASSERT(NULL == *callResponse);
    const int32_t nbOfInputParams = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_CLOSE_ITEM1 : NODEID_MET_CLOSE_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;
    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Variant_Delete(pVariantCallRequest);
}

void SOPC_TEST_FileTransfer_WriteMethod(SOPC_ClientConnection* scConnection,
                                        const bool bIsItem1,
                                        OpcUa_CallResponse** callResponse,
                                        uint32_t fileHandle,
                                        SOPC_ByteString* dataToWrite)
{
    const int32_t nbOfInputParams = 2;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc((size_t) nbOfInputParams, sizeof(SOPC_Variant));

    // parameter 1: fileHandle
    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2: data
    valueCallRequest.Bstring = *dataToWrite;
    initialize_variant_call_method_request(&pVariantCallRequest[1], SOPC_ByteString_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_WRITE_ITEM1 : NODEID_MET_WRITE_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;

    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Variant_Delete(pVariantCallRequest);
}

void SOPC_TEST_FileTransfer_ReadMethod(SOPC_ClientConnection* scConnection,
                                       const bool bIsItem1,
                                       OpcUa_CallResponse** callResponse,
                                       uint32_t fileHandle,
                                       int32_t length)
{
    const int32_t nbOfInputParams = 2;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc((size_t) nbOfInputParams, sizeof(SOPC_Variant));

    // parameter 1:
    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2:
    valueCallRequest.Int32 = length;
    initialize_variant_call_method_request(&pVariantCallRequest[1], SOPC_Int32_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_READ_ITEM1 : NODEID_MET_READ_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;

    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Variant_Delete(pVariantCallRequest);
}

void SOPC_TEST_FileTransfer_SetPositionMethod(SOPC_ClientConnection* scConnection,
                                              const bool bIsItem1,
                                              OpcUa_CallResponse** callResponse,
                                              uint32_t fileHandle,
                                              uint64_t position)
{
    const int32_t nbOfInputParams = 2;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc((size_t) nbOfInputParams, sizeof(SOPC_Variant));

    // parameter 1: fileHandle
    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2: position
    valueCallRequest.Uint64 = position;
    initialize_variant_call_method_request(&pVariantCallRequest[1], SOPC_UInt64_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_SETPOS_ITEM1 : NODEID_MET_SETPOS_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;

    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Variant_Delete(pVariantCallRequest);
}

uint64_t SOPC_TEST_FileTransfer_GetPositionMethod(SOPC_ClientConnection* scConnection,
                                                  const bool bIsItem1,
                                                  OpcUa_CallResponse** callResponse,
                                                  uint32_t fileHandle)
{
    uint64_t getPosition = INVALID_POSITION;
    const int32_t nbOfInputParams = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_GETPOS_ITEM1 : NODEID_MET_GETPOS_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;

    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, objectNodeId, methodId,
                                                                           nbOfInputParams, pVariantCallRequest);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_ClientHelperNew_ServiceSync(scConnection, callRequest, (void**) callResponse);
    OpcUa_CallResponse* resp = *callResponse;
    if (SOPC_STATUS_OK == status && NULL != resp && 1 == resp->NoOfResults)
    {
        OpcUa_CallMethodResult* result = &resp->Results[0];
        if (NULL != result->OutputArguments)
        {
            SOPC_Variant* variantResult = &result->OutputArguments[0];
            if (SOPC_UInt64_Id == variantResult->BuiltInTypeId)
            {
                getPosition = variantResult->Value.Uint64;
            }
        }
    }

    SOPC_Variant_Delete(pVariantCallRequest);

    return getPosition;
}
