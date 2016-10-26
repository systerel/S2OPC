/*
 * ua_sockets.h
 *
 *  Created on: Oct 20, 2016
 *      Author: vincent
 */
#include <stdint.h>
#include <ua_base_types.h>

#ifndef INGOPCS_SOCKETS_UA_SOCKETS_H_
#define INGOPCS_SOCKETS_UA_SOCKETS_H_

typedef enum {
    SOCKET_READ_EVENT,
    SOCKET_WRITE_EVENT,
    SOCKET_CLOSE_EVENT,
    SOCKET_EXCEPT_EVENT,
    SOCKET_TIMEOUT_EVENT,
    SOCKET_SHUTDOWN_EVENT,
    SOCKET_CONNECT_EVENT,
    SOCKET_ACCEPT_EVENT} UA_Socket_Event;

typedef enum {
    SOCKET_DISCONNECTED          = 0x00,
    SOCKET_CONNECTING            = 0x01, // Client connect waiting for write event
                                         // && SO_ERROR to be verified on event to confirm connection accepted
    SOCKET_CONNECTED             = 0x02, // Client: write event received after connect / Server: connection accepted
    SOCKET_LISTENING             = 0x03 // Server: listening socket
} UA_Socket_State;

typedef struct {
    int              sock;
    uint8_t          isUsed;
    UA_Socket_State  state;
    void*            eventCallback; // UA_Socket_EventCB Type
    void*            cbData;
} UA_Socket;

typedef StatusCode (UA_Socket_EventCB) (UA_Socket*  socket,
                                        uint32_t    socketEvent,
                                        void*       cbData,
                                        uint16_t    portNumber,
                                        uint8_t     isSSL);

typedef struct {
    uint32_t   nbSockets;
    UA_Socket  *sockets;
}
UA_SocketManager;

UA_SocketManager* UA_SocketManager_GetGlobal(void);

StatusCode UA_SocketManager_Initialize(UA_SocketManager* socketMgr,
                                       uint32_t          nbSockets);

UA_SocketManager* UA_SocketManager_Create(uint32_t nbSockets);

void UA_SocketManager_Clear(UA_SocketManager* socketMgr);

void UA_SocketManager_Delete(UA_SocketManager** socketMgr);

StatusCode UA_SocketManager_CreateClientSocket(UA_SocketManager*  socketManager,
                                               const char*        uri,
                                               UA_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               UA_Socket**        clientSocket);

StatusCode UA_SocketManager_CreateServerSocket(UA_SocketManager*  socketManager,
                                               const char*        uri,
                                               uint8_t            listenAllItfs,
                                               UA_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               UA_Socket**        listenerSocket);

StatusCode UA_SocketManager_Loop(UA_SocketManager* socketManager,
                                 uint32_t          msecTimeout,
                                 uint8_t           runOnce);

int32_t UA_Socket_Write (UA_Socket* socket,
                         uint8_t*   data,
                         uint32_t   count);

StatusCode UA_Socket_Read (UA_Socket* socket,
                           uint8_t*   data,
                           uint32_t   dataSize,
                           uint32_t*  readCount);

void UA_Socket_Close(UA_Socket* socket);


#endif /* INGOPCS_SOCKETS_UA_SOCKETS_H_ */
