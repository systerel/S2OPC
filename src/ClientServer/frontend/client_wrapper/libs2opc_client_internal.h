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
 * \privatesection
 *
 * \brief Internal module used to manage the wrapper for client config. It should not be used outside of the client
 * wraper implementation.
 *
 */

#include <stdbool.h>

#ifndef LIBS2OPC_CLIENT_INTERNAL_H_
#define LIBS2OPC_CLIENT_INTERNAL_H_

/**
 * \brief Function to call the callback to retrieve password for decryption of the Client private key.
 *
 * \param[out] outPassword   the newly allocated password.
 *
 * \return                   true in case of success, otherwise false.
 *
 */
bool SOPC_ClientInternal_GetKeyPassword(char** outPassword);

/**
 * \brief Function to know if the client's key is encrypted (if the callback has been defined).
 *
 */
bool SOPC_ClientInternal_IsEncryptedKey(void);

#endif /* LIBS2OPC_CLIENT_INTERNAL_H_ */
