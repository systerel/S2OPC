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

#include <assert.h>
#include <check.h>
#include <inttypes.h>
#include <stdio.h>

#include "check_helpers.h"
#include "sopc_array.h"
#include "sopc_mem_alloc.h"
#include "sopc_numeric_range.h"

static void str_append(SOPC_Array* str, const char* text)
{
    size_t len = strlen(text);
    ck_assert(SOPC_Array_Append_Values(str, text, len));
}

static char* format_numeric_range(const SOPC_NumericRange* range)
{
    SOPC_Array* str = SOPC_Array_Create(sizeof(char), 1, NULL);
    assert(str != NULL);

    for (size_t i = 0; i < range->n_dimensions; ++i)
    {
        char buf[21];
        SOPC_Dimension* dim = &range->dimensions[i];

        if (i > 0)
        {
            str_append(str, ",");
        }

        if (dim->start == dim->end)
        {
            snprintf(buf, sizeof(buf), "%" PRIu32, dim->start);
            str_append(str, buf);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%" PRIu32, dim->start);
            str_append(str, buf);
            str_append(str, ":");
            snprintf(buf, sizeof(buf), "%" PRIu32, dim->end);
            str_append(str, buf);
        }
    }

    char end = '\0';
    ck_assert(SOPC_Array_Append(str, end));

    return SOPC_Array_Into_Raw(str);
}

START_TEST(test_numeric_range_parser)
{
    struct
    {
        const char* input;
        SOPC_ReturnStatus result;
    } test_data[] = {{
                         .input = "",
                         .result = SOPC_STATUS_NOK,
                     },
                     {
                         .input = "5",
                         .result = SOPC_STATUS_OK,
                     },
                     {
                         .input = "5:7",
                         .result = SOPC_STATUS_OK,
                     },
                     {
                         .input = "7:7",
                         .result = SOPC_STATUS_NOK,
                     },
                     {
                         .input = "8:7",
                         .result = SOPC_STATUS_NOK,
                     },
                     {
                         .input = "3,1,7",
                         .result = SOPC_STATUS_OK,
                     },
                     {
                         .input = "1:2,2:3,9",
                         .result = SOPC_STATUS_OK,
                     },
                     {
                         .input = "4294967295,4294967295",
                         .result = SOPC_STATUS_OK,
                     },
                     {
                         .input = "4294967296",
                         .result = SOPC_STATUS_NOK,
                     },
                     {.input = NULL}};

    for (size_t i = 0; test_data[i].input != NULL; ++i)
    {
        SOPC_NumericRange* result = NULL;
        SOPC_ReturnStatus status = SOPC_NumericRange_Parse(test_data[i].input, &result);
        ck_assert_uint_eq(test_data[i].result, status);

        if (status == SOPC_STATUS_OK)
        {
            ck_assert_ptr_nonnull(result);
            char* str = format_numeric_range(result);
            ck_assert_str_eq(test_data[i].input, str);
            SOPC_Free(str);
        }
        else
        {
            ck_assert_ptr_null(result);
        }

        SOPC_NumericRange_Delete(result);
    }
}
END_TEST

Suite* tests_make_suite_numeric_range(void)
{
    Suite* s = suite_create("Numeric range parser");
    TCase* c;

    c = tcase_create("Numeric range parser");
    tcase_add_test(c, test_numeric_range_parser);
    suite_add_tcase(s, c);

    return s;
}
