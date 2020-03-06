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

#include "sopc_helper_endianness_cfg.h"
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
    (void) logConfiguration;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (bCommon_IsInitialized)
    {
        return SOPC_STATUS_OK;
    }

    /* Check IEEE-754 compliance */
    if (!SOPC_IEEE_Check())
    {
        status = SOPC_STATUS_NOK;
    }

    /* Initialize endianness */
    SOPC_Helper_EndiannessCfg_Initialize();

    bool result = false;
    /* Initialize logs */
    switch (logConfiguration.logSystem)
    {
    case SOPC_LOG_SYSTEM_FILE:
        result = SOPC_Logger_Initialize(logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath,
                                        logConfiguration.logSysConfig.fileSystemLogConfig.logMaxBytes,
                                        logConfiguration.logSysConfig.fileSystemLogConfig.logMaxFiles);
        if (result)
        {
            SOPC_Logger_SetTraceLogLevel(logConfiguration.logLevel);
        }
        else
        {
            status = SOPC_STATUS_NOK;
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
