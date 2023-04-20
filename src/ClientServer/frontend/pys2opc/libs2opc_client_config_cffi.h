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
 * \brief High level interface to configure an OPC UA client
 *
 */

#ifndef LIBS2OPC_CLIENT_CONFIG_H_
#define LIBS2OPC_CLIENT_CONFIG_H_

// #include <stdbool.h>
// #include "sopc_builtintypes.h"

/**
 * \brief Type of callback to retrieve password for decryption of the client application private key
 *        or the user x509 token private key.
 *
 * \param[out] outPassword   out parameter, the newly allocated password which shall be a zero-terminated string in case
 *                           of success.
 *
 * \return true in case of success, otherwise false.
 *
 * \warning The implementation of the user callback must free the \p outPassword and set it back to NULL in case of
 * failure.
 */
typedef bool SOPC_GetPassword_Fct(char** outPassword);

/**
 * \brief Define the callback to retrieve password for decryption of the client private key.
 *
 * This is optional but if used it shall be defined before starting client and loading its configuration.
 *
 * \param getClientKeyPassword  The callback to retrieve the password
 *
 * \return  SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p getClientKeyPassword is
 *          invalid.
 *
 * \note    This function must be called before the configuration of the secure channel.
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetClientKeyPasswordCallback(SOPC_GetPassword_Fct* getClientKeyPassword);

#endif /* LIBS2OPC_CLIENT_CONFIG_H_ */
