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

#include <check.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "push_client_connection_helper.h"
#include "push_server_methods.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_decl.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack_lib_itf.h"

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

SOPC_ReturnStatus create_custom_secure_connection(const char* clientCertPath,
                                                  const char* clientKeyPath,
                                                  bool encrypted,
                                                  const char* clientPKIStorePath,
                                                  const char* serverCertPath,
                                                  SOPC_SecureConnection_Config** scConfig)
{
    printf("Creating a secure connection for client with certificate %s.\n", clientCertPath);

    /* Configure client */
    SOPC_PKIProvider* clientPKI = NULL;
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(clientCertPath, clientKeyPath, encrypted);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(clientPKIStorePath, &clientPKI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(clientPKI);
    }

    /* Create the secure connection and set parameters */
    SOPC_SecureConnection_Config* scConfigBuilt = NULL;
    if (SOPC_STATUS_OK == status)
    {
        scConfigBuilt = SOPC_ClientConfigHelper_CreateSecureConnection(
            "0", "opc.tcp://localhost:4841", OpcUa_MessageSecurityMode_Sign, SOPC_SecurityPolicy_Basic256Sha256);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(scConfigBuilt, serverCertPath);
    }
    if (SOPC_STATUS_OK == status)
    {
        const char* password = getenv("TEST_PASSWORD_USER_ME");
        status = SOPC_SecureConnectionConfig_SetUserName(scConfigBuilt, "username_Basic256Sha256", "me", password);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&AskKeyPass_FromTerminal);
    }

    if (SOPC_STATUS_OK == status)
    {
        *scConfig = scConfigBuilt;
    }
    else
    {
        SOPC_Free(scConfigBuilt);
    }

    return status;
}
