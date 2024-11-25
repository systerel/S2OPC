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

#ifndef SOPC_SECURED_CHANNELS_AUDIT_H_
#define SOPC_SECURED_CHANNELS_AUDIT_H_

#include <inttypes.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_secure_channels_internal_ctx.h"

/** A structure containing the context of a Certificate check failure */
typedef struct SOPC_Audit_Certificate_Error_Context
{
    uint32_t opcua_error_code;
    const char* message;
    SOPC_ByteString certificate;
    SOPC_String subject;
    char* invalidURI;
    char* invalidHostname;
} SOPC_Audit_Certificate_Error_Context;

#ifdef S2OPC_HAS_AUDITING

/**
 * \brief Trigger a security event (OpenSecureChannelEvent_Type) for OpenSecuredChannel event.
 * \param message Content of the "Message" event attribute. Shall not be NULL.
 * \param statusCode Content of the "Message" event attribute.
 * \param reqHeader The OPN request header from which is extracted auditEntryId and timestamp.
 *                  If NULL or audit entry not set, an alternate id from peer connection info is used (if available).
 * \param scConnection The related secureConnection.  Shall not be NULL.
 * \param isRenew True for a RENEW, false for an ISSUE connection.
 * \param auditCertEventId The audit certificate event id generated in case of certificate validation error (or NULL)
 */
void SOPC_Audit_TriggerEvent_OpenSecuredChannel(const char* message,
                                                const SOPC_StatusCode statusCode,
                                                const OpcUa_RequestHeader* reqHeader,
                                                const SOPC_SecureConnection* scConnection,
                                                bool isRenew,
                                                const SOPC_ByteString* auditCertEventId);

/**
 * \brief Trigger a security event (AuditChannelEventType) for CloseSecuredChannel event.
 * \param message Content of the "Message" event attribute. Shall not be NULL.
 * \param statusCode Content of the "Message" event attribute.
 * \param reqHeader The OPN request header from which is extracted auditEntryId and timestamp.
 *                  If NULL or audit entry not set, an alternate id from peer connection info is used (if available).
 * \param scConnection The related secureConnection.  Shall not be NULL.
 */
void SOPC_Audit_TriggerEvent_CloseSecuredChannel(const char* message,
                                                 const SOPC_StatusCode statusCode,
                                                 const OpcUa_RequestHeader* reqHeader,
                                                 const SOPC_SecureConnection* scConnection);

/**
 * \brief Trigger an audit security event for Certificate Issue (AuditCertificateEventType or child).
 * \param context The certificate failure context, that can be created using
 * ::SOPC_Audit_Create_CertificateFailure_ContextCreate
 * \param auditEntryId The related audit entry id, it should be provided.
 * \param actionTimeStamp The action timestamp to set in the event
 * \param[out] eventId The generated event ID (can be NULL if not needed)
 */
void SOPC_Audit_TriggerEvent_CertificateFailure(const SOPC_Audit_Certificate_Error_Context* context,
                                                const char* auditEntryId,
                                                const SOPC_DateTime actionTimestamp,
                                                SOPC_ByteString* eventId);

/**
 * \brief Allocates a SOPC_Audit_Certificate_Error_Context
 * \param pContext The context to initialize
 * \param pCertlist The related certificate list
 */
void SOPC_Audit_Create_CertificateFailure_Context_Initialize(SOPC_Audit_Certificate_Error_Context* pContext,
                                                             const SOPC_CertificateList* pCertlist);

/**
 * \brief Allocates a SOPC_Audit_Certificate_Error_Context
 * \param pContext The context to initialize
 * \param pBsCert The related certificate as a byte string
 */
void SOPC_Audit_Create_CertificateFailure_Context_InitializeAlt(SOPC_Audit_Certificate_Error_Context* pContext,
                                                                const SOPC_ByteString* pBsCert);

/**
 * \brief Change the certificate content of the context to be a thumbprint instead of certificate
 * \note This should be used when failure occurred on receiver certificate thumbprint of asymmetric security header
 * occurred. This case does not seem to be defined in specification but is useful to log the thumbprint.
 *
 * \param pContext The context to modify
 * \param pBsCertTb The related certificate thumbprint as a byte string
 */
void SOPC_Audit_Create_CertificateFailure_Context_ChangeForThumbprint(SOPC_Audit_Certificate_Error_Context* pContext,
                                                                      const SOPC_ByteString* pBsCertTb);

/** \brief Clears the content and allocated context object*/
void SOPC_Audit_Create_CertificateFailure_Context_Clear(SOPC_Audit_Certificate_Error_Context* pCtxt);

#else // S2OPC_HAS_AUDITING

#define SOPC_Audit_TriggerEvent_OpenSecuredChannel(...)
#define SOPC_Audit_TriggerEvent_CloseSecuredChannel(...)
#define SOPC_Audit_TriggerEvent_CertificateFailure(...)
#define SOPC_Audit_Create_CertificateFailure_Context_Initialize(...)
#define SOPC_Audit_Create_CertificateFailure_Context_InitializeAlt(...)
#define SOPC_Audit_Create_CertificateFailure_Context_ChangeForThumbprint(...)
#define SOPC_Audit_Create_CertificateFailure_Context_Clear(...)

#endif // S2OPC_HAS_AUDITING

#endif /* SOPC_SECURED_CHANNELS_AUDIT_H_ */
