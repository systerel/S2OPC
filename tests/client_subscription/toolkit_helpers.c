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

/** \file
 *
 * \brief Helpers for the Toolkit API.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"

#include "toolkit_helpers.h"

SOPC_ReturnStatus Helpers_NewSCConfigFromLibSubCfg(const char* szServerUrl,
                                                   const char* szSecuPolicy,
                                                   OpcUa_MessageSecurityMode msgSecurityMode,
                                                   const char* szPathCertifAuth,
                                                   const char* szPathCertServer,
                                                   const char* szPathCertClient,
                                                   const char* szPathKeyClient,
                                                   const char* szPathCrl,
                                                   uint32_t iScRequestedLifetime,
                                                   SOPC_SecureChannel_Config** ppNewCfg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    SOPC_Certificate* pCrtCAu = NULL;
    SOPC_Certificate* pCrtSrv = NULL;
    SOPC_Certificate* pCrtCli = NULL;
    SOPC_AsymmetricKey* pKeyCli = NULL;
    SOPC_PKIProvider* pPki = NULL;

    /* Check parameters */
    if (NULL == szServerUrl || NULL == szSecuPolicy || OpcUa_MessageSecurityMode_Invalid == msgSecurityMode)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check security policy and parameters consistency */
    if (SOPC_STATUS_OK == status)
    {
        /* CRL is always optional */
        /* If security policy is None, then security mode shall be None, and paths, except CAuth, shall be NULL */
        if (strncmp(szSecuPolicy, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
        {
            if (OpcUa_MessageSecurityMode_None != msgSecurityMode || NULL != szPathCertClient ||
                NULL != szPathCertClient || NULL != szPathCertServer)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        /* Else, the security mode shall not be None, and all paths shall be non NULL (except CRL) */
        else
        {
            if (OpcUa_MessageSecurityMode_None == msgSecurityMode || NULL == szPathCertClient ||
                NULL == szPathCertClient || NULL == szPathCertServer)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        /* Certificate authority shall always exist */
        if (SOPC_STATUS_OK == status && NULL == szPathCertifAuth)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Load the certificates & CRL before the creation of the PKI, then the config */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateFromFile(szPathCertifAuth, &pCrtCAu);
        if (SOPC_STATUS_OK != status)
        {
            /* TODO: update logs */
            printf("# Error: Failed to load the CA\n");
        }
    }
    /* TODO: handle Revocation list */
    if (SOPC_STATUS_OK == status && NULL != szPathCrl)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to load Certificate Revocation List\n");
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
    if (SOPC_STATUS_OK == status && OpcUa_MessageSecurityMode_None != msgSecurityMode)
    {
        if (NULL != szPathCertServer)
        {
            status = SOPC_KeyManager_Certificate_CreateFromFile(szPathCertServer, &pCrtSrv);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load server certificate\n");
            }
        }

        if (SOPC_STATUS_OK == status && NULL != szPathCertClient)
        {
            status = SOPC_KeyManager_Certificate_CreateFromFile(szPathCertClient, &pCrtCli);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load client certificate\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_AsymmetricKey_CreateFromFile(szPathKeyClient, &pKeyCli, NULL, 0);
            if (SOPC_STATUS_OK != status)
            {
                printf("# Error: Failed to load client private key\n");
            }
        }
    }

    /* Create the configuration */
    if (SOPC_STATUS_OK == status)
    {
        pscConfig = malloc(sizeof(SOPC_SecureChannel_Config));

        if (NULL != pscConfig)
        {
            pscConfig->isClientSc = true;
            pscConfig->url = szServerUrl;
            pscConfig->crt_cli = pCrtCli;
            pscConfig->key_priv_cli = pKeyCli;
            pscConfig->crt_srv = pCrtSrv;
            pscConfig->pki = pPki;
            pscConfig->requestedLifetime = iScRequestedLifetime;
            pscConfig->reqSecuPolicyUri = szSecuPolicy;
            pscConfig->msgSecurityMode = msgSecurityMode;

            /* Handles the config to the caller */
            *ppNewCfg = pscConfig;
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    return status;
}
