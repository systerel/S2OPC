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

#include "sopc_user_app_itf.h"

/** Anonymous user security policy supported configuration */
const OpcUa_UserTokenPolicy c_userTokenPolicy_Anonymous = {
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .PolicyId = {9, true, (SOPC_Byte*) "anonymous"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
};

/** Username security policy is supported and configured with security policy None.
 * With this security policy, the password will never be encrypted and this policy
 * shall not be used on unsecured or unencrypted secure channels. */
const OpcUa_UserTokenPolicy c_userTokenPolicy_UserName_NoneSecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {8, true, (SOPC_Byte*) "username"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {sizeof(SOPC_SecurityPolicy_None_URI) - 1, true,
                          (SOPC_Byte*) SOPC_SecurityPolicy_None_URI}, /* None security policy shall be used only when
                         secure channel security policy is non-None since password will be non-encrypted */
};
