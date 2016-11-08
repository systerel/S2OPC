/*
 * p_sockets.c
 *
 *  Created on: Oct 26, 2016
 *      Author: vincent
 */

#include "p_sockets.h"
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <ua_stack_csts.h>

SOPC_StatusCode Socket_Network_Initialize(){
    return STATUS_OK;
}

SOPC_StatusCode Socket_Network_Clear(){
    return STATUS_OK;
}

SOPC_StatusCode Socket_AddrInfo_Get(char* hostname, char* port, Socket_AddressInfo** addrs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(Socket_AddressInfo));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if(port != NULL && addrs != NULL){
        if(getaddrinfo(hostname, port, &hints, addrs) != 0){
            status = STATUS_NOK;
        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

Socket_AddressInfo* Socket_AddrInfo_IterNext(Socket_AddressInfo* addr){
    Socket_AddressInfo* res = NULL;
    if(addr != NULL){
        res = addr->ai_next;
    }
    return res;
}

void Socket_AddrInfoDelete(Socket_AddressInfo** addrs){
    freeaddrinfo(*addrs);
    *addrs = NULL;
}

void Socket_Clear(Socket* sock){
    *sock = -1;
}

SOPC_StatusCode Socket_CreateNew(Socket_AddressInfo* addr,
                            uint8_t             setReuseAddr,
                            uint8_t             setNonBlocking,
                            Socket*             sock){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int true = 1;
    int setOptStatus = -1;
    if(addr != NULL && sock != NULL){
        status = STATUS_OK;
        *sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(*sock != -1){
            // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
            setOptStatus = setsockopt(*sock, IPPROTO_TCP, TCP_NODELAY, &true, sizeof(int));

            if(setOptStatus != -1){
                int rcvbufsize = OPCUA_P_TCPRCVBUFFERSIZE;
                setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_RCVBUF, &rcvbufsize, sizeof(int));
            }

            if(setOptStatus != -1){
                int sndbufsize = OPCUA_P_TCPSNDBUFFERSIZE;
                setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_SNDBUF, &sndbufsize, sizeof(int));
            }

            if(setOptStatus != -1 && setReuseAddr != FALSE){
                setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
            }

            if(setOptStatus != -1 && setNonBlocking != FALSE){
                setOptStatus = fcntl(*sock, F_SETFL, O_NONBLOCK);
            }
        }
        if(setOptStatus < 0){
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Socket_Listen(Socket              sock,
                         Socket_AddressInfo* addr)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int bindListenStatus = -1;
    if(addr != NULL){
        bindListenStatus = bind(sock, addr->ai_addr, addr->ai_addrlen);
        if(bindListenStatus != -1){
            bindListenStatus = listen(sock, SOMAXCONN);
        }
    }
    if(bindListenStatus != -1){
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Socket_Accept(Socket  listeningSock,
                         Socket* acceptedSock)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    struct sockaddr remoteAddr;
    socklen_t addrLen = 0;
    if(listeningSock != -1 && acceptedSock != NULL){
        *acceptedSock = accept(listeningSock,
                               &remoteAddr,
                               &addrLen);
//        printf("selectserver: new connection from %s on socket %d\n",
//                inet_ntop(remoteaddr.sa_family,
//                    get_in_addr((struct sockaddr*)&remoteaddr),
//                    remoteIP, INET6_ADDRSTRLEN),
//                acceptSock);
        if(*acceptedSock != -1){
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode Socket_Connect(Socket              sock,
                          Socket_AddressInfo* addr)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int connectStatus = -1;
    if(addr != NULL && sock != -1){
        connectStatus = connect(sock, addr->ai_addr, sizeof(Socket_AddressInfo));
        if(connectStatus < 0){
            if(errno == EINPROGRESS){
                // Non blocking connection started
                connectStatus = 0;
            }
        }
        if(connectStatus == 0){
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode Socket_CheckAckConnect(Socket sock){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int error = 0;
    socklen_t len = sizeof(int);
    if(sock != -1){
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
            error != 0)
        {
            status = STATUS_NOK;
        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

void SocketSet_Add(Socket     sock,
                   SocketSet* sockSet)
{
    if(sock != -1 && sockSet != NULL){
        FD_SET(sock, &sockSet->set);
        if(sock > sockSet->fdmax){
            sockSet->fdmax = sock;
        }
    }
}

int8_t SocketSet_IsPresent(Socket     sock,
                           SocketSet* sockSet)
{
    if(sock != -1 && sockSet != NULL){
        if(FD_ISSET(sock, &sockSet->set) == FALSE){
            return FALSE;
        }else{
            return 1;
        }
    }
    return FALSE;
}

void SocketSet_Clear(SocketSet* sockSet)
{
    if(sockSet != NULL){
        FD_ZERO(&sockSet->set);
        sockSet->fdmax = 0;
    }
}


int32_t Socket_WaitSocketEvents(SocketSet* readSet,
                                SocketSet* writeSet,
                                SocketSet* exceptSet,
                                uint32_t   waitMs)
{
    int32_t nbReady = 0;
    struct timeval timeout;
    struct timeval* val;
    int fdmax = 0;
    if(readSet->fdmax > writeSet->fdmax){
        fdmax = readSet->fdmax;
    }else{
        fdmax = writeSet->fdmax;
    }
    if(exceptSet->fdmax > fdmax){
        fdmax = exceptSet->fdmax;
    }

    if(waitMs == 0){
        val = NULL;
    }else{
        timeout.tv_sec = (waitMs / 1000);
        timeout.tv_usec = 1000 * (waitMs % 1000);
        val = &timeout;
    }
    nbReady = select(fdmax+1, &readSet->set, &writeSet->set, &exceptSet->set, val);
    if(nbReady > INT32_MAX || nbReady < INT32_MIN)
        return -1;
    return (int32_t) nbReady;
}

int32_t Socket_Write(Socket   sock,
                     uint8_t* data,
                     uint32_t count)
{
    uint32_t sentBytes = 0;
    int res = 0;
    if(sock != -1 &&
       data != NULL &&
       count <= INT32_MAX)
    {
        // TODO: Use write event to write later ?
        while(res >=0 && sentBytes < count){
            if(res != 0){
                usleep(50000);
            }
            res = send(sock, data, count, 0);
            if(res > 0){
                sentBytes += res;
            }
        }
        return sentBytes;
    }else{
        return -1;
    }
}


SOPC_StatusCode Socket_Read(Socket     sock,
                       uint8_t*   data,
                       uint32_t   dataSize,
                       uint32_t*  readCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
        if(sock != -1 && data != NULL && dataSize > 0){
            *readCount = recv(sock, data, dataSize, 0);
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

void Socket_Close(Socket*  sock)
{
    if(sock != NULL){
        if(close(*sock) != -1){
            *sock = -1;
        }
    }
}
