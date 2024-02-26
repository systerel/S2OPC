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

#include "sopc_circular_log_file.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

// Keep 100 bytes to print log file change
#define RESERVED_BYTES_PRINT_FILE_CHANGE 100

#if SOPC_HAS_FILESYSTEM

#define FILE_T FILE
#define FILE_OPEN(f) fopen((f), "w")
#define FILE_FLUSH(f) fflush(f)
#define FILE_CLOSE(f) fclose(f)
#define FILE_SET_LINE_BUFF_MODE(f) setvbuf((f), NULL, _IOLBF, BUFSIZ)

#else //   not SOPC_HAS_FILESYSTEM
#define FILE_T void
#define FILE_OPEN(f) (NULL)
#define FILE_FLUSH(f)
#define FILE_CLOSE(f)
#define FILE_SET_LINE_BUFF_MODE(f)

#endif // not  SOPC_HAS_FILESYSTEM

struct SOPC_CircularLogFile
{
    char* filePath;
    uint8_t fileNumberPos; // Position of number in filePath
    FILE_T* pFile;
    uint32_t nbBytes;
    uint16_t nbFiles;
    uint32_t maxBytes; // New file created when maxBytes reached
    uint16_t maxFiles;
};

/** Create the directory if not existent.
 * @return true in case of success*/
static void checkOrCreateDirectory(const char* path)
{
    if (NULL != path)
    {
        if (path[0] == 0)
        {
            // Use current folder
            path = "./";
        }
        const SOPC_FileSystem_CreationResult mkdirRes = SOPC_FileSystem_mkdir(path);
        if (!(mkdirRes == SOPC_FileSystem_Creation_OK || mkdirRes == SOPC_FileSystem_Creation_Error_PathAlreadyExists))
        {
            fprintf(stderr, "WARNING: Cannot create log directory ('%d'), defaulting to current directory\n", mkdirRes);
            SOPC_ASSERT(false);
        }
    }
}

SOPC_CircularLogFile* SOPC_CircularLogFile_Create(const SOPC_CircularLogFile_Configuration* config)
{
    FILE_T* hFile = NULL;
    struct SOPC_CircularLogFile* file = NULL;
    if (NULL != config && config->logDirPath != NULL && config->logFileName != NULL &&
        strlen(config->logFileName) > 0 && config->logMaxBytes > RESERVED_BYTES_PRINT_FILE_CHANGE &&
        config->logMaxFiles > 0)
    {
        // First, create path if not existent
        checkOrCreateDirectory(config->logDirPath);
        const char* logDirPath = (*config->logDirPath != 0) ? config->logDirPath : ".";

        // filenames are <path>/<name>_XXXXX.log
        const size_t filenameLen = strlen(logDirPath) + strlen(config->logFileName) +
                                   12; // 1 for / + 1 for _ + 5 for file number + 4 for file extension + 1 for '\0'
        if (filenameLen <= UINT8_MAX)
        {
            file = SOPC_Calloc(1, sizeof(*file));
        }

        if (file != NULL)
        {
            file->pFile = NULL;
            file->nbFiles = 0;
            // Filename: <path>/<name>_12345.log :  digits start at filename end -9, but
            // filenameLen is 1 + strlen(Filename). So first digits position is strlen(Filename) - 9 - 1
            file->fileNumberPos = (uint8_t)(filenameLen - 10); // See above
            // Attempt to create first log file:
            char* filePath = SOPC_Calloc(filenameLen, sizeof(char));
            if (filePath != NULL)
            {
                int res = sprintf(filePath, "%s/%s_%05u.log", logDirPath, config->logFileName, file->nbFiles);
                SOPC_ASSERT(res > 0);
                hFile = FILE_OPEN(filePath);
                file->filePath = filePath;
                file->maxBytes =
                    config->logMaxBytes - RESERVED_BYTES_PRINT_FILE_CHANGE; // Keep characters to display file change
                file->maxFiles = config->logMaxFiles;
                file->nbBytes = 0;
                file->pFile = hFile;
            }
            if (NULL == hFile)
            {
                if (filePath != NULL)
                {
                    SOPC_Free(filePath);
                }
                SOPC_Free(file);
                file = NULL;
            }
            else
            {
                FILE_SET_LINE_BUFF_MODE(hFile);
            }
        }
    }
    return file;
}

char* SOPC_CircularLogFile_GetFileName(const SOPC_CircularLogFile* pFile)
{
    if (NULL == pFile || NULL == pFile->pFile)
    {
        return NULL;
    }
    char* result = SOPC_strdup(pFile->filePath);
    return result;
}

void SOPC_CircularLogFile_PutLine(SOPC_CircularLogFile* pFile, const char* line)
{
    if (NULL == pFile || NULL == pFile->pFile)
        return;
    int res = fprintf(pFile->pFile, "%s\n", line);
    FILE_FLUSH(pFile->pFile);
    if (res > 0)
    {
        if ((uint64_t) res <= UINT32_MAX - pFile->nbBytes)
        {
            pFile->nbBytes += (uint32_t) res;
        }
        else
        {
            pFile->nbBytes = UINT32_MAX;
        }
        // Check if file is full.
        SOPC_ASSERT(pFile->maxBytes > 0);
        if (pFile->nbBytes >= pFile->maxBytes)
        {
            if (pFile->filePath != NULL)
            {
                if (pFile->nbFiles < pFile->maxFiles)
                {
                    pFile->nbFiles++;
                }
                else
                {
                    pFile->nbFiles = 1;
                }
                res = sprintf(&(pFile->filePath[pFile->fileNumberPos]), "%05u.log", pFile->nbFiles);
                SOPC_ASSERT(res > 0);
                // Display that next file will be opened
                fprintf(pFile->pFile, "LOG CONTINUE IN NEXT FILE: %s\n", pFile->filePath);
                if (NULL != pFile->pFile)
                {
                    FILE_CLOSE(pFile->pFile);
                }
                // Open new file
                pFile->pFile = FILE_OPEN(pFile->filePath);
                pFile->nbBytes = 0;
            }
        }
    }
    else
    {
        SOPC_CONSOLE_PRINTF("Log error: impossible to write in log %s\n", pFile->filePath);
        SOPC_ASSERT(false);
    }
}

void SOPC_CircularLogFile_Delete(SOPC_CircularLogFile** ppFile)
{
    if (NULL != ppFile && *ppFile != NULL)
    {
        SOPC_CircularLogFile* pFile = *ppFile;
        FILE_CLOSE(pFile->pFile);
        if (NULL != pFile->filePath)
        {
            SOPC_Free(pFile->filePath);
        }
        SOPC_Free(pFile);
        pFile = NULL;
    }
}
