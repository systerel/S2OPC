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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "sopc_assert.h"
#include "sopc_file_transfer.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "test_file_transfer_method.h"

// These following function ease the assignment for method call function:
static void SOPC_Initialize_CallMethodRequest(SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                              char* method_ID,
                                              int32_t nbOfInputParams,
                                              SOPC_Variant* inputParams)
{
    pCallRequest->methodNodeId = method_ID;
    pCallRequest->nbOfInputParams = nbOfInputParams;
    pCallRequest->inputParams = inputParams;
}

static void SOPC_Initialize_VariantCallMethodRequest(SOPC_Variant* pVariant,
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
                                           SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                           SOPC_ClientHelper_CallMethodResult* pCallResults,
                                           char* met_openId,
                                           SOPC_Byte mode)
{
    int32_t nbOfInputParams = 1;
    uint32_t fileHandle = INVALID_FILE_HANDLE;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Byte = mode;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_Byte_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_openId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    if (0 == statusMethodCall && 0 == pCallResults->status)
    {
        SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
        fileHandle = pVariantCallResult->Value.Uint32;
    }
    return fileHandle;
}

void SOPC_TEST_FileTransfer_CloseMethod(int32_t connectionId,
                                        SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                        SOPC_ClientHelper_CallMethodResult* pCallResults,
                                        char* met_closeId,
                                        uint32_t fileHandle)
{
    int32_t nbOfInputParams = 1;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_closeId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    SOPC_ASSERT(0 == statusMethodCall);
}

void SOPC_TEST_FileTransfer_WriteMethod(int32_t connectionId,
                                        SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                        SOPC_ClientHelper_CallMethodResult* pCallResults,
                                        char* met_writeId,
                                        uint32_t fileHandle,
                                        SOPC_ByteString* dataToWrite)
{
    int32_t nbOfInputParams = 2;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc(2, sizeof(SOPC_Variant));
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    // parameter 1: fileHandle
    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2: data
    valueCallRequest.Bstring = *dataToWrite;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[1], SOPC_ByteString_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_writeId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    SOPC_ASSERT(0 == statusMethodCall);
}

void SOPC_TEST_FileTransfer_ReadMethod(int32_t connectionId,
                                       SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                       SOPC_ClientHelper_CallMethodResult* pCallResults,
                                       char* met_readId,
                                       uint32_t fileHandle,
                                       int32_t length)
{
    int32_t nbOfInputParams = 2;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc(2, sizeof(SOPC_Variant));
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    // parameter 1:
    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2:
    valueCallRequest.Int32 = length;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[1], SOPC_Int32_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_readId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    SOPC_ASSERT(0 == statusMethodCall);
}

void SOPC_TEST_FileTransfer_SetPositionMethod(int32_t connectionId,
                                              SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                              SOPC_ClientHelper_CallMethodResult* pCallResults,
                                              char* met_setposId,
                                              uint32_t fileHandle,
                                              uint64_t position)
{
    int32_t nbOfInputParams = 2;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Calloc(2, sizeof(SOPC_Variant));
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    // parameter 1: fileHandle
    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[0], SOPC_UInt32_Id, valueCallRequest);

    // parameter 2: position
    valueCallRequest.Uint64 = position;
    SOPC_Initialize_VariantCallMethodRequest(&pVariantCallRequest[1], SOPC_UInt64_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_setposId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    SOPC_ASSERT(0 == statusMethodCall);
}

uint64_t SOPC_TEST_FileTransfer_GetPositionMethod(int32_t connectionId,
                                                  SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                  SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                  char* met_getposId,
                                                  uint32_t fileHandle)
{
    uint64_t getPosition = INVALID_POSITION;
    int32_t nbOfInputParams = 1;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_getposId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    if (0 == statusMethodCall && 0 == pCallResults->status)
    {
        SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
        getPosition = pVariantCallResult->Value.Uint64;
    }
    return getPosition;
}
