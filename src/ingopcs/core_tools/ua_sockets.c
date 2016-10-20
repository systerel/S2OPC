/*
 * ua_sockets.c
 *
 *  Created on: Oct 20, 2016
 *      Author: vincent
 */

#include "ua_sockets.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>

UA_SocketManager globalSocketMgr;

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
    StatusCode status = STATUS_INVALID_PARAMETERS;
    // TODO: set lower limit for nbSockets: INT32_MAX just to ensure select returns value <= INT32_MAX
    if(socketMgr != NULL && socketMgr->nbSockets == 0 && nbSockets <= INT32_MAX){
        socketMgr->sockets = malloc(sizeof(UA_Socket) * nbSockets);
        if(socketMgr->sockets != NULL){
            status = STATUS_OK;
            memset(socketMgr->sockets, 0, sizeof(UA_Socket) * nbSockets);
            socketMgr->nbSockets = nbSockets;
        }
    }
    return status;
}

void UA_SocketManager_Clear(UA_SocketManager* socketMgr){
    if(socketMgr != NULL &&
       socketMgr->nbSockets > 0 &&
       socketMgr->sockets != NULL){
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
    int true = 1;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    struct addrinfo hints, *res, *p;
    UA_Socket* freeSocket;
    int addrStatus;
    int connectStatus = -1;
    int setOptStatus = -1;
    char *hostname = NULL;
    char *port = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;

    if(socketManager != NULL && uri != NULL && clientSocket != NULL){
        status = ParseURI(uri, &hostname, &port);
        if(status == STATUS_OK){
            freeSocket = GetFreeSocket(socketManager);
            if(freeSocket == NULL){
                status = STATUS_NOK;
            }
        }

        if (status == STATUS_OK && (addrStatus = getaddrinfo(hostname, port, &hints, &res)) != 0) {
            //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrStatus));
            status = STATUS_NOK;
        }

        if(status == STATUS_OK){
//            void *addr;
//            char *ipver;
//            char ipstr[INET6_ADDRSTRLEN];
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            for(p = res;p != NULL && connectStatus < 0; p = p->ai_next) {
//                if (p->ai_family == AF_INET) { // IPv4
//                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
//                    addr = &(ipv4->sin_addr);
//                    ipver = "IPv4";
//                } else { // IPv6
//                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
//                    addr = &(ipv6->sin6_addr);
//                    ipver = "IPv6";
//                }
//
//                // convert the IP to a string and print it:
//                inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
//                printf("  %s: %s\n", ipver, ipstr);

                freeSocket->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (freeSocket->sock != -1)
                {

                    setOptStatus = setsockopt(freeSocket->sock, IPPROTO_TCP, TCP_NODELAY, &true, sizeof(int));

                    //TODO: Set sock opt: buffers sizes

                    if(setOptStatus != -1){
                        setOptStatus = fcntl(freeSocket->sock, F_SETFL, O_NONBLOCK);
                    }

                    if(setOptStatus != -1){
                        connectStatus = connect(freeSocket->sock, p->ai_addr, sizeof(struct sockaddr));
                        if(connectStatus < 0){
                            if(errno == EINPROGRESS){
                                // Non blocking connection started
                                connectStatus = 0;
                            }
                        }
                        if(connectStatus == 0){
                            freeSocket->state = SOCKET_CONNECTING;
                        }
                    }
                }

                if(freeSocket->sock != -1 && connectStatus < 0){
                    close(freeSocket->sock);
                    freeSocket->sock = 0;
                }
            }
            if(connectStatus < 0){
                status = STATUS_NOK;
            }
            if(port != NULL){
                free(port);
            }
            if(hostname != NULL){
                free(hostname);
            }
        }
    }
    if(status == STATUS_OK){
        freeSocket->isUsed = 1;
        freeSocket->eventCallback = socketCallback;
        freeSocket->cbData = callbackData;
        *clientSocket = freeSocket;
    }

    return status;
}

StatusCode UA_SocketManager_CreateServerSocket(UA_SocketManager*  socketManager,
                                               const char*        uri,
                                               uint8_t            listenAllItfs,
                                               UA_Socket_EventCB  socketCallback,
                                               void*              callbackData,
                                               UA_Socket**        listenerSocket){
    int true = 1;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    struct addrinfo hints, *res, *p;
    UA_Socket* freeSocket;
    int addrStatus;
    int bindListenStatus = -1;
    int setOptStatus = -1;
    char *hostname = NULL;
    char *port = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(socketManager != NULL && uri != NULL && listenerSocket != NULL){
        status = ParseURI(uri, &hostname, &port);
        if(status == STATUS_OK){
            freeSocket = GetFreeSocket(socketManager);
            if(freeSocket == NULL){
                status = STATUS_NOK;
            }
        }
        if(status == STATUS_OK){
            if(listenAllItfs != 0){
                free(hostname);
                hostname = NULL;
            }
            if ((addrStatus = getaddrinfo(NULL, port, &hints, &res)) != 0) {
                //fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
                status = STATUS_NOK;
            }
        }
        if(status == STATUS_OK){
            for(p = res;p != NULL && bindListenStatus < 0; p = p->ai_next) {

                freeSocket->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (freeSocket->sock != -1)
                {
                    setOptStatus = setsockopt(freeSocket->sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

                    if (setOptStatus != -1) {
                        setOptStatus = setsockopt(freeSocket->sock, IPPROTO_TCP, TCP_NODELAY, &true, sizeof(int));
                    }
                    //TODO: Set sock opt: buffers sizes

                    if(setOptStatus != -1){
                        setOptStatus = fcntl(freeSocket->sock, F_SETFL, O_NONBLOCK);
                    }

                    if(setOptStatus != -1){
                        bindListenStatus = bind(freeSocket->sock, p->ai_addr, p->ai_addrlen);
                    }

                    if(bindListenStatus != -1){
                        bindListenStatus = listen(freeSocket->sock, SOMAXCONN);
                    }

                    if(bindListenStatus < 0){
                        close(freeSocket->sock);
                        freeSocket->sock = 0;
                    }
                }
            }
            if(bindListenStatus < 0){
                status = STATUS_NOK;
            }else{
                freeSocket->isUsed = 1;
                freeSocket->state = SOCKET_LISTENING;
                freeSocket->eventCallback = socketCallback;
                freeSocket->cbData = callbackData;
                *listenerSocket = freeSocket;
            }
        }
        if(port != NULL){
            free(port);
        }
        if(hostname != NULL){
            free(hostname);
        }
    }
    return status;
}

StatusCode SocketManager_Loop(UA_SocketManager* socketManager,
                              uint32_t          msecTimeout,
                              uint8_t           runOnce){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    int32_t nbReady = 0;
    UA_Socket* uaSock = NULL;
    UA_Socket* acceptSock = NULL;
    UA_Socket_EventCB*  callback = NULL;
    fd_set read_fds, write_fds, except_fds;
    struct sockaddr remoteaddr;
    socklen_t addrlen = 0;
    //char remoteIP[INET6_ADDRSTRLEN];
    struct timeval timeout;
    int fdmax = 0;
    int error = 0;
    socklen_t len = 0;

    timeout.tv_sec = (msecTimeout / 1000);
    timeout.tv_usec = 1000 * (msecTimeout % 1000);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    if(socketManager != NULL && runOnce != FALSE){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){

        for(idx = 0; idx < socketManager->nbSockets; idx++){
            uaSock = &(socketManager->sockets[idx]);
            if(uaSock->isUsed != FALSE){
               if(uaSock->state == SOCKET_CONNECTING){
                   FD_SET(uaSock->sock, &write_fds);
               }else{
                   FD_SET(uaSock->sock, &read_fds);
               }
               FD_SET(uaSock->sock, &except_fds);
               if(uaSock->sock > fdmax){
                   fdmax = uaSock->sock;
               }
            }
        }

        // Returns number of ready descriptor or -1 in case of error
        nbReady = select(fdmax+1, &read_fds, &write_fds, &except_fds, &timeout);
        if(nbReady < 0){
            status =  STATUS_NOK;
        }else if(nbReady > 0){
            for(idx = 0; idx < socketManager->nbSockets; idx++){
                uaSock = &(socketManager->sockets[idx]);
                if(uaSock->isUsed != FALSE){
                    callback = (UA_Socket_EventCB*) uaSock->eventCallback;
                    if(uaSock->state == SOCKET_CONNECTING){
                        if(FD_ISSET(uaSock->sock, &write_fds)){
                            // Check connection erros: mandatory when non blocking connection
                            len = sizeof(int);
                            if (getsockopt(uaSock->sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
                                error != 0){
                                callback(uaSock,
                                         SOCKET_CLOSE_EVENT,
                                         uaSock->cbData,
                                         0,
                                         0);
                                close(uaSock->sock);
                                uaSock->sock = 0;
                                uaSock->isUsed = FALSE;
                                uaSock->state = SOCKET_DISCONNECTED;
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
                        if(FD_ISSET(uaSock->sock, &read_fds)){
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
                                    acceptSock->sock = accept(uaSock->sock,
                                                              &remoteaddr,
                                                              &addrlen);
                                    if(acceptSock->sock != -1){
//                                        printf("selectserver: new connection from %s on socket %d\n",
//                                                inet_ntop(remoteaddr.sa_family,
//                                                    get_in_addr((struct sockaddr*)&remoteaddr),
//                                                    remoteIP, INET6_ADDRSTRLEN),
//                                                acceptSock);
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

                    if(FD_ISSET(uaSock->sock, &except_fds)){
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
    uint32_t sentBytes = 0;
    int res = 0;
    if(socket != NULL &&
       data != NULL &&
       count <= INT32_MAX)
    {
        // TODO: Use write event to write later ?
        while(res >=0 && sentBytes < count){
            if(res != 0){
                //TODO: sleep
            }
            res = send(socket->sock, data, count, 0);
            if(res > 0){
                sentBytes += res;
            }
        }
        return sentBytes;
    }else{
        return -1;
    }
}

StatusCode UA_Socket_Read (UA_Socket* socket,
                           uint8_t*   data,
                           uint32_t   dataSize,
                           uint32_t*  readCount){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(socket != NULL && data != NULL && dataSize > 0){
        *readCount = recv(socket->sock, data, dataSize, 0);
        if(*readCount > 0){
            status = STATUS_OK;
        }else if(*readCount == 0){
            status = STATUS_NOK;//OpcUa_BadDisconnect;
        }else{
            status = STATUS_NOK;
            //TODO: OpcUa_BadWouldBlock, etc.
        }
    }
    return status;
}

void UA_Socket_Close(UA_Socket* socket){
    close(socket->sock);
}

