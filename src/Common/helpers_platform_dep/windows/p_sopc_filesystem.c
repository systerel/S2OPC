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

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

#include <direct.h>
#include <errno.h>
#include <windows.h>

SOPC_FileSystem_CreationResult SOPC_FileSystem_mkdir(const char* directoryPath)
{
    int res = _mkdir(directoryPath);
    if (res == 0)
    {
        return SOPC_FileSystem_Creation_OK;
    }
    else if (res == -1)
    {
        switch (errno)
        {
        case ENOENT:
            return SOPC_FileSystem_Creation_Error_PathPrefixInvalid;
        case EEXIST:
            return SOPC_FileSystem_Creation_Error_PathAlreadyExists;
        case EACCES:
            return SOPC_FileSystem_Creation_Error_PathPermisionDenied;
        default:
            return SOPC_FileSystem_Creation_Error_PathResolutionIssue;
        }
    }
    else
    {
        return SOPC_FileSystem_Creation_Error_UnknownIssue;
    }
}

SOPC_FileSystem_RemoveResult SOPC_FileSystem_rmdir(const char* directoryPath)
{
    int res = _rmdir(directoryPath);
    if (res == 0)
    {
        return SOPC_FileSystem_Remove_OK;
    }
    else if (res == -1)
    {
        switch (errno)
        {
        case ENOENT:
            return SOPC_FileSystem_Remove_Error_PathInvalid;
        case ENOTEMPTY:
            return SOPC_FileSystem_Remove_Error_PathNotEmpty;
        case EACCES:
            return SOPC_FileSystem_Remove_Error_PathPermisionDenied;
        default:
            return SOPC_FileSystem_Remove_Error_UnknownIssue;
        }
    }
    else
    {
        return SOPC_FileSystem_Remove_Error_UnknownIssue;
    }
}

static SOPC_ReturnStatus get_file_path(const char* pDirectoryPath, const char* pFileName, char** ppFilePath)
{
    SOPC_ASSERT(NULL != pDirectoryPath && NULL != pFileName && NULL != ppFilePath);

    char* tmp = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(pDirectoryPath, "/", &tmp);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StrConcat(tmp, pFileName, ppFilePath);
    }
    SOPC_Free(tmp);
    return status;
}

static bool is_regular_file(WIN32_FIND_DATA fileData)
{
    bool result = false;
    /* Only keep rugular file */
    if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        result = true;
    }
    return result;
}

static void SOPC_Free_CstringFromPtr(void* data)
{
    if (NULL != data)
    {
        SOPC_Free(*(char**) data);
    }
}

/* if isName is set to True then ppFileInfos is an array of names else of paths. */
static SOPC_FileSystem_GetDirResult get_dir_files_infos(const char* directoryPath,
                                                        SOPC_Array** ppFileInfos,
                                                        const bool bIsName)
{
    SOPC_ASSERT(NULL != directoryPath && NULL != ppFileInfos);
    SOPC_Array* pFileInfos = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    HANDLE fd = 0;
    WIN32_FIND_DATA fileData = {0};
    char* pFileInfo = NULL;
    bool bIsRegular = false;
    bool bResAppend = false;

    char* winPath = NULL;
    status = SOPC_StrConcat(directoryPath, "\\*", &winPath);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_FileSystem_GetDir_Error_UnknownIssue;
    }
    fd = FindFirstFile(winPath, &fileData);
    if (INVALID_HANDLE_VALUE == fd)
    {
        SOPC_Free(winPath);
        return SOPC_FileSystem_GetDir_Error_PathInvalid;
    }
    /* Create the array to store the informations */
    pFileInfos = SOPC_Array_Create(sizeof(char*), 0, SOPC_Free_CstringFromPtr);
    if (NULL == pFileInfos)
    {
        status = SOPC_STATUS_NOK;
    }
    /* Read before while */
    int res = FindNextFile(fd, &fileData);
    while (0 != res && SOPC_STATUS_OK == status)
    {
        /* We don't want sub folders! Is it a regular file? */
        bIsRegular = is_regular_file(fileData);
        /* Only keep rugular file */
        if (!bIsRegular)
        {
            /* Next iteration */
            res = FindNextFile(fd, &fileData);
            continue;
        }
        /* Prepare the C string to store in the array */
        if (bIsName)
        {
            /* FILE NAME */
            pFileInfo = SOPC_strdup(fileData.cFileName);
        }
        else
        {
            /* FILE PATH */
            status = get_file_path(directoryPath, fileData.cFileName, &pFileInfo);
        }
        /* Append the fileName or the filePath to the array */
        if (SOPC_STATUS_OK == status)
        {
            bResAppend = SOPC_Array_Append(pFileInfos, pFileInfo);
            status = bResAppend ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }
        /* Next iteration */
        res = FindNextFile(fd, &fileData);
    }
    /* Close after while */
    FindClose(fd);
    /* Clear */
    SOPC_Free(winPath);
    if (SOPC_STATUS_OK != status)
    {
        /* Clear */
        SOPC_Free(pFileInfo);
        SOPC_Array_Delete(pFileInfos);
        *ppFileInfos = NULL;
        return SOPC_FileSystem_GetDir_Error_UnknownIssue;
    }

    *ppFileInfos = pFileInfos;
    return SOPC_FileSystem_GetDir_OK;
}

SOPC_FileSystem_GetDirResult SOPC_FileSystem_GetDirFilePaths(const char* directoryPath, SOPC_Array** ppFilePaths)
{
    if (NULL == directoryPath || NULL == ppFilePaths)
    {
        return SOPC_FileSystem_GetDir_Error_InvalidParameters;
    }

    SOPC_FileSystem_GetDirResult res = get_dir_files_infos(directoryPath, ppFilePaths, false);

    return res;
}
