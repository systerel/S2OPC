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

/** \file
 *
 * \brief Unit tests of the method call manager: association of C functions to method NodeIds
 *        (add, replace, call, remove) and lifetime of the associated parameter (shared by the
 *        associations of SOPC_MethodCallManager_AddMethodWithType, kept during ongoing calls).
 */

#include "check_helpers.h"

#include <check.h>

#include "sopc_builtintypes.h"
#include "sopc_call_method_manager.h"
#include "sopc_macros.h"

static const SOPC_NodeId methodInstId = SOPC_NODEID_NUMERIC(1, 1000);
static const SOPC_NodeId methodTypeId = SOPC_NODEID_NUMERIC(1, 2000);
static const SOPC_NodeId unknownMethodId = SOPC_NODEID_NUMERIC(1, 3000);

/* Method parameter recording the method calls and the parameter frees */
typedef struct
{
    int nbCalls;
    int lastMethod; // 1 or 2: identifies the called method function
    int nbFreed;
    // When set, the method removes removeId association during the call
    SOPC_MethodCallManager* removeFromMcm;
    const SOPC_NodeId* removeId;
    bool freedDuringCall;
} TestParam;

static SOPC_StatusCode test_method(void* param, int methodNum)
{
    TestParam* testParam = (TestParam*) param;
    if (NULL != testParam)
    {
        testParam->nbCalls++;
        testParam->lastMethod = methodNum;
        if (NULL != testParam->removeFromMcm)
        {
            SOPC_ReturnStatus status =
                SOPC_MethodCallManager_RemoveMethod(testParam->removeFromMcm, testParam->removeId);
            SOPC_UNUSED_RESULT(status);
            // The parameter shall not be freed while the call is ongoing
            testParam->freedDuringCall = (0 != testParam->nbFreed);
        }
    }
    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode method_func_1(const SOPC_CallContext* callContextPtr,
                                     const SOPC_NodeId* objectId,
                                     uint32_t nbInputArgs,
                                     const SOPC_Variant* inputArgs,
                                     uint32_t* nbOutputArgs,
                                     SOPC_Variant** outputArgs,
                                     void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    return test_method(param, 1);
}

static SOPC_StatusCode method_func_2(const SOPC_CallContext* callContextPtr,
                                     const SOPC_NodeId* objectId,
                                     uint32_t nbInputArgs,
                                     const SOPC_Variant* inputArgs,
                                     uint32_t* nbOutputArgs,
                                     SOPC_Variant** outputArgs,
                                     void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    return test_method(param, 2);
}

// Parameter free function: counts the frees of the TestParam
static void param_free_count(void* data)
{
    ((TestParam*) data)->nbFreed++;
}

// Calls the method without arguments, returns true if the method was found
static bool call_method(SOPC_MethodCallManager* mcm, const SOPC_NodeId* methodId)
{
    uint32_t nbOutputArgs = 0;
    SOPC_Variant* outputArgs = NULL;
    SOPC_StatusCode callStatus = SOPC_BadStatusMask;
    bool found =
        SOPC_MethodCallManager_CallMethod(mcm, methodId, NULL, NULL, 0, NULL, &nbOutputArgs, &outputArgs, &callStatus);
    ck_assert(!found || SOPC_GoodGenericStatus == callStatus);
    return found;
}

START_TEST(test_mcm_add_call_remove)
{
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    ck_assert_ptr_nonnull(mcm);
    TestParam testParam = {0};

    ck_assert(!call_method(mcm, &methodInstId));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_AddMethod(mcm, &methodInstId, &method_func_1, &testParam,
                                                                      &param_free_count));
    ck_assert(call_method(mcm, &methodInstId));
    ck_assert_int_eq(1, testParam.nbCalls);
    ck_assert_int_eq(1, testParam.lastMethod);
    ck_assert(!call_method(mcm, &unknownMethodId));

    // Remove: param freed by its free function, method not found anymore
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_RemoveMethod(mcm, &methodInstId));
    ck_assert_int_eq(1, testParam.nbFreed);
    ck_assert(!call_method(mcm, &methodInstId));

    // Second remove and unknown method
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, SOPC_MethodCallManager_RemoveMethod(mcm, &methodInstId));
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, SOPC_MethodCallManager_RemoveMethod(mcm, &unknownMethodId));
    ck_assert_int_eq(1, testParam.nbFreed);

    // Add again after removal (without parameter)
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_AddMethod(mcm, &methodInstId, &method_func_2, NULL, NULL));
    ck_assert(call_method(mcm, &methodInstId));

    SOPC_MethodCallManager_Free(mcm);
    ck_assert_int_eq(1, testParam.nbFreed);
    ck_assert_int_eq(1, testParam.nbCalls);
}
END_TEST

START_TEST(test_mcm_replace)
{
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    ck_assert_ptr_nonnull(mcm);
    TestParam testParam1 = {0};
    TestParam testParam2 = {0};

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_AddMethod(mcm, &methodInstId, &method_func_1, &testParam1,
                                                                      &param_free_count));
    // Replacement frees the previous parameter
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_AddMethod(mcm, &methodInstId, &method_func_2, &testParam2,
                                                                      &param_free_count));
    ck_assert_int_eq(1, testParam1.nbFreed);
    ck_assert_int_eq(0, testParam2.nbFreed);

    ck_assert(call_method(mcm, &methodInstId));
    ck_assert_int_eq(0, testParam1.nbCalls);
    ck_assert_int_eq(1, testParam2.nbCalls);
    ck_assert_int_eq(2, testParam2.lastMethod);

    // Free of the manager frees remaining parameters
    SOPC_MethodCallManager_Free(mcm);
    ck_assert_int_eq(1, testParam1.nbFreed);
    ck_assert_int_eq(1, testParam2.nbFreed);
}
END_TEST

START_TEST(test_mcm_add_with_type_shared_param)
{
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    ck_assert_ptr_nonnull(mcm);
    TestParam testParam = {0};

    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_MethodCallManager_AddMethodWithType(mcm, &methodInstId, &methodTypeId, &method_func_1,
                                                              &testParam, &param_free_count));
    ck_assert(call_method(mcm, &methodInstId));
    ck_assert(call_method(mcm, &methodTypeId));
    ck_assert_int_eq(2, testParam.nbCalls);

    // Parameter shared by both associations: only freed once both are removed
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_RemoveMethod(mcm, &methodInstId));
    ck_assert_int_eq(0, testParam.nbFreed);
    ck_assert(!call_method(mcm, &methodInstId));
    ck_assert(call_method(mcm, &methodTypeId));
    ck_assert_int_eq(3, testParam.nbCalls);
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_RemoveMethod(mcm, &methodTypeId));
    ck_assert_int_eq(1, testParam.nbFreed);

    // Free of the manager with both associations: parameter freed once
    TestParam testParam2 = {0};
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_MethodCallManager_AddMethodWithType(mcm, &methodInstId, &methodTypeId, &method_func_2,
                                                              &testParam2, &param_free_count));
    SOPC_MethodCallManager_Free(mcm);
    ck_assert_int_eq(1, testParam2.nbFreed);
    ck_assert_int_eq(1, testParam.nbFreed);
}
END_TEST

START_TEST(test_mcm_remove_during_call)
{
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    ck_assert_ptr_nonnull(mcm);
    // The method removes its own association during the call
    TestParam testParam = {.removeFromMcm = mcm, .removeId = &methodInstId};

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_MethodCallManager_AddMethod(mcm, &methodInstId, &method_func_1, &testParam,
                                                                      &param_free_count));
    ck_assert(call_method(mcm, &methodInstId));
    ck_assert_int_eq(1, testParam.nbCalls);
    // Parameter kept during the call, freed once the call returned
    ck_assert(!testParam.freedDuringCall);
    ck_assert_int_eq(1, testParam.nbFreed);
    ck_assert(!call_method(mcm, &methodInstId));

    SOPC_MethodCallManager_Free(mcm);
    ck_assert_int_eq(1, testParam.nbFreed);
}
END_TEST

START_TEST(test_mcm_invalid_parameters)
{
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    ck_assert_ptr_nonnull(mcm);
    TestParam testParam = {0};
    uint32_t nbOutputArgs = 0;
    SOPC_Variant* outputArgs = NULL;
    SOPC_StatusCode callStatus = SOPC_GoodGenericStatus;

    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_MethodCallManager_RemoveMethod(NULL, &methodInstId));
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_MethodCallManager_RemoveMethod(mcm, NULL));
    ck_assert(!SOPC_MethodCallManager_CallMethod(NULL, &methodInstId, NULL, NULL, 0, NULL, &nbOutputArgs, &outputArgs,
                                                 &callStatus));
    ck_assert(
        !SOPC_MethodCallManager_CallMethod(mcm, NULL, NULL, NULL, 0, NULL, &nbOutputArgs, &outputArgs, &callStatus));
    ck_assert(
        !SOPC_MethodCallManager_CallMethod(mcm, &methodInstId, NULL, NULL, 0, NULL, &nbOutputArgs, &outputArgs, NULL));

    // Invalid combined association: none of the NodeIds associated, parameter not freed
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_MethodCallManager_AddMethodWithType(mcm, NULL, &methodTypeId, &method_func_1, &testParam,
                                                              &param_free_count));
    ck_assert(!call_method(mcm, &methodTypeId));
    ck_assert_int_eq(0, testParam.nbFreed);

    SOPC_MethodCallManager_Free(mcm);
    ck_assert_int_eq(0, testParam.nbFreed);
}
END_TEST

Suite* tests_make_suite_call_method_manager(void)
{
    Suite* s = suite_create("Method call manager");
    TCase* tc = tcase_create("Method call manager");
    tcase_add_test(tc, test_mcm_add_call_remove);
    tcase_add_test(tc, test_mcm_replace);
    tcase_add_test(tc, test_mcm_add_with_type_shared_param);
    tcase_add_test(tc, test_mcm_remove_during_call);
    tcase_add_test(tc, test_mcm_invalid_parameters);
    suite_add_tcase(s, tc);
    return s;
}
