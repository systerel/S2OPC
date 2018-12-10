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

#ifndef SOPC_SECURE_CONNECTION_STATE_MGR_INTERNAL_H_
#define SOPC_SECURE_CONNECTION_STATE_MGR_INTERNAL_H_

#include <stdbool.h>
#include <stdint.h>

bool SC_InitNewConnection(uint32_t* newConnectionIdx);
bool SC_CloseConnection(uint32_t connectionIdx, bool socketFailure);

#endif // SOPC_SECURE_CONNECTION_STATE_MGR_INTERNAL_H_
