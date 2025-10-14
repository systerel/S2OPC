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
#include <stdio.h>

#include "check_helpers.h"
#include "opcua_statuscodes.h"
#include "sopc_helper_encode.h"
#include "sopc_mem_alloc.h"

START_TEST(test_decode_with_number)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "VGhpc0lzQUJhc2U2NFRlc3Q=";
    char* output = NULL;
    unsigned char* buffer = NULL;
    size_t stringLen = 0;
    size_t base64Len = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &stringLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(18, (int) stringLen);
    int result = memcmp(buffer, "ThisIsABase64Test", strlen("ThisIsABase64Test"));
    ck_assert_int_eq(0, result);
    /* We do not re-encode the extra '\0' */
    status = SOPC_HelperEncode_Base64((const SOPC_Byte*) buffer, stringLen - 1, (char**) &output, &base64Len);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int) strlen(input) + 1, (int) base64Len);
    ck_assert_str_eq(input, output);

    SOPC_Free(output);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_without_padding)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "Tm9QYWRkaW5n";
    char* output = NULL;
    unsigned char* buffer = NULL;
    size_t stringLen = 0;
    size_t base64Len = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &stringLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(10, (int) stringLen);
    int result = memcmp(buffer, "NoPadding", strlen("NoPadding"));
    ck_assert_int_eq(0, result);
    /* We do not re-encode the extra '\0' */
    status = SOPC_HelperEncode_Base64((const SOPC_Byte*) buffer, stringLen - 1, (char**) &output, &base64Len);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int) strlen(input) + 1, (int) base64Len);
    ck_assert_str_eq(input, output);

    SOPC_Free(output);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_with_one_pad_char)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "VGhpc0lzQVRlc3Q=";
    unsigned char* buffer = NULL;
    char* output = NULL;
    size_t stringLen = 0;
    size_t base64Len = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &stringLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(12, (int) stringLen);
    int result = memcmp(buffer, "ThisIsATest", stringLen);
    ck_assert_int_eq(0, result);
    /* We do not re-encode the extra '\0' */
    status = SOPC_HelperEncode_Base64((const SOPC_Byte*) buffer, stringLen - 1, (char**) &output, &base64Len);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int) strlen(input) + 1, (int) base64Len);
    ck_assert_str_eq(input, output);

    SOPC_Free(output);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_with_two_pad_char)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "VHdvUGFkZGluZw==";
    unsigned char* buffer = NULL;
    char* output = NULL;
    size_t stringLen = 0;
    size_t base64Len = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &stringLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(11, (int) stringLen);
    int result = memcmp(buffer, "TwoPadding", strlen("TwoPadding"));
    ck_assert_int_eq(0, result);
    /* We do not re-encode the extra '\0' */
    status = SOPC_HelperEncode_Base64((const SOPC_Byte*) buffer, stringLen - 1, (char**) &output, &base64Len);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int) strlen(input) + 1, (int) base64Len);
    ck_assert_str_eq(input, output);

    SOPC_Free(output);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_with_three_pad_char)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "abcde===";
    unsigned char* buffer = NULL;
    size_t outLen = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &outLen);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_char_after_pad_char)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "abc=de";
    unsigned char* buffer = NULL;
    size_t outLen = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &outLen);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_Free(buffer);
}
END_TEST

START_TEST(test_decode_char_after_pad_char_with_end_pad)
{
    SOPC_ReturnStatus status = 0;
    const char* input = "abc=de=";
    unsigned char* buffer = NULL;
    size_t outLen = 0;
    status = SOPC_HelperDecode_Base64(input, &buffer, &outLen);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_Free(buffer);
}
END_TEST

Suite* tests_make_suite_base64(void)
{
    Suite* s;
    TCase *tc_correct_format, *tc_wrong_format;

    s = suite_create("Base64");

    tc_correct_format = tcase_create("Correct Format");
    tcase_add_test(tc_correct_format, test_decode_without_padding);
    tcase_add_test(tc_correct_format, test_decode_with_one_pad_char);
    tcase_add_test(tc_correct_format, test_decode_with_two_pad_char);
    tcase_add_test(tc_correct_format, test_decode_with_number);
    suite_add_tcase(s, tc_correct_format);

    tc_wrong_format = tcase_create("Wrong Format");
    tcase_add_test(tc_wrong_format, test_decode_with_three_pad_char);
    tcase_add_test(tc_wrong_format, test_decode_char_after_pad_char);
    tcase_add_test(tc_wrong_format, test_decode_char_after_pad_char_with_end_pad);

    suite_add_tcase(s, tc_wrong_format);

    return s;
}
