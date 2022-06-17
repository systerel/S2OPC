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
 * \brief Entry point for tests. Tests use libcheck.
 * https://libcheck.github.io/check/doc/check_html/check_3.html
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdlib.h> /* EXIT_* */

#include "check_helpers.h"

#include "sopc_atomic.h"
#include "sopc_time.h"

bool wait_value(int32_t* atomic, int32_t val)
{
    for (int i = 0; i < 100; ++i)
    {
        int32_t x = SOPC_Atomic_Int_Get(atomic);

        if (x == val)
        {
            return true;
        }

        SOPC_Sleep(10);
    }

    return false;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_make_suite_crypto_Aes256Sha256RsaPss());
    srunner_add_suite(sr, tests_make_suite_crypto_Aes128Sha256RsaOaep());
    srunner_add_suite(sr, tests_make_suite_crypto_B256S256());
    srunner_add_suite(sr, tests_make_suite_crypto_B256());
    srunner_add_suite(sr, tests_make_suite_crypto_None());
    srunner_add_suite(sr, tests_make_suite_crypto_PubSub256());
    srunner_add_suite(sr, tests_make_suite_crypto_tools());
    srunner_add_suite(sr, tests_make_suite_tools());
    srunner_add_suite(sr, tests_make_suite_threads());
    srunner_add_suite(sr, tests_make_suite_time());
    srunner_add_suite(sr, tests_make_suite_filesystem());
    srunner_add_suite(sr, tests_make_suite_timers());
    srunner_add_suite(sr, tests_make_suite_logger());
    srunner_add_suite(sr, tests_make_suite_dict(sr));
    srunner_add_suite(sr, tests_make_suite_array());
    srunner_add_suite(sr, tests_make_suite_event_handler());
    srunner_add_suite(sr, tests_make_suite_numeric_range());
    srunner_add_suite(sr, tests_make_suite_users());
    srunner_add_suite(sr, tests_make_suite_B_base_machines());
    srunner_add_suite(sr, tests_make_suite_encodeable_types());
    srunner_add_suite(sr, tests_make_suite_XML_parsers());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
