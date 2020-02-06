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

#ifndef SOPC_KEY_SETS_H_
#define SOPC_KEY_SETS_H_

#include "sopc_secret_buffer.h"

typedef struct SOPC_SC_SecurityKeySet
{
    SOPC_SecretBuffer* signKey;
    SOPC_SecretBuffer* encryptKey;
    SOPC_SecretBuffer* initVector;
} SOPC_SC_SecurityKeySet;

typedef struct
{
    SOPC_SC_SecurityKeySet* senderKeySet;
    SOPC_SC_SecurityKeySet* receiverKeySet;
} SOPC_SC_SecurityKeySets;

SOPC_SC_SecurityKeySet* SOPC_KeySet_Create(void);
void SOPC_KeySet_Delete(SOPC_SC_SecurityKeySet* keySet);

#endif /* SOPC_KEY_SETS_H_ */
