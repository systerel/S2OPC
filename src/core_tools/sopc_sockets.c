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

#include "sopc_sockets.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "base_tools.h"
#include "sopc_stack_csts.h"

SOPC_SocketManager globalSocketMgr;
uint8_t            globalInitialized = FALSE;

// Counter to check <= OPCUA_MAXCONNECTIONS
static uint32_t globalNbSockets = 0;

SOPC_StatusCode Internal_CheckURI(const char* uri,
                                  size_t* hostnameLength,
                                  size_t* portIdx,
                                  size_t* portLength){
    SOPC_StatusCode status = STATUS_NOK;
    size_t idx = 0;
    uint8_t isPort = FALSE;
    uint8_t hasPort = FALSE;
    uint8_t hasName = FALSE;
    uint8_t invalid = FALSE;
    uint8_t startIPv6 = FALSE;
    if(uri != NULL && hostnameLength != NULL && portLength != NULL){
        *hostnameLength = 0;
        *portIdx = 0;
        *portLength = 0;
        if(strlen(uri) + 4  > TCP_UA_MAX_URL_LENGTH){
            // Encoded value shall be less than 4096 bytes
            status = STATUS_INVALID_PARAMETERS;
        }else if(strlen(uri) > 10 && strncmp_ignore_case(uri, (const char*) "opc.tcp://", 10) == 0){
            // search for a ':' defining port for given IP
            // search for a '/' defining endpoint name for given IP => at least 1 char after it (len - 1)
            for(idx = 10; idx < strlen(uri) - 1; idx++){
                if(isPort){
                    if(uri[idx] >= '0' && uri[idx] <= '9'){
                        if(hasPort == FALSE){
                            // port definition
                            hasPort = 1;
                            *portIdx = idx;
                        }
                    }else if(uri[idx] == '/' && invalid == FALSE){
                        // Name of the endpoint after port, invalid otherwise
                        if(hasPort == FALSE){
                            invalid = 1;
                        }else{
                            *portLength = idx - *portIdx;
                            hasName = 1;
                        }
                    }else{
                        if(hasPort == FALSE || hasName == FALSE){
                            // unexpected character: we do not expect a endpoint name
                            invalid = 1;
                        }
                    }
                }else{
                     if(uri[idx] == ':' && startIPv6 == FALSE){
                        *hostnameLength = idx - 10;
                        isPort = 1;
                    }else if(uri[idx] == '['){
                        startIPv6 = 1;
                    }else if(uri[idx] == ']'){
                        if(startIPv6 == FALSE){
                            invalid = 1;
                        }else{
                            startIPv6 = FALSE;
                        }
                    }
                }
            }

            if(hasPort != FALSE && invalid == FALSE){
                status = STATUS_OK;
                if(*portLength == 0){
                    // No endpoint name after port provided
                    *portLength = idx - *portIdx + 1;
                }
            }

        }
    }
    return status;
}

SOPC_StatusCode ParseURI (const char* uri, char** hostname, char** port){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    size_t hostnameLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;
    char *lHostname = NULL;
    char *lPort = NULL;

    if(uri != NULL && hostname != NULL && port != NULL){
        status = Internal_CheckURI(uri, &hostnameLength, &portIdx, &portLength);
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

SOPC_StatusCode SOPC_Check_TCP_UA_URI(const char* uri){
    size_t hostLength, portIdx, portLength;

    return Internal_CheckURI(uri, &hostLength, &portIdx, &portLength);
}

SOPC_SocketManager* SOPC_SocketManager_GetGlobal(){
    return &globalSocketMgr;
}

SOPC_StatusCode SOPC_SocketManager_Config_Init(){
    SOPC_StatusCode status = STATUS_NOK;
    globalNbSockets = 0;
#if OPCUA_MULTITHREADED == FALSE
    status = SOPC_SocketManager_Initialize(SOPC_SocketManager_GetGlobal(), OPCUA_MAXCONNECTIONS);
#else
    status = STATUS_OK;
#endif //OPCUA_MULTITHREADED
    return status;
}

void SOPC_SocketManager_Config_Clear(){
#if OPCUA_MULTITHREADED == FALSE
    SOPC_SocketManager_Clear(SOPC_SocketManager_GetGlobal());
#else
    NULL;
#endif //OPCUA_MULTITHREADED
    return;
}

SOPC_SocketManager* SOPC_SocketManager_Create(uint32_t nbSockets){
    SOPC_SocketManager* socketMgr = NULL;
    if(nbSockets > 0){
        socketMgr = malloc(sizeof(SOPC_SocketManager));
        if(socketMgr != NULL){
            if(STATUS_OK != SOPC_SocketManager_Initialize(socketMgr, nbSockets)){
                free(socketMgr);
                socketMgr = NULL;
            }
        }
    }
    return socketMgr;
}

SOPC_StatusCode SOPC_SocketManager_Initialize(SOPC_SocketManager* socketMgr,
                                              uint32_t            nbSockets){
    uint32_t idx = 0;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(globalNbSockets + nbSockets > OPCUA_MAXCONNECTIONS)
        return STATUS_NOK;

    // TODO: set lower limit for nbSockets: INT32_MAX just to ensure select returns value <= INT32_MAX (3 sets)
    if(socketMgr != NULL && nbSockets <= INT32_MAX/3){
        socketMgr->sockets = malloc(sizeof(SOPC_Socket) * nbSockets);
        if(socketMgr->sockets != NULL){
            status = STATUS_OK;
            memset(socketMgr->sockets, 0, sizeof(SOPC_Socket) * nbSockets);
            socketMgr->nbSockets = nbSockets;
            for(idx = 0; idx < nbSockets; idx++){
                Socket_Clear(&(socketMgr->sockets[idx].sock));
            }
            globalNbSockets += nbSockets;

            Mutex_Inititalization(&socketMgr->mutex);
        }
    }
    return status;
}

void SOPC_SocketManager_Clear(SOPC_SocketManager* socketMgr){

    if(socketMgr != NULL &&
       socketMgr->nbSockets > 0 &&
       socketMgr->sockets != NULL){
        Mutex_Lock(&socketMgr->mutex);
        assert(globalNbSockets >= socketMgr->nbSockets);
        globalNbSockets -= socketMgr->nbSockets;
        free(socketMgr->sockets);
        socketMgr->sockets = NULL;
        socketMgr->nbSockets = 0;
        Mutex_Unlock(&socketMgr->mutex);
        Mutex_Clear(&socketMgr->mutex);
    }
}

void SOPC_SocketManager_Delete(SOPC_SocketManager** socketMgr){
    if(socketMgr != NULL){
        SOPC_SocketManager_Clear(*socketMgr);
        *socketMgr = NULL;
    }
}

SOPC_Socket* GetFreeSocket(SOPC_SocketManager* socketMgr){
    SOPC_Socket* result = NULL;
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

SOPC_StatusCode SOPC_SocketManager_InternalConnectClient(SOPC_Socket*        connectSocket,
                                                         Socket_AddressInfo* addr){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(connectSocket != NULL && addr != NULL){
        status = Socket_CreateNew(addr,
                                  FALSE, // Do not reuse
                                  1,     // Non blocking socket
                                  &connectSocket->sock);

        if (status == STATUS_OK){
            status = Socket_Connect(connectSocket->sock, addr);
        }

        if(status == STATUS_OK){
            connectSocket->state = SOCKET_CONNECTING;
        }

        if(status != STATUS_OK){
            Socket_Close(&connectSocket->sock);
            connectSocket->state = SOCKET_DISCONNECTED;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager* socketManager,
                                                      const char*         uri,
                                                      SOPC_Socket_EventCB socketCallback,
                                                      void*               callbackData,
                                                      SOPC_Socket**       clientSocket)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Socket_AddressInfo *res = NULL, *p = NULL;
    SOPC_Socket* freeSocket = NULL;
    SOPC_StatusCode connectStatus = STATUS_NOK;
    char *hostname = NULL;
    char *port = NULL;

    if(socketManager != NULL && uri != NULL && clientSocket != NULL){
        Mutex_Lock(&socketManager->mutex);
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
                connectStatus = SOPC_SocketManager_InternalConnectClient(freeSocket, p);
            }
            status = connectStatus;
        }

        if(status == STATUS_OK){
            freeSocket->isUsed = 1;
            freeSocket->eventCallback = socketCallback;
            freeSocket->cbData = callbackData;
            if(p != NULL){
                // Next attempts addresses for connections remaining: store to use in case of async. connection failure
                freeSocket->nextConnectAttemptAddr = p;
                freeSocket->connectAddrs = res;
            }

            *clientSocket = freeSocket;
        }


        if(port != NULL){
            free(port);
        }

        if(hostname != NULL){
            free(hostname);
        }

        Mutex_Unlock(&socketManager->mutex);
    }

    if(status != STATUS_OK || // connection already failed => do not keep addresses for next attempts
       (res != NULL && freeSocket->connectAddrs == NULL)) // async connecting but NO next attempts remaining (if current fails)
    {
        Socket_AddrInfoDelete(&res);
    }

    return status;
}

SOPC_StatusCode SOPC_SocketManager_CreateServerSocket(SOPC_SocketManager* socketManager,
                                                      const char*         uri,
                                                      uint8_t             listenAllItfs,
                                                      SOPC_Socket_EventCB socketCallback,
                                                      void*               callbackData,
                                                      SOPC_Socket**       listeningSocket)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Socket_AddressInfo *res = NULL, *p = NULL;
    uint8_t attemptWithIPV6 = 1;
    SOPC_Socket* freeSocket = NULL;
    SOPC_StatusCode listenStatus = STATUS_NOK;
    char *hostname = NULL;
    char *port = NULL;

    if(socketManager != NULL && uri != NULL && listeningSocket != NULL){
        Mutex_Lock(&socketManager->mutex);
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
            p = res;
            attemptWithIPV6 = 1; // IPV6 first since it supports IPV4
            while((p != NULL || attemptWithIPV6 != FALSE) && listenStatus != STATUS_OK){

                if(p == NULL && attemptWithIPV6 != FALSE){
                    // Failed with IPV6 addresses (or none was present), now try with not IPV6 addresses
                    attemptWithIPV6 = FALSE;
                    p = res;
                }else{
                    if((attemptWithIPV6 != FALSE && Socket_AddrInfo_IsIPV6(p) != FALSE) ||
                       (attemptWithIPV6 == FALSE && Socket_AddrInfo_IsIPV6(p) == FALSE)){
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
                            SOPC_Socket_Close(freeSocket);
                        }else{
                            listenStatus = STATUS_OK;
                        }
                    }
                    p = Socket_AddrInfo_IterNext(p);
                }
            }
        }

        if(port != NULL){
            free(port);
        }
        if(hostname != NULL){
            free(hostname);
        }


        if(status == STATUS_OK){
            freeSocket->isUsed = 1;
            freeSocket->eventCallback = socketCallback;
            freeSocket->cbData = callbackData;
            *listeningSocket = freeSocket;
        }

        Mutex_Unlock(&socketManager->mutex);
    }

    Socket_AddrInfoDelete(&res);

    return status;
}

SOPC_StatusCode SOPC_SocketManager_ConfigureAcceptedSocket(SOPC_Socket*        acceptedSocket,
                                                           SOPC_Socket_EventCB socketCallback,
                                                           void*               callbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(acceptedSocket != NULL && socketCallback != NULL){
        acceptedSocket->eventCallback = socketCallback;
        acceptedSocket->cbData = callbackData;
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode SOPC_SocketManager_Loop(SOPC_SocketManager* socketManager,
                                        uint32_t            msecTimeout){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    int32_t nbReady = 0;
    SOPC_Socket* uaSock = NULL;
    SOPC_Socket* acceptSock = NULL;
    SOPC_Socket_EventCB*  callback = NULL;
    SocketSet readSet, writeSet, exceptSet;

    SocketSet_Clear(&readSet);
    SocketSet_Clear(&writeSet);
    SocketSet_Clear(&exceptSet);

    if(socketManager != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        Mutex_Lock(&socketManager->mutex);

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

        Mutex_Unlock(&socketManager->mutex);

        // Returns number of ready descriptor or -1 in case of error
        nbReady = Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, msecTimeout);

        Mutex_Lock(&socketManager->mutex);

        if(nbReady < 0){
            status =  STATUS_NOK;
        }else if(nbReady > 0){
            for(idx = 0; idx < socketManager->nbSockets; idx++){
                uaSock = &(socketManager->sockets[idx]);
                if(uaSock->isUsed != FALSE){
                    callback = (SOPC_Socket_EventCB*) uaSock->eventCallback;
                    if(uaSock->state == SOCKET_CONNECTING){
                        if(SocketSet_IsPresent(uaSock->sock, &writeSet) != FALSE){
                            // Check connection erros: mandatory when non blocking connection
                            if(STATUS_OK != Socket_CheckAckConnect(uaSock->sock)){
                                SOPC_StatusCode newAttempt = STATUS_NOK;

                                // Check if next connection attempt available
                                Socket_AddressInfo* nextAddr = (Socket_AddressInfo*) uaSock->nextConnectAttemptAddr;
                                if(nextAddr != NULL){
                                    newAttempt = SOPC_SocketManager_InternalConnectClient(uaSock,
                                                                                          nextAddr);
                                    if(newAttempt != STATUS_OK){
                                        uaSock->nextConnectAttemptAddr = NULL;
                                    }else{
                                        uaSock->nextConnectAttemptAddr = Socket_AddrInfo_IterNext(nextAddr);
                                    }

                                    // No more attempts possible: free the attempts addresses
                                    if(uaSock->nextConnectAttemptAddr == NULL){
                                        Socket_AddrInfoDelete((Socket_AddressInfo**) &uaSock->connectAddrs);
                                        uaSock->connectAddrs = NULL;
                                    }
                                }

                                if(newAttempt != STATUS_OK){
                                    // No new attempt available, close socket
                                    callback(uaSock,
                                             SOCKET_CLOSE_EVENT,
                                             uaSock->cbData);
                                    SOPC_Socket_Close(uaSock);
                                }
                            }else{
                                callback(uaSock,
                                         SOCKET_CONNECT_EVENT,
                                         uaSock->cbData);
                                uaSock->state = SOCKET_CONNECTED;

                                // No more attempts expected: free the attempts addresses
                                if(uaSock->connectAddrs != NULL){
                                    Socket_AddrInfoDelete((Socket_AddressInfo**) &uaSock->connectAddrs);
                                    uaSock->connectAddrs = NULL;
                                    uaSock->nextConnectAttemptAddr = NULL;
                                }
                            }
                        }
                    }else{
                        if(SocketSet_IsPresent(uaSock->sock, &readSet) != FALSE){
                            if(uaSock->state == SOCKET_CONNECTED){
                                callback(uaSock,
                                         SOCKET_READ_EVENT,
                                         uaSock->cbData);
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
                                        callback(acceptSock,
                                                 SOCKET_ACCEPT_EVENT,
                                                 uaSock->cbData);
                                    }
                                }
                            }else{
                                Mutex_Unlock(&socketManager->mutex);
                                return STATUS_INVALID_STATE;
                            }
                        }
                    }

                    if(SocketSet_IsPresent(uaSock->sock, &exceptSet) != FALSE){
                        callback(uaSock,
                                 SOCKET_EXCEPT_EVENT,
                                 uaSock->cbData);
                    }
                }
            }
        }

        Mutex_Unlock(&socketManager->mutex);
    }

    return status;
}

int32_t SOPC_Socket_Write (SOPC_Socket* socket,
                           uint8_t*     data,
                           uint32_t     count){
    return Socket_Write(socket->sock, data, count);
}

SOPC_StatusCode SOPC_Socket_Read (SOPC_Socket* socket,
                                  uint8_t*     data,
                                  uint32_t     dataSize,
                                  int32_t*     readCount){
    return Socket_Read(socket->sock, data, dataSize, readCount);
}

void SOPC_Socket_Close(SOPC_Socket* socket){
    if(socket != NULL){
        Socket_Close(&socket->sock);
        socket->isUsed = FALSE;
        socket->state = SOCKET_DISCONNECTED;
        socket->eventCallback = NULL;
        socket->cbData = NULL;
        if(socket->connectAddrs != NULL){
            Socket_AddrInfoDelete((Socket_AddressInfo**) &socket->connectAddrs);
        }
        socket->connectAddrs = NULL;
        socket->nextConnectAttemptAddr = NULL;
    }
}

