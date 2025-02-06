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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_circular_log_file.h"
#include "sopc_common_constants.h"
#include "sopc_date_time.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

// Keep 100 bytes to print log file change
#define RESERVED_BYTES_PRINT_FILE_CHANGE 100
#define CATEGORY_MAX_LENGTH 9

static void trace_Internal(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, bool always, const char* format, ...);
static void vTrace_Internal(SOPC_Log_Instance* pLogInst,
                            SOPC_Log_Level level,
                            bool always,
                            const char* format,
                            va_list args);

const char* SOPC_CSTRING_LEVEL_ERROR = "(Error) ";
const char* SOPC_CSTRING_LEVEL_WARNING = "(Warning) ";
const char* SOPC_CSTRING_LEVEL_INFO = "";
const char* SOPC_CSTRING_LEVEL_DEBUG = "(Debug) ";
const char* SOPC_CSTRING_LEVEL_UNKNOWN = "(?) ";

struct SOPC_Log_Instance
{
    // 'actual' points to the actual instance. If not NULL, only category, started and level are relevant
    SOPC_Log_Instance* actual;
    char category[CATEGORY_MAX_LENGTH + 1]; // + 1 for NULL termination
    SOPC_Log_Level level;
    bool started;
    bool consoleFlag;
    // The Following fields are only used in an actual instance. (When actual is NULL)
    SOPC_Mutex mutex;
    uint8_t nbRefs;
    SOPC_CircularLogFile* pFile;     // NULL if not used
    SOPC_Log_UserDoLog* logCallback; // NULL if not used
    uint32_t logPosition;            // current position in callbackBuffer
    char* printBuffer;
};

static const char* levelToString(const SOPC_Log_Level level)
{
    switch ((int) level)
    {
    case SOPC_LOG_LEVEL_ERROR:
        return SOPC_CSTRING_LEVEL_ERROR;
    case SOPC_LOG_LEVEL_WARNING:
        return SOPC_CSTRING_LEVEL_WARNING;
    case SOPC_LOG_LEVEL_INFO:
        return SOPC_CSTRING_LEVEL_INFO;
    case SOPC_LOG_LEVEL_DEBUG:
        return SOPC_CSTRING_LEVEL_DEBUG;
    default:
        return SOPC_CSTRING_LEVEL_UNKNOWN;
    }
}

// Print starting timestamp
static bool SOPC_Log_Start(SOPC_Log_Instance* pLogInst)
{
    bool result = false;
    if (NULL != pLogInst && !pLogInst->started)
    {
        SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);

        SOPC_Mutex_Lock(&actual->mutex);
        // Note: check again the flag to avoid possible concurrent assignment detection by static analysis.
        //       Nevertheless since caller is logInst creation function, it should never occur.
        if (!pLogInst->started)
        {
            if ((NULL != actual->pFile) || (NULL != actual->logCallback))
            {
                pLogInst->started = true;
                trace_Internal(pLogInst, SOPC_LOG_LEVEL_INFO, true, "LOG START");
                result = true;
            }
            else
            {
                SOPC_CONSOLE_PRINTF("Log error: impossible to write in NULL stream.\n");
            }
        }
        SOPC_Mutex_Unlock(&actual->mutex);
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
    SOPC_Log_Instance* result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));

    if (NULL == result)
    {
        return NULL;
    }

    // Fill fields
    SOPC_Log_AlignCategory(category, result);
    result->logCallback = logCallback;
    result->nbRefs = 1;
    result->printBuffer = SOPC_Malloc(SOPC_LOG_MAX_USER_LINE_LENGTH + 1); // + NULL
    SOPC_ReturnStatus mutex_res = SOPC_Mutex_Initialization(&result->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutex_res);

    // All other fields are initialized to 0 with Calloc

    if (NULL != result->printBuffer)
    {
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
        SOPC_Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
    }

    return result;
}

SOPC_Log_Instance* SOPC_Log_CreateFileInstance(const SOPC_CircularLogFile_Configuration* pConf, const char* category)
{
    SOPC_Log_Instance* result = SOPC_Calloc(1, sizeof(SOPC_Log_Instance));

    if (NULL == result)
    {
        return NULL;
    }

    SOPC_CircularLogFile* pFile = SOPC_CircularLogFile_Create(pConf);

    if (NULL == pFile)
    {
        SOPC_Free(result);
        SOPC_CircularLogFile_Delete(&pFile);
        return NULL;
    }

    // Fill fields
    SOPC_Log_AlignCategory(category, result);
    result->printBuffer = SOPC_Malloc(SOPC_LOG_MAX_USER_LINE_LENGTH + 1); // + NULL
    SOPC_ASSERT(NULL != result->printBuffer);
    result->pFile = pFile;
    result->nbRefs = 1;
    SOPC_ReturnStatus mutex_res = SOPC_Mutex_Initialization(&result->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutex_res);
    // All other fields are initialized to 0 with Calloc

    // Starts the log instance
    bool started = SOPC_Log_Start(result);
    if (!started)
    {
        SOPC_CircularLogFile_Delete(&pFile);
        SOPC_Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
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
            SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);
            SOPC_Mutex_Lock(&actual->mutex);
            SOPC_ASSERT(actual->nbRefs < UINT8_MAX);
            {
                // This is a linked instance. Only category and level are setup.
                result->actual = actual;
                result->level = SOPC_LOG_LEVEL_ERROR;
                actual->nbRefs++;
                SOPC_Log_AlignCategory(category, result);
            }
            SOPC_Mutex_Unlock(&actual->mutex);

            bool started = SOPC_Log_Start(result);

            if (!started)
            {
                SOPC_Log_ClearInstance(&result);
            }
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
        SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);
        const char* levelName = "";
        result = true;

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
            result = false;
            break;
        }
        if (result)
        {
            SOPC_Mutex_Lock(&actual->mutex);
            pLogInst->level = level;

            trace_Internal(pLogInst, SOPC_LOG_LEVEL_INFO, true, "LOG LEVEL SET TO '%s'", levelName);
            SOPC_Mutex_Unlock(&actual->mutex);
        }
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
        SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);
        SOPC_Mutex_Lock(&actual->mutex);
        pLogInst->consoleFlag = activate;
        result = true;

        trace_Internal(pLogInst, SOPC_LOG_LEVEL_INFO, true, "LOG CONSOLE OUTPUT SET TO '%s'",
                       activate ? "TRUE" : "FALSE");
        SOPC_Mutex_Unlock(&actual->mutex);
    }
    return result;
}

char* SOPC_Log_GetCurrentFilename(const SOPC_Log_Instance* pLogInst)
{
    char* result = NULL;
    if (NULL != pLogInst)
    {
        const SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);
        result = SOPC_CircularLogFile_GetFileName(actual->pFile);
    }
    return result;
}

static void trace_Internal(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, bool always, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vTrace_Internal(pLogInst, level, always, format, args);
    va_end(args);
}

__attribute__((__format__(__printf__, 4, 0))) static void vTrace_Internal(SOPC_Log_Instance* pLogInst,
                                                                          SOPC_Log_Level level,
                                                                          bool always,
                                                                          const char* format,
                                                                          va_list args)
{
    if (NULL != pLogInst && ((pLogInst->started && level <= pLogInst->level) || always))
    {
        SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);
        SOPC_Mutex_Lock(&actual->mutex);

        // Format full line

        char* timestamp = SOPC_Time_GetStringOfCurrentTimeUTC(false);
        int remain = SOPC_LOG_MAX_USER_LINE_LENGTH + 1;
        SOPC_ASSERT(NULL != actual->printBuffer);

        const bool isFile = (NULL != actual->pFile);

        // Reminder : actual->printBuffer has a size of (SOPC_LOG_MAX_USER_LINE_LENGTH + 1)
        int pos = 0;
        if (isFile)
        {
            // In files, the log line is as follow (<LEVEL> skipped for INFO level):
            // [YYYY/MM/DD HH:MM:SS.sss] <Category> <LEVEL > <Log>
            pos = snprintf(actual->printBuffer, SOPC_LOG_MAX_USER_LINE_LENGTH + 1, "[%s] %s %s", timestamp,
                           pLogInst->category, levelToString(level));
            remain -= pos;
        }

        if (remain > 0)
        {
            vsnprintf(actual->printBuffer + pos, (size_t) remain, format, args);
        }
        actual->printBuffer[SOPC_LOG_MAX_USER_LINE_LENGTH] = 0;

        // File
        if (isFile)
        {
            SOPC_CircularLogFile_PutLine(actual->pFile, actual->printBuffer);

            // Write line in dedicated sections
            if (pLogInst->consoleFlag)
            {
                SOPC_CONSOLE_PRINTF("%s\n", actual->printBuffer);
            }
        }

        // User
        if (NULL != actual->logCallback)
        {
            // In user log system, there is no "\n" added.
            actual->logCallback(timestamp, pLogInst->category, level, actual->printBuffer);
        }

        SOPC_Free(timestamp);
        *actual->printBuffer = 0;

        SOPC_Mutex_Unlock(&actual->mutex);
    }
}

void SOPC_Log_VTrace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, va_list args)
{
    vTrace_Internal(pLogInst, level, false, format, args);
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
        SOPC_Log_Instance* actual = (pLogInst->actual == NULL ? pLogInst : pLogInst->actual);

        SOPC_Mutex_Lock(&actual->mutex);
        if (pLogInst->started)
        {
            trace_Internal(pLogInst, SOPC_LOG_LEVEL_INFO, true, "LOG STOP");
        }

        // Only allow to delete "actual" session if there are no remaining references
        SOPC_ASSERT(pLogInst->actual != NULL || actual->nbRefs == 1);

        if (actual->nbRefs <= 1)
        {
            // Last reference is always the actual session
            if (pLogInst->actual != NULL)
            {
                SOPC_CONSOLE_PRINTF("Cannot delete instance file before associated ones!\n");
                SOPC_Log_Trace(pLogInst, SOPC_LOG_LEVEL_ERROR, "Cannot delete instance file before associated ones!");
                SOPC_ASSERT(false);
            }

            // There are no more instances.
            if (NULL != pLogInst->pFile)
            {
                SOPC_CircularLogFile_Delete(&pLogInst->pFile);
            }

            SOPC_Free(pLogInst->printBuffer);

            SOPC_Mutex_Unlock(&actual->mutex);
            SOPC_Mutex_Clear(&actual->mutex);
        }
        else
        {
            actual->nbRefs--;
            SOPC_Mutex_Unlock(&actual->mutex);
        }
        SOPC_Free(pLogInst);
        *ppLogInst = NULL;
    }
}
