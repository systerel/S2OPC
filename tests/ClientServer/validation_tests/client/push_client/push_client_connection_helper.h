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

SOPC_ReturnStatus create_custom_secure_connection(const char* clientCertPath,
                                                  const char* clientKeyPath,
                                                  bool encrypted,
                                                  const char* clientPKIStorePath,
                                                  const char* serverCertPath,
                                                  SOPC_SecureConnection_Config** scConfig);

#endif /* PUSH_CLIENT_CONNECTION_HELPER_H_ */
