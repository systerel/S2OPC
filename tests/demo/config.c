/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_pki_stack.h"

#include "config.h"

/* Only supports one set of certificates at a time. They are all shared by the configs. */
int nCfgCreated = 0; /* Number of created configs with certificates, to remember when to release certificates */
SOPC_Certificate* pCrtCli = NULL;
SOPC_Certificate* pCrtSrv = NULL;
SOPC_AsymmetricKey* pKeyCli = NULL;
SOPC_Certificate* pCrtCAu = NULL;
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
        SOPC_KeyManager_Certificate_Free(pCrtCli);
        SOPC_KeyManager_Certificate_Free(pCrtSrv);
        SOPC_KeyManager_AsymmetricKey_Free(pKeyCli);
        SOPC_PKIProviderStack_Free(pPki);
        SOPC_KeyManager_Certificate_Free(pCrtCAu);
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
        status = SOPC_KeyManager_Certificate_CreateFromFile(PATH_CLIENT_PUBL, &pCrtCli);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to load client certificate\n");
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateFromFile(PATH_SERVER_PUBL, &pCrtSrv);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load server certificate\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_AsymmetricKey_CreateFromFile(PATH_CLIENT_PRIV, &pKeyCli, NULL, 0);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load client private key\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateFromFile(PATH_CACERT_PUBL, &pCrtCAu);
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
