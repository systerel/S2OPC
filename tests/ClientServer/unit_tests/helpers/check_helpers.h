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
 * \brief Tests suites are gathered here.
 * Inspired from https://github.com/libcheck/check/blob/master/tests/check_check.h
 */

#ifndef CHECK_HELPERS_H
#define CHECK_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include <check.h>

bool wait_value(int32_t* atomic, int32_t val);

Suite* tests_make_suite_crypto_Aes256Sha256RsaPss(void);
Suite* tests_make_suite_crypto_Aes128Sha256RsaOaep(void);
Suite* tests_make_suite_crypto_B256S256(void);
Suite* tests_make_suite_crypto_B256(void);
Suite* tests_make_suite_crypto_None(void);
Suite* tests_make_suite_crypto_PubSub256(void);
Suite* tests_make_suite_crypto_tools(void);

Suite* tests_make_suite_tools(void);

Suite* tests_make_suite_threads(void);

Suite* tests_make_suite_time(void);

Suite* tests_make_suite_filesystem(void);

Suite* tests_make_suite_timers(void);

Suite* tests_make_suite_logger(void);

Suite* tests_make_suite_dict(SRunner* sr);

Suite* tests_make_suite_array(void);

Suite* tests_make_suite_event_handler(void);

Suite* tests_make_suite_numeric_range(void);

Suite* tests_make_suite_users(void);

Suite* tests_make_suite_B_base_machines(void);

Suite* tests_make_suite_encodeable_types(void);

Suite* tests_make_suite_XML_parsers(void);

#endif // CHECK_HELPERS_H
