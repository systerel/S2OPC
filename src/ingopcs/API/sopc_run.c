/*
 * sopc_run.c
 *
 *  Created on: Nov 9, 2016
 *      Author: vincent
 */

#include "sopc_run.h"
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
#endif //OPCUA_MULTITHREADED
    return status;
}
