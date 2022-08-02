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
#include "sopc_file_transfer.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "test_file_transfer_method.h"

// Assessors : getter and setter for the global variable
static int32_t testCaseNum = 1;

static int32_t SOPC_GetTestCaseNumber(void)
{
    return testCaseNum;
}

static int32_t SOPC_IncrementTestCaseNumber(int32_t increment)
{
    testCaseNum = testCaseNum + increment;
    return testCaseNum;
}

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

int32_t SOPC_TEST_FileTransfer_GetTestCaseNumber(void)
{
    return testCaseNum;
}

void SOPC_TEST_FileTransfer_SetTestCaseNumber(int32_t number)
{
    testCaseNum = number;
}

uint32_t SOPC_TEST_FileTransfer_OpenMethod(int32_t connectionId,
                                           bool sameTestCase,
                                           SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                           SOPC_ClientHelper_CallMethodResult* pCallResults,
                                           char* met_openId,
                                           SOPC_Byte mode)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    int32_t nbOfInputParams = 1;
    uint32_t fileHandle = 0;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Byte = mode;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_Byte_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_openId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": open method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": open method failed.\n", SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": open method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": open method failed: Result code: 0x%08" PRIX32
                                   "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
        }
        else
        {
            SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
            fileHandle = pVariantCallResult->Value.Uint32;
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": open method succeed: fileHandle is: %" PRIu32 "\n",
                   SOPC_GetTestCaseNumber(), fileHandle);
#else
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": open method succeed: fileHandle is: %" PRIu32
                                   "\n",
                                   SOPC_GetTestCaseNumber(), fileHandle);
#endif
        }
    }
    return fileHandle;
}

SOPC_StatusCode SOPC_TEST_FileTransfer_CloseMethod(int32_t connectionId,
                                                   bool sameTestCase,
                                                   SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                   SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                   char* met_closeId,
                                                   uint32_t fileHandle)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    SOPC_StatusCode status = 0xFF; // invalid value
    int32_t nbOfInputParams = 1;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_closeId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": close method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": close method failed.\n", SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": close method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": close method failed: Result code: 0x%08" PRIX32
                                   "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
            status = pCallResults->status;
        }
        else
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": close method succeed.\n", SOPC_GetTestCaseNumber());
#else
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": close method succeed.\n",
                                   SOPC_GetTestCaseNumber());
#endif
            status = SOPC_GoodGenericStatus;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_TEST_FileTransfer_WriteMethod(int32_t connectionId,
                                                   bool sameTestCase,
                                                   SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                   SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                   char* met_writeId,
                                                   uint32_t fileHandle,
                                                   SOPC_ByteString* dataToWrite)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    SOPC_StatusCode status = 0xFF; // invalid value
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
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": write method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": write method failed.\n", SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": write method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": write method failed: Result code: 0x%08" PRIX32
                                   "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
            status = pCallResults->status;
        }
        else
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": write method succeed.\n", SOPC_GetTestCaseNumber());
#else
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": write method succeed.\n",
                                   SOPC_GetTestCaseNumber());
#endif
            status = SOPC_GoodGenericStatus;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_TEST_FileTransfer_ReadMethod(int32_t connectionId,
                                                  bool sameTestCase,
                                                  SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                  SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                  char* met_readId,
                                                  uint32_t fileHandle,
                                                  int32_t length)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    SOPC_StatusCode status = 0xFF; // invalid value
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
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": read method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": read method failed.\n", SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": read method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": read method failed: Result code: 0x%08" PRIX32
                                   "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
            status = pCallResults->status;
        }
        else
        {
            if (pCallResults->outputParams->Value.Array.Length < 0)
            {
#if TEST_DEBUG_FT
                printf("<TC_SOPC_FileTransfer_%03" PRId32
                       ": read method failed: no bytes to read or no access granted.\n",
                       SOPC_GetTestCaseNumber());
#else
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "<TC_SOPC_FileTransfer_%03" PRId32
                                       ": read method failed: no bytes to read or no access granted.\n",
                                       SOPC_GetTestCaseNumber());
#endif
            }
            else
            {
#if TEST_DEBUG_FT
                printf("<TC_SOPC_FileTransfer_%03" PRId32 ": read method succeed.\n", SOPC_GetTestCaseNumber());
                SOPC_Variant_Print(pCallResults->outputParams);
#else
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "<TC_SOPC_FileTransfer_%03" PRId32 ": read method succeed.\n",
                                       SOPC_GetTestCaseNumber());
#endif
                status = SOPC_GoodGenericStatus;
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_TEST_FileTransfer_SetPositionMethod(int32_t connectionId,
                                                         bool sameTestCase,
                                                         SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                         SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                         char* met_setposId,
                                                         uint32_t fileHandle,
                                                         uint64_t position)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    SOPC_StatusCode status = 0xFF; // invalid value
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
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": set position method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": set position method failed.\n",
                               SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": set position method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32
                                   ": set position method failed: Result code: 0x%08" PRIX32 "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
            status = pCallResults->status;
        }
        else
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": succeed: set the cursor position to: %" PRIu64 "\n",
                   SOPC_GetTestCaseNumber(), position);
#else
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": succeed: set the cursor position to: %" PRIu64
                                   "\n",
                                   SOPC_GetTestCaseNumber(), position);
#endif
            status = SOPC_GoodGenericStatus;
        }
    }
    return status;
}

uint64_t SOPC_TEST_FileTransfer_GetPositionMethod(int32_t connectionId,
                                                  bool sameTestCase,
                                                  SOPC_ClientHelper_CallMethodRequest* pCallRequest,
                                                  SOPC_ClientHelper_CallMethodResult* pCallResults,
                                                  char* met_getposId,
                                                  uint32_t fileHandle)
{
    if (false == sameTestCase)
    {
        SOPC_IncrementTestCaseNumber(1);
    }
    uint64_t getPosition = 0;
    int32_t nbOfInputParams = 1;
    size_t nbOfElements = 1;
    SOPC_VariantValue valueCallRequest;
    SOPC_Variant* pVariantCallRequest = SOPC_Variant_Create();
    SOPC_ClientHelper_CallMethodResults_Clear(nbOfElements, pCallResults);

    valueCallRequest.Uint32 = fileHandle;
    SOPC_Initialize_VariantCallMethodRequest(pVariantCallRequest, SOPC_UInt32_Id, valueCallRequest);

    SOPC_Initialize_CallMethodRequest(pCallRequest, met_getposId, nbOfInputParams, pVariantCallRequest);
    int32_t statusMethodCall = SOPC_ClientHelper_CallMethod(connectionId, pCallRequest, nbOfElements, pCallResults);
    if (statusMethodCall < 0)
    {
#if TEST_DEBUG_FT
        printf("<TC_SOPC_FileTransfer_%03" PRId32 ": get position method failed.\n", SOPC_GetTestCaseNumber());
#else
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "<TC_SOPC_FileTransfer_%03" PRId32 ": get position method failed.\n",
                               SOPC_GetTestCaseNumber());
#endif
    }
    else
    {
        if (0 != pCallResults->status)
        {
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": get position method failed: Result code: 0x%08" PRIX32 "\n",
                   SOPC_GetTestCaseNumber(), pCallResults->status);
#else
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32
                                   ": set position method failed: Result code: 0x%08" PRIX32 "\n",
                                   SOPC_GetTestCaseNumber(), pCallResults->status);
#endif
            getPosition = 0xFF; // invalid value
        }
        else
        {
            SOPC_Variant* pVariantCallResult = pCallResults->outputParams;
            getPosition = pVariantCallResult->Value.Uint64;
#if TEST_DEBUG_FT
            printf("<TC_SOPC_FileTransfer_%03" PRId32 ": get position returned value: %" PRIu64 "\n",
                   SOPC_GetTestCaseNumber(), getPosition);
#else
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<TC_SOPC_FileTransfer_%03" PRId32 ": get position returned value: %" PRIu64 "\n",
                                   SOPC_GetTestCaseNumber(), getPosition);
#endif
        }
    }
    return getPosition;
}
