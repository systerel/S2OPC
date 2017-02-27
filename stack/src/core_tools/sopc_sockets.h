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

#include <stdint.h>
#include "sopc_raw_sockets.h"
#include "sopc_base_types.h"
#include "sopc_mutexes.h"
#include "sopc_action_queue.h"

#ifndef SOPC_SOCKETS_H_
#define SOPC_SOCKETS_H_

#define TCP_UA_MAX_URL_LENGTH 4096

typedef enum {
    SOCKET_READ_EVENT,
    SOCKET_WRITE_EVENT,
    SOCKET_CLOSE_EVENT,
    SOCKET_EXCEPT_EVENT,
//    SOCKET_TIMEOUT_EVENT,
    SOCKET_CONNECT_EVENT,
    SOCKET_ACCEPT_EVENT} SOPC_Socket_Event;

typedef enum {
    SOCKET_DISCONNECTED          = 0x00,
    SOCKET_CONNECTING            = 0x01, // Client connect waiting for write event
                                         // && SO_ERROR to be verified on event to confirm connection accepted
    SOCKET_CONNECTED             = 0x02, // Client: write event received after connect / Server: connection accepted
    SOCKET_LISTENING             = 0x03 // Server: listening socket
} SOPC_Socket_State;


struct SOPC_Socket;

typedef SOPC_StatusCode (SOPC_Socket_EventCB) (struct SOPC_Socket* socket,
                                               uint32_t            socketEvent,
                                               void*               cbData);

typedef enum {
    SOCKET_TRANSACTION_START,
    SOCKET_TRANSACTION_CONTINUE,
    SOCKET_TRANSACTION_END,
    SOCKET_TRANSACTION_SOCKET_ERROR, // on socket use
    SOCKET_TRANSACTION_END_ERROR, // error message that ends transaction
    SOCKET_TRANSACTION_START_END
} SOPC_Socket_Transaction_Event;

typedef enum {
    SOCKET_TRANSACTION_STATE_NONE // event START => state STARTED | event START_END or ERROR => state NONE
    = 0x00, // Guarantee that init to 0 value is expected enum value
    SOCKET_TRANSACTION_STATE_STARTED, // event END or END_ERROR => state NONE | event ERROR => state ERROR
    SOCKET_TRANSACTION_STATE_ERROR // event END_ERROR => state NONE
} SOPC_Socket_Transaction_State; // other cases are write refuse cases

typedef struct SOPC_Socket {
    Socket                        sock;
    SOPC_ActionQueue*             writeQueue;
    uint8_t                       isNotWritable;
    uint8_t                       isUsed;
    SOPC_Socket_State             state;
    SOPC_Socket_EventCB*          eventCallback; // SOPC_Socket_EventCB Type
    SOPC_Socket_Transaction_State transactionState;
    uint32_t                      transactionId;
    void*                         cbData;
    void*                         connectAddrs; // Possible connection addresses (to free on connection)
    void*                         nextConnectAttemptAddr; // Next connection attempt address

} SOPC_Socket;

typedef struct {
    uint32_t    nbSockets;
    SOPC_Socket *sockets;
    Mutex       mutex;
}
SOPC_SocketManager;

// Callback called after socket operation indicating success or failure
typedef void SOPC_Socket_EndOperation_CB(void*           callbackData,
                                         SOPC_StatusCode createStatus);

// Callback called after socket creation indicating success or failure
typedef void SOPC_Socket_EndCreate_CB(void*           callbackData,
                                      SOPC_StatusCode createStatus,
                                      SOPC_Socket*    newSocket);

// Check TCP UA URI format
SOPC_StatusCode SOPC_Check_TCP_UA_URI(const char* uri);

SOPC_SocketManager* SOPC_SocketManager_GetGlobal(void);

SOPC_StatusCode SOPC_SocketManager_Config_Init(void);

void SOPC_SocketManager_Config_Clear(void);

SOPC_StatusCode SOPC_SocketManager_Initialize(SOPC_SocketManager* socketMgr,
                                              uint32_t            nbSockets);

SOPC_SocketManager* SOPC_SocketManager_Create(uint32_t nbSockets);

void SOPC_SocketManager_Clear(SOPC_SocketManager* socketMgr);

void SOPC_SocketManager_Delete(SOPC_SocketManager** socketMgr);

SOPC_StatusCode SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager* socketManager,
                                                      const char*         uri,
                                                      SOPC_Socket_EventCB socketCallback,
                                                      void*               callbackData,
                                                      SOPC_Socket**       clientSocket);

SOPC_StatusCode SOPC_CreateAction_SocketCreateClient(const char*                  uri,
                                                     SOPC_Socket_EventCB*         socketCallback,
                                                     void*                        callbackData,
                                                     SOPC_Socket_EndCreate_CB*    endCreateCb,
                                                     void*                        endCreateCbData);

SOPC_StatusCode SOPC_SocketManager_CreateServerSocket(SOPC_SocketManager* socketManager,
                                                      const char*         uri,
                                                      uint8_t             listenAllItfs,
                                                      SOPC_Socket_EventCB socketCallback,
                                                      void*               callbackData,
                                                      SOPC_Socket**       listenerSocket);

SOPC_StatusCode SOPC_CreateAction_SocketCreateServer(const char*                  uri,
                                                     SOPC_Socket_EventCB*         socketCallback,
                                                     void*                        callbackData,
                                                     SOPC_Socket_EndCreate_CB*    endCreateCb,
                                                     void*                        endCreateCbData);

SOPC_StatusCode SOPC_SocketManager_ConfigureAcceptedSocket(SOPC_Socket*        acceptedSocket,
                                                           SOPC_Socket_EventCB socketCallback,
                                                           void*               callbackData);

SOPC_StatusCode SOPC_SocketManager_Loop(SOPC_SocketManager* socketManager,
                                        uint32_t            msecTimeout);

SOPC_StatusCode SOPC_CreateAction_SocketWrite(SOPC_Socket*                  socket,
                                              uint8_t*                      data,
                                              uint32_t                      count,
                                              SOPC_Socket_Transaction_Event transactionEvent,
                                              uint32_t                      transactionId,
                                              SOPC_Socket_EndOperation_CB*  endWriteCb,
                                              void*                         endWriteCbData);

SOPC_StatusCode SOPC_CreateAction_SocketWriteEvent(SOPC_Socket* socket);

SOPC_StatusCode SOPC_Socket_Read (SOPC_Socket* socket,
                                  uint8_t*     data,
                                  uint32_t     dataSize,
                                  int32_t*     readCount);


SOPC_StatusCode SOPC_CreateAction_SocketEvent(SOPC_Socket_EventCB* callback,
                                              SOPC_Socket*         socket,
                                              SOPC_Socket_Event    event,
                                              void*                callbackData);

void SOPC_Socket_Close(SOPC_Socket* socket);


#endif /* SOPC_SOCKETS_H_ */
