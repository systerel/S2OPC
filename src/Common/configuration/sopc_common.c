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

#include "sopc_common.h"

#include "sopc_filesystem.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_helper_string.h"
#include "sopc_ieee_check.h"
#include "sopc_logger.h"

/* static variables */

static bool bCommon_IsInitialized = false;

/* Functions */

bool SOPC_Common_IsInitialized(void)
{
    return bCommon_IsInitialized;
}

SOPC_ReturnStatus SOPC_Common_Initialize(SOPC_Log_Configuration logConfiguration)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (bCommon_IsInitialized)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    /* Check IEEE-754 compliance */
    if (!SOPC_IEEE_Check())
    {
        return SOPC_STATUS_NOK;
    }

    /* Initialize endianness */
    SOPC_Helper_EndiannessCfg_Initialize();

    bool result = false;
    int cmpRes = 0;
    SOPC_FileSystem_CreationResult mkdirRes = SOPC_FileSystem_Creation_Error_UnknownIssue;
    /* Initialize logs */
    switch (logConfiguration.logSystem)
    {
    case SOPC_LOG_SYSTEM_FILE:
        if (NULL != logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath)
        {
            cmpRes = SOPC_strcmp_ignore_case("", logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath);
        } // else: nothing to compare and no directory to create
        if (0 == cmpRes)
        {
            // Nothing to create
            mkdirRes = SOPC_FileSystem_Creation_OK;
        }
        else
        {
            mkdirRes = SOPC_FileSystem_mkdir(logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath);
        }
        if (SOPC_FileSystem_Creation_OK != mkdirRes && SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdirRes)
        {
            fprintf(stderr, "WARNING: Cannot create log directory ('%d'), defaulting to current directory\n", mkdirRes);
            logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "";
        }

        result = SOPC_Logger_Initialize(&logConfiguration);
        if (result)
        {
            SOPC_Logger_SetTraceLogLevel(logConfiguration.logLevel);
        }
        else
        {
            /* Status stays OK given that we don't have other alternatives for now */
            fprintf(stderr, "ERROR: S2OPC Logs initialization failed!\n");
        }
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    /* Set the IsInitialized status if everything was successful */
    if (SOPC_STATUS_OK == status)
    {
        bCommon_IsInitialized = true;
    }

    return status;
}

void SOPC_Common_Clear(void)
{
    bCommon_IsInitialized = false;
    SOPC_Logger_Clear();
}

SOPC_ReturnStatus SOPC_Common_SetLogLevel(SOPC_Log_Level level)
{
    SOPC_Logger_SetTraceLogLevel(level);
    return SOPC_STATUS_OK;
}

SOPC_Log_Configuration SOPC_Common_GetDefaultLogConfiguration(void)
{
    SOPC_Log_Configuration defaultLogConfiguration = {
        .logLevel = SOPC_LOG_LEVEL_INFO,
        .logSystem = SOPC_LOG_SYSTEM_FILE,
        .logSysConfig = {.fileSystemLogConfig = {.logDirPath = "", .logMaxBytes = 1048576, .logMaxFiles = 50}}};
    return defaultLogConfiguration;
}
