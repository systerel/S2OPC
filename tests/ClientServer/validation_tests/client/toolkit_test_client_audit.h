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

#ifndef SOPC_TOOLKIT_TEST_CLIENT_AUDIT_H_
#define SOPC_TOOLKIT_TEST_CLIENT_AUDIT_H_

#include <stddef.h>

typedef struct Test_ExpectedValue
{
    const char* clause;
    const char* value;
} Test_ExpectedValue;

// Need to escape '?' to avoid "trigraph" compiler warning...
#define TEST_ANY_DATE "?\?\?\?/\?\?/\?\? ??:??:??.?\?\?"

// Note: check possible jokers as detailed in .c file
static const Test_ExpectedValue Audit_OpenSCEvent_None[] = {
    {"0:SourceName", "SecureChannel/OpenSecureChannel"},
    {"0:ClientUserId", "System/OpenSecureChannel"},
    {"0:ClientCertificate", ""},
    {"0:ClientCertificateThumbprint", ""},
    {"0:RequestType", "0"},
    {"0:SecurityPolicyUri", "http://opcfoundation.org/UA/SecurityPolicy#None"},
    {"0:SecurityMode", "1"},
    {"0:RequestedLifetime", "60000"},
    {"0:SecureChannelId", "*1"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2060"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "*"},
    {"0:Severity", "1"},
    {NULL, NULL}};

static const Test_ExpectedValue Audit_OpenSCEvent_User[] = {
    {"0:SourceName", "SecureChannel/OpenSecureChannel"},
    {"0:ClientUserId", "System/OpenSecureChannel"},
    {"0:ClientCertificate", "*3"},
    {"0:ClientCertificateThumbprint", "*4"},
    {"0:RequestType", "0"},
    {"0:SecurityPolicyUri", "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"},
    {"0:SecurityMode", "3"},
    {"0:RequestedLifetime", "*"},
    {"0:SecureChannelId", "*1"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2060"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "*"},
    {"0:Severity", "1"},
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CertificateInvalidEvent_Fail[] = {
    {"0:SourceName", "Security/Certificate"},
#ifndef S2OPC_CRYPTO_CYCLONE
    // Subject Name of the Certificate (it should be user one but in this case event SC certificate one seems OK)
    {"0:ClientUserId", "C=FR, ST=France, L=Aix-en-Provence, O=Systerel, CN=S2OPC Demo Certificate for Client Tests"},
#else
    {"0:ClientUserId", ""},
#endif
    // Certificate thumbprint is provided instead of certificate as in this case we only have the receiver thumbprint
    {"0:Certificate", "CDDBA5D672C60CDEF94ADCB69C025719959281E0"},
    {"0:StatusCodeId", "0x80120000"}, // BadCertificateInvalid
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "false"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:EventId", "*42"},
    {"0:EventType", "i=2086"},
    {"0:SourceNode", "i=0"},
    {"0:Message", "*"},
    {"0:Severity", "100"},

    {NULL, NULL}};

static const Test_ExpectedValue Audit_OpenSCEvent_Fail[] = {
    {"0:SourceName", "SecureChannel/OpenSecureChannel"},
    {"0:ClientUserId", "System/OpenSecureChannel"},
    {"0:ClientCertificate", ""},           // ClientCertificate not kept in this case
    {"0:ClientCertificateThumbprint", ""}, // no certificate => no thumbprint
    {"0:RequestType", "0"},
    {"0:SecurityPolicyUri", ""},  // SecurityPolicyUri not kept in this case
    {"0:SecurityMode", "0"},      // SecurityMode not kept in this case
    {"0:RequestedLifetime", "0"}, // RequestedLifetime not kept in this case
    {"0:SecureChannelId", "*1"},
    {"0:StatusCodeId", "0x80120000"}, // BadCertificateInvalid
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "false"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2060"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "*"},
    {"0:Severity", "10"},
    // Certificate failure specific field
    {"0:CertificateErrorEventId", "*42"}, // same as EventId of certificate event
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CreateSessionNoSecu_Ok[] = {
    {"0:SourceName", "Session/CreateSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "System/CreateSession"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2071"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Create session successful"},
    {"0:Severity", "1"},
    // Session specific
    {"0:SessionId", "*5"},
    {"0:SecureChannelId", "*1"}, // same as in OpenSC for those 3 fields
    {"0:ClientCertificate", ""},
    {"0:ClientCertificateThumbprint", ""},
    {"0:RevisedSessionTimeout", "60000"},
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CreateSession_Ok[] = {
    {"0:SourceName", "Session/CreateSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "System/CreateSession"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2071"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Create session successful"},
    {"0:Severity", "1"},
    // Session specific
    {"0:SessionId", "*5"},
    {"0:SecureChannelId", "*1"}, // same as in OpenSC for those 3 fields
    {"0:ClientCertificate", "*3"},
    {"0:ClientCertificateThumbprint", "*4"},
    {"0:RevisedSessionTimeout", "60000"},
    {NULL, NULL}};

static const Test_ExpectedValue Audit_ActivateSessionAnon_Ok[] = {
    {"0:SourceName", "Session/ActivateSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", ""},
    {"0:EventId", "*"},
    {"0:EventType", "i=2075"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Activate session successful"},
    {"0:Severity", "1"},
    // Session specific
    {"0:SessionId", "*5"},                        // same as CreateSession
    {"0:SecureChannelId", "*1"},                  // same as in OpenSC
    {"0:ClientSoftwareCertificates", "[[...]]"},  // cannot check content without a correct dump
    {"0:UserIdentityToken", "<ExtensionObject>"}, // cannot check content without a correct dump
    {NULL, NULL}};

static const Test_ExpectedValue Audit_ActivateSessionUser1_Ok[] = {
    {"0:SourceName", "Session/ActivateSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "user1"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2075"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Activate session successful"},
    {"0:Severity", "1"},
    // Session specific
    {"0:SessionId", "*5"},                        // same as CreateSession
    {"0:SecureChannelId", "*1"},                  // same as in OpenSC
    {"0:ClientSoftwareCertificates", "[[...]]"},  // cannot check content without a correct dump
    {"0:UserIdentityToken", "<ExtensionObject>"}, // cannot check content without a correct dump
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CloseSessionUser1_Ok[] = {
    {"0:SourceName", "Session/CloseSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "user1"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2069"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Session closed on request"},
    {"0:Severity", "1"},
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CloseSessionAnon_Ok[] = {
    {"0:SourceName", "Session/CloseSession"},
    {"0:StatusCodeId", "0x00000000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", ""},
    {"0:EventId", "*"},
    {"0:EventType", "i=2069"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Session closed on request"},
    {"0:Severity", "1"},
    // Session specific
    {"0:SessionId", "*5"}, // same as CreateSession
    {NULL, NULL}};

static const Test_ExpectedValue Audit_CloseSession_Terminated_OnSCclosed[] = {
    {"0:SourceName", "Session/Terminated"},
    {"0:StatusCodeId", "0x80860000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "false"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", ""},
    {"0:EventId", "*"},
    {"0:EventType", "i=2069"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Session terminated by server"},
    {"0:Severity", "10"},
    // Session specific
    {"0:SessionId", "*5"}, // same as CreateSession
    {NULL, NULL}};

static const Test_ExpectedValue Audit_SCClose_Ok[] = {
    {"0:SourceName", "SecureChannel/CloseSecureChannel"},
    {"0:SecureChannelId", "*1"},
    {"0:StatusCodeId", "0x80860000"},
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "true"},
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "System/CloseSecureChannel"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2059"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : CLO request message received"},
    {"0:Severity", "1"},
    {NULL, NULL}};

static const Test_ExpectedValue* audit_NoneOk[] = {Audit_OpenSCEvent_None,
                                                   Audit_CreateSessionNoSecu_Ok,
                                                   Audit_ActivateSessionAnon_Ok,
                                                   Audit_CloseSessionAnon_Ok,
                                                   Audit_SCClose_Ok,
                                                   NULL};

static const Test_ExpectedValue* audit_User1Ok[] = {
    Audit_OpenSCEvent_User,     Audit_CreateSession_Ok, Audit_ActivateSessionUser1_Ok,
    Audit_CloseSessionUser1_Ok, Audit_SCClose_Ok,       NULL};

static const Test_ExpectedValue* audit_User1_BadServerCertificate[] = {Audit_CertificateInvalidEvent_Fail,
                                                                       Audit_OpenSCEvent_Fail, NULL};

static const Test_ExpectedValue Audit_ActivateSession_BadUserPass[] = {
    {"0:SourceName", "Session/ActivateSession"},
    {"0:StatusCodeId", "0x80210000"}, // OpcUa_BadIdentityTokenRejected
    {"0:ActionTimeStamp", TEST_ANY_DATE},
    {"0:Status", "false"}, // Status is Failed!
    {"0:ServerId", "urn:S2OPC:localhost"},
    {"0:ClientAuditEntryId", "*2"},
    {"0:ClientUserId", "user1"},
    {"0:EventId", "*"},
    {"0:EventType", "i=2075"},
    {"0:SourceNode", "i=2253"},
    {"0:Message", "LocalizedText (default only) = : Activate session failed"},
    {"0:Severity", "10"},
    // Session specific
    {"0:SessionId", "*5"},                        // same as CreateSession
    {"0:SecureChannelId", "*1"},                  // same as in OpenSC
    {"0:ClientSoftwareCertificates", "[[...]]"},  // cannot check content without a correct dump
    {"0:UserIdentityToken", "<ExtensionObject>"}, // cannot check content without a correct dump
    {NULL, NULL}};

static const Test_ExpectedValue* audit_User1_BadPass[] = {
    Audit_OpenSCEvent_User,
    Audit_CreateSession_Ok,
    Audit_ActivateSession_BadUserPass,
    Audit_SCClose_Ok,
    Audit_CloseSession_Terminated_OnSCclosed,
    NULL}; // Note: we receive a session terminated because SC is closed and session was not activated prior to it.

#endif // SOPC_TOOLKIT_TEST_CLIENT_AUDIT_H_
