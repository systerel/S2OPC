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
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#define METHOD_CALL_SUCCEEDED 0

// These following function ease the assignment for method call function:
static void initialize_call_method_request(SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                           char* objectNodeId,
                                           char* method_ID,
                                           const int32_t nbOfInputParams,
                                           SOPC_Variant* inputParams)
{
    pCallRequest->objectNodeId = objectNodeId;
    pCallRequest->methodNodeId = method_ID;
    pCallRequest->nbOfInputParams = nbOfInputParams;
    pCallRequest->inputParams = inputParams;
}

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

uint32_t SOPC_TEST_FileTransfer_OpenMethod(int32_t connectionId,
                                           const bool bIsItem1,
                                           SOPC_ClientHelper_CallMethodResult* pCallResults,
                                           SOPC_Byte mode)
{
    const int32_t nbOfInputParams = 1;
    uint32_t fileHandle = INVALID_FILE_HANDLE;
    const size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Byte = mode;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_Byte_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_OPEN_ITEM1 : NODEID_MET_OPEN_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    if (METHOD_CALL_SUCCEEDED == statusMethodCall && SOPC_GoodGenericStatus == pCallResults->status)
    {
        if (1 == pCallResults->nbOfOutputParams && NULL != pCallResults->outputParams)
        {
            SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
            if (SOPC_UInt32_Id == pVariantCallResult->BuiltInTypeId)
            {
                fileHandle = pVariantCallResult->Value.Uint32;
            }
        }
    }
    return fileHandle;
}

void SOPC_TEST_FileTransfer_CloseMethod(int32_t connectionId,
                                        const bool bIsItem1,
                                        SOPC_ClientHelper_CallMethodResult* pCallResults,
                                        uint32_t fileHandle)
{
    const int32_t nbOfInputParams = 1;
    const size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_CLOSE_ITEM1 : NODEID_MET_CLOSE_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    SOPC_ASSERT(METHOD_CALL_SUCCEEDED == statusMethodCall);
}

void SOPC_TEST_FileTransfer_WriteMethod(int32_t connectionId,
                                        const bool bIsItem1,
                                        SOPC_ClientHelper_CallMethodResult* pCallResults,
                                        uint32_t fileHandle,
                                        SOPC_ByteString* dataToWrite)
{
    const int32_t nbOfInputParams = 2;
    const size_t nbOfElements = 1;
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
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    SOPC_ASSERT(METHOD_CALL_SUCCEEDED == statusMethodCall);
}

void SOPC_TEST_FileTransfer_ReadMethod(int32_t connectionId,
                                       const bool bIsItem1,
                                       SOPC_ClientHelper_CallMethodResult* pCallResults,
                                       uint32_t fileHandle,
                                       int32_t length)
{
    const int32_t nbOfInputParams = 2;
    const size_t nbOfElements = 1;
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
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    SOPC_ASSERT(METHOD_CALL_SUCCEEDED == statusMethodCall);
}

void SOPC_TEST_FileTransfer_SetPositionMethod(int32_t connectionId,
                                              const bool bIsItem1,
                                              SOPC_ClientHelper_CallMethodResult* pCallResults,
                                              uint32_t fileHandle,
                                              uint64_t position)
{
    const int32_t nbOfInputParams = 2;
    const size_t nbOfElements = 1;
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
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    SOPC_ASSERT(METHOD_CALL_SUCCEEDED == statusMethodCall);
}

uint64_t SOPC_TEST_FileTransfer_GetPositionMethod(int32_t connectionId,
                                                  const bool bIsItem1,
                                                  SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                  uint32_t fileHandle)
{
    uint64_t getPosition = INVALID_POSITION;
    const int32_t nbOfInputParams = 1;
    const size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();

    valueCallRequest.Uint32 = fileHandle;
    initialize_variant_call_method_request(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    char* methodId = bIsItem1 ? NODEID_MET_GETPOS_ITEM1 : NODEID_MET_GETPOS_ITEM2;
    char* objectNodeId = bIsItem1 ? NODEID_FILE_TYPE_ITEM1 : NODEID_FILE_TYPE_ITEM2;
    SOPC_ClientHelper_CallMethodRequest call = {0};
    initialize_call_method_request(&call, objectNodeId, methodId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, &call, nbOfElements, pCallResults);
    if (METHOD_CALL_SUCCEEDED == statusMethodCall && SOPC_GoodGenericStatus == pCallResults->status)
    {
        if (1 == pCallResults->nbOfOutputParams && NULL != pCallResults->outputParams)
        {
            SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
            if (SOPC_UInt64_Id == pVariantCallResult->BuiltInTypeId)
            {
                getPosition = pVariantCallResult->Value.Uint64;
            }
        }
    }
    return getPosition;
}
