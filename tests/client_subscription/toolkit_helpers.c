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
#include "sopc_encodeable.h"
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

SOPC_ReturnStatus Helpers_NewCreateSubscriptionRequest(double fPublishIntervalMs,
                                                       uint32_t iCntLifetime,
                                                       uint32_t iCntMaxKeepAlive,
                                                       void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_CreateSubscriptionRequest* pReq = NULL;

    if (NULL == ppRequest)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_CreateSubscriptionRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        pReq->RequestedPublishingInterval = fPublishIntervalMs;
        pReq->RequestedLifetimeCount = iCntLifetime;
        pReq->RequestedMaxKeepAliveCount = iCntMaxKeepAlive;
        pReq->MaxNotificationsPerPublish = MAX_NOTIFICATIONS_PER_REQUEST;
        pReq->PublishingEnabled = true;
        pReq->Priority = 0;
        *ppRequest = (void*) pReq;
    }

    return status;
}

SOPC_ReturnStatus Helpers_NewPublishRequest(bool bAck, uint32_t iSubId, uint32_t iSeqNum, void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_PublishRequest* pReq = NULL;

    if (NULL == ppRequest)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_PublishRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (bAck)
        {
            pReq->NoOfSubscriptionAcknowledgements = 1;
            pReq->SubscriptionAcknowledgements =
                (OpcUa_SubscriptionAcknowledgement*) malloc(sizeof(OpcUa_SubscriptionAcknowledgement));
            if (NULL == pReq->SubscriptionAcknowledgements)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                pReq->SubscriptionAcknowledgements->SubscriptionId = iSubId;
                pReq->SubscriptionAcknowledgements->SequenceNumber = iSeqNum;
            }
        }
        else
        {
            pReq->NoOfSubscriptionAcknowledgements = 0;
            pReq->SubscriptionAcknowledgements = NULL;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppRequest = pReq;
    }

    return status;
}
