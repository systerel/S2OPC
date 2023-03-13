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
#include "libs2opc_client_internal.h"
#include "sopc_assert.h"
#include "sopc_logger.h"

static SOPC_GetPassword_Fct* FctGetClientKeyPassword = NULL;
static SOPC_GetPassword_Fct* FctGetUserKeyPassword = NULL;

static SOPC_ReturnStatus SetPasswordCallback(SOPC_GetPassword_Fct** destCb, SOPC_GetPassword_Fct* getKeyPassword)
{
    SOPC_ASSERT(NULL != destCb);
    if (NULL == getKeyPassword)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *destCb = getKeyPassword;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigClient_SetClientKeyPasswordCallback(SOPC_GetPassword_Fct* getClientKeyPassword)
{
    return SetPasswordCallback(&FctGetClientKeyPassword, getClientKeyPassword);
}

SOPC_ReturnStatus SOPC_HelperConfigClient_SetUserKeyPasswordCallback(SOPC_GetPassword_Fct* getUserKeyPassword)
{
    return SetPasswordCallback(&FctGetUserKeyPassword, getUserKeyPassword);
}

static bool SOPC_ClientInternal_GetPassword(SOPC_GetPassword_Fct* passwordCb, const char* cbName, char** outPassword)
{
    if (NULL == passwordCb)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "The following callback isn't configured: %s", cbName);
        return false;
    }
    return passwordCb(outPassword);
}

// Get password to decrypt user private key from internal callback
bool SOPC_ClientInternal_GetClientKeyPassword(char** outPassword)
{
    return SOPC_ClientInternal_GetPassword(FctGetClientKeyPassword, "ClientKeyPasswordCallback", outPassword);
}

// Get password to decrypt user private key from internal callback
bool SOPC_ClientInternal_GetUserKeyPassword(char** outPassword)
{
    return SOPC_ClientInternal_GetPassword(FctGetUserKeyPassword, "UserKeyPasswordCallback", outPassword);
}

bool SOPC_ClientInternal_IsEncryptedClientKey(void)
{
    return NULL != FctGetClientKeyPassword;
}

bool SOPC_ClientInternal_IsEncryptedUserKey(void)
{
    return NULL != FctGetUserKeyPassword;
}
