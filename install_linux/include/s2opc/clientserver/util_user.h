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
 * Utils to manipulate user token and evaluate user token policy
 */

#ifndef UTIL_USER_H_
#define UTIL_USER_H_

#include <stdbool.h>

#include "constants.h"

/* Retrieve the user token type from user token structure */
constants__t_user_token_type_i util_get_user_token_type_from_token(
    const constants__t_user_token_i user_authentication_bs__p_user_token);

/* Get the encryption algorithm URI given the security policy */
const char* util_getEncryptionAlgorithm(constants__t_SecurityPolicy secpol);

/*
 * Check user token compliance with the user token policy. Returns true if it is compliant, false otherwise.
 * In case of success returns the associated security policy as output parameter.
 * If the user token policy is empty, the security policy of secure channel is returned.
 */
bool util_check_user_token_policy_compliance(const SOPC_SecureChannel_Config* scConfig,
                                             const OpcUa_UserTokenPolicy* userTokenPolicy,
                                             const constants__t_user_token_type_i user_token_type,
                                             const constants__t_user_token_i user_token,
                                             bool check_encryption_algo,
                                             constants__t_SecurityPolicy* secpol);

#endif /* UTIL_USER_H_ */
