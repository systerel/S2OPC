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

/** \file
 *
 * \brief Helpers for the Toolkit API.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_common_constants.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#include "pki_permissive.h"
#include "toolkit_helpers.h"

/* LibSub logger callback, wrapped by a variadic printf-like */
static SOPC_LibSub_LogCbk* cbkLog = &Helpers_LoggerStdout;

SOPC_ReturnStatus Helpers_NewSCConfigFromLibSubCfg(const char* szServerUrl,
                                                   const char* szSecuPolicy,
                                                   OpcUa_MessageSecurityMode msgSecurityMode,
                                                   bool bDisablePKI,
                                                   const char* szPathCertifAuth,
                                                   const char* szPathCertServer,
                                                   const char* szPathCertClient,
                                                   const char* szPathKeyClient,
                                                   const char* szPathCrl,
                                                   uint32_t iScRequestedLifetime,
                                                   const OpcUa_GetEndpointsResponse* expectedEndpoints,
                                                   SOPC_Client_Config* clientAppCfg,
                                                   SOPC_SecureChannel_Config** ppNewCfg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    SOPC_SerializedCertificate* pCrtSrv = NULL;
    SOPC_SerializedCertificate* pCrtCli = NULL;
    SOPC_SerializedAsymmetricKey* pKeyCli = NULL;
    SOPC_PKIProvider* pPki = NULL;

    if (NULL == szServerUrl || NULL == szSecuPolicy || OpcUa_MessageSecurityMode_Invalid == msgSecurityMode ||
        NULL == clientAppCfg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check security policy and parameters consistency */
    /* CRL is always optional */
    /* If security policy is None, then security mode shall be None, and paths,
     * except CAuth and server cert (as issued), shall be NULL */
    if (strncmp(szSecuPolicy, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
    {
        if (OpcUa_MessageSecurityMode_None != msgSecurityMode || NULL != szPathCertClient || NULL != szPathKeyClient)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                        "Invalid parameters: security policy is None, but security mode is not None or paths to "
                        "client certificate and keys are not NULL.");
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    /* Else, the security mode shall not be None, and all paths shall be non NULL (except CRL) */
    else
    {
        if (OpcUa_MessageSecurityMode_None == msgSecurityMode || NULL == szPathCertClient || NULL == szPathKeyClient ||
            NULL == szPathCertServer)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                        "Invalid parameters: security policy is not None, but security mode is None or paths to "
                        "certificates and keys are NULL.");
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    /* Certificate authority shall only exist when PKI is not disabled */
    if (bDisablePKI && NULL != szPathCertifAuth)
    {
        Helpers_Log(
            SOPC_LOG_LEVEL_ERROR,
            "Invalid parameters: path to PKI can be NULL if and only if PKI security verifications are disabled.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (!bDisablePKI)
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            char* lPathsTrustedRoots[] = {(char*) szPathCertifAuth, NULL};
            char* lPathsCRL[] = {(char*) szPathCrl, NULL};
            char* lPathsTrustedLinks[] = {NULL};
            char* lPathsUntrustedRoots[] = {NULL};
            char* lPathsUntrustedLinks[] = {NULL};
            char* lPathsIssuedCerts[] = {(char*) szPathCertServer, NULL};
            SOPC_GCC_DIAGNOSTIC_RESTORE
            status = SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                           lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL, &pPki);
            if (SOPC_STATUS_OK != status)
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to create PKI.");
            }
        }
        else
        {
            Helpers_Log(SOPC_LOG_LEVEL_WARNING, "DISABLED CERTIFICATE VERIFICATION.");
            status = SOPC_PKIPermissive_Create(&pPki);
            if (SOPC_STATUS_OK != status)
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to create PKI.");
            }
        }
    }
    if (SOPC_STATUS_OK == status && OpcUa_MessageSecurityMode_None != msgSecurityMode)
    {
        if (NULL != szPathCertServer)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(szPathCertServer, &pCrtSrv);
            if (SOPC_STATUS_OK != status)
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to load server certificate.");
            }
        }

        if (SOPC_STATUS_OK == status && NULL != szPathCertClient)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(szPathCertClient, &pCrtCli);
            if (SOPC_STATUS_OK != status)
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to load client certificate.");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(szPathKeyClient, &pKeyCli);
            if (SOPC_STATUS_OK != status)
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to load client private key.");
            }
        }
    }

    /* Create the configuration */
    if (SOPC_STATUS_OK == status)
    {
        pscConfig = SOPC_Calloc(1, sizeof(SOPC_SecureChannel_Config));

        if (NULL != pscConfig)
        {
            pscConfig->isClientSc = true;
            pscConfig->crt_cli = pCrtCli;
            pscConfig->key_priv_cli = pKeyCli;
            pscConfig->crt_srv = pCrtSrv;
            pscConfig->pki = pPki;
            pscConfig->requestedLifetime = iScRequestedLifetime;
            pscConfig->msgSecurityMode = msgSecurityMode;
            pscConfig->expectedEndpoints = expectedEndpoints;
            pscConfig->clientConfigPtr = clientAppCfg;

            /* These input strings are verified non NULL */
            pscConfig->url = SOPC_Malloc(strlen(szServerUrl) + 1);
            pscConfig->reqSecuPolicyUri = SOPC_Malloc(strlen(szSecuPolicy) + 1);
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            if (NULL != pscConfig->url && NULL != pscConfig->reqSecuPolicyUri)
            {
                strcpy((char*) pscConfig->url, szServerUrl);
                strcpy((char*) pscConfig->reqSecuPolicyUri, szSecuPolicy);
                /* Handles the config to the caller */
                *ppNewCfg = pscConfig;
            }
            else
            {
                SOPC_Free((void*) pscConfig->url);
                SOPC_Free((void*) pscConfig->reqSecuPolicyUri);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            SOPC_GCC_DIAGNOSTIC_RESTORE
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_PKIProvider_Free(&pPki);
        SOPC_KeyManager_SerializedCertificate_Delete(pCrtSrv);
        SOPC_KeyManager_SerializedCertificate_Delete(pCrtCli);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(pKeyCli);
        SOPC_Free(pscConfig);
    }

    return status;
}

void Helpers_SecureChannel_Config_Free(SOPC_SecureChannel_Config** ppscConfig)
{
    if (NULL == ppscConfig || NULL == *ppscConfig)
    {
        return;
    }

    SOPC_SecureChannel_Config* pscConfig = *ppscConfig;

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) pscConfig->crt_cli);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete((SOPC_SerializedAsymmetricKey*) pscConfig->key_priv_cli);
    SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) pscConfig->crt_srv);
    SOPC_PKIProvider_Free((SOPC_PKIProvider**) (&pscConfig->pki));
    SOPC_Free((void*) pscConfig->url);
    SOPC_Free((void*) pscConfig->reqSecuPolicyUri);
    pscConfig->clientConfigPtr = NULL;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_Free(pscConfig);

    *ppscConfig = NULL;
}

SOPC_ReturnStatus Helpers_NewCreateSubscriptionRequest(double fPublishIntervalMs,
                                                       uint32_t iCntMaxKeepAlive,
                                                       uint32_t iCntLifetime,
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

SOPC_ReturnStatus Helpers_NewDeleteSubscriptionRequest(uint32_t subscriptionId, void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_DeleteSubscriptionsRequest* pReq = NULL;

    if (NULL == ppRequest)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_DeleteSubscriptionsRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        uint32_t* pSubscriptionId = (uint32_t*) SOPC_Malloc(sizeof(uint32_t));
        if (NULL == pSubscriptionId)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            *pSubscriptionId = subscriptionId;
            pReq->NoOfSubscriptionIds = 1;
            pReq->SubscriptionIds = pSubscriptionId;
            *ppRequest = (void*) pReq;
        }
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
            status = SOPC_Encodeable_Create(&OpcUa_SubscriptionAcknowledgement_EncodeableType,
                                            (void**) &pReq->SubscriptionAcknowledgements);
            if (SOPC_STATUS_OK == status)
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
    else if (NULL != pReq)
    {
        if (NULL != pReq->SubscriptionAcknowledgements)
        {
            SOPC_Free(pReq->SubscriptionAcknowledgements);
        }
        SOPC_Encodeable_Delete(&OpcUa_PublishRequest_EncodeableType, (void**) &pReq);
    }

    return status;
}

SOPC_ReturnStatus Helpers_NewCreateMonitoredItemsRequest(SOPC_NodeId** lpNid,
                                                         const uint32_t* liAttrId,
                                                         int32_t nElems,
                                                         uint32_t iSubId,
                                                         OpcUa_TimestampsToReturn tsToReturn,
                                                         uint32_t* liCliHndl,
                                                         uint32_t iQueueSize,
                                                         void** ppRequest)
{
    OpcUa_CreateMonitoredItemsRequest* pReq = NULL;
    OpcUa_MonitoredItemCreateRequest* pitc = NULL;

    if (NULL == ppRequest || NULL == lpNid || NULL == liAttrId || NULL == liCliHndl || 0 >= nElems)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    for (int i = 0; i < nElems; ++i)
    {
        if (NULL == lpNid[i] || 0 == liAttrId[i] || 22 < liAttrId[i])
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    SOPC_ReturnStatus status =
        SOPC_Encodeable_Create(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pReq);

    if (SOPC_STATUS_OK == status)
    {
        pitc = SOPC_Calloc((size_t) nElems, sizeof(OpcUa_MonitoredItemCreateRequest));
        if (NULL == pitc)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pReq->SubscriptionId = iSubId;
        pReq->TimestampsToReturn = tsToReturn;
        pReq->NoOfItemsToCreate = nElems;
        pReq->ItemsToCreate = pitc;
        for (int i = 0; i < nElems; ++i)
        {
            OpcUa_MonitoredItemCreateRequest_Initialize(&pitc[i]);
            pitc[i].ItemToMonitor.NodeId = *lpNid[i];
            pitc[i].ItemToMonitor.AttributeId = liAttrId[i];
            SOPC_String_Initialize(&pitc[i].ItemToMonitor.IndexRange);
            SOPC_QualifiedName_Initialize(&pitc[i].ItemToMonitor.DataEncoding);
            pitc[i].MonitoringMode = OpcUa_MonitoringMode_Reporting;
            pitc[i].RequestedParameters.ClientHandle = liCliHndl[i];
            pitc[i].RequestedParameters.SamplingInterval = 0;
            SOPC_ExtensionObject_Initialize(&pitc[i].RequestedParameters.Filter);
            pitc[i].RequestedParameters.QueueSize = iQueueSize;
            pitc[i].RequestedParameters.DiscardOldest = true;
        }

        *ppRequest = (void*) pReq;
    }
    else if (NULL != pReq)
    {
        SOPC_Encodeable_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pReq);
    }

    return status;
}

SOPC_ReturnStatus Helpers_NewValueFromDataValue(SOPC_DataValue* pVal, SOPC_LibSub_Value** pplsVal)
{
    SOPC_LibSub_Value* plsVal = NULL;

    if (NULL == pVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    plsVal = SOPC_Calloc(1, sizeof(SOPC_LibSub_Value));
    if (NULL == plsVal)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Create the value, according to the type of the DataValue, only for non arrays and non matrix */
    plsVal->value = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_VariantArrayType_SingleValue != pVal->Value.ArrayType)
    {
        plsVal->type = SOPC_LibSub_DataType_other;
    }
    else
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
            plsVal->type = SOPC_LibSub_DataType_integer;
            plsVal->value = SOPC_Malloc(sizeof(int64_t));
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
                default:
                    break;
                }
            }
            break;
        case SOPC_UInt64_Id:
            if (INT64_MAX >= pVal->Value.Value.Uint64)
            {
                plsVal->type = SOPC_LibSub_DataType_integer;
                plsVal->value = SOPC_Malloc(sizeof(int64_t));
                if (NULL == plsVal->value)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
                else
                {
                    *(int64_t*) plsVal->value = (int64_t) pVal->Value.Value.Uint64;
                }
            }
            else
            {
                plsVal->type = SOPC_LibSub_DataType_other;
            }
            break;
        case SOPC_String_Id:
            plsVal->type = SOPC_LibSub_DataType_string;
            if (pVal->Value.Value.String.Length > 0)
            {
                plsVal->value = SOPC_String_GetCString(&pVal->Value.Value.String);
                if (NULL == plsVal->value)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            /* else we leave value NULL */
            break;
        case SOPC_ByteString_Id:
            plsVal->type = SOPC_LibSub_DataType_bytestring;
            if (pVal->Value.Value.Bstring.Length > 0)
            {
                plsVal->length = pVal->Value.Value.Bstring.Length;
                plsVal->value = SOPC_Malloc((size_t) plsVal->length);
                if (NULL != plsVal->value)
                {
                    memcpy(plsVal->value, pVal->Value.Value.Bstring.Data, (size_t) plsVal->length);
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            /* else we leave value NULL and length = 0 */
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
            plsVal->type = SOPC_LibSub_DataType_other;
            break;
        }
    }

    /* Always copy the raw value */
    if (SOPC_STATUS_OK == status)
    {
        plsVal->raw_value = SOPC_Variant_Create();
        if (NULL == plsVal->raw_value)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Variant_Copy(plsVal->raw_value, &pVal->Value);
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
            SOPC_Free(plsVal->value);
            plsVal->value = NULL;
            SOPC_Variant_Delete(plsVal->raw_value);
            SOPC_Free(plsVal);
            plsVal = NULL;
        }
    }

    return status;
}

SOPC_LibSub_Timestamp Helpers_OPCTimeToNTP(SOPC_DateTime ts)
{
    /* We are not before 1601 */
    assert(0 <= ts);
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

void Helpers_Log(const SOPC_Log_Level log_level, const char* format, ...)
{
    if (NULL == cbkLog)
    {
        return;
    }

    va_list args;
    char buffer[2048];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, args);
    va_end(args);

    cbkLog(log_level, buffer);
}

void Helpers_SetLogger(SOPC_LibSub_LogCbk cbk)
{
    cbkLog = cbk;
}

void Helpers_LoggerStdout(const SOPC_Log_Level log_level, const SOPC_LibSub_CstString text)
{
    SOPC_UNUSED_ARG(text); // In case SOPC_CONSOLE_PRINTF is not defined
    SOPC_CONSOLE_PRINTF("# ");
    switch (log_level)
    {
    case SOPC_LOG_LEVEL_ERROR:
        SOPC_CONSOLE_PRINTF("Error");
        break;
    case SOPC_LOG_LEVEL_WARNING:
        SOPC_CONSOLE_PRINTF("Warning");
        break;
    case SOPC_LOG_LEVEL_INFO:
        SOPC_CONSOLE_PRINTF("Info");
        break;
    case SOPC_LOG_LEVEL_DEBUG:
        SOPC_CONSOLE_PRINTF("Debug");
        break;
    default:
        assert(false && "Unkown log level.");
        break;
    }
    SOPC_CONSOLE_PRINTF(": %s\n", text);
}
