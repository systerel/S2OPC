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

// WARNING: this source file is only included if the XML library (Expat) is available

#if !defined(SOPC_WITH_EXPAT) || SOPC_WITH_EXPAT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client_config.h"
#include "libs2opc_client_internal.h"
#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"

#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_user_app_itf.h"

#include "xml_expat/sopc_config_loader.h"

static FILE* SOPC_HelperInternal_OpenFileFromPath(const char* filename)
{
    FILE* fd = fopen(filename, "r");

    if (fd == NULL)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Configuration file %s cannot be opened. Please check path.\n", filename);
    }

    return fd;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_ConfigureFromXML(const char* clientConfigPath,
                                                           SOPC_ConfigClientXML_Custom* customConfig,
                                                           size_t* nbScConfigs,
                                                           SOPC_SecureConnection_Config*** scConfigArray)
{
    if (NULL == clientConfigPath || NULL == nbScConfigs || NULL == scConfigArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_UNUSED_ARG(customConfig);

    FILE* fd = SOPC_HelperInternal_OpenFileFromPath(clientConfigPath);

    if (fd == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    bool res = SOPC_ConfigClient_Parse(fd, &pConfig->clientConfig);
    fclose(fd);

    if (!res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Error parsing configuration file %s. Please check logged errors.\n", clientConfigPath);
    }

    *nbScConfigs = pConfig->clientConfig.nbSecureConnections;
    *scConfigArray = pConfig->clientConfig.secureConnections;

    return SOPC_STATUS_OK;
}

#endif /* SOPC_WITH_EXPAT */
