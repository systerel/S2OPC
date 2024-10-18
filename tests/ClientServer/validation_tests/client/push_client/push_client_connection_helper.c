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

#include <stdio.h>
#include <stdlib.h>

#include "push_client_connection_helper.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_pki_stack.h"

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

SOPC_ReturnStatus SOPC_Create_Custom_Secure_Connection(const char* clientCertPath,
                                                       const char* clientKeyPath,
                                                       bool encrypted,
                                                       const char* clientPKIStorePath,
                                                       const char* serverCertPath,
                                                       SOPC_SecureConnection_Config** scConfig)
{
    SOPC_ASSERT(NULL != clientCertPath);
    SOPC_ASSERT(NULL != clientKeyPath);
    SOPC_ASSERT(NULL != clientPKIStorePath);
    SOPC_ASSERT(NULL != serverCertPath);
    SOPC_ASSERT(NULL != scConfig && NULL == *scConfig);

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
        if (NULL == scConfigBuilt)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(scConfigBuilt, serverCertPath);
    }
    if (SOPC_STATUS_OK == status)
    {
        const char* password = getenv("TEST_PASSWORD_USER_ME");
        if (NULL == password)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "TEST_PASSWORD_USER_ME not set!\n");
        }
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

    return status;
}
