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

#include "sopc_raw_sockets.h"

#include "opcua_statuscodes.h"
#include "sopc_threads.h"


static WSADATA wsaData;

SOPC_StatusCode Socket_Network_Initialize(){
    SOPC_StatusCode status = STATUS_OK;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(result != 0){
        status = STATUS_NOK;
    }
    return status;
}

SOPC_StatusCode Socket_Network_Clear(){
    SOPC_StatusCode status = STATUS_OK;
    int result = WSACleanup();
    if(result != 0){
        status = STATUS_NOK;
    }
    return status;
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

uint8_t Socket_AddrInfo_IsIPV6(Socket_AddressInfo* addr){
    return addr->ai_family == AF_INET6;
}

void Socket_AddrInfoDelete(Socket_AddressInfo** addrs){
    if(addrs != NULL){
        freeaddrinfo(*addrs);
        *addrs = NULL;
    }
}

void Socket_Clear(Socket* sock){
    *sock = SOPC_INVALID_SOCKET;
}

SOPC_StatusCode Socket_Configure(Socket  sock,
                                 bool    setNonBlocking)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    u_long ltrue = 1;
    int setOptStatus = -1;
    if(sock != SOPC_INVALID_SOCKET){
        status = STATUS_OK;
        // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
        setOptStatus = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*) &trueInt, sizeof(int));

        /*
        if(setOptStatus != SOCKET_ERROR){
            int rcvbufsize = UINT16_MAX;
            setOptStatus = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &rcvbufsize, sizeof(int));
        }

        if(setOptStatus != SOCKET_ERROR){
            int sndbufsize = UINT16_MAX;
            setOptStatus = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &sndbufsize, sizeof(int));
        }
        */

        if(setOptStatus != SOCKET_ERROR && setNonBlocking != false){
            setOptStatus = ioctlsocket(sock, FIONBIO, &ltrue); // True => Non blocking
        }

        if(setOptStatus == SOCKET_ERROR){
            status = STATUS_NOK;
        }
    }

    return status;
}

SOPC_StatusCode Socket_CreateNew(Socket_AddressInfo* addr,
                                 bool                setReuseAddr,
                                 bool                setNonBlocking,
                                 Socket*             sock){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    int setOptStatus = SOCKET_ERROR;
    if(addr != NULL && sock != NULL){
        status = STATUS_OK;
        *sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(*sock != SOPC_INVALID_SOCKET){

            status = Socket_Configure(*sock, setNonBlocking);

            if(status == STATUS_OK){
                setOptStatus = 0;
            } // else SOCKET_ERROR due to init

            if(setOptStatus != SOCKET_ERROR && setReuseAddr != false){
                setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
            }

            // Enforce IPV6 sockets can be used for IPV4 connections (if socket is IPV6)
            if(setOptStatus != SOCKET_ERROR && addr->ai_family == AF_INET6){
                const int falseInt = false;
                setOptStatus = setsockopt(*sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*) &falseInt, sizeof(int));
            }
        }
        if(setOptStatus == SOCKET_ERROR){
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
        bindListenStatus = bind(sock, addr->ai_addr, (int) addr->ai_addrlen);
        if(bindListenStatus != SOCKET_ERROR){
            bindListenStatus = listen(sock, SOMAXCONN);
        }
    }
    if(bindListenStatus != SOCKET_ERROR){
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode Socket_Accept(Socket  listeningSock,
                              bool    setNonBlocking,
                              Socket* acceptedSock)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(listeningSock != SOPC_INVALID_SOCKET && acceptedSock != NULL){
        *acceptedSock = accept(listeningSock, NULL, NULL);
        if(*acceptedSock != SOPC_INVALID_SOCKET){
            status = Socket_Configure(*acceptedSock, setNonBlocking);
        }
    }
    return status;
}

SOPC_StatusCode Socket_Connect(Socket              sock,
                               Socket_AddressInfo* addr)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int connectStatus = -1;
    int wsaError = 0;
    if(addr != NULL && sock != SOPC_INVALID_SOCKET){
        connectStatus = connect(sock, addr->ai_addr, (int) addr->ai_addrlen);
        if(connectStatus == SOCKET_ERROR){
        	wsaError = WSAGetLastError();
            if(wsaError == WSAEWOULDBLOCK){
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
    if(sock != SOPC_INVALID_SOCKET){
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*) &error, &len) < 0 ||
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
    if(sock != SOPC_INVALID_SOCKET && sockSet != NULL){
        FD_SET(sock, sockSet);
    }
}

bool SocketSet_IsPresent(Socket     sock,
                         SocketSet* sockSet)
{
    if(sock != SOPC_INVALID_SOCKET && sockSet != NULL){
        if(false == FD_ISSET(sock, sockSet)){
            return false;
        }else{
            return true;
        }
    }
    return false;
}

void SocketSet_Clear(SocketSet* sockSet)
{
    if(sockSet != NULL){
        FD_ZERO(sockSet);
    }
}


int32_t Socket_WaitSocketEvents(SocketSet* readSet,
                                SocketSet* writeSet,
                                SocketSet* exceptSet,
                                uint32_t   waitMs)
{
    int32_t nbReady = 0;
    const int ignored = 0; // ignored in winsocks
    struct timeval timeout;
    struct timeval* val;

    if(waitMs == 0){
        val = NULL;
    }else{
        timeout.tv_sec = (waitMs / 1000);
        timeout.tv_usec = 1000 * (waitMs % 1000);
        val = &timeout;
    }
    nbReady = select(ignored, readSet, writeSet, exceptSet, val);
    if(nbReady == SOCKET_ERROR || nbReady > INT32_MAX || nbReady < INT32_MIN)
        return -1;
    return (int32_t) nbReady;
}

SOPC_StatusCode Socket_Write(Socket    sock,
                             uint8_t*  data,
                             uint32_t  count,
                             uint32_t* sentBytes)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int res = 0;
    int wsaError = 0;
    if(sock != SOPC_INVALID_SOCKET &&
            data != NULL &&
            count <= INT32_MAX &&
            sentBytes != NULL)
    {
        status = STATUS_NOK;
        res = send(sock, (char*) data, count, 0);
        *sentBytes = (uint32_t) res;
        if(res == SOCKET_ERROR){
            wsaError = WSAGetLastError();
            // ERROR CASE
            if(wsaError == WSAEWOULDBLOCK){
                // Try again in those cases
                status = OpcUa_BadWouldBlock;
            } // keep STATUS_NOK
        }else if(res >= 0 && (uint32_t) res == count){
            status = STATUS_OK;
        } //else STATUS_NOK
    }
    return status;
}


SOPC_StatusCode Socket_Read(Socket     sock,
                            uint8_t*   data,
                            uint32_t   dataSize,
                            int32_t*   readCount)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
        if(sock != SOPC_INVALID_SOCKET && data != NULL && dataSize > 0){
            *readCount = recv(sock, (char*) data, dataSize, 0);
            if(*readCount > 0){
                status = STATUS_OK;
            }else if(*readCount == 0){
                status = OpcUa_BadDisconnect;
            }else if(*readCount == -1){
                if(WSAGetLastError() == WSAEWOULDBLOCK){
                    status = OpcUa_BadWouldBlock;
                }
            }else{
                status = STATUS_NOK;
            }
        }
        return status;
}

void Socket_Close(Socket*  sock)
{
    if(sock != NULL && *sock != SOPC_INVALID_SOCKET){
        if(closesocket(*sock) != SOCKET_ERROR){
            *sock = -1;
        }
    }
}
