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

/** \file sopc_askpass.h
 *
 * \brief A platform independent API to ask for passwords.
 */

#ifndef SOPC_ASKPASS_H_
#define SOPC_ASKPASS_H_

#include <stdbool.h>

/* TODO: The SOPC_AskPass_FromTerminal function is not implemented as described and uses an environment variable instead
 */

/**
 * \brief               Ask for a password in an interactive and secure way from a console.
 *
 * \param outPassword   The newly allocated password, you should free it.
 *
 * \return              true in case of success, otherwise false.
 *
 * \note                stdin is not use.
 *
 * \note                This function is useful to configure the client/server callback to retrieve
 *                      the password of the private key. It can be passed directly as input argument to
 *                      ::SOPC_HelperConfigClient_SetKeyPasswordCallback or
 *                      ::SOPC_HelperConfigServer_SetKeyPasswordCallback
 *
 */
bool SOPC_AskPass_FromTerminal(char** outPassword);

#endif /* SOPC_ASKPASS_H_ */
