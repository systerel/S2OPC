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

#ifndef SOPC_CHUNKS_MGR_INTERNAL_H_
#define SOPC_CHUNKS_MGR_INTERNAL_H_

#include "sopc_secure_channels_internal_ctx.h"

bool SC_Chunks_DecodeReceivedBuffer(SOPC_SecureConnection_ChunkMgrCtx* ctx,
                                    SOPC_Buffer* receivedBuffer,
                                    SOPC_StatusCode* error);

bool SC_Chunks_TreatTcpPayload(SOPC_SecureConnection* scConnection, uint32_t* requestId, SOPC_StatusCode* errorStatus);

#endif // SOPC_CHUNKS_MGR_INTERNAL_H_
