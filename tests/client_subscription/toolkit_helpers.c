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

#include <assert.h>
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

SOPC_ReturnStatus Helpers_NewCreateMonitoredItemsRequest(SOPC_NodeId* pNid,
                                                         uint32_t iAttrId,
                                                         uint32_t iSubId,
                                                         OpcUa_TimestampsToReturn tsToReturn,
                                                         uint32_t iCliHndl,
                                                         uint32_t iQueueSize,
                                                         void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_CreateMonitoredItemsRequest* pReq = NULL;
    OpcUa_MonitoredItemCreateRequest* pitc = NULL;

    if (NULL == pNid || NULL == ppRequest || 0 == iAttrId || 22 < iAttrId)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        pitc = (OpcUa_MonitoredItemCreateRequest*) malloc(sizeof(OpcUa_MonitoredItemCreateRequest));
        if (NULL == pitc)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pReq->SubscriptionId = iSubId;
        pReq->TimestampsToReturn = tsToReturn;
        pReq->NoOfItemsToCreate = 1;
        pReq->ItemsToCreate = pitc;
        pitc->ItemToMonitor.NodeId = *pNid;
        pitc->ItemToMonitor.AttributeId = iAttrId;
        SOPC_String_Initialize(&pitc->ItemToMonitor.IndexRange);
        SOPC_QualifiedName_Initialize(&pitc->ItemToMonitor.DataEncoding);
        pitc->MonitoringMode = OpcUa_MonitoringMode_Reporting;
        pitc->RequestedParameters.ClientHandle = iCliHndl;
        pitc->RequestedParameters.SamplingInterval = 0;
        SOPC_ExtensionObject_Initialize(&pitc->RequestedParameters.Filter);
        pitc->RequestedParameters.QueueSize = iQueueSize;
        pitc->RequestedParameters.DiscardOldest = true;

        *ppRequest = (void*) pReq;
    }

    return status;
}

SOPC_ReturnStatus Helpers_NewValueFromDataValue(SOPC_DataValue* pVal, SOPC_LibSub_Value** pplsVal)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LibSub_Value* plsVal = NULL;

    if (NULL == pVal || SOPC_VariantArrayType_SingleValue != pVal->Value.ArrayType)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        plsVal = (SOPC_LibSub_Value*) malloc(sizeof(SOPC_LibSub_Value));
        if (NULL == plsVal)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Create the value, according to the type of the DataValue */
    if (SOPC_STATUS_OK == status)
    {
        switch (pVal->Value.BuiltInTypeId)
        {
        case SOPC_Boolean_Id:
        case SOPC_SByte_Id:
        case SOPC_Byte_Id:
        case SOPC_Int16_Id:
        case SOPC_UInt16_Id:
        case SOPC_Int32_Id:
        case SOPC_UInt32_Id:
        case SOPC_Int64_Id:
        case SOPC_UInt64_Id:
            plsVal->type = SOPC_LibSub_DataType_integer;
            plsVal->value = malloc(sizeof(int64_t));
            if (NULL == plsVal->value)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                switch (pVal->Value.BuiltInTypeId)
                {
                case SOPC_Boolean_Id:
                    plsVal->type = SOPC_LibSub_DataType_bool;
                    *(int64_t*) plsVal->value = pVal->Value.Value.Boolean;
                    break;
                case SOPC_SByte_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Sbyte;
                    break;
                case SOPC_Byte_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Byte;
                    break;
                case SOPC_Int16_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Int16;
                    break;
                case SOPC_UInt16_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Uint16;
                    break;
                case SOPC_Int32_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Int32;
                    break;
                case SOPC_UInt32_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Uint32;
                    break;
                case SOPC_Int64_Id:
                    *(int64_t*) plsVal->value = pVal->Value.Value.Int64;
                    break;
                case SOPC_UInt64_Id:
                    if (INT64_MAX < pVal->Value.Value.Uint64)
                    {
                        status = SOPC_STATUS_NOK;
                    }
                    else
                    {
                        *(int64_t*) plsVal->value = (int64_t) pVal->Value.Value.Uint64;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SOPC_String_Id:
            plsVal->type = SOPC_LibSub_DataType_string;
            plsVal->value = SOPC_String_GetCString(&pVal->Value.Value.String);
            break;
        case SOPC_ByteString_Id:
            plsVal->type = SOPC_LibSub_DataType_bytestring;
            plsVal->value = SOPC_String_GetCString((SOPC_String*) &pVal->Value.Value.Bstring);
            break;
        case SOPC_Null_Id:
        case SOPC_Float_Id:
        case SOPC_Double_Id:
        case SOPC_DateTime_Id:
        case SOPC_Guid_Id:
        case SOPC_XmlElement_Id:
        case SOPC_NodeId_Id:
        case SOPC_ExpandedNodeId_Id:
        case SOPC_StatusCode_Id:
        case SOPC_QualifiedName_Id:
        case SOPC_LocalizedText_Id:
        case SOPC_ExtensionObject_Id:
        case SOPC_DataValue_Id:
        case SOPC_Variant_Id:
        case SOPC_DiagnosticInfo_Id:
        default:
            status = SOPC_STATUS_NOK;
            break;
        }

        if (SOPC_STATUS_OK == status)
        {
            /* Maybe string copy failed */
            if (NULL == plsVal->value)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }

    /* Quality and Timestamps */
    if (SOPC_STATUS_OK == status)
    {
        plsVal->quality = pVal->Status;
        plsVal->source_timestamp = Helpers_OPCTimeToNTP(pVal->SourceTimestamp);
        plsVal->server_timestamp = Helpers_OPCTimeToNTP(pVal->ServerTimestamp);
        /* Value is ready, modify given pointer */
        *pplsVal = plsVal;
    }

    /* Partial mallocs */
    if (SOPC_STATUS_OK != status)
    {
        if (NULL != plsVal)
        {
            if (NULL != plsVal->value)
            {
                free(plsVal->value);
                plsVal->value = NULL;
            }
            free(plsVal);
            plsVal = NULL;
        }
    }

    return status;
}

SOPC_LibSub_Timestamp Helpers_OPCTimeToNTP(SOPC_DateTime ts)
{
    /* We are not before 1601 */
    assert(0 <= ts);
    /* We are not after year 30848 */
    assert(INT64_MAX >= ts);
    /* So we can use unsigned arithmetics, and get rid of warnings. */
    uint64_t uts = (uint64_t) ts;

    /* First, subtract the difference between epochs */
    uts -= 9435484800 * 10000000;
    /* For the fraction of seconds, take modulo, then multiply, then divide,
     * which never overflows because modulo is < 2^32 */
    uint64_t fraction = (((uts % 10000000) << 32) / 10000000);
    /* for the second part, divide first, then multiply, which keeps the most significant bits of ts */
    uint64_t seconds = ((uts / 10000000) << 32);

    /* seconds are always > 2**32, fraction is always < 2**32, it is possible to OR them without mask */
    return seconds | fraction;
}
