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

#include "sopc_types_wrapper.h"

#ifndef WRAPPERSAPI_OPCUA_SOCKETS_H_
#define WRAPPERSAPI_OPCUA_SOCKETS_H_

OPCUA_BEGIN_EXTERN_C

#define OPCUA_SOCKET_NO_FLAG                    0   /* standard behaviour */
#define OPCUA_SOCKET_REJECT_ON_NO_THREAD        1   /* thread pooling; reject connection if no worker thread i available */
#define OPCUA_SOCKET_DONT_CLOSE_ON_EXCEPT       2   /* don't close a socket if an except event occured */
#define OPCUA_SOCKET_SPAWN_THREAD_ON_ACCEPT     4   /* assing each accepted socket a new thread */

/*============================================================================
 * Create a new socket manager or initialize the global one (OpcUa_Null first).
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_Create(        OpcUa_SocketManager*    pSocketManager,
                                                    OpcUa_UInt32            nSockets,
                                                    OpcUa_UInt32            nFlags);

/*============================================================================
 *
 *===========================================================================*/
OpcUa_Void OpcUa_SocketManager_Delete(              OpcUa_SocketManager*    pSocketManager);


/*============================================================================
 *
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_Loop(     OpcUa_SocketManager     pSocketManager,
                                               OpcUa_UInt32            msecTimeout,
                                               OpcUa_Boolean           bRunOnce);

OPCUA_END_EXTERN_C

#endif /* _OPCUA_SOCKETS_H_ */
