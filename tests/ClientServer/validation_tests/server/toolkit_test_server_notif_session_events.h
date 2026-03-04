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

#ifndef TOOLKIT_TEST_SERVER_NOTIF_SESSION_EVENTS_H

#include "libs2opc_server_config.h"

#include "opcua_statuscodes.h"

typedef struct Test_ExpectedEvent
{
    SOPC_ServerSessionEvent event;
    SOPC_SessionId id;
    const char* sessionName;
    const char* appName;
    const char* userId;
    bool isNullClientHostInfo;
    SOPC_StatusCode opStatus;
} Test_ExpectedEvent;

static const Test_ExpectedEvent SessionCreation_Anon[] = {
    {SESSION_CREATION, 20, "S2OPC_client_session", "S2OPC_DemoClient", NULL, false, SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent SessionActivation_Anon[] = {{SESSION_ACTIVATION, 20, "S2OPC_client_session",
                                                             "S2OPC_DemoClient", "ANONYMOUS_RESERVED", false,
                                                             SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent SessionClosure_Anon[] = {{SESSION_CLOSURE, 20, "S2OPC_client_session",
                                                          "S2OPC_DemoClient", "ANONYMOUS_RESERVED", false,
                                                          SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent* session_AnonOK[] = {SessionCreation_Anon, SessionActivation_Anon, SessionClosure_Anon};

static const Test_ExpectedEvent SessionCreation_User[] = {
    {SESSION_CREATION, 20, "S2OPC_Session_20", "S2OPC_TestClient", NULL, false, SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent SessionActivation_User[] = {
    {SESSION_ACTIVATION, 20, "S2OPC_Session_20", "S2OPC_TestClient", "user1", false, SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent SessionClosure_User[] = {
    {SESSION_CLOSURE, 20, "S2OPC_Session_20", "S2OPC_TestClient", "user1", false, SOPC_GoodGenericStatus}};

static const Test_ExpectedEvent* session_UserOK[] = {SessionCreation_User, SessionActivation_User, SessionClosure_User};

static const Test_ExpectedEvent SessionActivationFailed_User[] = {{SESSION_ACTIVATION, 20, "S2OPC_Session_20",
                                                                   "S2OPC_TestClient", "unknownUser", false,
                                                                   OpcUa_BadIdentityTokenRejected}};

static const Test_ExpectedEvent SessionActivationFailed_Pwd[] = {
    {SESSION_ACTIVATION, 20, "S2OPC_Session_20", "S2OPC_TestClient", "user1", false, OpcUa_BadIdentityTokenRejected}};

static const Test_ExpectedEvent SessionClosureFailed_User[] = {
    {SESSION_CLOSURE, 20, NULL, NULL, NULL, true, OpcUa_BadSecureChannelClosed}};

static const Test_ExpectedEvent* session_UserNOK[] = {SessionCreation_User, SessionActivationFailed_User,
                                                      SessionClosureFailed_User};

static const Test_ExpectedEvent* session_PwdNOK[] = {SessionCreation_User, SessionActivationFailed_Pwd,
                                                     SessionClosureFailed_User};

#endif // TOOLKIT_TEST_SERVER_NOTIF_SESSION_EVENTS_H
