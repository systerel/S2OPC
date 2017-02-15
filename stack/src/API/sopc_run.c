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

#include "sopc_run.h"
#include "sopc_threads.h"
#include "sopc_base_types.h"
#include "sopc_sockets.h"

SOPC_StatusCode SOPC_TreatReceivedMessages(uint32_t msecTimeout){
    SOPC_StatusCode status = STATUS_NOK;
#if OPCUA_MULTITHREADED
    // Should not be used in multithreaded mode
    status = STATUS_NOK;
#else
    status = SOPC_SocketManager_Loop(SOPC_SocketManager_GetGlobal(),
                                     msecTimeout);
    if(STATUS_OK == status){
        SOPC_Sleep(1);
    }
#endif //OPCUA_MULTITHREADED
    return status;
}
