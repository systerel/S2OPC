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

#include "sopc_assert.h"
#include "sopc_common_constants.h"
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
    char category[CATEGORY_MAX_LENGTH + 1]; // + 1 for NULL termination
    SOPC_Log_Level level;
    SOPC_Log_File* file;
    SOPC_Log_UserDoLog* logCallback;
    uint32_t logPosition; // current position in callbackBuffer
    char* callbackBuffer;
    bool consoleFlag;
    bool started;
};

void SOPC_Log_Initialize(void)
{
    SOPC_CSTRING_UNIQUE_LOG_PREFIX = SOPC_Time_GetStringOfCurrentTimeUTC(true);
    uniquePrefixSet = true;
}

static void SOPC_Log_InstanceFileClose(SOPC_Log_File* pLogFile)
{
#if SOPC_HAS_FILESYSTEM
    if (NULL != pLogFile)
    {
        if (NULL != pLogFile->pFile)
        {
            fclose(pLogFile->pFile);
            pLogFile->pFile = NULL;
        }
    }
#endif
}

static FILE* SOPC_Log_InstanceFileOpen(const char* filename)
{
    FILE* result = NULL;
#if SOPC_HAS_FILESYSTEM
    if (NULL != filename)
    {
        result = fopen(filename, "w");
    }
#endif
    return result;
}

static void SOPC_Log_Flush(SOPC_Log_File* pLogFile)
{
#if SOPC_HAS_FILESYSTEM
    if (NULL != pLogFile->pFile)
    {
        fflush(pLogFile->pFile);
    }
#endif
}

/**
 * \brief same as SOPC_Log_PutLogLine with va_list
 */

static void SOPC_Log_VPutLogLine(SOPC_Log_Instance* pLogInst,
                                 bool addNewline,
                                 bool inhibitConsole,
                                 const char* format,
                                 va_list args)
{
    int res = 0;
    char* logBuffer = ((NULL != pLogInst) ? pLogInst->callbackBuffer : NULL);
    if (NULL != pLogInst && pLogInst->started)
    {
        if (!inhibitConsole && pLogInst->consoleFlag)
        {
            va_list args_copy;
            va_copy(args_copy, args);
            vprintf(format, args_copy);
            va_end(args_copy);
            if (true == addNewline)
            {
                SOPC_CONSOLE_PRINTF("\n");
            }
        }
        if (NULL != pLogInst->logCallback && NULL != logBuffer)
        {
            // reminder : logBuffer size is (SOPC_LOG_MAX_USER_LINE_LENGTH + 1)
            const int newPos = vsnprintf(&logBuffer[pLogInst->logPosition],
                                         SOPC_LOG_MAX_USER_LINE_LENGTH + 1 - pLogInst->logPosition, format, args);
            SOPC_ASSERT(newPos > 0);
            pLogInst->logPosition = (uint32_t) newPos;
            logBuffer[SOPC_LOG_MAX_USER_LINE_LENGTH] = 0;
            if (addNewline)
            {
                pLogInst->logCallback(pLogInst->category, logBuffer);
                pLogInst->logPosition = 0;
            }
        }
        /* Note : "else" increases robustness as va_list "args" cannot be passed twice */
        else if (NULL != pLogInst->file->pFile)
        {
            res = vfprintf(pLogInst->file->pFile, format, args);
            if (true == addNewline)
            {
                res += fprintf(pLogInst->file->pFile, "\n");
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
                SOPC_CONSOLE_PRINTF("Log error: impossible to write in log %s\n", pLogInst->file->filePath);
                SOPC_Log_InstanceFileClose(pLogInst->file);
            }
        }
    }
}

/**
 * \brief Print a log line in the right container (file, user callback ,...)
 */
static void SOPC_Log_PutLogLine(SOPC_Log_Instance* pLogInst,
                                bool addNewline,
                                bool inhibitConsole,
                                const char* format,
                                ...)
{
    va_list args;
    va_start(args, format);
    SOPC_Log_VPutLogLine(pLogInst, addNewline, inhibitConsole, format, args);
    va_end(args);
}

static void SOPC_Log_TracePrefixNoLock(SOPC_Log_Instance* pLogInst,
                                       SOPC_Log_Level level,
                                       bool withCategory,
                                       bool inhibitConsole)
{
    char* timestamp = NULL;
    const char* sLevel = NULL;
    if ((pLogInst->file->pFile != NULL || NULL != pLogInst->logCallback) && pLogInst->started)
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
        if (NULL != pLogInst->logCallback || !withCategory)
        {
            // In case of user log, the category is provided in the callback, so it does not need to be printed here
            SOPC_Log_PutLogLine(pLogInst, false, inhibitConsole, "[%s] %s", timestamp, sLevel);
        }
        else
        {
            SOPC_Log_PutLogLine(pLogInst, false, inhibitConsole, "[%s] %s %s", timestamp, pLogInst->category, sLevel);
        }
        SOPC_Free(timestamp);
    }
}

// Print starting timestamp
static bool SOPC_Log_Start(SOPC_Log_Instance* pLogInst)
{
    bool result = false;
    if (NULL != pLogInst && !pLogInst->started)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        if ((NULL != pLogInst->file->pFile) || (NULL != pLogInst->logCallback))
        {
            pLogInst->started = true;
            SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);

            SOPC_Log_PutLogLine(pLogInst, true, true, "LOG START");
            result = true;
        }
        else
        {
            SOPC_CONSOLE_PRINTF("Log error: impossible to write in NULL stream.\n");
        }
        Mutex_Unlock(&pLogInst->file->fileMutex);
    }
    return result;
}

// Fill pLogInst->category with category, aligned and truncated to CATEGORY_MAX_LENGTH
static void SOPC_Log_AlignCategory(const char* category, SOPC_Log_Instance* pLogInst)
{
    if (NULL == pLogInst)
    {
        return;
    }
    if (NULL != category)
    {
        const size_t category_len = strlen(category);
        if (category_len > 0 && category_len <= CATEGORY_MAX_LENGTH)
        {
            memcpy(pLogInst->category, category, category_len);
            pLogInst->category[category_len] = '\0';
        }
        else
        {
            memcpy(pLogInst->category, category, CATEGORY_MAX_LENGTH);
            pLogInst->category[CATEGORY_MAX_LENGTH] = '\0';
        }
    }
    else
    {
        pLogInst->category[0] = '\0';
    }
}

SOPC_Log_Instance* SOPC_Log_CreateUserInstance(const char* category, SOPC_Log_UserDoLog* logCallback)
{
    SOPC_Log_Instance* result = NULL;
    SOPC_Log_File* file = NULL;
    result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));
    if (result != NULL)
    {
        file = SOPC_Malloc(sizeof(SOPC_Log_File));
        if (file != NULL)
        {
            // As the Log instance is a user-defined callback, The FILE structure is not used.
            // Set pFile to NULL so that log implementation only uses callbackBuffer field
            // Only the fields nbFiles and nbRefs are significant, because they allow
            // detection of instance closure.
            file->pFile = NULL;
            file->fileNumberPos = 0;
            file->filePath = NULL;
            file->maxBytes = 0;
            file->maxFiles = 0;
            file->nbBytes = 0;
            file->nbFiles = 1;
            file->nbRefs = 1;
            result->file = file;
            result->logCallback = logCallback;
            result->logPosition = 0;
            result->callbackBuffer = SOPC_Malloc(SOPC_LOG_MAX_USER_LINE_LENGTH + 1); // + NULL
            if (NULL != result->callbackBuffer)
            {
                Mutex_Initialization(&result->file->fileMutex);
                // Fill fields
                SOPC_Log_AlignCategory(category, result);
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
            else
            {
                /* Allocation of callbackBuffer failed */
                SOPC_Free(file);
                SOPC_Free(result);
                result = NULL;
            }
        }
        else
        {
            /* Allocation of file failed */
            SOPC_Free(result);
            result = NULL;
        }
    }

    return result;
}

SOPC_Log_Instance* SOPC_Log_CreateFileInstance(const char* logDirPath,
                                               const char* logFileName,
                                               const char* category,
                                               uint32_t maxBytes,
                                               uint16_t maxFiles)
{
    SOPC_Log_Instance* result = NULL;
    SOPC_Log_File* file = NULL;
    char* filePath = NULL;
    int res = 0;
    FILE* hFile = NULL;

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
        if (NULL != file)
        {
            file->pFile = NULL;
            file->nbFiles = 0;
            // + 2 for the 2 '_'
            file->fileNumberPos =
                (uint8_t)(strlen(logDirPath) + strlen(SOPC_CSTRING_UNIQUE_LOG_PREFIX) + strlen(logFileName) + 2);
            // Attempt to create first log file:
            // 5 for file number + 4 for file extension +1 for '\0' terminating character => 10
            filePath = SOPC_Calloc(10u + file->fileNumberPos, sizeof(char));
            if (filePath != NULL)
            {
                res = sprintf(filePath, "%s%s_%s_%05u.log", logDirPath, SOPC_CSTRING_UNIQUE_LOG_PREFIX, logFileName,
                              file->nbFiles);
                SOPC_ASSERT(res > 0);
                hFile = SOPC_Log_InstanceFileOpen(filePath);
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
                setvbuf(hFile, NULL, _IOLBF, BUFSIZ);
            }
        }
        if (NULL != file)
        {
            file->filePath = filePath;
            file->maxBytes = maxBytes - RESERVED_BYTES_PRINT_FILE_CHANGE; // Keep characters to display file change
            file->maxFiles = maxFiles;
            file->nbBytes = 0;
            file->nbRefs = 1;
            file->pFile = hFile;

            bool started = false;
            SOPC_ReturnStatus mutex_res = Mutex_Initialization(&file->fileMutex);
            if (mutex_res == SOPC_STATUS_OK)
            {
                result->file = file;
                // Fill fields
                SOPC_Log_AlignCategory(category, result);
                result->consoleFlag = false;
                result->level = SOPC_LOG_LEVEL_ERROR;
                result->started = false;
                // Ensure logCallback and callbackBuffer are initialized to 0 in case of FILE log.
                result->logCallback = NULL;
                result->logPosition = 0;
                result->callbackBuffer = 0;
                // Starts the log instance
                started = SOPC_Log_Start(result);
            }

            if (!started)
            {
                fclose(hFile);
                if (mutex_res == SOPC_STATUS_OK)
                {
                    Mutex_Clear(&result->file->fileMutex);
                }

                SOPC_Free(result->file->filePath);
                SOPC_Free(result->callbackBuffer);
                result->callbackBuffer = NULL;

                SOPC_Free(file);
                SOPC_Free(result);
                result = NULL;
            }
        }
        else
        {
            SOPC_Free(result);
            result = NULL;
        }
    }

    return result;
}

SOPC_Log_Instance* SOPC_Log_CreateInstanceAssociation(SOPC_Log_Instance* pLogInst, const char* category)
{
    SOPC_Log_Instance* result = NULL;

    if (NULL != pLogInst)
    {
        result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));

        if (result != NULL)
        {
            Mutex_Lock(&pLogInst->file->fileMutex);
            if (pLogInst->file->nbRefs < UINT8_MAX)
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
        SOPC_Log_AlignCategory(category, result);
        result->consoleFlag = false;
        result->level = SOPC_LOG_LEVEL_ERROR;
        result->callbackBuffer = pLogInst->callbackBuffer;
        result->logCallback = pLogInst->logCallback;
        result->logPosition = 0;

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
    if (NULL != pLogInst && pLogInst->started)
    {
        const char* levelName = "";
        char unknownNameLevel[20];
        Mutex_Lock(&pLogInst->file->fileMutex);
        result = true;
        SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);

        switch (level)
        {
        case SOPC_LOG_LEVEL_ERROR:
            levelName = "ERROR";
            break;
        case SOPC_LOG_LEVEL_WARNING:
            levelName = "WARNING";
            break;
        case SOPC_LOG_LEVEL_INFO:
            levelName = "INFO";
            break;
        case SOPC_LOG_LEVEL_DEBUG:
            levelName = "DEBUG";
            break;
        default:
            snprintf(unknownNameLevel, sizeof(unknownNameLevel), "?(%u)", level);
            unknownNameLevel[sizeof(unknownNameLevel) - 1] = 0;
            levelName = unknownNameLevel;
            result = false;
            break;
        }
        if (true == result)
        {
            pLogInst->level = level;
        }

        SOPC_Log_PutLogLine(pLogInst, true, true, "LOG LEVEL SET TO '%s'", levelName);
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
    bool result = false;
    if (NULL != pLogInst && pLogInst->started)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        pLogInst->consoleFlag = activate;
        result = true;

        SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, true, true);

        SOPC_Log_PutLogLine(pLogInst, true, true, "LOG CONSOLE OUTPUT SET TO '%s'", activate ? "TRUE" : "FALSE");
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
#if SOPC_HAS_FILESYSTEM
    SOPC_ASSERT(pLogInst != NULL);
    int res = 0;
    /* Note : for USER-defined logs, maxBytes is set to 0 */
    if (pLogInst->file->maxBytes > 0 && pLogInst->file->nbBytes >= pLogInst->file->maxBytes)
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
            SOPC_ASSERT(res > 0);
            // Display that next file will be opened
            SOPC_Log_TracePrefixNoLock(pLogInst, SOPC_LOG_LEVEL_INFO, false, true);
            SOPC_Log_PutLogLine(pLogInst, true, true, "LOG CONTINUE IN NEXT FILE: %s", pLogInst->file->filePath);

            SOPC_Log_InstanceFileClose(pLogInst->file);
            pLogInst->file->pFile = SOPC_Log_InstanceFileOpen(pLogInst->file->filePath);
            pLogInst->file->nbBytes = 0;
        }
    }
#endif
}

void SOPC_Log_VTrace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, va_list args)
{
    if (NULL != pLogInst && pLogInst->started && level <= pLogInst->level)
    {
        Mutex_Lock(&pLogInst->file->fileMutex);
        // Check file open
        SOPC_Log_TracePrefixNoLock(pLogInst, level, true, false);
        SOPC_Log_VPutLogLine(pLogInst, true, false, format, args);

        if (NULL != pLogInst->file->pFile)
        {
            SOPC_Log_Flush(pLogInst->file);
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
            SOPC_Log_PutLogLine(pLogInst, true, true, "LOG STOP");

            pLogInst->started = false;
        }
        // Note: Unlock mutex in both branches
        if (pLogInst->file->nbRefs <= 1)
        {
            SOPC_Log_InstanceFileClose(pLogInst->file);
            Mutex_Unlock(&pLogInst->file->fileMutex);
            Mutex_Clear(&pLogInst->file->fileMutex);
            if (NULL != pLogInst->file->filePath)
            {
                SOPC_Free(pLogInst->file->filePath);
            }
            if (NULL != pLogInst->callbackBuffer)
            {
                SOPC_Free(pLogInst->callbackBuffer);
                pLogInst->callbackBuffer = NULL;
            }
            SOPC_Free(pLogInst->file);
            pLogInst->file = NULL;
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
