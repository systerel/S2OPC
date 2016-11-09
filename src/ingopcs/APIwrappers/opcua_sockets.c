/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "opcua_sockets.h"


/*============================================================================
 * Create a new socket manager or initialize the global one (OpcUa_Null first).
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_Create(        OpcUa_SocketManager*    pSocketManager,
                                                    OpcUa_UInt32            nSockets,
                                                    OpcUa_UInt32            nFlags){
    (void) pSocketManager;
    (void) nSockets;
    (void) nFlags;
    // Done internally in ingopcs: no need to creation outside of the stack
    return STATUS_OK;
}

/*============================================================================
 *
 *===========================================================================*/
OpcUa_Void OpcUa_SocketManager_Delete(              OpcUa_SocketManager*    pSocketManager){
    (void) pSocketManager;
    NULL;
}


/*============================================================================
 *
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_Loop(     OpcUa_SocketManager     pSocketManager,
                                               OpcUa_UInt32            msecTimeout,
                                               OpcUa_Boolean           bRunOnce){
    OpcUa_StatusCode status = STATUS_OK;
    if(pSocketManager == NULL){
#if OPCUA_MULTITHREADED
        return STATUS_NOK;
#else
        pSocketManager = SOPC_SocketManager_GetGlobal();
#endif //OPCUA_MULTITHREADED
    }
    /* the serving loop */
    do
    {
        status |= SOPC_SocketManager_Loop(pSocketManager, msecTimeout);
    } while(!bRunOnce);
    if(STATUS_OK != status){
        status = STATUS_NOK;
    }
    return status;
}
