/*
 * wrapper_sockets.c
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#include "wrapper_sockets.h"

SOPC_StatusCode OpcUa_SocketManager_Create(SOPC_SocketManager* sManager,
                                           uint32_t            nbSockets,
                                           uint32_t            flags){
    (void) sManager;
    (void) nbSockets;
    (void) flags;
    // Done internally in ingopcs: no need to creation outside of the stack
    return STATUS_OK;
}

void OpcUa_SocketManager_Delete(SOPC_SocketManager* sManager){
    (void) sManager;
}

SOPC_StatusCode OpcUa_SocketManager_Loop(SOPC_SocketManager* sManager,
                                         uint32_t            msTimeout,
                                         uint8_t             runOnce){
    SOPC_StatusCode status = STATUS_OK;
    if(sManager == NULL){
#if OPCUA_MULTITHREADED
        return STATUS_NOK;
#else
        sManager = SOPC_SocketManager_GetGlobal();
#endif //OPCUA_MULTITHREADED
    }
    /* the serving loop */
    do
    {
        status |= SOPC_SocketManager_Loop(sManager, msTimeout);
    } while(!runOnce);
    if(STATUS_OK != status){
        status = STATUS_NOK;
    }
    return status;
}
