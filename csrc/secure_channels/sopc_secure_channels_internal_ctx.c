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

#include <stddef.h>
#include <string.h>

#include "sopc_secure_channels_internal_ctx.h"

SOPC_SecureListener secureListenersArray[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1];
SOPC_SecureConnection secureConnectionsArray[SOPC_MAX_SECURE_CONNECTIONS + 1];
uint32_t lastSecureConnectionArrayIdx = 0;

void SOPC_SecureChannelsInternalContext_Initialize()
{
    memset(secureListenersArray, 0, sizeof(SOPC_SecureListener) * (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1));
    memset(secureConnectionsArray, 0, sizeof(SOPC_SecureConnection) * (SOPC_MAX_SECURE_CONNECTIONS + 1));
}

SOPC_SecureConnection* SC_GetConnection(uint32_t connectionIdx)
{
    SOPC_SecureConnection* scConnection = NULL;
    if (connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS)
    {
        scConnection = &(secureConnectionsArray[connectionIdx]);
    }
    return scConnection;
}

void SOPC_SecureChannelsInternalContext_Clear() {}
