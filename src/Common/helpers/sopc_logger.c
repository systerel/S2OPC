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

#include "sopc_logger.h"

#include <stdio.h>
#include <string.h>

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_date_time.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

static const char* traceName = "Trace";
static char* filePath = NULL;

static SOPC_Log_Instance* commonTrace = NULL;
static SOPC_Log_Instance* clientServerTrace = NULL;
static SOPC_Log_Instance* pubSubTrace = NULL;
static SOPC_Log_Instance* secuAudit = NULL;
static SOPC_Log_Instance* opcUaAudit = NULL;

// User section logs:
typedef SOPC_Log_Instance* UserLogElement;

// Array containing all user log sections managed by toolkit.
static SOPC_Array* userLogArray = NULL;
static void userLogArray_Free_Func(void* data)
{
    UserLogElement elt = (UserLogElement) data;
    SOPC_Log_ClearInstance(&elt);
}

static const char* SOPC_Log_SecuAuditCategory = "SecuAudit";

/*
 * \brief Initializes the file logger and create the necessary log file(s)
 *
 * \param pLogInst  An existing log instance used to print in the same log file

 *
 * */
static bool SOPC_Logger_AuditInitialize(void);

bool SOPC_Logger_Initialize(const SOPC_Log_Configuration* const logConfiguration)
{
    if (NULL != filePath)
    {
        // Already configured.
        return false;
    }

    const SOPC_Log_System logSystem = (NULL == logConfiguration) ? SOPC_LOG_SYSTEM_NO_LOG : logConfiguration->logSystem;

    bool result = false;

    char* uniqueLogPrefix = SOPC_Time_GetStringOfCurrentTimeUTC(true);
    SOPC_ASSERT(NULL != uniqueLogPrefix);

    // Format is <date>_<traceName> : +2 = '_' and '\0'
    filePath = (char*) SOPC_Malloc(strlen(traceName) + strlen(uniqueLogPrefix) + 2);
    SOPC_ASSERT(NULL != filePath);
    sprintf(filePath, "%s_%s", traceName, uniqueLogPrefix);
    SOPC_Free(uniqueLogPrefix);

    switch (logSystem)
    {
    case SOPC_LOG_SYSTEM_FILE:
    {
        const SOPC_LogSystem_File_Configuration* logCfg = &logConfiguration->logSysConfig.fileSystemLogConfig;
        SOPC_CircularLogFile_Configuration conf = {.logDirPath = logCfg->logDirPath,
                                                   .logFileName = filePath,
                                                   .logMaxBytes = logCfg->logMaxBytes,
                                                   .logMaxFiles = logCfg->logMaxFiles};
        secuAudit = SOPC_Log_CreateFileInstance(&conf, SOPC_Log_SecuAuditCategory);
        result = SOPC_Logger_AuditInitialize();

        break;
    }
    case SOPC_LOG_SYSTEM_USER:
        secuAudit = SOPC_Log_CreateUserInstance(SOPC_Log_SecuAuditCategory,
                                                logConfiguration->logSysConfig.userSystemLogConfig.doLog);
        result = SOPC_Logger_AuditInitialize();
        break;
    case SOPC_LOG_SYSTEM_NO_LOG:
        result = true;
        break;
    default:
        result = false;
        break;
    }
    if (SOPC_LOG_SYSTEM_NO_LOG != logSystem)
    {
        if (result)
        {
            SOPC_Logger_SetTraceLogLevel(logConfiguration->logLevel);
        }
        else
        {
            /* Status stays OK given that we don't have other alternatives for now */
            fprintf(stderr, "ERROR: S2OPC Logs initialization failed!\n");
        }
    }
    return result;
}

SOPC_Log_Instance* SOPC_Logger_AddUserInstance(const char* category)
{
    SOPC_Log_Instance* result = NULL;
    if (NULL == userLogArray)
    {
        userLogArray = SOPC_Array_Create(sizeof(UserLogElement), 1, &userLogArray_Free_Func);
    }
    if (NULL != userLogArray)
    {
        result = SOPC_Log_CreateInstanceAssociation(secuAudit, category);
    }
    if (result != NULL)
    {
        const bool appendOk = SOPC_Array_Append_Values(userLogArray, result, 1);
        if (!appendOk)
        {
            SOPC_Log_ClearInstance(&result);
            result = NULL;
        }
    }
    return result;
}

static bool SOPC_Logger_AuditInitialize(void)
{
    bool result = false;
    if (secuAudit != NULL)
    {
        result = SOPC_Log_SetLogLevel(secuAudit, SOPC_LOG_LEVEL_INFO); // Set INFO level for secu audit

        if (result != false)
        {
            commonTrace = SOPC_Log_CreateInstanceAssociation(secuAudit, "Common");
            if (commonTrace == NULL)
            {
                SOPC_CONSOLE_PRINTF("WARNING: Common log creation failed, no Common log will be recorded !");
            }

            clientServerTrace = SOPC_Log_CreateInstanceAssociation(secuAudit, "ClientServer");
            if (clientServerTrace == NULL)
            {
                SOPC_CONSOLE_PRINTF(
                    "WARNING: ClientServer log creation failed, no ClientServer log will be recorded !");
            }

            pubSubTrace = SOPC_Log_CreateInstanceAssociation(secuAudit, "PubSub");
            if (pubSubTrace == NULL)
            {
                SOPC_CONSOLE_PRINTF("WARNING: PubSub log creation failed, no PubSub log will be recorded !");
            }

            opcUaAudit = SOPC_Log_CreateInstanceAssociation(secuAudit, "OpcUa");
            if (opcUaAudit == NULL)
            {
                SOPC_CONSOLE_PRINTF("WARNING: OpcUa audit log creation failed, no OpcUa audit log will be recorded !");
            }
            else
            {
                SOPC_Log_SetLogLevel(opcUaAudit, SOPC_LOG_LEVEL_INFO); // Set INFO level for opcUa audit
            }
        }
        else
        {
            SOPC_Log_ClearInstance(&secuAudit);
        }
    }
    else
    {
        SOPC_CONSOLE_PRINTF("WARNING: log creation failed, no log will be recorded !\n");
    }
    return result;
}

void SOPC_Logger_SetTraceLogLevel(SOPC_Log_Level level)
{
    if (commonTrace != NULL)
    {
        SOPC_Log_SetLogLevel(commonTrace, level);
    }
    if (clientServerTrace != NULL)
    {
        SOPC_Log_SetLogLevel(clientServerTrace, level);
    }
    if (pubSubTrace != NULL)
    {
        SOPC_Log_SetLogLevel(pubSubTrace, level);
    }
}

SOPC_Log_Level SOPC_Logger_GetTraceLogLevel(void)
{
    return SOPC_Log_GetLogLevel(commonTrace);
}

void SOPC_Logger_SetConsoleOutput(bool activate)
{
    if (secuAudit != NULL)
    {
        SOPC_Log_SetConsoleOutput(secuAudit, activate);
    }
    if (commonTrace != NULL)
    {
        SOPC_Log_SetConsoleOutput(commonTrace, activate);
    }
    if (opcUaAudit != NULL)
    {
        SOPC_Log_SetConsoleOutput(opcUaAudit, activate);
    }
    if (clientServerTrace != NULL)
    {
        SOPC_Log_SetConsoleOutput(clientServerTrace, activate);
    }
    if (pubSubTrace != NULL)
    {
        SOPC_Log_SetConsoleOutput(pubSubTrace, activate);
    }
}

static void logger_Trace(SOPC_Log_Module logModule, SOPC_Log_Level logLevel, const char* format, va_list args)
{
    switch (logModule)
    {
    case SOPC_LOG_MODULE_COMMON:
        if (commonTrace != NULL)
        {
            SOPC_Log_VTrace(commonTrace, logLevel, format, args);
        }
        break;
    case SOPC_LOG_MODULE_CLIENTSERVER:
        if (clientServerTrace != NULL)
        {
            SOPC_Log_VTrace(clientServerTrace, logLevel, format, args);
        }
        break;
    case SOPC_LOG_MODULE_PUBSUB:
        if (pubSubTrace != NULL)
        {
            SOPC_Log_VTrace(pubSubTrace, logLevel, format, args);
        }
        break;
    default:
        break;
    }
}

void SOPC_Logger_TraceError(SOPC_Log_Module logModule, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_Trace(logModule, SOPC_LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

void SOPC_Logger_TraceWarning(SOPC_Log_Module logModule, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_Trace(logModule, SOPC_LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void SOPC_Logger_TraceInfo(SOPC_Log_Module logModule, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_Trace(logModule, SOPC_LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void SOPC_Logger_TraceDebug(SOPC_Log_Module logModule, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_Trace(logModule, SOPC_LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void SOPC_Logger_TraceSecurityAudit(const char* format, ...)
{
    va_list args;
    if (secuAudit != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(secuAudit, SOPC_LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceSecurityAuditWarning(const char* format, ...)
{
    va_list args;
    if (secuAudit != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(secuAudit, SOPC_LOG_LEVEL_WARNING, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceOpcUaAudit(const char* format, ...)
{
    va_list args;
    if (opcUaAudit != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(opcUaAudit, SOPC_LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceOpcUaAuditWarning(const char* format, ...)
{
    va_list args;
    if (opcUaAudit != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(opcUaAudit, SOPC_LOG_LEVEL_WARNING, format, args);
        va_end(args);
    }
}

void SOPC_Logger_Clear(void)
{
    if (userLogArray == NULL)
    {
        SOPC_Array_Delete(userLogArray);
        userLogArray = NULL;
    }
    if (commonTrace != NULL)
    {
        SOPC_Log_ClearInstance(&commonTrace);
    }
    if (clientServerTrace != NULL)
    {
        SOPC_Log_ClearInstance(&clientServerTrace);
    }
    if (pubSubTrace != NULL)
    {
        SOPC_Log_ClearInstance(&pubSubTrace);
    }
    if (opcUaAudit != NULL)
    {
        SOPC_Log_ClearInstance(&opcUaAudit);
    }
    // secuAudit is the "actual" instance and must be deleted last.
    if (secuAudit != NULL)
    {
        SOPC_Log_ClearInstance(&secuAudit);
    }
    SOPC_Free(filePath);
    filePath = NULL;
}
