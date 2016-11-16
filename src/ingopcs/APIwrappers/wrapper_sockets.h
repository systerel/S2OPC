/*
 * wrapper_sockets.h
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#ifndef SOPC_WRAPPER_SOCKETS_H_
#define SOPC_WRAPPER_SOCKETS_H_

#include "sopc_base_types.h"
#include "sopc_sockets.h"

BEGIN_EXTERN_C

SOPC_StatusCode OpcUa_SocketManager_Create(SOPC_SocketManager* sManager,
                                           uint32_t            nbSockets,
                                           uint32_t            flags);

void OpcUa_SocketManager_Delete(SOPC_SocketManager* sManager);

SOPC_StatusCode OpcUa_SocketManager_Loop(SOPC_SocketManager* sManager,
                                         uint32_t            msTimeout,
                                         uint8_t             runOnce);

END_EXTERN_C

#endif /* SOPC_WRAPPER_SOCKETS_H_ */
