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

#include "sopc_helper_string.h"
#include "sopc_stack_csts.h"
#include "sopc_action_queue.h"
#include "sopc_action_queue_manager.h"
#include "sopc_time.h"

SOPC_SocketManager globalSocketMgr;
uint8_t            globalInitialized = FALSE;

// Counter to check <= OPCUA_MAXCONNECTIONS
static uint32_t globalNbSockets = 0;

typedef struct SOPC_Action_WriteParameter {
    SOPC_Socket*                  socket;
    uint8_t                       isOnlyWriteEvent;
    uint8_t*                      data;
    uint32_t                      count;
    uint32_t                      posIdx;
    SOPC_Socket_Transaction_Event transactionEvent;
    uint32_t                      transactionId;
    SOPC_Socket_EndOperation_CB*  endWriteCb; // run after succesfull write (not as an Action in queue !) => atomicity
    void*                         endWriteCbData;
} SOPC_Action_WriteParameter;

typedef struct SOPC_Action_SocketEventParameter {
    SOPC_Socket_EventCB* callback;
    SOPC_Socket*         socket;
    SOPC_Socket_Event    event;
    void*                callbackData;
} SOPC_Action_SocketEventParameter;

typedef struct SOPC_Action_CreateSocketParameter {
    const char*                  uri;
    SOPC_Socket_EventCB*         socketCallback;
    void*                        callbackData;
    SOPC_Socket_EndCreate_CB*    endCreateCb;
    void*                        endCreateCbData;
} SOPC_Action_CreateSocketParameter;

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
        }else if(strlen(uri) > 10 && SOPC_strncmp_ignore_case(uri, (const char*) "opc.tcp://", 10) == 0){
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
    status = SOPC_SocketManager_Initialize(SOPC_SocketManager_GetGlobal(), OPCUA_MAXCONNECTIONS);
    return status;
}

void SOPC_SocketManager_Config_Clear(){
    SOPC_SocketManager_Clear(SOPC_SocketManager_GetGlobal());
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

            Mutex_Initialization(&socketMgr->mutex);
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
                // Initialize local write socket buffer
                if(STATUS_OK == SOPC_ActionQueue_Init(&socketMgr->sockets[idx].writeQueue, "Socket write msgs")){
                    result = &(socketMgr->sockets[idx]);
                }
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

void SOPC_Action_SocketCreateClient(void* arg){
    SOPC_Action_CreateSocketParameter* param = (SOPC_Action_CreateSocketParameter*) arg;
    SOPC_Socket* newSocket = NULL;
    if(param != NULL){
        // Non blocking connection attempt
        SOPC_StatusCode status = SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager_GetGlobal(),
                                                                       param->uri,
                                                                       param->socketCallback,
                                                                       param->callbackData,
                                                                       &newSocket);
        param->endCreateCb(param->endCreateCbData,
                           status,
                           newSocket);
        free(param);
    }
}

SOPC_StatusCode SOPC_CreateAction_SocketCreateClient(const char*                  uri,
                                                     SOPC_Socket_EventCB*         socketCallback,
                                                     void*                        callbackData,
                                                     SOPC_Socket_EndCreate_CB*    endCreateCb,
                                                     void*                        endCreateCbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_CreateSocketParameter* param = NULL;
    if(NULL != uri){
        param = malloc(sizeof(SOPC_Action_CreateSocketParameter));
        status = STATUS_NOK;
        if(NULL != param){
            param->uri = uri;
            param->socketCallback = socketCallback;
            param->callbackData = callbackData;
            param->endCreateCb = endCreateCb;
            param->endCreateCbData = endCreateCbData;
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Action_SocketCreateClient,
                                                       param,
                                                       "Create client socket");
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

void SOPC_Action_SocketCreateServer(void* arg){
    SOPC_Action_CreateSocketParameter* param = (SOPC_Action_CreateSocketParameter*) arg;
    SOPC_Socket* newSocket = NULL;
    if(param != NULL){
        // Non blocking connection attempt
        SOPC_StatusCode status = SOPC_SocketManager_CreateServerSocket(SOPC_SocketManager_GetGlobal(),
                                                                       param->uri,
                                                                       1, // listen all interfaces
                                                                       param->socketCallback,
                                                                       param->callbackData,
                                                                       &newSocket);
        param->endCreateCb(param->endCreateCbData, status, newSocket);
        free(param);
    }
}

SOPC_StatusCode SOPC_CreateAction_SocketCreateServer(const char*                  uri,
                                                     SOPC_Socket_EventCB*         socketCallback,
                                                     void*                        callbackData,
                                                     SOPC_Socket_EndCreate_CB*    endCreateCb,
                                                     void*                        endCreateCbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_CreateSocketParameter* param = NULL;
    if(NULL != uri){
        param = malloc(sizeof(SOPC_Action_CreateSocketParameter));
        status = STATUS_NOK;
        if(NULL != param){
            param->uri = uri;
            param->socketCallback = socketCallback;
            param->callbackData = callbackData;
            param->endCreateCb = endCreateCb;
            param->endCreateCbData = endCreateCbData;
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Action_SocketCreateServer,
                                                       param,
                                                       "Create server socket");
        }
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

SOPC_StatusCode SOPC_SocketManager_TreatSocketsEvents(SOPC_SocketManager* socketManager,
                                                      uint32_t            msecTimeout){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    uint8_t socketsUsed = FALSE;
    int32_t nbReady = 0;
    SOPC_Socket* uaSock = NULL;
    SOPC_Socket* acceptSock = NULL;
    SOPC_Socket_EventCB*  callback = NULL;
    void*                 cbData   = NULL;
    SocketSet readSet, writeSet, exceptSet;
    uint8_t error = FALSE;

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
               socketsUsed = !FALSE;
               if(uaSock->state == SOCKET_CONNECTING){
                   SocketSet_Add(uaSock->sock, &writeSet);
               }else{
                   SocketSet_Add(uaSock->sock, &readSet);
               }
               SocketSet_Add(uaSock->sock, &exceptSet);
            }
        }

        Mutex_Unlock(&socketManager->mutex);

        if(socketsUsed == FALSE){
            // In case no socket is in use, do not call select (*WaitSocketEvents)
            // since it returns an error with WinSockets
            SOPC_Sleep(msecTimeout);
            nbReady = 0;
        }else{
            // Returns number of ready descriptor or -1 in case of error
            nbReady = Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, msecTimeout);
        }

        Mutex_Lock(&socketManager->mutex);

        if(nbReady < 0){
            status =  STATUS_NOK;
        }else if(nbReady > 0){
            for(idx = 0; idx < socketManager->nbSockets && error == FALSE; idx++){
                uaSock = &(socketManager->sockets[idx]);
                if(uaSock->isUsed != FALSE){
                    callback = (SOPC_Socket_EventCB*) uaSock->eventCallback;
                    cbData = uaSock->cbData;
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
                                    status = SOPC_CreateAction_SocketEvent(callback,
                                                                           uaSock,
                                                                           SOCKET_CLOSE_EVENT,
                                                                           cbData);
                                }
                            }else{
                                uaSock->state = SOCKET_CONNECTED;
                                // No more attempts expected: free the attempts addresses
                                if(uaSock->connectAddrs != NULL){
                                    Socket_AddrInfoDelete((Socket_AddressInfo**) &uaSock->connectAddrs);
                                    uaSock->connectAddrs = NULL;
                                    uaSock->nextConnectAttemptAddr = NULL;
                                }
                                status = SOPC_CreateAction_SocketEvent(callback,
                                                                       uaSock,
                                                                       SOCKET_CONNECT_EVENT,
                                                                       cbData);
                            }
                        }
                    }else{
                        if(SocketSet_IsPresent(uaSock->sock, &readSet) != FALSE){
                            if(uaSock->state == SOCKET_CONNECTED){
                                status = SOPC_CreateAction_SocketEvent(callback,
                                                                       uaSock,
                                                                       SOCKET_READ_EVENT,
                                                                       cbData);
                            }else if(uaSock->state == SOCKET_LISTENING){
                                acceptSock = GetFreeSocket(socketManager);
                                if(acceptSock == NULL){
                                    // TODO: No more free sockets
                                    status = STATUS_NOK;
                                }else{
                                    status = Socket_Accept(uaSock->sock,
                                                           1, // Non blocking socket
                                                           &acceptSock->sock);
                                    if(status == STATUS_OK){
                                        acceptSock->isUsed = 1;
                                        acceptSock->state = SOCKET_CONNECTED;
                                        status = SOPC_CreateAction_SocketEvent(callback,
                                                                               acceptSock,
                                                                               SOCKET_ACCEPT_EVENT,
                                                                               cbData);
                                    }
                                }
                            }else{
                                error = 1; // Stop loop
                                status = STATUS_INVALID_STATE;
                            }
                        }else if(SocketSet_IsPresent(uaSock->sock, &writeSet) != FALSE){
                            if(uaSock->state == SOCKET_CONNECTED){
                                // Indicates that socket is writable again
                                status = SOPC_CreateAction_SocketWriteEvent(uaSock);
                            }else{
                                error = 1; // Stop loop
                                status = STATUS_INVALID_STATE;
                            }
                        }

                    }

                    if(SocketSet_IsPresent(uaSock->sock, &exceptSet) != FALSE){
                        status = SOPC_CreateAction_SocketEvent(callback,
                                                               acceptSock,
                                                               SOCKET_EXCEPT_EVENT,
                                                               cbData);
                    }
                }
            }
        }

        Mutex_Unlock(&socketManager->mutex);
    }

    return status;
}

SOPC_StatusCode SOPC_Socket_CheckAndUpdateTransaction(SOPC_Socket*                  socket,
                                                      SOPC_Socket_Transaction_Event transactionEvent,
                                                      uint32_t                      transactionId)
{
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    assert(NULL != socket);
    switch(socket->transactionState){
        case SOCKET_TRANSACTION_STATE_NONE:
            switch(transactionEvent){
                case SOCKET_TRANSACTION_START:
                    status = STATUS_OK;
                    socket->transactionState = SOCKET_TRANSACTION_STATE_STARTED;
                    socket->transactionId = transactionId;
                    break;
                case SOCKET_TRANSACTION_START_END:
                    status = STATUS_OK;
                    // no change: even in case of error, atomic transaction is terminated (no error message to send)
                    break;
                case SOCKET_TRANSACTION_CONTINUE:
                case SOCKET_TRANSACTION_END:
                case SOCKET_TRANSACTION_END_ERROR:
                    break; // invalid state
                case SOCKET_TRANSACTION_SOCKET_ERROR:
                    status = STATUS_OK;
                    break; // no state update: atomic transaction is terminated (no error message to send)
            }
            break;
        case SOCKET_TRANSACTION_STATE_STARTED:
            switch(transactionEvent){
                case SOCKET_TRANSACTION_START:
                case SOCKET_TRANSACTION_START_END:
                    break; // invalid state
                case SOCKET_TRANSACTION_CONTINUE:
                    if(socket->transactionId == transactionId){
                        status = STATUS_OK;
                        // no state change change
                    }
                    break;
                case SOCKET_TRANSACTION_END:
                case SOCKET_TRANSACTION_END_ERROR:
                    if(socket->transactionId == transactionId){
                        status = STATUS_OK;
                        socket->transactionState = SOCKET_TRANSACTION_STATE_NONE;
                        socket->transactionId = 0;
                    }
                    break;
                case SOCKET_TRANSACTION_SOCKET_ERROR:
                    if(socket->transactionId == transactionId){
                        status = STATUS_OK;
                        socket->transactionState = SOCKET_TRANSACTION_STATE_ERROR;
                    }
                    break; // invalid state
            }
            break;
        case SOCKET_TRANSACTION_STATE_ERROR:
            switch(transactionEvent){
                case SOCKET_TRANSACTION_START:
                case SOCKET_TRANSACTION_START_END:
                case SOCKET_TRANSACTION_CONTINUE:
                case SOCKET_TRANSACTION_END:
                    break; // invalid state
                case SOCKET_TRANSACTION_END_ERROR:
                    if(socket->transactionId == transactionId){
                        status = STATUS_OK;
                        socket->transactionState = SOCKET_TRANSACTION_STATE_NONE;
                        socket->transactionId = 0;
                    }
                    break;
                case SOCKET_TRANSACTION_SOCKET_ERROR:
                    status = STATUS_OK;
                    // no change
                    break;
            }
            break;
    }
    return status;
}

void SOPC_Socket_TreatWriteActions(SOPC_ActionQueue* queue){
    SOPC_StatusCode writeQueueStatus = STATUS_OK;
    SOPC_StatusCode status = STATUS_OK;
    uint8_t writeBlocked = FALSE;
    SOPC_ActionFunction* nullFct = NULL;
    void* arg = NULL;
    const char* txt = NULL;
    SOPC_Action_WriteParameter* param = NULL;
    uint32_t sentBytes = 0;

    while(STATUS_OK == status && STATUS_OK == writeQueueStatus && writeBlocked == FALSE){
        writeQueueStatus = SOPC_Action_NonBlockingDequeue(queue, &nullFct, &arg, &txt);
        param = (SOPC_Action_WriteParameter*) arg;
        if(STATUS_OK == writeQueueStatus && param->socket->sock != SOPC_INVALID_SOCKET){
            if(param->socket->sock != SOPC_INVALID_SOCKET){
                SOPC_StatusCode transactionStatus = SOPC_Socket_CheckAndUpdateTransaction(param->socket,
                                                                                          param->transactionEvent,
                                                                                          param->transactionId);
                if(STATUS_OK != transactionStatus){
                    // ignore write operation
                    free(param);
                }else{
                    sentBytes = 0;
                    status = Socket_Write(param->socket->sock, param->data, param->count, &sentBytes);
                    if(STATUS_OK == status){
                        if(NULL != param->endWriteCb){
                            // Ignore status ? Or call error (but should be same callback...)
                            param->endWriteCb(param->endWriteCbData, status);
                        }
                        free(param);
                    }else if(status == OpcUa_BadWouldBlock){
                        writeBlocked = 1;
                    }else{
                        transactionStatus = SOPC_Socket_CheckAndUpdateTransaction(param->socket,
                                                                                  SOCKET_TRANSACTION_SOCKET_ERROR,
                                                                                  param->transactionId);
                        if(NULL != param->endWriteCb){
                            // Ignore status ? Or call error (but should be same callback...)
                            param->endWriteCb(param->endWriteCbData, status);
                        }
                        free(param->data);
                        free(param);
                    }
                }
            }else{
                status = STATUS_INVALID_STATE;
            }
        }
    }

    if(writeBlocked != FALSE){
        SOPC_ActionFunction* unusedFct = NULL;
        void* unusedArg = NULL;
        // TODO: manage this case in the future ? (it means a blocking write occurred when recovering from precedent)
        // => It is necessary to replace current action as next to be done (i.e. reorganize the queue to do so)
        assert(SOPC_Action_NonBlockingDequeue(queue, &unusedFct, &unusedArg, &txt) == OpcUa_BadWouldBlock);
        param->count = param->count - sentBytes;
        param->posIdx = param->posIdx + sentBytes;
        param->socket->isNotWritable = 1;
        writeQueueStatus = SOPC_Action_BlockingEnqueue(queue, NULL, (void*) param, NULL);
    }else{
        assert(status != STATUS_OK || writeQueueStatus == OpcUa_BadWouldBlock);
        status = STATUS_OK;
    }
    // TODO: if status != STATUS_OK => SOPC_CreatAction_SocketError (transport event CB with error)
}

void SOPC_Action_SocketWrite(void* arg){
    SOPC_Action_WriteParameter* param = (SOPC_Action_WriteParameter*) arg;
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    SOPC_ActionQueue* writeQueue = param->socket->writeQueue;
    if(param->isOnlyWriteEvent == FALSE){
        // Store data to write
        status = SOPC_Action_BlockingEnqueue(writeQueue, NULL, arg, NULL);
    }else if(param->socket->isNotWritable == FALSE){
        // It is only an event to indicate new attempt to write data
        status = STATUS_OK;
        param->socket->isNotWritable = 1;
        // No data to keep in param, and will not be freed in TreatWriteActions
        free(param);
    }
    if(STATUS_OK == status && param->socket->isNotWritable == FALSE){
        SOPC_Socket_TreatWriteActions(writeQueue);
    }
    return;
}

SOPC_StatusCode SOPC_CreateAction_SocketWrite(SOPC_Socket*                  socket,
                                              uint8_t*                      data,
                                              uint32_t                      count,
                                              SOPC_Socket_Transaction_Event transactionEvent,
                                              uint32_t                      transactionId,
                                              SOPC_Socket_EndOperation_CB*  endWriteCb,
                                              void*                         endWriteCbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_WriteParameter* param = NULL;
    if(NULL != socket && data != NULL && count > 0){
        status = STATUS_NOK;
        param = malloc(sizeof(SOPC_Action_WriteParameter));
        if(NULL != param){
            status = STATUS_OK;
            param->socket = socket;
            param->isOnlyWriteEvent = FALSE;
            param->count = count;
            param->posIdx = 0;
            param->transactionEvent = transactionEvent;
            param->transactionId = transactionId;
            param->endWriteCb = endWriteCb;
            param->endWriteCbData = endWriteCbData;
        }
        // Copy buffer to be written
        if(STATUS_OK == status){
            param->data = data;
        }
        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Action_SocketWrite,
                                                       param,
                                                       "Write action");
        }
    }
    return status;
}

// To be used when socket write blocked precedently and socket in in write set on select
SOPC_StatusCode SOPC_CreateAction_SocketWriteEvent(SOPC_Socket* socket){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_WriteParameter* param = NULL;
    if(NULL != socket){
        // Socket must be in a non writable state due to precedent attempt to write
        if(socket->isNotWritable == 1)
        {
            param = malloc(sizeof(SOPC_Action_WriteParameter));
            if(NULL != param){
                status = STATUS_OK;
                memset(param, 0, sizeof(SOPC_Action_WriteParameter));
                param->socket = socket;
                param->isOnlyWriteEvent = 1;
            }
        }else{
            status = STATUS_INVALID_STATE;
        }
        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Action_SocketWrite,
                                                       param,
                                                       "Write event action");
        }
    }
    return status;
}

void SOPC_Action_SocketEvent(void* arg){
    SOPC_Action_SocketEventParameter* param = (SOPC_Action_SocketEventParameter*) arg;
    if(param != NULL){
        if(param->callback != NULL &&
           param->socket != NULL &&
           param->socket->sock != SOPC_INVALID_SOCKET)
        {
            param->callback(param->socket,
                            param->event,
                            param->callbackData);
        }
        free(param);
    }
    return;
}

// To be used when socket write blocked precedently and socket in in write set on select
SOPC_StatusCode SOPC_CreateAction_SocketEvent(SOPC_Socket_EventCB* callback,
                                              SOPC_Socket*         socket,
                                              SOPC_Socket_Event    event,
                                              void*                callbackData)
{
    // TODO ? : callback can be static, only 2 differents.
    // 1 for connection socket / 1 for listener socket (only accept event managed)
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_SocketEventParameter* param = NULL;
    if(NULL != callback && NULL != socket){
        status = STATUS_NOK;
        param = malloc(sizeof(SOPC_Action_SocketEventParameter));
        if(NULL != param){
            status = STATUS_OK;
            param->callback = callback;
            param->socket = socket;
            param->event = event;
            param->callbackData = callbackData;
        }
        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Action_SocketEvent,
                                                       param,
                                                       "Socket event");
        }
    }
    return status;
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
        SOPC_ActionQueue_Free(&socket->writeQueue);
    }
}

