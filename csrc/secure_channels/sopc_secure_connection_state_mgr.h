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

#ifndef SOPC_SECURE_CONNECTION_STATE_MGR_H_
#define SOPC_SECURE_CONNECTION_STATE_MGR_H_

#include <stdint.h>

#include "sopc_secure_channels_api.h"

void SOPC_SecureConnectionStateMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                                              uint32_t eltId,
                                              void* params,
                                              uintptr_t auxParam);

#endif /* SOPC_SECURE_CONNECTION_STATE_MGR_H_ */
