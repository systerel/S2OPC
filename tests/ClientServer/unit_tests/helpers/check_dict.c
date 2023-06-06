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

#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

static uint64_t str_hash(const uintptr_t str)
{
    // We only hash on the first 3 chars to allow easy conflicts
    size_t hash_len = strlen((const char*) str);

    if (hash_len > 3)
    {
        hash_len = 3;
    }

    return SOPC_DJBHash((const uint8_t*) str, hash_len);
}

static bool str_equal(const uintptr_t a, const uintptr_t b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

static void uintptr_t_free(uintptr_t data)
{
    SOPC_Free((void*) data);
}

static uint64_t uintptr_hash(const uintptr_t data)
{
    uintptr_t val = (uintptr_t) data;
    return SOPC_DJBHash((const uint8_t*) &val, sizeof(uintptr_t));
}

static bool direct_equal(const uintptr_t a, const uintptr_t b)
{
    return a == b;
}

START_TEST(test_dict_insert_get)
{
    bool found;
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);

    ck_assert_uint_eq(0, SOPC_Dict_Size(d));
    ck_assert_ptr_null((void*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(!found);
    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Bonjour", (uintptr_t) "Monde"));
    ck_assert_uint_eq(2, SOPC_Dict_Size(d));
    ck_assert_str_eq("Monde", (char*) SOPC_Dict_Get(d, (uintptr_t) "Bonjour", &found));
    ck_assert(found);
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_insert_overwrite)
{
    bool found;
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "What ?"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("What ?", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_bucket_conflict)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);

    static char* k1 = "Hello";
    static char* k2 = "Helllllicopter";

    ck_assert(str_hash((uintptr_t) k1) == str_hash((uintptr_t) k2));

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) k1, (uintptr_t) "World"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) k1, NULL));

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) k2, (uintptr_t) "Flap flap"));
    ck_assert_uint_eq(2, SOPC_Dict_Size(d));
    ck_assert_str_eq("Flap flap", (char*) SOPC_Dict_Get(d, (uintptr_t) k2, NULL));

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_non_null_empty_key)
{
    SOPC_Dict* d = SOPC_Dict_Create(UINTPTR_MAX, uintptr_hash, direct_equal, NULL, NULL);

    ck_assert(SOPC_Dict_Insert(d, 0, (uintptr_t) "Zero"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert(SOPC_Dict_Insert(d, 1, (uintptr_t) "One"));
    ck_assert_uint_eq(2, SOPC_Dict_Size(d));

    ck_assert_str_eq("Zero", (char*) SOPC_Dict_Get(d, 0, NULL));
    ck_assert_str_eq("One", (char*) SOPC_Dict_Get(d, 1, NULL));

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_free)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, uintptr_t_free);

    static const char key[] = "Hello";
    static const char value[] = "World";

    char* k = SOPC_Calloc(1 + sizeof(key), sizeof(char));
    memcpy(k, key, sizeof(key));

    char* v = SOPC_Calloc(1 + sizeof(value), sizeof(char));
    memcpy(v, value, sizeof(value));

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) k, (uintptr_t) v));

    // We can't really test that the memory is freed here, but our tests should
    // run with ASAN enabled, and it should catch memory leaks.
    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_resize)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, uintptr_hash, direct_equal, NULL, NULL);

    // Check that many inserts cause a resize by inserting more values than
    // DICT_INITIAL_SIZE in sopc_dict.c

    size_t initial_cap = SOPC_Dict_Capacity(d);

    for (uint32_t i = 1; i < 1024; ++i)
    {
        ck_assert(SOPC_Dict_Insert(d, i, i));
    }

    ck_assert_uint_gt(SOPC_Dict_Capacity(d), initial_cap);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_get_key)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    static char* s = "Hello";
    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) s, (uintptr_t) NULL));
    bool found;
    char* key = (char*) SOPC_Dict_GetKey(d, (uintptr_t) "Hello", &found);
    ck_assert(found);
    ck_assert_ptr_eq(s, key);

    key = (char*) SOPC_Dict_GetKey(d, (uintptr_t) "World", &found);
    ck_assert(!found);
    ck_assert_ptr_null(key);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_remove_no_tombstone)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    SOPC_Dict_Remove(d, (uintptr_t) "Hello"); // Should assert

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_set_tombstone_late)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    SOPC_Dict_SetTombstoneKey(d, 0x01); // Should assert

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_remove)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    SOPC_Dict_SetTombstoneKey(d, 0x01);

    bool found;

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Remove(d, (uintptr_t) "Other");
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Remove(d, (uintptr_t) "Hello");
    ck_assert_uint_eq(0, SOPC_Dict_Size(d));
    ck_assert_ptr_null((void*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(!found);

    ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
    ck_assert_uint_eq(1, SOPC_Dict_Size(d));
    ck_assert_str_eq("World", (char*) SOPC_Dict_Get(d, (uintptr_t) "Hello", &found));
    ck_assert(found);

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_tombstone_reuse)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    SOPC_Dict_SetTombstoneKey(d, 0x01);

    size_t initial_cap = SOPC_Dict_Capacity(d);

    for (int i = 0; i < 1024; ++i)
    {
        ck_assert(SOPC_Dict_Insert(d, (uintptr_t) "Hello", (uintptr_t) "World"));
        SOPC_Dict_Remove(d, (uintptr_t) "Hello");
    }

    ck_assert_uint_eq(0, SOPC_Dict_Size(d));
    ck_assert_uint_eq(initial_cap, SOPC_Dict_Capacity(d));

    SOPC_Dict_Delete(d);
}
END_TEST

START_TEST(test_dict_compact)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, uintptr_hash, direct_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    SOPC_Dict_SetTombstoneKey(d, 0x01);

    size_t initial_cap = SOPC_Dict_Capacity(d);

    for (int i = 0; i < 1024; ++i)
    {
        ck_assert(SOPC_Dict_Insert(d, (uintptr_t)(i + 2), (uintptr_t) NULL));
    }

    ck_assert_uint_gt(SOPC_Dict_Capacity(d), initial_cap);

    for (int i = 0; i < 1024; ++i)
    {
        SOPC_Dict_Remove(d, (uintptr_t)(i + 2));
    }

    ck_assert_uint_eq(0, SOPC_Dict_Size(d));
    ck_assert_uint_eq(initial_cap, SOPC_Dict_Capacity(d));

    SOPC_Dict_Delete(d);
}
END_TEST

static void dict_callback_increment_u32(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_UNUSED_ARG(key);
    SOPC_UNUSED_ARG(value);

    uint32_t* val = (uint32_t*) user_data;
    ++(*val);
}

START_TEST(test_dict_foreach_empty)
{
    SOPC_Dict* d = SOPC_Dict_Create((uintptr_t) NULL, uintptr_hash, direct_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    uint32_t iteration_counter = 0;
    SOPC_Dict_ForEach(d, dict_callback_increment_u32, (uintptr_t) &iteration_counter);
    SOPC_Dict_Delete(d);
    ck_assert_uint_eq(0, iteration_counter);
}
END_TEST

typedef struct
{
    size_t size;
    bool* keys_iterated;
    bool* values_iterated;
} foreach_data_t;

static void dict_callback_mark(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    foreach_data_t* cb_data = (foreach_data_t*) user_data;
    size_t _key = (size_t) key;
    size_t _value = (size_t) value;

    ck_assert_uint_lt(_key, cb_data->size);
    ck_assert_uint_lt(_value, cb_data->size);
    cb_data->keys_iterated[_key] = true;
    cb_data->values_iterated[_value] = true;
}

START_TEST(test_dict_foreach)
{
    SOPC_Dict* d = SOPC_Dict_Create(UINTPTR_MAX, uintptr_hash, direct_equal, NULL, NULL);
    ck_assert_ptr_nonnull(d);

    ck_assert(SOPC_Dict_Insert(d, 0, 2));
    ck_assert(SOPC_Dict_Insert(d, 1, 1));
    ck_assert(SOPC_Dict_Insert(d, 2, 0));

    bool keys_iterated[3];
    bool values_iterated[3];

    foreach_data_t cb_data = {
        .size = 3,
        .keys_iterated = keys_iterated,
        .values_iterated = values_iterated,
    };

    SOPC_Dict_ForEach(d, dict_callback_mark, (uintptr_t) &cb_data);
    SOPC_Dict_Delete(d);

    for (size_t i = 0; i < 3; ++i)
    {
        ck_assert(keys_iterated[i]);
        ck_assert(values_iterated[i]);
    }
}
END_TEST

Suite* tests_make_suite_dict(SRunner* sr)
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

    if (srunner_fork_status(sr) != CK_NOFORK)
    {
        // 6 == SIGABRT
        tcase_add_test_raise_signal(tc_dict, test_dict_remove_no_tombstone, 6);
        tcase_add_test_raise_signal(tc_dict, test_dict_set_tombstone_late, 6);
    }

    tcase_add_test(tc_dict, test_dict_remove);
    tcase_add_test(tc_dict, test_dict_tombstone_reuse);
    tcase_add_test(tc_dict, test_dict_compact);
    tcase_add_test(tc_dict, test_dict_foreach_empty);
    tcase_add_test(tc_dict, test_dict_foreach);
    suite_add_tcase(s, tc_dict);

    return s;
}
