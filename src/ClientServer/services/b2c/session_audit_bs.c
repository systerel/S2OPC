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

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_audit_bs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_event.h"
#include "sopc_event_helpers.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_toolkit_config_internal.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_internal.h"

#include "app_cb_call_context_internal.h"
#include "constants.h"
#include "util_b2c.h"

/* Define some default values for Event severities */
#ifndef SOPC_AUDIT_SEVERITY_CREATE_SESSION_SUCCESS
#define SOPC_AUDIT_SEVERITY_CREATE_SESSION_SUCCESS 1
#endif

#ifndef SOPC_AUDIT_SEVERITY_CREATE_SESSION_FAILURE
#define SOPC_AUDIT_SEVERITY_CREATE_SESSION_FAILURE 10
#endif

#ifndef SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_SUCCESS
#define SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_SUCCESS 1
#endif

#ifndef SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_FAILURE
#define SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_FAILURE 10
#endif

#ifndef SOPC_AUDIT_SEVERITY_CLOSE_SESSION_SUCCESS
#define SOPC_AUDIT_SEVERITY_CLOSE_SESSION_SUCCESS 1
#endif

#ifndef SOPC_AUDIT_SEVERITY_CLOSE_SESSION_FAILURE
#define SOPC_AUDIT_SEVERITY_CLOSE_SESSION_FAILURE 10
#endif

#ifdef S2OPC_HAS_AUDITING
/*------------------------
   LOCAL VARIABLES
  ------------------------*/
static const SOPC_NodeId ServerObject_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);
static const SOPC_NodeId auditCreateSessionEvent_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCreateSessionEventType);
static const SOPC_NodeId auditActivateSessionEvent_Type =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditActivateSessionEventType);
static const SOPC_NodeId auditSessionEvent_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditSessionEventType);

// Note : SubjectName and Token are not setup in the same context, and thus are not gathered in a single struct.

/**
 * Associate SessionId to session index (B model id).
 *
 * Note: this is a temporary solution as the session does not store this NodeId which is defined only in
 * ::msg_session_bs__write_create_session_msg_session_token, once node is actually generated it should be stored in the
 * session parameters and retrieved from it when needed.
 */
static SOPC_NodeId session_Nids[SOPC_MAX_SESSIONS];

/* Current request audit information */
static const char* auditEntryId = NULL;
static SOPC_DateTime actionTimeStamp = 0;

/********************/
/* LOCAL FUNCTIONS  */
/********************/
/*********************************************************************************/
static void set_UserIdentityToken(SOPC_Event* const event, const SOPC_ExtensionObject* token)
{
    SOPC_ASSERT(NULL != token);
    SOPC_ReturnStatus res;

    SOPC_Variant var;
    SOPC_Variant_Initialize(&var);

    if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
        (&OpcUa_AnonymousIdentityToken_EncodeableType == token->Body.Object.ObjType ||
         &OpcUa_X509IdentityToken_EncodeableType == token->Body.Object.ObjType ||
         &OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType))
    {
        // Copy the token
        var.BuiltInTypeId = SOPC_ExtensionObject_Id;
        var.Value.ExtObject = (SOPC_ExtensionObject*) SOPC_Malloc(sizeof(SOPC_ExtensionObject));
        SOPC_ExtensionObject_Initialize(var.Value.ExtObject);
        res = SOPC_ExtensionObject_Copy(var.Value.ExtObject, token);
        if (res == SOPC_STATUS_OK &&
            &OpcUa_UserNameIdentityToken_EncodeableType == var.Value.ExtObject->Body.Object.ObjType)
        {
            OpcUa_UserNameIdentityToken* userToken = var.Value.ExtObject->Body.Object.Value;
            // Remove password for UserName token
            SOPC_ByteString_Clear(&userToken->Password);
        }
    }

    SOPC_Event_SetVariableFromStrPath(event, "0:UserIdentityToken", &var);
    SOPC_Variant_Clear(&var);
}

/*********************************************************************************/
static void copy_SignedSoftwareCertificates(SOPC_Event* const event,
                                            const char* attrName,
                                            int32_t noOfSSCs,
                                            const OpcUa_SignedSoftwareCertificate* ssc)
{
    if (NULL == event || NULL == attrName || (NULL == ssc && noOfSSCs > 0))
    {
        return;
    }

    /* The attribute is an array of structures defined as:
     * - certificateData : ByteString
     * - signature : ByteString
     */
    SOPC_Variant var;
    SOPC_Variant_Initialize(&var);

    var.BuiltInTypeId = SOPC_ExtensionObject_Id;
    var.ArrayType = SOPC_VariantArrayType_Array;

    OpcUa_SignedSoftwareCertificate* ssCert = NULL;
    SOPC_ExtensionObject* obj = NULL;
    SOPC_ReturnStatus copyStatus = SOPC_STATUS_OK;
    if (noOfSSCs > 0)
    {
        obj = SOPC_Calloc((size_t) noOfSSCs, sizeof(*obj));
    }
    for (int32_t i = 0; NULL != obj && i < noOfSSCs; i++)
    {
        SOPC_ExtensionObject_Initialize(&obj[i]);
        if (SOPC_STATUS_OK == copyStatus)
        {
            copyStatus = SOPC_ExtensionObject_CreateObject(&obj[i], &OpcUa_SignedSoftwareCertificate_EncodeableType,
                                                           (void**) &ssCert);
        }
        if (SOPC_STATUS_OK == copyStatus)
        {
            copyStatus = SOPC_EncodeableObject_Copy(&OpcUa_SignedSoftwareCertificate_EncodeableType, ssCert, &ssc[i]);
        }
    }
    var.Value.Array.Length = noOfSSCs;
    var.Value.Array.Content.ExtObjectArr = obj;
    if (SOPC_STATUS_OK == copyStatus)
    {
        SOPC_Event_SetVariableFromStrPath(event, attrName, &var);
    }

    SOPC_Variant_Clear(&var);
}

/*********************************************************************************/

static char* getSubjectName(int32_t certLen, SOPC_Byte* certData)
{
    if (certLen <= 0 || NULL == certData)
    {
        return NULL;
    }
    char* result = NULL;
    SOPC_CertificateList* pCrtUser = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(certData, (uint32_t) certLen, &pCrtUser);
    if (SOPC_STATUS_OK == status)
    {
        uint32_t len;
        char* subject = NULL;
        status = SOPC_KeyManager_Certificate_GetSubjectName(pCrtUser, &subject, &len);
        if (SOPC_STATUS_OK == status)
        {
            result = subject;
            subject = NULL;
        }
        SOPC_Free(subject);
    }
    SOPC_KeyManager_Certificate_Free(pCrtUser);
    return result;
}

/**
 * Extract the X509 subject name or user Id. The returned value shall be freed.
 *  Return NULL otherwise. */
static char* getClientUserIdFromUserToken(const SOPC_ExtensionObject* token)
{
    if (NULL == token)
    {
        return NULL;
    }

    char* result = NULL;

    if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
        &OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = (OpcUa_UserNameIdentityToken*) (token->Body.Object.Value);

        result = SOPC_String_GetCString(&userToken->UserName);
    }
    else if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
             &OpcUa_X509IdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_X509IdentityToken* pX509Token = (OpcUa_X509IdentityToken*) (token->Body.Object.Value);
        result = getSubjectName(pX509Token->CertificateData.Length, pX509Token->CertificateData.Data);
    }

    return result;
}

/**
 * Extract the X509 subject name or user Id. The returned value shall be freed.
 *  Return NULL otherwise. */
static char* getClientUserIdFromUser(const SOPC_User* user)
{
    if (NULL == user)
    {
        return NULL;
    }

    char* result = NULL;

    if (SOPC_User_IsUsername(user))
    {
        result = SOPC_String_GetCString(SOPC_User_GetUsername(user));
    }
    else if (SOPC_User_IsCertificate(user))
    {
        const SOPC_ByteString* bsCert = SOPC_User_GetCertificate(user);
        result = getSubjectName(bsCert->Length, bsCert->Data);
    }

    return result;
}

static void set_AuditSession_Attributes(SOPC_Event* event, const SOPC_NodeId* sessionId)
{
    if (NULL != sessionId)
    {
        SOPC_EventHelpers_SetNodeId(event, "0:SessionId", sessionId);
    }
    else
    {
        SOPC_EventHelpers_SetNull(event, "0:SessionId");
    }
}

static SOPC_Event* prepare_session_event(const SOPC_NodeId* eventType)
{
    SOPC_Event* event = NULL;

    if (SOPC_Audit_HasOption(SOPC_Audit_Options_AuditSession) && NULL != eventType)
    {
        // Create the event object
        SOPC_ServerHelper_CreateEvent(eventType, &event);
    }

    return event;
}

static void send_event(SOPC_Event* event, const char* title, const char* message)
{
    SOPC_ReturnStatus status = SOPC_ServerInternal_TriggerAuditEvent(event);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Audit %s(%s) failed with code=> %d", title, message,
                                 (int) status);
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Audit %s(%s) created", title, message);
    }
}

static void set_sessionNid(SOPC_SessionId sessionId, const SOPC_NodeId* sessionNid)
{
    SOPC_NodeId* result = NULL;
    if (sessionId < SOPC_MAX_SESSIONS)
    {
        // Clear previous context
        SOPC_NodeId_Clear(&session_Nids[sessionId]);
        result = &session_Nids[sessionId];
        if (NULL != sessionNid)
        {
            SOPC_ReturnStatus res = SOPC_NodeId_Copy(result, sessionNid);
            SOPC_UNUSED_RESULT(res);
        }
    }
}

static const SOPC_NodeId* get_SessionNid(SOPC_SessionId sessionId)
{
    return (sessionId < SOPC_MAX_SESSIONS ? &session_Nids[sessionId] : NULL);
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_audit_bs__INITIALISATION(void)
{
    memset(&session_Nids, 0, sizeof(session_Nids));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_audit_bs__clear_audit_info(void)
{
    auditEntryId = NULL;
    actionTimeStamp = 0;
}

void session_audit_bs__server_notify_session_create(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_i session_audit_bs__req_msg,
    const constants__t_msg_i session_audit_bs__resp_msg,
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    // Retrieve Session contexts
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_Event* event = prepare_session_event(&auditCreateSessionEvent_Type);

    if (NULL == pConfig || NULL == event)
    {
        return;
    }

    const SOPC_NodeId* sessionId = NULL;
    const OpcUa_CreateSessionRequest* pReq = (const OpcUa_CreateSessionRequest*) session_audit_bs__req_msg;
    const OpcUa_CreateSessionResponse* pResp = (const OpcUa_CreateSessionResponse*) session_audit_bs__resp_msg;
    SOPC_EventHelpers_SetDouble(event, "0:RevisedSessionTimeout", pResp->RevisedSessionTimeout);

    SOPC_StatusCode statusRet;
    util_status_code__B_to_C(session_audit_bs__oper_ret_code, &statusRet);

    //////////////////////////////////////////////////////
    // Set inherited event attributes
    const bool isOk = SOPC_IsGoodStatus(statusRet);
    const char* message = (isOk ? "Create session successful" : "Create session failed");
    const uint16_t severity =
        (isOk ? SOPC_AUDIT_SEVERITY_CREATE_SESSION_SUCCESS : SOPC_AUDIT_SEVERITY_CREATE_SESSION_FAILURE);
    if (isOk)
    {
        sessionId = &pResp->SessionId;
        set_sessionNid(session_audit_bs__session, sessionId);
    } // else: keep sessionId NULL

    SOPC_EventsHelpers_SetBaseEventType(event, "Session/CreateSession", message, severity);
    SOPC_EventsHelpers_SetAuditSecurityEventType(event, statusRet);
    // The SourceNode Property for Events of this type shall be assigned to the Server Object.
    SOPC_EventHelpers_SetNodeId(event, "0:SourceNode", &ServerObject_NodeId);
    SOPC_EventsHelpers_SetAuditEventType(
        event, auditEntryId, actionTimeStamp,
        SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri), "System/CreateSession",
        isOk);

    //////////////////////////////////////////////////////
    // Set AuditSession Inherited attributes
    set_AuditSession_Attributes(event, sessionId);

    //////////////////////////////////////////////////////
    // Fill specific part of auditCreateSessionEvent_Type
    // SecureChannelId : String
    SOPC_SecureChannel_Config* scConfigPtr =
        SOPC_ToolkitServer_GetSecureChannelConfig(session_audit_bs__p_channel_config);
    if (NULL != scConfigPtr)
    {
        char scIdStr[11] = {0}; // 10 digits + EOL
        snprintf(scIdStr, 11, "%" PRIu32, scConfigPtr->secureChannelId);
        SOPC_EventHelpers_SetCString(event, "0:SecureChannelId", scIdStr);
    }

    // ClientCertificate : ByteString
    if (pReq->ClientCertificate.Length > 0)
    {
        SOPC_EventHelpers_SetByteString(event, "0:ClientCertificate", pReq->ClientCertificate.Data,
                                        (uint32_t) pReq->ClientCertificate.Length);

        // ClientCertificateThumbprint : String
        SOPC_CertificateList* cert = NULL;
        SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(
            pReq->ClientCertificate.Data, (uint32_t) pReq->ClientCertificate.Length, &cert);
        char* clientThumbPrint = NULL;
        if (SOPC_STATUS_OK == status)
        {
            clientThumbPrint = SOPC_KeyManager_Certificate_GetCstring_SHA1(cert);
        }
        SOPC_EventHelpers_SetCString(event, "0:ClientCertificateThumbprint", clientThumbPrint);
        SOPC_Free(clientThumbPrint);
        SOPC_KeyManager_Certificate_Free(cert);
    }
    else
    {
        SOPC_EventHelpers_SetByteString(event, "0:ClientCertificate", NULL, 0);
        SOPC_EventHelpers_SetCString(event, "0:ClientCertificateThumbprint", NULL);
    }

    send_event(event, "CreateSession", message);
}

void session_audit_bs__server_notify_session_activate(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_i session_audit_bs__req_msg,
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    // Retrieve Session contexts
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_Event* event = prepare_session_event(&auditActivateSessionEvent_Type);

    // Note: sessionId can be NULL!
    if (NULL == session_audit_bs__req_msg || NULL == pConfig || NULL == event)
    {
        return;
    }

    const OpcUa_ActivateSessionRequest* pReq = (const OpcUa_ActivateSessionRequest*) session_audit_bs__req_msg;

    SOPC_StatusCode statusRet;
    util_status_code__B_to_C(session_audit_bs__oper_ret_code, &statusRet);

    //////////////////////////////////////////////////////
    // Set inherited event attributes
    const bool isOk = (statusRet & SOPC_BadStatusMask) == 0;
    const char* message = (isOk ? "Activate session successful" : "Activate session failed");
    const uint16_t severity =
        (isOk ? SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_SUCCESS : SOPC_AUDIT_SEVERITY_ACTIVATE_SESSION_FAILURE);
    char* clientUserId = getClientUserIdFromUserToken(&pReq->UserIdentityToken);
    const SOPC_NodeId* sessionId = get_SessionNid(session_audit_bs__session);

    SOPC_EventsHelpers_SetBaseEventType(event, "Session/ActivateSession", message, severity);
    SOPC_EventsHelpers_SetAuditSecurityEventType(event, statusRet);
    SOPC_EventHelpers_SetNodeId(event, "0:SourceNode", &ServerObject_NodeId);
    SOPC_EventsHelpers_SetAuditEventType(
        event, auditEntryId, actionTimeStamp,
        SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri), clientUserId, isOk);
    SOPC_Free(clientUserId);
    //////////////////////////////////////////////////////
    // Set AuditSession Inherited attributes
    set_AuditSession_Attributes(event, sessionId);

    //////////////////////////////////////////////////////
    // Fill specific part of auditActivateSessionEvent_Type
    // ClientSoftwareCertificates : SignedSoftwareCertificate[]
    copy_SignedSoftwareCertificates(event, "0:ClientSoftwareCertificates", pReq->NoOfClientSoftwareCertificates,
                                    pReq->ClientSoftwareCertificates);

    // UserIdentityToken : UserIdentityToken
    set_UserIdentityToken(event, &pReq->UserIdentityToken);

    // SecureChannelId : String
    SOPC_SecureChannel_Config* scConfigPtr =
        SOPC_ToolkitServer_GetSecureChannelConfig(session_audit_bs__p_channel_config);
    if (NULL != scConfigPtr)
    {
        char scIdStr[11] = {0}; // 10 digits + EOL
        snprintf(scIdStr, 11, "%" PRIu32, scConfigPtr->secureChannelId);
        SOPC_EventHelpers_SetCString(event, "0:SecureChannelId", scIdStr);
    }

    send_event(event, "ActivateSession", message);
}

void session_audit_bs__server_notify_session_closed(
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    // Retrieve Session contexts
    const SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_Event* event = prepare_session_event(&auditSessionEvent_Type);

    // Note: sessionId can be NULL!
    if (NULL == pConfig || NULL == event)
    {
        return;
    }

    SOPC_StatusCode statusRet;
    util_status_code__B_to_C(session_audit_bs__oper_ret_code, &statusRet);

    // Set inherited event attributes
    const bool isOk = SOPC_IsGoodStatus(statusRet);
    const uint16_t severity =
        (isOk ? SOPC_AUDIT_SEVERITY_CLOSE_SESSION_SUCCESS : SOPC_AUDIT_SEVERITY_CLOSE_SESSION_FAILURE);
    const char* message = (isOk ? "Session closed on request" : "Session terminated by server");
    const char* sourceName = (isOk ? "Session/CloseSession" : "Session/Terminated");

    if (!isOk && OpcUa_BadTimeout == statusRet)
    {
        message = "Session timed out and terminated by server";
        sourceName = "Session/Timeout";
    }
    else if (!isOk && OpcUa_BadSessionIdInvalid == statusRet)
    {
        message = "Session with invalid id cannot be closed";
        sourceName = "Session/CloseSession";
    }

    const SOPC_CallContext* callContext = SOPC_CallContext_GetCurrent();
    const SOPC_User* user = SOPC_CallContext_GetUser(callContext);

    char* clientUserId = getClientUserIdFromUser(user);
    const SOPC_NodeId* sessionId = get_SessionNid(session_audit_bs__session);

    SOPC_EventsHelpers_SetBaseEventType(event, sourceName, message, severity);

    SOPC_EventsHelpers_SetAuditSecurityEventType(event, statusRet);
    SOPC_EventHelpers_SetNodeId(event, "0:SourceNode", &ServerObject_NodeId);
    SOPC_EventsHelpers_SetAuditEventType(
        event, auditEntryId, actionTimeStamp,
        SOPC_String_GetRawCString(&pConfig->serverConfig.serverDescription.ApplicationUri), clientUserId, isOk);

    //////////////////////////////////////////////////////
    // Set AuditSession Inherited attributes
    set_AuditSession_Attributes(event, sessionId);

    send_event(event, "CloseSession", message);

    // Cleanup obsolete context
    SOPC_Free(clientUserId);
    set_sessionNid(session_audit_bs__session, NULL);
}

void session_audit_bs__set_no_req_audit_info(const constants__t_channel_config_idx_i session_audit_bs__p_channel_config)
{
    SOPC_SecureChannel_Config* scConfigPtr =
        SOPC_ToolkitServer_GetSecureChannelConfig(session_audit_bs__p_channel_config);
    if (NULL != scConfigPtr)
    {
        auditEntryId = scConfigPtr->clientAuditInfo;
    }
    actionTimeStamp = SOPC_Time_GetCurrentTimeUTC();
}

void session_audit_bs__set_request_audit_info(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_header_i session_audit_bs__req_msg_header)
{
    OpcUa_RequestHeader* reqHeader = (OpcUa_RequestHeader*) session_audit_bs__req_msg_header;
    if (reqHeader->AuditEntryId.Length > 0)
    {
        auditEntryId = SOPC_String_GetRawCString(&reqHeader->AuditEntryId);
    }
    else
    {
        SOPC_SecureChannel_Config* scConfigPtr =
            SOPC_ToolkitServer_GetSecureChannelConfig(session_audit_bs__p_channel_config);
        if (NULL != scConfigPtr)
        {
            auditEntryId = scConfigPtr->clientAuditInfo;
        }
    }
    actionTimeStamp = reqHeader->Timestamp;
}
#else // ! S2OPC_HAS_AUDITING

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_audit_bs__INITIALISATION(void) {}

void session_audit_bs__clear_audit_info(void) {}

void session_audit_bs__server_notify_session_activate(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_i session_audit_bs__req_msg,
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    SOPC_UNUSED_ARG(session_audit_bs__p_channel_config);
    SOPC_UNUSED_ARG(session_audit_bs__req_msg);
    SOPC_UNUSED_ARG(session_audit_bs__session);
    SOPC_UNUSED_ARG(session_audit_bs__oper_ret_code);
}

void session_audit_bs__server_notify_session_closed(
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    SOPC_UNUSED_ARG(session_audit_bs__session);
    SOPC_UNUSED_ARG(session_audit_bs__oper_ret_code);
}

void session_audit_bs__server_notify_session_create(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_i session_audit_bs__req_msg,
    const constants__t_msg_i session_audit_bs__resp_msg,
    const constants__t_session_i session_audit_bs__session,
    const constants_statuscodes_bs__t_StatusCode_i session_audit_bs__oper_ret_code)
{
    SOPC_UNUSED_ARG(session_audit_bs__p_channel_config);
    SOPC_UNUSED_ARG(session_audit_bs__req_msg);
    SOPC_UNUSED_ARG(session_audit_bs__resp_msg);
    SOPC_UNUSED_ARG(session_audit_bs__session);
    SOPC_UNUSED_ARG(session_audit_bs__oper_ret_code);
}

void session_audit_bs__set_no_req_audit_info(const constants__t_channel_config_idx_i session_audit_bs__p_channel_config)
{
    SOPC_UNUSED_ARG(session_audit_bs__p_channel_config);
}

void session_audit_bs__set_request_audit_info(
    const constants__t_channel_config_idx_i session_audit_bs__p_channel_config,
    const constants__t_msg_header_i session_audit_bs__req_msg_header)
{
    SOPC_UNUSED_ARG(session_audit_bs__p_channel_config);
    SOPC_UNUSED_ARG(session_audit_bs__req_msg_header);
}
#endif // S2OPC_HAS_AUDITING
