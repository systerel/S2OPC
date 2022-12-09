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

#include "libs2opc_client_config.h"
#include "sopc_logger.h"

SOPC_GetClientKeyPassword_Fct* FctGetClientKeyPassword = NULL;

SOPC_ReturnStatus SOPC_HelperConfigClient_SetKeyPasswordCallback(SOPC_GetClientKeyPassword_Fct* getClientKeyPassword)
{
    if (NULL != FctGetClientKeyPassword)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The following user callback is already configure: SOPC_GetClientKeyPassword_Fct");
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == getClientKeyPassword)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    FctGetClientKeyPassword = getClientKeyPassword;
    return SOPC_STATUS_OK;
}

// Get password to decrypt client private key from internal callback
bool SOPC_HelperConfigClient_ClientKeyUsrPwdCb(char** outPassword)
{
    if (NULL == FctGetClientKeyPassword)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The following user callback isn't configure: SOPC_GetClientKeyPassword_Fct");
        return false;
    }
    bool res = FctGetClientKeyPassword(outPassword);
    return res;
}

bool SOPC_HelperConfigClient_IsEncryptedClientKey(void)
{
    return NULL != FctGetClientKeyPassword;
}
