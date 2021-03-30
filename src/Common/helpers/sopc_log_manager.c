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

#include "sopc_log_manager.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_time.h"

// Keep 100 bytes to print log file change
#define RESERVED_BYTES_PRINT_FILE_CHANGE 100
#define CATEGORY_MAX_LENGTH 9

const char* SOPC_CSTRING_LEVEL_ERROR = "(Error) ";
const char* SOPC_CSTRING_LEVEL_WARNING = "(Warning) ";
const char* SOPC_CSTRING_LEVEL_INFO = "";
const char* SOPC_CSTRING_LEVEL_DEBUG = "(Debug) ";
const char* SOPC_CSTRING_LEVEL_UNKNOWN = "(?) ";

static bool uniquePrefixSet = false;

SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
static char* SOPC_CSTRING_UNIQUE_LOG_PREFIX = "UNINIT_LOG";
SOPC_GCC_DIAGNOSTIC_RESTORE

typedef struct SOPC_Log_File
{
    Mutex fileMutex;
    char* filePath;
    uint8_t fileNumberPos;
    FILE* pFile;
    uint8_t nbRefs;
    uint32_t nbBytes;
    uint16_t nbFiles;
    uint32_t maxBytes; // New file created when maxBytes reached
    uint16_t maxFiles;
} SOPC_Log_File;

struct SOPC_Log_Instance
{
    char category[CATEGORY_MAX_LENGTH + 2]; // + 2 for blank space after category + NULL termination
    SOPC_Log_Level level;
    SOPC_Log_File* file;
    bool consoleFlag;
    bool started;
};

void SOPC_Log_Initialize(void)
{
    SOPC_CSTRING_UNIQUE_LOG_PREFIX = SOPC_Time_GetStringOfCurrentTimeUTC(true);
    uniquePrefixSet = true;
}

static void SOPC_Log_TracePrefixNoLock(SOPC_Log_Instance* pLogInst,
                                       SOPC_Log_Level level,
                                       bool withCategory,
                                       bool inhibitConsole)
{
    char* timestamp = NULL;
    const char* sLevel = NULL;
    int res;
    if (pLogInst->file->pFile != NULL && pLogInst->started)
    {
        timestamp = SOPC_Time_GetStringOfCurrentTimeUTC(false);
        switch (level)
        {
        case SOPC_LOG_LEVEL_ERROR:
            sLevel = SOPC_CSTRING_LEVEL_ERROR;
            break;
        case SOPC_LOG_LEVEL_WARNING:
            sLevel = SOPC_CSTRING_LEVEL_WARNING;
            break;
        case SOPC_LOG_LEVEL_INFO:
            sLevel = SOPC_CSTRING_LEVEL_INFO;
            ;
            break;
        case SOPC_LOG_LEVEL_DEBUG:
            sLevel = SOPC_CSTRING_LEVEL_DEBUG;
            break;
        default:
            sLevel = SOPC_CSTRING_LEVEL_UNKNOWN;
            break;
        }
        if (!withCategory)
        {
            res = fprintf(pLogInst->file->pFile, "[%s] %s", timestamp, sLevel);
            if (!inhibitConsole && pLogInst->consoleFlag)
            {
                printf("[%s] %s", timestamp, sLevel);
            }
        }
        else
        {
            res = fprintf(pLogInst->file->pFile, "[%s] %s%s", timestamp, pLogInst->category, sLevel);
            if (!inhibitConsole && pLogInst->consoleFlag)
            {
                printf("[%s] %s%s", timestamp, pLogInst->category, sLevel);
            }
        }
        if (res > 0)
        {
            if ((uint64_t) res <= UINT32_MAX - pLogInst->file->nbBytes)
            {
                pLogInst->file->nbBytes += (uint32_t) res;
            }
            else
            {
                pLogInst->file->nbBytes = UINT32_MAX;
            }
        }
        else
        {
            printf("Log error: impossible to write in log %s\n", pLogInst->file->filePath);
            fclose(pLogInst->file->pFile);
            pLogInst->file->pFile = NULL;
        }
        SOPC_Free(timestamp);
    }
}

// Print starting timestamp
static bool SOPC_Log_Start(SOPC_Log_Instance* pLogInst)
{
    int res = 0;
    bool result = false;
    if (NULL != pLogInst && !pLogInst->started)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        if (NULL != pLogInst->file->pFile)
        {
            pLogInst->started = true;
            SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);
            if (NULL != pLogInst->file->pFile)
            {
                res = fprintf(pLogInst->file->pFile, "LOG START\n");
                if (res > 0)
                {
                    if ((uint64_t) res <= UINT32_MAX - pLogInst->file->nbBytes)
                    {
                        pLogInst->file->nbBytes += (uint32_t) res;
                    }
                    else
                    {
                        pLogInst->file->nbBytes = UINT32_MAX;
                    }
                    result = true;
                }
                else
                {
                    printf("Log error: impossible to write in log %s\n", pLogInst->file->filePath);
                    fclose(pLogInst->file->pFile);
                    pLogInst->file->pFile = NULL;
                }
            }
        }
        else
        {
            printf("Log error: impossible to write in NULL stream.");
        }
        Mutex_Unlock(&pLogInst->file->fileMutex);
    }
    return result;
}

SOPC_Log_Instance* SOPC_Log_CreateInstance(const char* logDirPath,
                                           const char* logFileName,
                                           const char* category,
                                           uint32_t maxBytes,
                                           uint16_t maxFiles)
{
    SOPC_Log_Instance* result = NULL;
    SOPC_Log_File* file = NULL;
    size_t category_len = 0;
    char* filePath = NULL;
    int res = 0;

    // Check parameters valid
    if (logDirPath != NULL && logFileName != NULL && strlen(logFileName) > 0 &&
        strlen(logDirPath) + strlen(SOPC_CSTRING_UNIQUE_LOG_PREFIX) + strlen(logFileName) + 2 <= UINT8_MAX &&
        maxBytes > RESERVED_BYTES_PRINT_FILE_CHANGE && maxFiles > 0)
    {
        result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));
    }
    if (result != NULL)
    {
        file = SOPC_Malloc(sizeof(SOPC_Log_File));
        // Define file path and try to open it
        if (file != NULL)
        {
            file->pFile = NULL;
            // + 2 for the 2 '_'
            file->fileNumberPos =
                (uint8_t)(strlen(logDirPath) + strlen(SOPC_CSTRING_UNIQUE_LOG_PREFIX) + strlen(logFileName) + 2);
            // Attempt to create first log file:
            // 5 for file number + 4 for file extension +1 for '\0' terminating character => 10
            filePath = SOPC_Calloc(10u + file->fileNumberPos, sizeof(char));
            if (filePath != NULL)
            {
                res = sprintf(filePath, "%s%s_%s_00001.log", logDirPath, SOPC_CSTRING_UNIQUE_LOG_PREFIX, logFileName);
                assert(res > 0);
                file->pFile = fopen(filePath, "w");
            }
            if (NULL == file->pFile)
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
                setvbuf(file->pFile, NULL, _IOLBF, BUFSIZ);
            }
        }
        if (NULL != file && NULL != file->pFile)
        {
            file->filePath = filePath;
            file->maxBytes = maxBytes - RESERVED_BYTES_PRINT_FILE_CHANGE; // Keep characters to display file change
            file->maxFiles = maxFiles;
            file->nbBytes = 0;
            file->nbFiles = 1;
            file->nbRefs = 1;
            result->file = file;
        }
        else
        {
            SOPC_Free(result);
            result = NULL;
        }
    }

    if (result != NULL)
    {
        Mutex_Initialization(&result->file->fileMutex);
        // Fill fields
        if (category != NULL)
        {
            category_len = strlen(category);
            if (category_len > 0 && category_len <= CATEGORY_MAX_LENGTH)
            {
                memcpy(result->category, category, category_len);
                result->category[category_len] = ' ';
                result->category[category_len + 1] = '\0';
            }
            else
            {
                memcpy(result->category, category, CATEGORY_MAX_LENGTH);
                result->category[CATEGORY_MAX_LENGTH] = ' ';
                result->category[CATEGORY_MAX_LENGTH + 1] = '\0';
            }
        }
        else
        {
            result->category[0] = ' ';
            result->category[1] = '\0';
        }
        result->consoleFlag = false;
        result->level = SOPC_LOG_LEVEL_ERROR;
        result->started = false;
        // Starts the log instance
        bool started = SOPC_Log_Start(result);

        if (!started)
        {
            SOPC_Log_ClearInstance(&result);
        }
    }
    return result;
}

SOPC_Log_Instance* SOPC_Log_CreateInstanceAssociation(SOPC_Log_Instance* pLogInst, const char* category)
{
    size_t category_len = 0;
    SOPC_Log_Instance* result = NULL;

    if (NULL != pLogInst)
    {
        result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));

        if (result != NULL)
        {
            Mutex_Lock(&pLogInst->file->fileMutex);
            if (pLogInst->file->pFile != NULL && pLogInst->file->nbRefs < UINT8_MAX)
            {
                result->file = pLogInst->file;
                pLogInst->file->nbRefs++;
            }
            else
            {
                SOPC_Free(result);
                result = NULL;
            }
            Mutex_Unlock(&pLogInst->file->fileMutex);
        }
    }

    if (result != NULL)
    {
        // Fill fields
        if (category != NULL)
        {
            category_len = strlen(category);
            if (category_len > 0 && category_len <= CATEGORY_MAX_LENGTH)
            {
                memcpy(result->category, category, category_len);
                result->category[category_len] = ' ';
                result->category[category_len + 1] = '\0';
            }
            else
            {
                memcpy(result->category, category, CATEGORY_MAX_LENGTH);
                result->category[CATEGORY_MAX_LENGTH] = ' ';
                result->category[CATEGORY_MAX_LENGTH + 1] = '\0';
            }
        }
        else
        {
            result->category[0] = ' ';
            result->category[1] = '\0';
        }
        result->consoleFlag = false;
        result->level = SOPC_LOG_LEVEL_ERROR;

        // Starts the log instance
        bool started = SOPC_Log_Start(result);

        if (!started)
        {
            SOPC_Log_ClearInstance(&result);
        }
    }
    return result;
}

// Define the log level active (all traces <= level are displayed)
bool SOPC_Log_SetLogLevel(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level)
{
    bool result = false;
    int res = 0;
    if (NULL != pLogInst && pLogInst->started)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        pLogInst->level = level;
        result = true;
        SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);

        if (pLogInst->file->pFile != NULL)
        {
            switch (level)
            {
            case SOPC_LOG_LEVEL_ERROR:
                res = fprintf(pLogInst->file->pFile, "LOG LEVEL SET TO 'ERROR'\n");
                break;
            case SOPC_LOG_LEVEL_WARNING:
                res = fprintf(pLogInst->file->pFile, "LOG LEVEL SET TO 'WARNING'\n");
                break;
            case SOPC_LOG_LEVEL_INFO:
                res = fprintf(pLogInst->file->pFile, "LOG LEVEL SET TO 'INFO'\n");
                break;
            case SOPC_LOG_LEVEL_DEBUG:
                res = fprintf(pLogInst->file->pFile, "LOG LEVEL SET TO 'DEBUG'\n");
                break;
            default:
                res = fprintf(pLogInst->file->pFile, "LOG LEVEL SET TO '?(%u)'\n", level);
                break;
            }
            if (res > 0)
            {
                if ((uint64_t) res <= UINT32_MAX - pLogInst->file->nbBytes)
                {
                    pLogInst->file->nbBytes += (uint32_t) res;
                }
                else
                {
                    pLogInst->file->nbBytes = UINT32_MAX;
                }
            }
        }
        Mutex_Unlock(&pLogInst->file->fileMutex);
    }
    return result;
}

SOPC_Log_Level SOPC_Log_GetLogLevel(SOPC_Log_Instance* pLogInst)
{
    SOPC_Log_Level level = SOPC_LOG_LEVEL_ERROR;
    if (NULL != pLogInst)
    {
        level = pLogInst->level;
    }
    return level;
}

// Activate output in console
bool SOPC_Log_SetConsoleOutput(SOPC_Log_Instance* pLogInst, bool activate)
{
    int res = 0;
    bool result = false;
    if (NULL != pLogInst && pLogInst->started)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        pLogInst->consoleFlag = activate;
        result = true;

        SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);

        if (pLogInst->file->pFile != NULL)
        {
            res = fprintf(pLogInst->file->pFile, "LOG CONSOLE OUTPUT SET TO '%s'\n", activate ? "TRUE" : "FALSE");
            if (res > 0)
            {
                if ((uint64_t) res <= UINT32_MAX - pLogInst->file->nbBytes)
                {
                    pLogInst->file->nbBytes += (uint32_t) res;
                }
                else
                {
                    pLogInst->file->nbBytes = UINT32_MAX;
                }
            }
        }
        Mutex_Unlock(&pLogInst->file->fileMutex);
    }
    return result;
}

char* SOPC_Log_GetFilePathPrefix(SOPC_Log_Instance* pLogInst)
{
    char* filePathPrefix = NULL;
    if (NULL != pLogInst && NULL != pLogInst->file->filePath)
    {
        filePathPrefix = SOPC_Calloc(1u + pLogInst->file->fileNumberPos, sizeof(char));
        if (NULL != filePathPrefix)
        {
            memcpy(filePathPrefix, pLogInst->file->filePath, (size_t) pLogInst->file->fileNumberPos);
            filePathPrefix[pLogInst->file->fileNumberPos] = '\0';
        }
    }
    return filePathPrefix;
}

static void SOPC_Log_CheckFileChangeNoLock(SOPC_Log_Instance* pLogInst)
{
    assert(pLogInst != NULL);
    int res = 0;
    if (pLogInst->file->nbBytes >= pLogInst->file->maxBytes)
    {
        if (pLogInst->file->filePath != NULL)
        {
            if (pLogInst->file->nbFiles < pLogInst->file->maxFiles)
            {
                pLogInst->file->nbFiles++;
            }
            else
            {
                pLogInst->file->nbFiles = 1;
            }
            res = sprintf(&(pLogInst->file->filePath[pLogInst->file->fileNumberPos]), "%05u.log",
                          pLogInst->file->nbFiles);
            assert(res > 0);
            // Display that next file will be opened
            SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, false, true);
            if (pLogInst->file->pFile != NULL)
            {
                res = fprintf(pLogInst->file->pFile, "LOG CONTINUE IN NEXT FILE: %s\n", pLogInst->file->filePath);
                assert(res > 0);
            }

            fclose(pLogInst->file->pFile);
            pLogInst->file->pFile = fopen(pLogInst->file->filePath, "w");
            pLogInst->file->nbBytes = 0;
        }
    }
}

void SOPC_Log_VTrace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, va_list args)
{
    int res = 0;
    int res2 = 0;
    if (NULL != pLogInst && pLogInst->started && level <= pLogInst->level)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        // Check file open
        SOPC_Log_TracePrefixNoLock(pLogInst, level, true, false);
        if (NULL != pLogInst->file->pFile)
        {
            if (pLogInst->consoleFlag)
            {
                va_list args_copy;
                va_copy(args_copy, args);
                vprintf(format, args_copy);
                va_end(args_copy);
                printf("\n");
            }
            res = vfprintf(pLogInst->file->pFile, format, args);
            res2 = fprintf(pLogInst->file->pFile, "\n");
            if (res > 0 && res2 > 0)
            {
                if (UINT32_MAX - pLogInst->file->nbBytes > (uint64_t) res + (uint64_t) res2)
                {
                    pLogInst->file->nbBytes += (uint32_t)(res + res2);
                }
                else
                {
                    pLogInst->file->nbBytes = UINT32_MAX;
                }
                fflush(pLogInst->file->pFile);
            }
            else
            {
                printf("Log error: impossible to write in log %s\n", pLogInst->file->filePath);
                fclose(pLogInst->file->pFile);
                pLogInst->file->pFile = NULL;
            }
            SOPC_Log_CheckFileChangeNoLock(pLogInst);
        }
        Mutex_Unlock(&pLogInst->file->fileMutex);
    }
}

// Print new trace in log file (and console if applicable)
void SOPC_Log_Trace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    SOPC_Log_VTrace(pLogInst, level, format, args);
    va_end(args);
}

// Deallocate the Log instance structure
void SOPC_Log_ClearInstance(SOPC_Log_Instance** ppLogInst)
{
    SOPC_Log_Instance* pLogInst = NULL;
    if (ppLogInst != NULL && *ppLogInst != NULL)
    {
        pLogInst = *ppLogInst;
        Mutex_Lock(&pLogInst->file->fileMutex);
        if (pLogInst->started)
        {
            SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);
            if (pLogInst->file->pFile != NULL)
            {
                fprintf(pLogInst->file->pFile, "LOG STOP\n");
            }

            pLogInst->started = false;
        }
        // Note: Unlock mutex in both branches
        if (pLogInst->file->nbRefs <= 1)
        {
            fclose(pLogInst->file->pFile);
            pLogInst->file->pFile = NULL;
            Mutex_Unlock(&pLogInst->file->fileMutex);
            Mutex_Clear(&pLogInst->file->fileMutex);
            SOPC_Free(pLogInst->file->filePath);
            SOPC_Free(pLogInst->file);
        }
        else
        {
            pLogInst->file->nbRefs--;
            Mutex_Unlock(&pLogInst->file->fileMutex);
        }
        SOPC_Free(pLogInst);
        *ppLogInst = NULL;
    }
}

void SOPC_Log_Clear(void)
{
    if (uniquePrefixSet)
    {
        SOPC_Free(SOPC_CSTRING_UNIQUE_LOG_PREFIX);
        uniquePrefixSet = false;
    }
}
