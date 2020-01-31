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

static SOPC_Log_Instance* toolkitTrace = NULL;
static SOPC_Log_Instance* secuAudit = NULL;
static SOPC_Log_Instance* opcUaAudit = NULL;

bool SOPC_Logger_Initialize(const char* logDirPath, uint32_t maxBytes, uint16_t maxFiles)
{
    bool result = false;
    if (logDirPath != NULL)
    {
        SOPC_Log_Initialize();
        secuAudit = SOPC_Log_CreateInstance(logDirPath, "Trace", "SecuAudit", maxBytes, maxFiles);
        if (secuAudit != NULL)
        {
            result = SOPC_Log_SetLogLevel(secuAudit, SOPC_LOG_LEVEL_INFO); // Set INFO level for secu audit

            if (result != false)
            {
                toolkitTrace = SOPC_Log_CreateInstanceAssociation(secuAudit, "Toolkit");
                if (toolkitTrace == NULL)
                {
                    printf("WARNING: toolkit log creation failed, no toolkit log will be recorded !");
                }

                opcUaAudit = SOPC_Log_CreateInstanceAssociation(secuAudit, "OpcUa");
                if (opcUaAudit == NULL)
                {
                    printf("WARNING: OpcUa audit log creation failed, no OpcUa audit log will be recorded !");
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
            printf("WARNING: log creation failed, no log will be recorded !\n");
        }
    }
    return result;
}

void SOPC_Logger_SetTraceLogLevel(SOPC_Log_Level level)
{
    if (toolkitTrace != NULL)
    {
        SOPC_Log_SetLogLevel(toolkitTrace, level);
    }
}

void SOPC_Logger_SetConsoleOutput(bool activate)
{
    if (secuAudit != NULL)
    {
        SOPC_Log_SetConsoleOutput(secuAudit, activate);
    }
    if (toolkitTrace != NULL)
    {
        SOPC_Log_SetConsoleOutput(toolkitTrace, activate);
    }
    if (opcUaAudit != NULL)
    {
        SOPC_Log_SetConsoleOutput(opcUaAudit, activate);
    }
}

void SOPC_Logger_TraceError(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_ERROR, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceWarning(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_WARNING, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceInfo(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceDebug(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_DEBUG, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceSecurityAudit(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceSecurityAuditWarning(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_WARNING, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceOpcUaAudit(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void SOPC_Logger_TraceOpcUaAuditWarning(const char* format, ...)
{
    va_list args;
    if (toolkitTrace != NULL)
    {
        va_start(args, format);
        SOPC_Log_VTrace(toolkitTrace, SOPC_LOG_LEVEL_WARNING, format, args);
        va_end(args);
    }
}

void SOPC_Logger_Clear(void)
{
    if (secuAudit != NULL)
    {
        SOPC_Log_ClearInstance(&secuAudit);
    }
    if (toolkitTrace != NULL)
    {
        SOPC_Log_ClearInstance(&toolkitTrace);
    }
    if (opcUaAudit != NULL)
    {
        SOPC_Log_ClearInstance(&opcUaAudit);
    }
    SOPC_Log_Clear();
}
