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

#include <stddef.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_audit.h"
#include "sopc_crypto_decl.h"
#include "sopc_event.h"
#include "sopc_event_helpers.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secure_channels_audit.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"

#ifdef S2OPC_HAS_AUDITING

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_internal.h"

/********************/
/* LOCAL VARIABLES  */
/********************/
static const SOPC_NodeId AuditOpenSecureChannelEvent_Type =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditOpenSecureChannelEventType);
static const SOPC_NodeId AuditChannelEvent_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditChannelEventType);
static const SOPC_NodeId AuditCertificateExpiredEventType =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateExpiredEventType);
static const SOPC_NodeId AuditCertificateUntrustedEventType =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateUntrustedEventType);
static const SOPC_NodeId AuditCertificateInvalidEventType =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateInvalidEventType);
static const SOPC_NodeId AuditCertificateDataMismatchEventType =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateDataMismatchEventType);
static const SOPC_NodeId AuditCertificateEventType = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateEventType);
static const SOPC_NodeId AuditCertificateRevokedEventType =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateRevokedEventType);

// Predefined OPCUA ENUM values
#define ENUM_OPEN_ISSUE_VALUE (0u)
#define ENUM_OPEN_RENEW_VALUE (1u)

/* Define some default values for Event severities */
#ifndef SEVERITY_OPEN_SECURE_CHANNEL_SUCCESS
#define SEVERITY_OPEN_SECURE_CHANNEL_SUCCESS 1
#endif

#ifndef SEVERITY_OPEN_SECURE_CHANNEL_FAILURE
#define SEVERITY_OPEN_SECURE_CHANNEL_FAILURE 10
#endif

#ifndef SEVERITY_CLOSE_SECURE_CHANNEL_SUCCESS
#define SEVERITY_CLOSE_SECURE_CHANNEL_SUCCESS 1
#endif

#ifndef SEVERITY_CLOSE_SECURE_CHANNEL_FAILURE
#define SEVERITY_CLOSE_SECURE_CHANNEL_FAILURE 10
#endif

#ifndef SEVERITY_CERTIFICATE_ISSUE
#define SEVERITY_CERTIFICATE_ISSUE 100
#endif

/********************/
/* EXTERN FUNCTIONS */
/********************/
/*********************************************************************************/
static char* SOPC_Audit_GetAltClientAuditInfo(const SOPC_SecureConnection* scConnection)
{
    return scConnection->altClientAuditInfo;
}

void SOPC_Audit_TriggerEvent_OpenSecuredChannel(const char* message,
                                                const SOPC_StatusCode statusCode,
                                                const OpcUa_RequestHeader* reqHeader,
                                                const SOPC_SecureConnection* scConnection,
                                                bool isRenew,
                                                const SOPC_ByteString* auditCertEventId)
{
    if (!SOPC_Audit_HasOption(SOPC_Audit_Options_AuditSecureChannel))
    {
        return;
    }
    // Check parameters
    SOPC_ASSERT(NULL != scConnection && NULL != message);
    // Get AuditEntryId and Timestamp from request header if defined
    const char* auditEntryId = (NULL != reqHeader && reqHeader->AuditEntryId.Length > 0)
                                   ? SOPC_String_GetRawCString(&reqHeader->AuditEntryId)
                                   : NULL;
    if (NULL == auditEntryId)
    {
        auditEntryId = SOPC_Audit_GetAltClientAuditInfo(scConnection);
    }
    SOPC_DateTime actionTimeStamp = (NULL != reqHeader) ? reqHeader->Timestamp : SOPC_Time_GetCurrentTimeUTC();

    // Retrieve SecureChannel contexts
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->secureChannelConfigIdx);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    // Create the event object
    SOPC_Event* event = NULL;
    if (NULL != pConfig)
    {
        status = SOPC_ServerHelper_CreateEvent(&AuditOpenSecureChannelEvent_Type, &event);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set inherited event attributes
        SOPC_EventsHelpers_SetBaseEventType(event, "SecureChannel/OpenSecureChannel", message,
                                            SOPC_IsGoodStatus(statusCode) ? SEVERITY_OPEN_SECURE_CHANNEL_SUCCESS
                                                                          : SEVERITY_OPEN_SECURE_CHANNEL_FAILURE);
        SOPC_EventsHelpers_SetAuditSecurityEventType(event, statusCode);
        SOPC_EventsHelpers_SetAuditChannelEventType(event, scConnection->currentSecurityToken.secureChannelId);
        // Note: we always set the System prefix as the user is not available here
        SOPC_EventsHelpers_SetAuditEventType(
            event, auditEntryId, actionTimeStamp,
            SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri),
            "System/OpenSecureChannel", (statusCode & SOPC_BadStatusMask) == 0);

        // Fill specific part of OpenSecureChannelEvent_Type
        SOPC_CertificateList* clientCert = NULL;
        if (scConnection->clientCertificate != NULL)
        {
            clientCert = scConnection->clientCertificate;
        }
        else
        {
            // Trye to use the temporary certificate if defined
            clientCert = scConnection->serverAsymmSecuInfo.clientCertificate;
        }
        // CertificateErrorEventId is optional (not used here)
        // ClientCertificate
        if (NULL != clientCert)
        {
            uint8_t* pDest = NULL;
            uint32_t lenAllocated = INT32_MAX;
            SOPC_ReturnStatus localStatus = SOPC_KeyManager_Certificate_ToDER(clientCert, &pDest, &lenAllocated);
            if (SOPC_STATUS_OK == localStatus && lenAllocated < INT32_MAX && pDest != NULL)
            {
                SOPC_EventHelpers_SetByteString(event, "0:ClientCertificate", pDest, lenAllocated);
            }
            else
            {
                SOPC_EventHelpers_SetByteString(event, "0:ClientCertificate", NULL, 0);
            }
            SOPC_Free(pDest);
        }
        else
        {
            SOPC_EventHelpers_SetByteString(event, "0:ClientCertificate", NULL, 0);
        }
        // ClientCertificateThumbprint
        char* clientThumbPrint = SOPC_KeyManager_Certificate_GetCstring_SHA1(clientCert);
        SOPC_EventHelpers_SetCString(event, "0:ClientCertificateThumbprint", clientThumbPrint);
        SOPC_Free(clientThumbPrint);

        SOPC_EventHelpers_SetInt32(event, "0:RequestType", (isRenew ? ENUM_OPEN_RENEW_VALUE : ENUM_OPEN_ISSUE_VALUE));
        if (NULL != scConfig)
        {
            SOPC_EventHelpers_SetCString(event, "0:SecurityPolicyUri", scConfig->reqSecuPolicyUri);
            SOPC_EventHelpers_SetInt32(event, "0:SecurityMode", (int32_t) scConfig->msgSecurityMode);
            SOPC_EventHelpers_SetUInt32(event, "0:RequestedLifetime", scConfig->requestedLifetime);
        }
        else
        {
            SOPC_EventHelpers_SetCString(event, "0:SecurityPolicyUri",
                                         scConnection->serverAsymmSecuInfo.securityPolicyUri);
            SOPC_EventHelpers_SetInt32(event, "0:SecurityMode", OpcUa_MessageSecurityMode_Invalid);
            SOPC_EventHelpers_SetUInt32(event, "0:RequestedLifetime", 0);
        }
        if (NULL != auditCertEventId && auditCertEventId->Length > 0)
        {
            SOPC_EventHelpers_SetByteString(event, "0:CertificateErrorEventId", auditCertEventId->Data,
                                            (uint32_t) auditCertEventId->Length);
        }
        status = SOPC_ServerInternal_TriggerAuditEvent(event);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Audit OpenSecuredChannel(%s) failed with code=> %d",
                                 message, (int) status);
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Audit OpenSecuredChannel(%s) created", message);
    }
}

/*********************************************************************************/
void SOPC_Audit_TriggerEvent_CloseSecuredChannel(const char* message,
                                                 const SOPC_StatusCode statusCode,
                                                 const OpcUa_RequestHeader* reqHeader,
                                                 const SOPC_SecureConnection* scConnection)
{
    if (!SOPC_Audit_HasOption(SOPC_Audit_Options_AuditSecureChannel))
    {
        return;
    }
    // Check parameters
    SOPC_ASSERT(NULL != scConnection && NULL != message);
    // Get AuditEntryId and Timestamp from request header if defined
    const char* auditEntryId = (NULL != reqHeader && reqHeader->AuditEntryId.Length > 0)
                                   ? SOPC_String_GetRawCString(&reqHeader->AuditEntryId)
                                   : NULL;
    SOPC_DateTime actionTimeStamp = (NULL != reqHeader) ? reqHeader->Timestamp : 0;
    if (NULL == auditEntryId)
    {
        auditEntryId = SOPC_Audit_GetAltClientAuditInfo(scConnection);
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    // Retrieve SecureChannel contexts
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->secureChannelConfigIdx);

    // Create the event object
    SOPC_Event* event = NULL;
    if (NULL != pConfig && NULL != scConfig)
    {
        status = SOPC_ServerHelper_CreateEvent(&AuditChannelEvent_Type, &event);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set inherited event attributes
        SOPC_EventsHelpers_SetBaseEventType(event, "SecureChannel/CloseSecureChannel", message,
                                            SOPC_IsGoodStatus(statusCode) || OpcUa_BadSecureChannelClosed == statusCode
                                                ? SEVERITY_CLOSE_SECURE_CHANNEL_SUCCESS
                                                : SEVERITY_CLOSE_SECURE_CHANNEL_FAILURE);
        SOPC_EventsHelpers_SetAuditSecurityEventType(event, statusCode);
        SOPC_EventsHelpers_SetAuditChannelEventType(event, scConnection->currentSecurityToken.secureChannelId);
        // Note: we always set the System prefix as the user is not available here
        SOPC_EventsHelpers_SetAuditEventType(
            event, auditEntryId, actionTimeStamp,
            SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri),
            "System/CloseSecureChannel", (statusCode == OpcUa_BadSecureChannelClosed));

        status = SOPC_ServerInternal_TriggerAuditEvent(event);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Audit CloseSecuredChannel(%s) failed with code=> %d",
                                 message, (int) status);
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Audit CloseSecuredChannel(%s) created", message);
    }
}

/*********************************************************************************/
void SOPC_Audit_TriggerEvent_CertificateFailure(const SOPC_Audit_Certificate_Error_Context* context,
                                                const char* auditEntryId,
                                                const SOPC_DateTime actionTimestamp,
                                                SOPC_ByteString* eventId)
{
    if (!SOPC_Audit_HasOption(SOPC_Audit_Options_AuditCertificate) || NULL == context)
    {
        return;
    }

    // Retrieve SecureChannel contexts
    SOPC_Event* event = NULL;
    const SOPC_NodeId* eventTypeId = NULL;
    const char* subject = SOPC_String_GetRawCString(&context->subject);
    const char* message = (context->message ? context->message : "");

    switch (context->opcua_error_code)
    {
    case SOPC_CertificateValidationError_IssuerTimeInvalid:
    case SOPC_CertificateValidationError_TimeInvalid:
        // AuditCertificateExpiredEventType
        eventTypeId = &AuditCertificateExpiredEventType;
        break;
    case SOPC_CertificateValidationError_Untrusted:
        // AuditCertificateUntrustedEventType
        eventTypeId = &AuditCertificateUntrustedEventType;
        break;
    case SOPC_CertificateValidationError_Revoked:
    case SOPC_CertificateValidationError_IssuerRevoked:
        // AuditCertificateRevokedEventType
        eventTypeId = &AuditCertificateRevokedEventType;
        break;
    case SOPC_CertificateValidationError_Invalid:
        // AuditCertificateInvalidEventType
        eventTypeId = &AuditCertificateInvalidEventType;
        break;

    case SOPC_CertificateValidationError_HostNameInvalid:
    case SOPC_CertificateValidationError_UriInvalid:
        // AuditCertificateDataMismatchEventType
        eventTypeId = &AuditCertificateDataMismatchEventType;
        SOPC_EventHelpers_SetCString(event, "0:InvalidUri", context->invalidURI);
        SOPC_EventHelpers_SetCString(event, "0:InvalidHostname", context->invalidHostname);
        message = "Mismatching Certificate Data";
        break;
    default:
        // AuditCertificateEventType
        eventTypeId = &AuditCertificateEventType;

        break;
    }

    SOPC_ServerHelper_CreateEvent(eventTypeId, &event);
    if (NULL != event)
    {
        SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
        // Set inherited event attributes
        SOPC_EventsHelpers_SetBaseEventType(event, "Security/Certificate", message, SEVERITY_CERTIFICATE_ISSUE);
        SOPC_EventsHelpers_SetAuditSecurityEventType(event, context->opcua_error_code);
        SOPC_EventsHelpers_SetAuditEventType(
            event, auditEntryId, actionTimestamp,
            SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri), subject, false);

        // Set AuditCertificateEventType attributes
        if (context->certificate.Length >= 0)
        {
            SOPC_EventHelpers_SetByteString(event, "0:Certificate", context->certificate.Data,
                                            (uint32_t) context->certificate.Length);
        }

        if (NULL != eventId)
        {
            SOPC_ReturnStatus tmpStatus = SOPC_ByteString_Copy(eventId, SOPC_Event_GetEventId(event));
            SOPC_UNUSED_RESULT(tmpStatus);
        }

        // Send event
        SOPC_ReturnStatus status = SOPC_ServerInternal_TriggerAuditEvent(event);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Certificate Event(%s) failed with code=> %d",
                                     message, (int) status);
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Certificate Event(%s) created", message);
        }
    }
}

/*********************************************************************************/
void SOPC_Audit_Create_CertificateFailure_Context_Initialize(SOPC_Audit_Certificate_Error_Context* pContext,
                                                             const SOPC_CertificateList* pCertlist)
{
    if (NULL != pContext)
    {
        memset(pContext, 0, sizeof(*pContext));

        uint32_t len;
        char* subject = NULL;
        SOPC_ReturnStatus statusCert = SOPC_KeyManager_Certificate_GetSubjectName(pCertlist, &subject, &len);
        if (SOPC_STATUS_OK == statusCert && NULL != subject)
        {
            pContext->subject.Data = (SOPC_Byte*) subject;
            pContext->subject.Length = (int32_t) strlen(subject);
        }

        uint8_t* pDest = NULL;
        uint32_t lenAllocated = INT32_MAX;
        SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_ToDER(pCertlist, &pDest, &lenAllocated);
        if (SOPC_STATUS_OK == status && lenAllocated < INT32_MAX && pDest != NULL)
        {
            status = SOPC_ByteString_CopyFromBytes(&pContext->certificate, pDest, (int32_t) lenAllocated);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "Certificate Event(Initialize) failed with code=> %d", (int) status);
                memset(&pContext->certificate, 0, sizeof(pContext->certificate)); // Ensure the object is cleared
            }
        }
        SOPC_Free(pDest);
    }
}

void SOPC_Audit_Create_CertificateFailure_Context_InitializeAlt(SOPC_Audit_Certificate_Error_Context* pContext,
                                                                const SOPC_ByteString* pBsCert)
{
    if (NULL != pContext && NULL != pBsCert)
    {
        SOPC_CertificateList* cert = NULL;
        SOPC_ReturnStatus status =
            SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBsCert->Data, (uint32_t) pBsCert->Length, &cert);
        if (SOPC_STATUS_OK == status && NULL != cert)
        {
            SOPC_Audit_Create_CertificateFailure_Context_Initialize(pContext, cert);
        }
        SOPC_KeyManager_Certificate_Free(cert);
    }
}

void SOPC_Audit_Create_CertificateFailure_Context_ChangeForThumbprint(SOPC_Audit_Certificate_Error_Context* pContext,
                                                                      const SOPC_ByteString* pBsCertTb)
{
    if (NULL == pContext || NULL == pBsCertTb)
    {
        return;
    }
    // Clear previous certificate if any
    SOPC_ByteString_Clear(&pContext->certificate);
    SOPC_ReturnStatus status =
        SOPC_ByteString_CopyFromBytes(&pContext->certificate, pBsCertTb->Data, (int32_t) pBsCertTb->Length);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Certificate thumbprint Event(Change) failed with code=> %d", (int) status);
        memset(&pContext->certificate, 0, sizeof(pContext->certificate)); // Ensure the object is cleared
    }
}

/*********************************************************************************/
void SOPC_Audit_Create_CertificateFailure_Context_Clear(SOPC_Audit_Certificate_Error_Context* pContext)
{
    if (NULL != pContext)
    {
        SOPC_String_Clear(&pContext->subject);
        SOPC_ByteString_Clear(&pContext->certificate);
        SOPC_Free(pContext->invalidHostname);
        SOPC_Free(pContext->invalidURI);
    }
}

#endif // S2OPC_HAS_AUDITING
