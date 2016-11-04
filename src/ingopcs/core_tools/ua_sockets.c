/*
 * ua_sockets.c
 *
 *  Created on: Oct 20, 2016
 *      Author: vincent
 */

#include "ua_sockets.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ua_stack_csts.h>

UA_SocketManager globalSocketMgr;

// Counter to check <= UA_MAXCONNECTIONS
static uint32_t globalNbSockets = 0;

StatusCode ParseURI (const char* uri, char** hostname, char** port){
    StatusCode status = STATUS_NOK;
    size_t idx = 0;
    uint8_t isPort = FALSE;
    uint8_t hasPort = FALSE;
    size_t hostnameLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;
    uint8_t invalid = FALSE;
    char *lHostname = NULL;
    char *lPort = NULL;
    if(uri != NULL && hostname != NULL && port != NULL){

        if(strlen(uri) + 4  > 4096){
            // Encoded value shall be less than 4096 bytes
            status = STATUS_INVALID_PARAMETERS;
        }else if(strlen(uri) > 10 && memcmp(uri, (const char*) "opc.tcp://", 10) == 0){
            // search for a ':' defining port for given IP
            // search for a '/' defining endpoint name for given IP => at least 1 char after it (len - 1)
            for(idx = 10; idx < strlen(uri) - 1; idx++){
                if(isPort){
                    if(uri[idx] >= '0' && uri[idx] <= '9'){
                        if(hasPort == FALSE){
                            // port definition
                            hasPort = 1;
                            portIdx = idx;
                        }
                    }else if(uri[idx] == '/'){
                        // end of port definition + at least one character remaining
                        if(hasPort != FALSE && invalid == FALSE){
                            portLength = idx - portIdx;
                            status = STATUS_OK;
                        }
                    }else if(hasPort == FALSE){
                        // unexpected character
                        invalid = 1;
                    }
                }else{
                    if(uri[idx] == ':'){
                        hostnameLength = idx - 10;
                        isPort = 1;
                    }
                }
            }

            if(invalid != 0){
                status = STATUS_NOK;
            }
        }
    }

    if(status == STATUS_OK){
        if(portIdx != 0 && hostnameLength != 0 && portLength != 0){
            lHostname = malloc(sizeof(char) * (hostnameLength+1));
            if(lHostname == NULL)
                return STATUS_INVALID_PARAMETERS;
            if(lHostname != memcpy(lHostname, &(uri[10]), hostnameLength)){
                free(lHostname);
                return STATUS_INVALID_PARAMETERS;
            }
            lHostname[hostnameLength] = '\0';

            lPort = malloc(sizeof(char) * (portLength+1));
            if(lPort == NULL){
                free(lHostname);
                return STATUS_INVALID_PARAMETERS;
            }
            if(lPort != memcpy(lPort, &(uri[portIdx]), portLength)){
                free(lHostname);
                free(lPort);
                return STATUS_INVALID_PARAMETERS;
            }
            lPort[portLength] = '\0';
            *hostname = lHostname;
            *port = lPort;
        }else{
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

UA_SocketManager* UA_SocketManager_GetGlobal(){
    return &globalSocketMgr;
}

UA_SocketManager* UA_SocketManager_Create(uint32_t nbSockets){
    UA_SocketManager* socketMgr = NULL;
    if(nbSockets > 0){
        socketMgr = malloc(sizeof(UA_SocketManager));
        if(socketMgr != NULL){
            if(STATUS_OK != UA_SocketManager_Initialize(socketMgr, nbSockets)){
                free(socketMgr);
                socketMgr = NULL;
            }
        }
    }
    return socketMgr;
}

StatusCode UA_SocketManager_Initialize(UA_SocketManager* socketMgr,
                                       uint32_t          nbSockets){
    uint32_t idx = 0;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(globalNbSockets + nbSockets > OPCUA_MAXCONNECTIONS)
        return STATUS_NOK;

    // TODO: set lower limit for nbSockets: INT32_MAX just to ensure select returns value <= INT32_MAX (3 sets)
    if(socketMgr != NULL && socketMgr->nbSockets == 0 && nbSockets <= INT32_MAX/3){
        socketMgr->sockets = malloc(sizeof(UA_Socket) * nbSockets);
        if(socketMgr->sockets != NULL){
            status = STATUS_OK;
            memset(socketMgr->sockets, 0, sizeof(UA_Socket) * nbSockets);
            socketMgr->nbSockets = nbSockets;
            for(idx = 0; idx < nbSockets; idx++){
                Socket_Clear(&(socketMgr->sockets[idx].sock));
            }
            globalNbSockets += nbSockets;
        }
    }
    return status;
}

void UA_SocketManager_Clear(UA_SocketManager* socketMgr){
    if(socketMgr != NULL &&
       socketMgr->nbSockets > 0 &&
       socketMgr->sockets != NULL){
        assert(globalNbSockets >= socketMgr->nbSockets);
        globalNbSockets -= socketMgr->nbSockets;
        free(socketMgr->sockets);
        socketMgr->sockets = NULL;
        socketMgr->nbSockets = 0;
    }
}

void UA_SocketManager_Delete(UA_SocketManager** socketMgr){
    if(socketMgr != NULL){
        UA_SocketManager_Clear(*socketMgr);
        *socketMgr = NULL;
    }
}

UA_Socket* GetFreeSocket(UA_SocketManager* socketMgr){
    UA_Socket* result = NULL;
    size_t idx = 0;
    if(socketMgr != NULL){
        for(idx = 0; idx < socketMgr->nbSockets; idx++){
            if(result == NULL && socketMgr->sockets[idx].isUsed == FALSE){
                result = &(socketMgr->sockets[idx]);
            }
        }
    }
    return result;
}

StatusCode UA_SocketManager_CreateClientSocket(UA_SocketManager*  socketManager,
                                               const char*        uri,
                                               UA_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               UA_Socket**        clientSocket)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    Socket_AddressInfo *res, *p;
    UA_Socket* freeSocket;
    StatusCode connectStatus = STATUS_NOK;
    char *hostname = NULL;
    char *port = NULL;

    if(socketManager != NULL && uri != NULL && clientSocket != NULL){
        status = ParseURI(uri, &hostname, &port);
        if(status == STATUS_OK){
            freeSocket = GetFreeSocket(socketManager);
            if(freeSocket == NULL){
                status = STATUS_NOK;
            }
        }

        if(status == STATUS_OK){
            status = Socket_AddrInfo_Get(hostname, port, &res);
        }

        if(status == STATUS_OK){
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            for(p = res;p != NULL && connectStatus != STATUS_OK; p = Socket_AddrInfo_IterNext(p)) {


                status = Socket_CreateNew(p,
                                          FALSE, // Do not reuse
                                          1,     // Non blocking socket
                                          &freeSocket->sock);

                if (status == STATUS_OK){
                    connectStatus = Socket_Connect(freeSocket->sock, p);
                }

                if(connectStatus == STATUS_OK){
                    freeSocket->state = SOCKET_CONNECTING;
                }

                if(connectStatus != STATUS_OK){
                    UA_Socket_Close(freeSocket);
                }
            }
            status = connectStatus;
        }
        if(port != NULL){
            free(port);
        }
        if(hostname != NULL){
            free(hostname);
        }
    }

    if(status == STATUS_OK){
        freeSocket->isUsed = 1;
        freeSocket->eventCallback = socketCallback;
        freeSocket->cbData = callbackData;
        *clientSocket = freeSocket;
    }

    Socket_AddrInfoDelete(&res);

    return status;
}

StatusCode UA_SocketManager_CreateServerSocket(UA_SocketManager*  socketManager,
                                               const char*        uri,
                                               uint8_t            listenAllItfs,
                                               UA_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               UA_Socket**        clientSocket)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    Socket_AddressInfo *res, *p;
    UA_Socket* freeSocket;
    StatusCode listenStatus = STATUS_NOK;
    char *hostname = NULL;
    char *port = NULL;

    if(socketManager != NULL && uri != NULL && clientSocket != NULL){
        status = ParseURI(uri, &hostname, &port);
        if(status == STATUS_OK){
            freeSocket = GetFreeSocket(socketManager);
            if(freeSocket == NULL){
                status = STATUS_NOK;
            }
        }

        if(status == STATUS_OK){
            if(listenAllItfs != FALSE){
                free(hostname);
                hostname = NULL;
            }
            status = Socket_AddrInfo_Get(hostname, port, &res);
        }

        if(status == STATUS_OK){
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            for(p = res;p != NULL && listenStatus != STATUS_OK; p = Socket_AddrInfo_IterNext(p)) {


                status = Socket_CreateNew(p,
                                          1, // Reuse
                                          1, // Non blocking socket
                                          &freeSocket->sock);

                if (status == STATUS_OK){
                    status = Socket_Listen(freeSocket->sock, p);
                }

                if(status == STATUS_OK){
                    freeSocket->state = SOCKET_LISTENING;
                }

                if(status != STATUS_OK){
                    UA_Socket_Close(freeSocket);
                }
            }
        }
        if(port != NULL){
            free(port);
        }
        if(hostname != NULL){
            free(hostname);
        }
    }

    if(status == STATUS_OK){
        freeSocket->isUsed = 1;
        freeSocket->eventCallback = socketCallback;
        freeSocket->cbData = callbackData;
        *clientSocket = freeSocket;
    }

    Socket_AddrInfoDelete(&res);

    return status;
}

StatusCode UA_SocketManager_Loop(UA_SocketManager* socketManager,
                                 uint32_t          msecTimeout){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    int32_t nbReady = 0;
    UA_Socket* uaSock = NULL;
    UA_Socket* acceptSock = NULL;
    UA_Socket_EventCB*  callback = NULL;
    SocketSet readSet, writeSet, exceptSet;

    SocketSet_Clear(&readSet);
    SocketSet_Clear(&writeSet);
    SocketSet_Clear(&exceptSet);

    if(socketManager != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){

        // Add used sockets in the correct socket sets
        for(idx = 0; idx < socketManager->nbSockets; idx++){
            uaSock = &(socketManager->sockets[idx]);
            if(uaSock->isUsed != FALSE){
               if(uaSock->state == SOCKET_CONNECTING){
                   SocketSet_Add(uaSock->sock, &writeSet);
               }else{
                   SocketSet_Add(uaSock->sock, &readSet);
               }
               SocketSet_Add(uaSock->sock, &exceptSet);
            }
        }

        // Returns number of ready descriptor or -1 in case of error
        nbReady = Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, msecTimeout);

        if(nbReady < 0){
            status =  STATUS_NOK;
        }else if(nbReady > 0){
            for(idx = 0; idx < socketManager->nbSockets; idx++){
                uaSock = &(socketManager->sockets[idx]);
                if(uaSock->isUsed != FALSE){
                    callback = (UA_Socket_EventCB*) uaSock->eventCallback;
                    if(uaSock->state == SOCKET_CONNECTING){
                        if(SocketSet_IsPresent(uaSock->sock, &writeSet) != FALSE){
                            // Check connection erros: mandatory when non blocking connection
                            if(STATUS_OK != Socket_CheckAckConnect(uaSock->sock)){
                                callback(uaSock,
                                         SOCKET_CLOSE_EVENT,
                                         uaSock->cbData,
                                         0,
                                         0);
                                UA_Socket_Close(uaSock);
                            }else{
                                callback(uaSock,
                                         SOCKET_CONNECT_EVENT,
                                         uaSock->cbData,
                                         0,
                                         0);
                                uaSock->state = SOCKET_CONNECTED;
                            }
                        }
                    }else{
                        if(SocketSet_IsPresent(uaSock->sock, &readSet) != FALSE){
                            if(uaSock->state == SOCKET_CONNECTED){
                                callback(uaSock,
                                         SOCKET_READ_EVENT,
                                         uaSock->cbData,
                                         0,
                                         0);
                            }else if(uaSock->state == SOCKET_LISTENING){
                                acceptSock = GetFreeSocket(socketManager);
                                if(acceptSock == NULL){
                                    // TODO: No more free sockets
                                    status = STATUS_NOK;
                                }else{
                                    status = Socket_Accept(uaSock->sock,
                                                           &acceptSock->sock);
                                    if(status == STATUS_OK){
                                        acceptSock->isUsed = 1;
                                        acceptSock->state = SOCKET_CONNECTED;
                                        acceptSock->eventCallback = uaSock->eventCallback;
                                        acceptSock->cbData = uaSock->cbData;
                                    }
                                }
                            }else{
                                return STATUS_INVALID_STATE;
                            }
                        }
                    }

                    if(SocketSet_IsPresent(uaSock->sock, &exceptSet) != FALSE){
                        callback(uaSock,
                                 SOCKET_EXCEPT_EVENT,
                                 uaSock->cbData,
                                 0,
                                 0);
                    }
                }
            }
        }
    }

    return status;
}

int32_t UA_Socket_Write (UA_Socket* socket,
                         uint8_t*   data,
                         uint32_t   count){
    return Socket_Write(socket->sock, data, count);
}

StatusCode UA_Socket_Read (UA_Socket* socket,
                           uint8_t*   data,
                           uint32_t   dataSize,
                           uint32_t*  readCount){
    return Socket_Read(socket->sock, data, dataSize, readCount);
}

void UA_Socket_Close(UA_Socket* socket){
    if(socket != NULL){
        Socket_Close(&socket->sock);
        socket->isUsed = FALSE;
        socket->state = SOCKET_DISCONNECTED;
        socket->eventCallback = NULL;
        socket->cbData = NULL;
    }
}

