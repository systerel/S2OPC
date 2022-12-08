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
 * \brief Interface to configure the client toolkit
 *
 */

#ifndef LIBS2OPC_CLIENT_TOOLKIT_CONFIG_H_
#define LIBS2OPC_CLIENT_TOOLKIT_CONFIG_H_

#include <stdbool.h>
#include "sopc_builtintypes.h"

/**
 * \brief Type of callback to receive user password for decryption of the Client private key.
 *
 * \param ppPassword      out parameter, the newly allocated password.
 * \param writtenStatus   out parameter, the status code of the callback process.
 *
 * \warning The callback function shall not do anything blocking or long treatment.
 *          The implementation of the user callback must free the \p ppPassword in case of failure.
 */
typedef void SOPC_ClientKeyUsrPwd_Fct(SOPC_String** ppPassword, SOPC_StatusCode* writtenStatus);

/**
 * \brief Define the client private key password callback to be used.
 *
 * \param clientKeyUsrPwdCb  The user callback to retrieve the password
 *
 * \return  SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p clientKeyUsrPwdCb is
 * invalid.
 *
 * \note    This function must be called before the configuration the secure Channel.
 *          When the callback is configured, it is called before the instantiation of the serialized ClientKey.
 *
 * \warning The callback function shall not do anything blocking or long treatment.
 */
SOPC_ReturnStatus SOPC_HelperConfigClient_SetClientKeyUsrPwdCallback(SOPC_ClientKeyUsrPwd_Fct* clientKeyUsrPwdCb);

/**
 * \brief Function to call the callback to receive user password for decryption of the Client private key.
 *
 * \param ppPassword      out parameter, the newly allocated password.
 * \param writtenStatus   out parameter, the status code of the callback process.
 *
 */
void SOPC_HelperConfigClient_ClientKeyUsrPwdCb(SOPC_String** ppPassword, SOPC_StatusCode* writtenStatus);

/**
 * \brief Function to know if the client's key is encrypted (if the callback has been defined).
 *
 */
bool SOPC_HelperConfigClient_IsEncryptedClientKey(void);

#endif /* LIBS2OPC_CLIENT_TOOLKIT_CONFIG_H_ */
