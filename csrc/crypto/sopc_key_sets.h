/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_KEY_SETS_H_
#define SOPC_KEY_SETS_H_

#include "sopc_secret_buffer.h"

typedef struct SOPC_SC_SecurityKeySet{
    SOPC_SecretBuffer* signKey;
    SOPC_SecretBuffer* encryptKey;
    SOPC_SecretBuffer* initVector;
} SOPC_SC_SecurityKeySet;

typedef struct {
    SOPC_SC_SecurityKeySet* senderKeySet;
    SOPC_SC_SecurityKeySet* receiverKeySet;
} SOPC_SC_SecurityKeySets;

SOPC_SC_SecurityKeySet* SOPC_KeySet_Create(void);
void SOPC_KeySet_Delete(SOPC_SC_SecurityKeySet* keySet);

#endif /* SOPC_KEY_SETS_H_ */
