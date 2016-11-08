/*
 *sopc_sockets.h
 *
 *  Created on: Oct 20, 2016
 *      Author: vincent
 */
#include <stdint.h>
#include <p_sockets.h>
#include <sopc_base_types.h>

#ifndef INGOPCS_SOCKETS_SOPC_SOCKETS_H_
#define INGOPCS_SOCKETS_SOPC_SOCKETS_H_

typedef enum {
    SOCKET_READ_EVENT,
    SOCKET_WRITE_EVENT,
    SOCKET_CLOSE_EVENT,
    SOCKET_EXCEPT_EVENT,
    SOCKET_TIMEOUT_EVENT,
    SOCKET_SHUTDOWN_EVENT,
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
                                        uint32_t          socketEvent,
                                        void*             cbData,
                                        uint16_t          portNumber,
                                        uint8_t           isSSL);

typedef struct SOPC_Socket {
    Socket             sock;
    uint8_t            isUsed;
    SOPC_Socket_State    state;
    SOPC_Socket_EventCB* eventCallback; // SOPC_Socket_EventCB Type
    void*              cbData;
} SOPC_Socket;

typedef struct {
    uint32_t   nbSockets;
    SOPC_Socket  *sockets;
}
SOPC_SocketManager;

SOPC_SocketManager* SOPC_SocketManager_GetGlobal(void);

SOPC_StatusCode SOPC_SocketManager_Initialize(SOPC_SocketManager* socketMgr,
                                       uint32_t          nbSockets);

SOPC_SocketManager* SOPC_SocketManager_Create(uint32_t nbSockets);

void SOPC_SocketManager_Clear(SOPC_SocketManager* socketMgr);

void SOPC_SocketManager_Delete(SOPC_SocketManager** socketMgr);

SOPC_StatusCode SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager*  socketManager,
                                               const char*        uri,
                                               SOPC_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               SOPC_Socket**        clientSocket);

SOPC_StatusCode SOPC_SocketManager_CreateServerSocket(SOPC_SocketManager*  socketManager,
                                               const char*        uri,
                                               uint8_t            listenAllItfs,
                                               SOPC_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               SOPC_Socket**        listenerSocket);

SOPC_StatusCode SOPC_SocketManager_Loop(SOPC_SocketManager* socketManager,
                                 uint32_t          msecTimeout);

int32_t SOPC_Socket_Write (SOPC_Socket* socket,
                         uint8_t*   data,
                         uint32_t   count);

SOPC_StatusCode SOPC_Socket_Read (SOPC_Socket* socket,
                           uint8_t*   data,
                           uint32_t   dataSize,
                           uint32_t*  readCount);

void SOPC_Socket_Close(SOPC_Socket* socket);


#endif /* INGOPCS_SOCKETS_SOPC_SOCKETS_H_ */
