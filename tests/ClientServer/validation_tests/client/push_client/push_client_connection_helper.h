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

#ifndef PUSH_CLIENT_CONNECTION_HELPER_H_
#define PUSH_CLIENT_CONNECTION_HELPER_H_

#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "push_server_methods.h"

/**
 * @brief Create a secure connection in Sign mode.
 *
 * @param[in] clientCertPath The path of the Client certificate.
 * @param[in] clientKeyPath The path of the Client private key.
 * @param[in] encrypted If its private key is encrypted or not.
 * @param[in] clientPKIStorePath The path of the Client PKI store.
 * @param[in] serverCertPath The path of the Server certificate.
 * @param[out] scConfig The secure connection returned by the toolkit in case of success.
 * NULL if failed.
 * @return SOPC_ReturnStatus
 *
 * @remark The serverCertPath needs to be precised and the server endpoint is
 * "opc.tcp://localhost:4841", userPolicy is fixed and connection mode is username/password
 * with username="me". The password of the username needs to be defined as global variable
 * under TEST_PASSWORD_USER_ME.
 */
SOPC_ReturnStatus SOPC_Create_Custom_Secure_Connection(const char* clientCertPath,
                                                       const char* clientKeyPath,
                                                       bool encrypted,
                                                       const char* clientPKIStorePath,
                                                       const char* serverCertPath,
                                                       SOPC_SecureConnection_Config** scConfig);

#endif /* PUSH_CLIENT_CONNECTION_HELPER_H_ */
