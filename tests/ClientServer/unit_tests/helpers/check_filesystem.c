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
#include <string.h>

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

static bool find_sub_path(const char* pFindSubPath, SOPC_Array* pArray)
{
    ck_assert(NULL != pFindSubPath && NULL != pArray);
    bool find = false;
    char* pFilePath = NULL;
    char* pStart = NULL;
    size_t nbFiles = SOPC_Array_Size(pArray);
    for (size_t idx = 0; idx < nbFiles; idx++)
    {
        pFilePath = SOPC_Array_Get(pArray, char*, idx);
        pStart = strstr(pFilePath, pFindSubPath);
        if (NULL != pStart)
        {
            find = true;
        }
    }
    return find;
}

START_TEST(test_get_dir_file_paths)
{
    SOPC_Array* pFilePaths = NULL;
    /* Invalid parameters */
    SOPC_FileSystem_GetDirResult getRes = SOPC_FileSystem_GetDirFilePaths(NULL, NULL);
    ck_assert(getRes == SOPC_FileSystem_GetDir_Error_InvalidParameters);
    getRes = SOPC_FileSystem_GetDirFilePaths("./test_get_dir_file_paths", NULL);
    ck_assert(getRes == SOPC_FileSystem_GetDir_Error_InvalidParameters);
    getRes = SOPC_FileSystem_GetDirFilePaths(NULL, &pFilePaths);
    ck_assert(getRes == SOPC_FileSystem_GetDir_Error_InvalidParameters);
    /* Invalid path */
    getRes = SOPC_FileSystem_GetDirFilePaths("./test_get_dir_file_paths", &pFilePaths);
    ck_assert(getRes == SOPC_FileSystem_GetDir_Error_PathInvalid);
    /* Empty folder */
    SOPC_FileSystem_CreationResult creationRes = SOPC_FileSystem_mkdir("./test_get_dir_file_paths");
    ck_assert(creationRes == SOPC_FileSystem_Creation_OK);
    getRes = SOPC_FileSystem_GetDirFilePaths("./test_get_dir_file_paths", &pFilePaths);
    ck_assert(getRes == SOPC_FileSystem_GetDir_OK);
    ck_assert(NULL != pFilePaths);
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    ck_assert(0 == nbFiles);
    SOPC_Array_Delete(pFilePaths);
    /* Creates files */
    FILE* file_1 = fopen("./test_get_dir_file_paths/file_1.txt", "w+");
    ck_assert(NULL != file_1);
    FILE* file_2 = fopen("./test_get_dir_file_paths/file_2.txt", "w+");
    ck_assert(NULL != file_2);
    fclose(file_1);
    fclose(file_2);
    /* Retrieves them */
    getRes = SOPC_FileSystem_GetDirFilePaths("./test_get_dir_file_paths", &pFilePaths);
    ck_assert(getRes == SOPC_FileSystem_GetDir_OK);
    ck_assert(NULL != pFilePaths);
    nbFiles = SOPC_Array_Size(pFilePaths);
    ck_assert(2 == nbFiles);
    bool find_file_1 = find_sub_path("/test_get_dir_file_paths/file_1.txt", pFilePaths);
    ck_assert(true == find_file_1);
    bool find_file_2 = find_sub_path("/test_get_dir_file_paths/file_2.txt", pFilePaths);
    ck_assert(true == find_file_2);
    /* Clear */
    SOPC_Array_Delete(pFilePaths);
    ck_assert(0 == remove("./test_get_dir_file_paths/file_1.txt"));
    ck_assert(0 == remove("./test_get_dir_file_paths/file_2.txt"));
    SOPC_FileSystem_RemoveResult rmdirRes = SOPC_FileSystem_rmdir("./test_get_dir_file_paths");
    ck_assert(rmdirRes == SOPC_FileSystem_Remove_OK);
}
END_TEST

Suite* tests_make_suite_filesystem(void)
{
    Suite* s;
    TCase *tc_mkdir, *tc_rmdir, *tc_get_dir_file_paths;

    s = suite_create("File system helper tests");
    tc_mkdir = tcase_create("mkdir");
    tcase_add_test(tc_mkdir, test_mkdir);
    suite_add_tcase(s, tc_mkdir);

    tc_rmdir = tcase_create("rmdir");
    tcase_add_test(tc_rmdir, test_rmdir);
    suite_add_tcase(s, tc_rmdir);

    tc_get_dir_file_paths = tcase_create("get_dir_file_paths");
    tcase_add_test(tc_get_dir_file_paths, test_get_dir_file_paths);
    suite_add_tcase(s, tc_get_dir_file_paths);

    return s;
}
