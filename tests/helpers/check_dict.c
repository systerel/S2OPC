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
#include <stdlib.h>
#include <string.h>

#include "check_helpers.h"

#include "sopc_dict.h"
#include "sopc_hash.h"

static uint64_t str_hash(const void* str)
{
    // We only hash on the first 3 chars to allow easy conflicts
    size_t hash_len = strlen(str);

    if (hash_len > 3)
    {
        hash_len = 3;
    }

    return SOPC_DJBHash((const uint8_t*) str, hash_len);
}

static bool str_equal(const void* a, const void* b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

static uint64_t uintptr_hash(const void* data)
{
    uintptr_t val = (uintptr_t) data;
    return SOPC_DJBHash((const uint8_t*) &val, sizeof(uintptr_t));
}

static bool direct_equal(const void* a, const void* b)
{
    return a == b;
}

START_TEST(test_dict_insert_get)
{
    bool found;
    SOPC_Dict* d = SOPC_Dict_Create(NULL, str_hash, str_equal, NULL, NULL);

    ck_assert_ptr_null(SOPC_Dict_Get(d, "Hello", &found));
    ck_assert(!found);
    ck_assert(SOPC_Dict_Insert(d, "Hello", "World"));
    ck_assert_str_eq("World", SOPC_Dict_Get(d, "Hello", &found));
    ck_assert(found);

    ck_assert(SOPC_Dict_Insert(d, "Bonjour", "Monde"));
    ck_assert_str_eq("Monde", SOPC_Dict_Get(d, "Bonjour", &found));
    ck_assert(found);
    ck_assert_str_eq("World", SOPC_Dict_Get(d, "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_insert_overwrite)
{
    bool found;
    SOPC_Dict* d = SOPC_Dict_Create(NULL, str_hash, str_equal, NULL, NULL);

    ck_assert(SOPC_Dict_Insert(d, "Hello", "World"));
    ck_assert_str_eq("World", SOPC_Dict_Get(d, "Hello", &found));
    ck_assert(found);

    ck_assert(SOPC_Dict_Insert(d, "Hello", "What ?"));
    ck_assert_str_eq("What ?", SOPC_Dict_Get(d, "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_bucket_conflict)
{
    SOPC_Dict* d = SOPC_Dict_Create(NULL, str_hash, str_equal, NULL, NULL);

    static char* k1 = "Hello";
    static char* k2 = "Helllllicopter";

    ck_assert(str_hash(k1) == str_hash(k2));

    ck_assert(SOPC_Dict_Insert(d, k1, "World"));
    ck_assert_str_eq("World", SOPC_Dict_Get(d, k1, NULL));

    ck_assert(SOPC_Dict_Insert(d, k2, "Flap flap"));
    ck_assert_str_eq("Flap flap", SOPC_Dict_Get(d, k2, NULL));

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_non_null_empty_key)
{
    SOPC_Dict* d = SOPC_Dict_Create((void*) UINTPTR_MAX, uintptr_hash, direct_equal, NULL, NULL);

    ck_assert(SOPC_Dict_Insert(d, (void*) 0, "Zero"));
    ck_assert(SOPC_Dict_Insert(d, (void*) 1, "One"));

    ck_assert_str_eq("Zero", SOPC_Dict_Get(d, (void*) 0, NULL));
    ck_assert_str_eq("One", SOPC_Dict_Get(d, (void*) 1, NULL));

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_free)
{
    SOPC_Dict* d = SOPC_Dict_Create(NULL, str_hash, str_equal, free, free);

    static const char key[] = "Hello";
    static const char value[] = "World";

    char* k = calloc(1 + sizeof(key), sizeof(char));
    memcpy(k, key, sizeof(key));

    char* v = calloc(1 + sizeof(value), sizeof(char));
    memcpy(v, value, sizeof(value));

    ck_assert(SOPC_Dict_Insert(d, k, v));

    // We can't really test that the memory is freed here, but our tests should
    // run with ASAN enabled, and it should catch memory leaks.
    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_resize)
{
    SOPC_Dict* d = SOPC_Dict_Create(NULL, uintptr_hash, direct_equal, NULL, NULL);

    // Check that many inserts cause a resize by inserting more values than
    // DICT_INITIAL_SIZE in sopc_dict.c

    for (uint32_t i = 1; i < 1024; ++i)
    {
        void* v = (void*) (uintptr_t) i;
        ck_assert(SOPC_Dict_Insert(d, v, v));
    }

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_get_key)
{
    SOPC_Dict* d = SOPC_Dict_Create(NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    static char* s = "Hello";
    ck_assert(SOPC_Dict_Insert(d, s, NULL));
    bool found;
    char* key = SOPC_Dict_GetKey(d, "Hello", &found);
    ck_assert(found);
    ck_assert_ptr_eq(s, key);

    key = SOPC_Dict_GetKey(d, "World", &found);
    ck_assert(!found);
    ck_assert_ptr_null(key);

    SOPC_Dict_Delete(d);
}
END_TEST

Suite* tests_make_suite_dict(void)
{
    Suite* s;
    TCase* tc_dict;

    s = suite_create("Dictionary tests");
    tc_dict = tcase_create("Dictionary");

    tcase_add_test(tc_dict, test_dict_insert_get);
    tcase_add_test(tc_dict, test_dict_insert_overwrite);
    tcase_add_test(tc_dict, test_dict_bucket_conflict);
    tcase_add_test(tc_dict, test_dict_non_null_empty_key);
    tcase_add_test(tc_dict, test_dict_free);
    tcase_add_test(tc_dict, test_dict_resize);
    tcase_add_test(tc_dict, test_dict_get_key);
    suite_add_tcase(s, tc_dict);

    return s;
}
