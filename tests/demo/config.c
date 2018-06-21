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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_pki_stack.h"

#include "config.h"

/* Only supports one set of certificates at a time. They are all shared by the configs. */
int nCfgCreated = 0; /* Number of created configs with certificates, to remember when to release certificates */
SOPC_SerializedCertificate* pCrtCli = NULL;
SOPC_SerializedCertificate* pCrtSrv = NULL;
SOPC_SerializedAsymmetricKey* pKeyCli = NULL;
SOPC_SerializedCertificate* pCrtCAu = NULL;
SOPC_PKIProvider* pPki = NULL;

SOPC_ReturnStatus Config_LoadCertificates(void);

SOPC_SecureChannel_Config* Config_NewSCConfig(const char* reqSecuPolicyUri, OpcUa_MessageSecurityMode msgSecurityMode)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    bool bInconsistentPolicyMode = false;

    /* Check that SecuPolicy None <=> SecuMode None */
    bInconsistentPolicyMode =
        strncmp(reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0;
    bInconsistentPolicyMode ^= OpcUa_MessageSecurityMode_None == msgSecurityMode;
    if (bInconsistentPolicyMode)
    {
        printf(
            "# Error: inconsistent security mode, message security mode may be None if and only if security policy is "
            "None.\n");
        status = SOPC_STATUS_NOK;
    }

    /* Try to load the certificates before the creation of the config */
    if (SOPC_STATUS_OK == status && OpcUa_MessageSecurityMode_None != msgSecurityMode)
    {
        status = Config_LoadCertificates();
    }

    /* Create the configuration */
    if (SOPC_STATUS_OK == status)
    {
        pscConfig = malloc(sizeof(SOPC_SecureChannel_Config));

        if (NULL != pscConfig)
        {
            pscConfig->isClientSc = true;
            pscConfig->url = ENDPOINT_URL;
            pscConfig->crt_cli = NULL;
            pscConfig->key_priv_cli = NULL;
            pscConfig->crt_srv = NULL;
            pscConfig->pki = pPki;
            pscConfig->requestedLifetime = SC_LIFETIME;
            pscConfig->reqSecuPolicyUri = reqSecuPolicyUri;
            pscConfig->msgSecurityMode = msgSecurityMode;

            if (OpcUa_MessageSecurityMode_None != msgSecurityMode)
            {
                pscConfig->crt_cli = pCrtCli;
                pscConfig->crt_srv = pCrtSrv;
                pscConfig->key_priv_cli = pKeyCli;
            }
        }
    }

    return pscConfig;
}

void Config_DeleteSCConfig(SOPC_SecureChannel_Config** ppscConfig)
{
    if (NULL == ppscConfig || NULL == *ppscConfig)
        return;

    if (NULL != (*ppscConfig)->crt_cli)
    {
        nCfgCreated -= 1;
    }

    free(*ppscConfig);
    *ppscConfig = NULL;

    /* Garbage collect, if needed */
    if (0 == nCfgCreated)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(pCrtCli);
        SOPC_KeyManager_SerializedCertificate_Delete(pCrtSrv);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(pKeyCli);
        SOPC_PKIProviderStack_Free(pPki);
        SOPC_KeyManager_SerializedCertificate_Delete(pCrtCAu);
        pCrtCli = NULL;
        pCrtSrv = NULL;
        pKeyCli = NULL;
        pPki = NULL;
        pCrtCAu = NULL;
    }
}

SOPC_ReturnStatus Config_LoadCertificates(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 == nCfgCreated)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(PATH_CLIENT_PUBL, &pCrtCli);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to load client certificate\n");
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(PATH_SERVER_PUBL, &pCrtSrv);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load server certificate\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(PATH_CLIENT_PRIV, &pKeyCli);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load client private key\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(PATH_CACERT_PUBL, &pCrtCAu);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load CA\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(pCrtCAu, NULL, &pPki);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to create PKI\n");
            }
        }
    }

    nCfgCreated += 1; /* If it failed once, do not try again */

    return status;
}
