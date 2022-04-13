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

/** \file
 *
 * \brief A client library that supports and automates the subscription.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_log_manager.h"
#include "sopc_types.h"
#include "sopc_version.h"
#include "toolkit_helpers.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"
#include "libs2opc_client_common.h"
#include "libs2opc_common_config.h"

/* ==================
 * API implementation
 * ==================
 */

SOPC_LibSub_CstString SOPC_LibSub_GetVersion(void)
{
    return "S2OPC Toolkit subscribe library v" SOPC_TOOLKIT_VERSION;
}

SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = {.logLevel = pCfg->toolkit_logger.level,
                                               .logSystem = SOPC_LOG_SYSTEM_FILE,
                                               .logSysConfig.fileSystemLogConfig = {
                                                   .logDirPath = pCfg->toolkit_logger.log_path,
                                                   .logMaxBytes = pCfg->toolkit_logger.maxBytes,
                                                   .logMaxFiles = pCfg->toolkit_logger.maxFiles,
                                               }};

    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientCommon_Initialize(pCfg, NULL);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_CommonHelper_Clear();
    }
    return status;
}

void SOPC_LibSub_Clear(void)
{
    SOPC_ClientCommon_Clear();
    SOPC_CommonHelper_Clear();
}

SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId)
{
    SOPC_ReturnStatus status = SOPC_ClientCommon_ConfigureConnection(pCfg, pCfgId);
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Configured(void)
{
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId)
{
    SOPC_ReturnStatus status = SOPC_ClientCommon_Connect(cfgId, pCliId);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientCommon_CreateSubscription(*pCliId, NULL);
    }
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                const SOPC_LibSub_CstString* lszNodeId,
                                                const SOPC_LibSub_AttributeId* lattrId,
                                                int32_t nElements,
                                                SOPC_LibSub_DataId* lDataId)
{
    OpcUa_CreateMonitoredItemsResponse response;
    SOPC_EncodeableObject_Initialize(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, &response);
    SOPC_ReturnStatus status =
        SOPC_ClientCommon_AddToSubscription(cliId, lszNodeId, lattrId, nElements, lDataId, &response);
    if (SOPC_STATUS_OK == status && response.NoOfResults != (int32_t) nElements)
    {
        status = SOPC_STATUS_NOK;
    }
    for (int32_t i = 0; i < nElements && i < response.NoOfResults; ++i)
    {
        SOPC_StatusCode ResultStatus = response.Results[i].StatusCode;
        if (SOPC_IsGoodStatus(ResultStatus))
        {
            Helpers_Log(SOPC_LOG_LEVEL_INFO, "MonitoredItem with index '%" PRIi32 "' created.", i);
        }
        else
        {
            Helpers_Log(SOPC_LOG_LEVEL_WARNING,
                        "Server could not create monitored item with index '%" PRIi32 "', sc = 0x%08" PRIX32 ".", i,
                        ResultStatus);
        }
    }

    SOPC_EncodeableObject_Clear(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, &response);
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_AsyncSendRequestOnSession(SOPC_LibSub_ConnectionId cliId,
                                                        void* requestStruct,
                                                        uintptr_t requestContext)
{
    SOPC_ReturnStatus status = SOPC_ClientCommon_AsyncSendRequestOnSession(cliId, requestStruct, requestContext);
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Disconnect(const SOPC_LibSub_ConnectionId cliId)
{
    SOPC_ReturnStatus status = SOPC_ClientCommon_Disconnect(cliId);
    return status;
}
