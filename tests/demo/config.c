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

#include <stdio.h>
#include <stdlib.h>

#include "sopc_pki_stack.h"

#include "config.h"

/* Only supports one set of certificates at a time. They are all shared by the configs. */
int nCrtCreated = 0; /* Number of created configs with certificates, to remember when to release certificates */
SOPC_Certificate* pCrtCli = NULL;
SOPC_Certificate* pCrtSrv = NULL;
SOPC_AsymmetricKey* pKeyCli = NULL;
SOPC_Certificate* pCrtCAu = NULL;
int nPkiCreated = 0;
SOPC_PKIProvider* pPki = NULL;

SOPC_ReturnStatus Config_LoadCertificates(void);
SOPC_ReturnStatus Config_LoadPki(void);

SOPC_SecureChannel_Config* Config_NewSCConfig(const char* reqSecuPolicyUri, OpcUa_MessageSecurityMode msgSecurityMode)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    /* It is hoped that this function is not called with inconsistent secPolicy and secMode (None <=> None) */

    /* Try to load the certificates before the creation of the config */
    if (OpcUa_MessageSecurityMode_None != msgSecurityMode)
    {
        /* TODO: it was useless to split these, should merge them... */
        status = Config_LoadCertificates();
        status = Config_LoadPki();
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
        nCrtCreated -= 1;
    }

    free(*ppscConfig);
    *ppscConfig = NULL;

    /* Garbage collect, if needed */
    if (0 == nCrtCreated)
    {
        SOPC_KeyManager_Certificate_Free(pCrtCli);
        SOPC_KeyManager_Certificate_Free(pCrtSrv);
        SOPC_KeyManager_AsymmetricKey_Free(pKeyCli);
        pCrtCli = NULL;
        pCrtSrv = NULL;
        pKeyCli = NULL;
    }

    nPkiCreated -= 1;
    if (0 == nPkiCreated)
    {
        SOPC_PKIProviderStack_Free(pPki);
        pPki = NULL;
    }

    if (NULL == pPki && NULL == pCrtCli)
    {
        SOPC_KeyManager_Certificate_Free(pCrtCAu);
        pCrtCAu = NULL;
    }
}

SOPC_ReturnStatus Config_LoadCertificates(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 == nCrtCreated && nPkiCreated > 0)
    {
        printf("# Error: PKI was created without CA Cert.\n");
        printf("  Please create scConfigs for secured Sc before unsecured ones.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && 0 == nCrtCreated)
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
                printf(">>Stub_Client: Failed to load CA\n");
            }
        }
    }

    nCrtCreated += 1; /* If it failed once, do not try again */

    return status;
}

SOPC_ReturnStatus Config_LoadPki(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (0 == nPkiCreated)
    {
        status = SOPC_PKIProviderStack_Create(pCrtCAu, NULL, &pPki);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to create PKI\n");
        }
    }

    nPkiCreated += 1; /* If it failed once, do not try again */

    return status;
}
