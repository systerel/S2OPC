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

#include "libs2opc_client_toolkit_config.h"
#include "sopc_logger.h"

SOPC_ClientKeyUsrPwd_Fct* FctclientKeyUsrPwdCb = NULL;

SOPC_ReturnStatus SOPC_HelperConfigClient_SetClientKeyUsrPwdCallback(SOPC_ClientKeyUsrPwd_Fct* clientKeyUsrPwdCb)
{
    if (NULL != FctclientKeyUsrPwdCb)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The following user callback is already configure: SOPC_ClientKeyUsrPwd_Fct");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == clientKeyUsrPwdCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    FctclientKeyUsrPwdCb = clientKeyUsrPwdCb;
    return SOPC_STATUS_OK;
}

// Get user password to decrypt client private key from internal callback
void SOPC_HelperConfigClient_ClientKeyUsrPwdCb(SOPC_String** ppPassword, SOPC_StatusCode* writtenStatus)
{
    if (NULL == FctclientKeyUsrPwdCb)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The following user callback isn't configure: SOPC_ClientKeyUsrPwd_Fct");
        *writtenStatus = SOPC_STATUS_INVALID_PARAMETERS;
        return;
    }
    FctclientKeyUsrPwdCb(ppPassword, writtenStatus);
}

bool SOPC_HelperConfigClient_IsEncryptedClientKey(void)
{
    return NULL != FctclientKeyUsrPwdCb;
}
