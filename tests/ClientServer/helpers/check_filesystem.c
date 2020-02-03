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
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"

#include "sopc_filesystem.h"

START_TEST(test_mkdir)
{
    SOPC_FileSystem_CreationResult res = SOPC_FileSystem_mkdir("./tmp/subtmp");
    ck_assert(res == SOPC_FileSystem_Creation_Error_PathPrefixInvalid);
    res = SOPC_FileSystem_mkdir("./");
    ck_assert(res == SOPC_FileSystem_Creation_Error_PathAlreadyExists);
    res = SOPC_FileSystem_mkdir("./tmp");
    ck_assert(res == SOPC_FileSystem_Creation_OK);
    res = SOPC_FileSystem_mkdir("./tmp");
    ck_assert(res == SOPC_FileSystem_Creation_Error_PathAlreadyExists);
    res = SOPC_FileSystem_mkdir("./tmp/subtmp");
    ck_assert(res == SOPC_FileSystem_Creation_OK);
}
END_TEST

START_TEST(test_rmdir)
{
    SOPC_FileSystem_RemoveResult res = SOPC_FileSystem_rmdir("./tmp");
    ck_assert(res == SOPC_FileSystem_Remove_Error_PathNotEmpty);
    res = SOPC_FileSystem_rmdir("./tmp/subtmp");
    ck_assert(res == SOPC_FileSystem_Remove_OK);
    res = SOPC_FileSystem_rmdir("./tmp/subtmp");
    ck_assert(res == SOPC_FileSystem_Remove_Error_PathInvalid);
    res = SOPC_FileSystem_rmdir("./tmp");
    ck_assert(res == SOPC_FileSystem_Remove_OK);
}
END_TEST

Suite* tests_make_suite_filesystem(void)
{
    Suite* s;
    TCase *tc_mkdir, *tc_rmdir;

    s = suite_create("File system helper tests");
    tc_mkdir = tcase_create("mkdir");
    tcase_add_test(tc_mkdir, test_mkdir);
    suite_add_tcase(s, tc_mkdir);

    tc_rmdir = tcase_create("rmdir");
    tcase_add_test(tc_rmdir, test_rmdir);
    suite_add_tcase(s, tc_rmdir);

    return s;
}
