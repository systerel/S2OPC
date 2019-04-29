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

#ifndef SOPC_HELPER_URI_H_
#define SOPC_HELPER_URI_H_

#include <stdbool.h>
#include <stddef.h>

#define TCP_UA_MAX_URL_LENGTH 4096

/** @brief: return true and output parameters if parsed with success. false otherwise */
bool SOPC_Helper_URI_ParseTcpUaUri(const char* uri, size_t* hostnameLength, size_t* portIdx, size_t* portLength);

/** \brief: Splits an URI and stores it in newly created buffers */
bool SOPC_Helper_URI_SplitTcpUaUri(const char* uri, char** hostname, char** port);

#endif /* SOPC_HELPER_URI_H_ */
