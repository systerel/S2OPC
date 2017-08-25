/*
 *  Copyright (C) 2017 Systerel and others.
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

#ifndef SOPC_SOCKETS_INTERNAL_CTX_H_
#define SOPC_SOCKETS_INTERNAL_CTX_H_

#include <stdbool.h>

#include "sopc_toolkit_constants.h"
#include "sopc_raw_sockets.h"
#include "sopc_mutexes.h"
#include "sopc_async_queue.h"

typedef enum {
    SOCKET_STATE_CLOSED = 0,
    SOCKET_STATE_CONNECTING, // Client connect waiting for write event
                       // && SO_ERROR to be verified on event to confirm connection accepted
    SOCKET_STATE_CONNECTED,  // Client: write event received after connect / Server: connection accepted (socket level + SC connection level)
    SOCKET_STATE_LISTENING,  // Server: listening socket
    SOCKET_STATE_ACCEPTED    // Server: accepted socket connection at socket level only (missing SC connection level)
} SOPC_Socket_State;

typedef struct SOPC_Socket {
    uint32_t                      socketIdx; /* Index in the socket array */
    uint32_t                      connectionId; /* High-level associated connection id
                                                  (secure channel connection index when state = CONNECTING/CONNECTED only
                                                   OR endpoint description configuration index when state = LISTENING only) */
    Socket                        sock;
    SOPC_AsyncQueue*              writeQueue;
    uint8_t                       isNotWritable;
    bool                          isUsed; /* Indicates if the socket is free (false) or used (true) */
    SOPC_Socket_State             state;
    // addresses for connection
    void*                         connectAddrs; // Possible connection addresses (to free on connection)
    void*                         nextConnectAttemptAddr; // Next connection attempt address
    // number of connection for a listener (state = LISTENING)
    uint32_t                      listenerConnections;
} SOPC_Socket;

/** @brief Array containing all sockets that can be used */
extern SOPC_Socket socketsArray[SOPC_MAX_SOCKETS];
/** @brief Mutex to lock when using array containing all sockets
 *         or changing isUsed attribute of a socket (which determine validity in array).
 *
 *  Note: necessary since the socket network event manager runs on a cyclic thread
 *        in addition to the event dispatcher thread */
extern Mutex       socketsMutex;

/** @brief Initialize the array of sockets */
void SOPC_SocketsInternalContext_Initialize(void);

/** @brief Clear the array of sockets */
void SOPC_SocketsInternalContext_Clear(void);

/** @brief Returns an unused socket from the array of sockets or NULL if none available
 *         In case socket is not a listnener, the write buffer queue is initialized.
 *  Note: caller must lock the mutex before calling it
 */
SOPC_Socket* SOPC_SocketsInternalContext_GetFreeSocketNoLock(bool isListener);

/** @brief Close the socket and set it as not used anymore.
 *  Note: caller must lock the mutex before calling it
 *  Note2: defined here since it modifies validity of socket in array
 */
void SOPC_SocketsInternalContext_CloseSocketNoLock(uint32_t socketIdx);

/** @brief Close the socket and set it as not used anymore.
 *  (automatic lock of the mutex during call).
 *  Note: defined here since it modifies validity of socket in array
 */
void SOPC_SocketsInternalContext_CloseSocket(uint32_t socketIdx);


#endif /* SOPC_SOCKETS_INTERNAL_CTX_H_ */
