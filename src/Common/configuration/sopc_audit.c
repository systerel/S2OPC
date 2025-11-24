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

#include "sopc_audit.h"

#include <stddef.h>

#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#ifdef S2OPC_HAS_AUDITING

/** Content of internal configuration. */
typedef struct Audit_Configuration_Internal
{
    SOPC_Log_Instance* auditEntry;
    SOPC_Audit_OptionMask options;
} Audit_Configuration_Internal;

/* static variables */
static Audit_Configuration_Internal* gAuditCfg = NULL;

/* Functions */
bool SOPC_Audit_Initialize(const SOPC_Audit_Configuration* optAuditConfig)
{
    if (NULL != gAuditCfg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "SOPC_Audit_Initialize because audit is already started");
        return false;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL != optAuditConfig)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON, "Initializing the Auditing option (Options flags=%04X)",
                              (unsigned) optAuditConfig->options);
        if (0 != (optAuditConfig->options & ~SOPC_Audit_SupportedSecuOptions))
        {
            result = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            if (NULL != optAuditConfig->auditEntryPath)
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON, "Initializing the Auditing Entry option in '%s'",
                                      optAuditConfig->auditEntryPath);
            }
            else
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON,
                                      "No Auditing Entry activated since configuration path provided is NULL");
            }
            gAuditCfg = (Audit_Configuration_Internal*) SOPC_Malloc(sizeof(*gAuditCfg));
            if (NULL == gAuditCfg)
            {
                result = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                gAuditCfg->options = optAuditConfig->options;
                SOPC_CircularLogFile_Configuration logcfg;
                logcfg.logDirPath = optAuditConfig->auditEntryPath;
                logcfg.logFileName = "AuditLog";
                logcfg.logMaxBytes = optAuditConfig->logMaxBytes;
                logcfg.logMaxFiles = optAuditConfig->logMaxFiles;
                gAuditCfg->auditEntry = SOPC_Log_CreateFileInstance(&logcfg, "AUDIT");
                SOPC_Log_SetLogLevel(gAuditCfg->auditEntry, SOPC_LOG_LEVEL_INFO);
            }
        }
    }
    return (SOPC_STATUS_OK == result);
}

bool SOPC_Audit_IsAuditing(void)
{
    return (NULL != gAuditCfg);
}

bool SOPC_Audit_HasOption(SOPC_Audit_OptionMask opt)
{
    return (NULL != gAuditCfg) && ((opt & gAuditCfg->options) != 0);
}

void SOPC_Audit_Clear(void)
{
    if (NULL != gAuditCfg)
    {
        SOPC_Log_ClearInstance(&gAuditCfg->auditEntry);
        SOPC_Free(gAuditCfg);
        gAuditCfg = NULL;
    }
}

SOPC_Log_Instance* SOPC_Audit_LogEntry(void)
{
    return gAuditCfg->auditEntry;
}
#else // !S2OPC_HAS_AUDITING

bool SOPC_Audit_Initialize(const SOPC_Audit_Configuration* optAuditConfig)
{
    return NULL == optAuditConfig;
}

bool SOPC_Audit_HasOption(SOPC_Audit_OptionMask opt)
{
    SOPC_UNUSED_ARG(opt);
    return false;
}

bool SOPC_Audit_IsAuditing(void)
{
    return false;
}

void SOPC_Audit_Clear(void) {}

SOPC_Log_Instance* SOPC_Audit_LogEntry(void)
{
    return NULL;
}

#endif
