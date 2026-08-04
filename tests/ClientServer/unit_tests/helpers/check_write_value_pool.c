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
#include <string.h>

#include "check_helpers.h"

#include "util_write_value_pool.h"

/* Differential tests: SOPC_WriteValue_FastInitialize/FastClear must be byte-for-byte
 * equivalent to the generic OpcUa_WriteValue_Initialize/Clear they replace in the
 * MI notification hot path, so that a future change to the OpcUa_WriteValue struct
 * (regenerated field list) is caught here instead of silently diverging. */

START_TEST(test_write_value_fast_initialize_matches_generic)
{
    OpcUa_WriteValue wvGeneric;
    OpcUa_WriteValue wvFast;
    /* Poison both buffers identically first: Initialize must overwrite every field
     * it owns, and any byte left untouched (e.g. struct padding) will still match
     * between the two since both started from the same pattern. */
    memset(&wvGeneric, 0xAA, sizeof(wvGeneric));
    memset(&wvFast, 0xAA, sizeof(wvFast));

    OpcUa_WriteValue_Initialize(&wvGeneric);
    SOPC_WriteValue_FastInitialize(&wvFast);

    ck_assert_int_eq(0, memcmp(&wvGeneric, &wvFast, sizeof(OpcUa_WriteValue)));
}
END_TEST

START_TEST(test_write_value_fast_clear_matches_generic)
{
    OpcUa_WriteValue wvGeneric;
    OpcUa_WriteValue wvFast;
    /* Poison both identically first: struct padding is never written by Initialize,
     * Clear or the field assignments below, so without this the two stack slots'
     * leftover garbage would differ and produce a false-positive memcmp diff. */
    memset(&wvGeneric, 0xAA, sizeof(wvGeneric));
    memset(&wvFast, 0xAA, sizeof(wvFast));
    OpcUa_WriteValue_Initialize(&wvGeneric);
    SOPC_WriteValue_FastInitialize(&wvFast);

    /* Fill both identically with owned/dynamically-allocated content, to exercise
     * the real (de)allocating paths of NodeId/String/Variant Clear, not just a memset. */
    wvGeneric.NodeId.IdentifierType = SOPC_IdentifierType_String;
    wvGeneric.NodeId.Namespace = 1;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvGeneric.NodeId.Data.String, "MyNode"));
    wvFast.NodeId.IdentifierType = SOPC_IdentifierType_String;
    wvFast.NodeId.Namespace = 1;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvFast.NodeId.Data.String, "MyNode"));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvGeneric.IndexRange, "1:2,0:3"));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvFast.IndexRange, "1:2,0:3"));

    wvGeneric.AttributeId = 13;
    wvFast.AttributeId = 13;
    wvGeneric.writeByPassPermissionMask = 1;
    wvFast.writeByPassPermissionMask = 1;

    wvGeneric.Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    wvGeneric.Value.Value.BuiltInTypeId = SOPC_String_Id;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvGeneric.Value.Value.Value.String, "payload"));
    wvGeneric.Value.Status = 0x80000000; // Bad
    wvFast.Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    wvFast.Value.Value.BuiltInTypeId = SOPC_String_Id;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wvFast.Value.Value.Value.String, "payload"));
    wvFast.Value.Status = 0x80000000; // Bad

    OpcUa_WriteValue_Clear(&wvGeneric);
    SOPC_WriteValue_FastClear(&wvFast);

    ck_assert_int_eq(0, memcmp(&wvGeneric, &wvFast, sizeof(OpcUa_WriteValue)));
}
END_TEST

START_TEST(test_write_value_pool_alloc_fast_init_clear_free_roundtrip)
{
    /* Smoke test of the actual call pattern used in the MI notification hot path:
     * pool Alloc (initialized) + fill + ClearAndFree (which uses
     * FastFreeContent internally). Run under ASan/valgrind in CI to catch leaks. */
    SOPC_WriteValuePool_Initialize();

    /* Reference: generic initialization on zeroed memory (same padding as a pool block) */
    OpcUa_WriteValue wvRef;
    memset(&wvRef, 0, sizeof(wvRef));
    OpcUa_WriteValue_Initialize(&wvRef);

    OpcUa_WriteValue* wv = SOPC_WriteValuePool_Alloc();
    ck_assert_ptr_nonnull(wv);
    ck_assert_ptr_eq(&OpcUa_WriteValue_EncodeableType, wv->encodeableType);
    ck_assert_int_eq(-1, wv->IndexRange.Length);
    ck_assert_int_eq(0, memcmp(&wvRef, wv, sizeof(OpcUa_WriteValue)));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&wv->IndexRange, "0:9"));
    wv->Value.Value.BuiltInTypeId = SOPC_Int32_Id;
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    wv->Value.Value.Value.Int32 = 42;

    SOPC_WriteValuePool_ClearAndFree(wv);

    /* A reused pool block (freelist link and previous content) is initialized again */
    wv = SOPC_WriteValuePool_Alloc();
    ck_assert_ptr_nonnull(wv);
    ck_assert_int_eq(0, memcmp(&wvRef, wv, sizeof(OpcUa_WriteValue)));

    SOPC_WriteValuePool_ClearAndFree(wv);
}
END_TEST

Suite* tests_make_suite_write_value_pool(void)
{
    Suite* s = suite_create("WriteValue fast pool");
    TCase* tc = tcase_create("WriteValuePool");

    tcase_add_test(tc, test_write_value_fast_initialize_matches_generic);
    tcase_add_test(tc, test_write_value_fast_clear_matches_generic);
    tcase_add_test(tc, test_write_value_pool_alloc_fast_init_clear_free_roundtrip);

    suite_add_tcase(s, tc);
    return s;
}
