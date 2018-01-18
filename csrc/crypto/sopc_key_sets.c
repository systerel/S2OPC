/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_key_sets.h"
#include <stddef.h>
#include <stdlib.h>

SOPC_SC_SecurityKeySet* SOPC_KeySet_Create()
{
    SOPC_SC_SecurityKeySet* keySet = malloc(sizeof(SOPC_SC_SecurityKeySet));
    return keySet;
}

void SOPC_KeySet_Delete(SOPC_SC_SecurityKeySet* keySet)
{
    if (keySet != NULL)
    {
        SOPC_SecretBuffer_DeleteClear(keySet->encryptKey);
        SOPC_SecretBuffer_DeleteClear(keySet->initVector);
        SOPC_SecretBuffer_DeleteClear(keySet->signKey);
        free(keySet);
    }
}
