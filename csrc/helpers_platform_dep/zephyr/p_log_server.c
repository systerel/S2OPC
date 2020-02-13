/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "kernel.h"
#include "net/socket.h"

/* s2opc includes */

#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_time.h"

/* platform dep includes */

#include "p_log_server.h"
#include "p_sockets.h"

/* Max pending connection based on max pending connections allowed by zephyr */

#ifdef CONFIG_NET_SOCKETS_POLL_MAX
#define MAX_LOG_SRV_PENDING_CONNECTION CONFIG_NET_SOCKETS_POLL_MAX
#else
#define MAX_LOG_SRV_PENDING_CONNECTION 4
#endif

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_LOG_SRV_CLIENTS_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_LOG_SRV_CLIENTS_SOCKET 4
#endif

/* log server configuration */

#define LOGSRV_CONFIG_MAX_DATA_CHANNEL 4096  /*Max data*/
#define LOGSRV_CONFIG_MAX_EVENT_CHANNEL 4096 /*Max events */
#define LOGSRV_CONFIG_MAX_LOG_SRV 1          /*Max clients*/
#define LOGSRV_CONFIG_PERIOD_MS (5)          /*Poll log mem file and send to connected clients*/

#define LOGSRV_TIMESTAMP_SIZE 32
#define LOGSRV_MAX_CLIENTS MAX_LOG_SRV_CLIENTS_SOCKET

#define LOGSRV_PERIOD_US (LOGSRV_CONFIG_PERIOD_MS * 1000)

// Server socket status
typedef enum E_LOG_SERVER_STATUS
{
    E_LOG_SRV_BINDING,
    E_LOG_SRV_ONLINE
} eLogServerStatus;

// Server instance status
typedef enum E_LOG_SERVER_SYNC_STATUS
{
    E_LOG_SRV_SYNC_NOT_INITIALIZED,
    E_LOG_SRV_SYNC_INITIALIZING,
    E_LOG_SRV_SYNC_DEINITIALIZING,
    E_LOG_SRV_SYNC_INITIALIZED,
    E_LOG_SRV_SYNC_SIZE = INT32_MAX,
} eLogSrvSyncStatus;

// Log event
typedef struct T_LOG_SERVER_CHANNEL_EVT
{
    uint32_t size;
    uint32_t offset;
} tLogChannelEvt;

// Log file
typedef struct T_LOG_SERVER_CHANNEL
{
    tLogChannelEvt event[LOGSRV_CONFIG_MAX_EVENT_CHANNEL];
    uint8_t data[LOGSRV_CONFIG_MAX_DATA_CHANNEL * 2];
    uint32_t evtWr;
    uint32_t dataWr;
    uint32_t nbData;
    uint32_t nbEvts;
    char lastUTCTimeStamp[LOGSRV_TIMESTAMP_SIZE];
} tLogChannel;

// Log server
typedef struct T_LOG_SERVER
{
    volatile eLogServerStatus status;
    volatile bool bQuit;
    zsock_fd_set readfs;
    zsock_fd_set workfs;
    int32_t sockLogServer;
    int32_t maxSock;
    int32_t socketLogClt[LOGSRV_MAX_CLIENTS];
    tLogChannel logChannel;
    Mutex lockLogChannel;
    uint32_t logCltRdIdx[LOGSRV_MAX_CLIENTS];

    uint16_t port;
    Thread threadMonitor;
} tLogServer;

// Log server instance
typedef struct T_LOG_SERVER_HANDLE
{
    volatile eLogSrvSyncStatus status;
    tLogServer* pLogServer;
} tLogServerHanlde;

// Log server instances tables
static tLogServerHanlde gLogSrvHandles[LOGSRV_CONFIG_MAX_LOG_SRV];

// *** Private internal functions declaration ***

static inline void P_LOGSRV_SOCKET_SetBlocking(int socket, bool blockMode);
static inline void P_LOGSRV_destroy_server_socket(tLogServer* pLogSrv);
static inline int32_t P_LOGSRV_create_server_socket(tLogServer* pLogSrv);
static inline uint32_t P_LOGSRV_accept_client_connection(tLogServer* pLogSrv);
static inline void P_LOGSRV_close_client_connection(tLogServer* pLogSrv, uint32_t indexClient);

static inline void P_LOGSRV_LOGCHANNEL_Push(tLogChannel* pCh, const uint8_t* buffer, uint32_t size, bool bIncludeDate);

static inline SOPC_ReturnStatus P_LOGSRV_Print(tLogServer* p, const uint8_t* buffer, uint32_t size, bool bIncludeDate);
static inline void P_LOGSRV_Destroy(tLogServer** p);
static inline tLogServer* P_LOGSRV_Create(uint32_t port);

static inline void P_LOGSRV_SYNC_STATUS_set_quit_flag(SOPC_LogServer_Handle handle);
static inline eLogSrvSyncStatus P_LOGSRV_SYNC_STATUS_increment_in_use(SOPC_LogServer_Handle handle);
static inline eLogSrvSyncStatus P_LOGSRV_SYNC_STATUS_decrement_in_use(SOPC_LogServer_Handle handle);

// *** Private internal functions definitions ***

// Set socket blocking mode
static inline void P_LOGSRV_SOCKET_SetBlocking(int socket, bool blockMode)
{
    int fl = -1;
    int res = -1;
    fl = fcntl(socket, F_GETFL, 0);
    assert(-1 != fl);
    if (blockMode)
    {
        fl &= ~O_NONBLOCK;
    }
    else
    {
        fl |= O_NONBLOCK;
    }

    res = fcntl(socket, F_SETFL, fl);
    assert(-1 != res);
}

// Close server socket
static inline void P_LOGSRV_destroy_server_socket(tLogServer* pLogSrv)
{
    // Check socket server value
    if (NULL != pLogSrv && SOPC_INVALID_SOCKET != pLogSrv->sockLogServer)
    {
        // Remove socket from server file descriptors
        FD_CLR(pLogSrv->sockLogServer, &pLogSrv->readfs);
        // Shutdown and close connection
        zsock_shutdown(pLogSrv->sockLogServer, ZSOCK_SHUT_RDWR);
        zsock_close(pLogSrv->sockLogServer);
        // Mark socket as invalid
        pLogSrv->sockLogServer = SOPC_INVALID_SOCKET;
        // Indicate to others applications one more free socket
        P_SOCKET_decrement_nb_sockets();
    }
}

// Close client connection
static inline void P_LOGSRV_close_client_connection(tLogServer* pLogSrv, uint32_t indexClient)
{
    if (NULL != pLogSrv && indexClient < LOGSRV_MAX_CLIENTS &&
        SOPC_INVALID_SOCKET != pLogSrv->socketLogClt[indexClient])
    {
        // Remove socket from server file descriptors
        FD_CLR(pLogSrv->socketLogClt[indexClient], &pLogSrv->readfs);
        // Shutdown and close connection
        shutdown(pLogSrv->socketLogClt[indexClient], ZSOCK_SHUT_RDWR);
        close(pLogSrv->socketLogClt[indexClient]);
        // Mark socket as invalid
        pLogSrv->socketLogClt[indexClient] = SOPC_INVALID_SOCKET;
        // Indicate to others applications one more free socket
        P_SOCKET_decrement_nb_sockets();
    }
}

// Create server socket, bind and listen
// Returns: 0 if successes, < 0 if fails
static inline int32_t P_LOGSRV_create_server_socket(tLogServer* pLogSrv)
{
    int32_t socketResult = 0;
    // Indicates to others application that one socket is needed
    uint32_t valueNbSocket = P_SOCKET_increment_nb_sockets();
    // If result is beyond limit, return error
    if (valueNbSocket > MAX_LOG_SRV_CLIENTS_SOCKET)
    {
        P_SOCKET_decrement_nb_sockets();
        socketResult = -1;
    }

    // If createion is allowed
    if (0 == socketResult)
    {
        // Create socket
        pLogSrv->sockLogServer = zsock_socket(AF_INET, SOCK_STREAM, 0);
        if (pLogSrv->sockLogServer >= 0)
        {
            // Set socket option addr reusable
            int32_t opt = 0;
            socketResult = zsock_setsockopt(pLogSrv->sockLogServer, //
                                            SOL_SOCKET,             //
                                            SO_REUSEADDR,           //
                                            &opt,                   //
                                            sizeof(opt));           //
            if (0 == socketResult)
            {
                // Bind all interfaces
                struct sockaddr_in sin = {
                    .sin_family = AF_INET,                    //
                    .sin_port = htons(pLogSrv->port),         //
                    .sin_addr = {.s_addr = htonl(INADDR_ANY)} //
                };

                socketResult = zsock_bind(pLogSrv->sockLogServer,      //
                                          (struct sockaddr*) &sin,     //
                                          sizeof(struct sockaddr_in)); //

                // Set non blocking mode
                if (0 == socketResult)
                {
                    P_LOGSRV_SOCKET_SetBlocking(pLogSrv->sockLogServer, false);
                    socketResult = zsock_listen(pLogSrv->sockLogServer, MAX_LOG_SRV_PENDING_CONNECTION);
                }
            }
        } // socket creation error
        else
        {
            //  Indicate to others applications one more free socket
            P_SOCKET_decrement_nb_sockets();
            socketResult = -1;
        }
    }

    // Socket creation, bind and listen successfully,
    // add to file descriptor used by select
    if (0 == socketResult)
    {
        ZSOCK_FD_SET(pLogSrv->sockLogServer, &pLogSrv->readfs);
        if (pLogSrv->sockLogServer >= pLogSrv->maxSock)
        {
            pLogSrv->maxSock = pLogSrv->sockLogServer + 1;
        }
    }
    else // Binding or creation error, destroy socket if created
    {
        // If socket has been created, destroy it
        if (pLogSrv->sockLogServer >= 0)
        {
            P_LOGSRV_destroy_server_socket(pLogSrv);
        }
    }

    return socketResult;
}

// Accept new client connection
static inline uint32_t P_LOGSRV_accept_client_connection(tLogServer* pLogSrv)
{
    int32_t socketResult = 0;
    uint32_t valueNbSockets = P_SOCKET_increment_nb_sockets();

    if (valueNbSockets > MAX_LOG_SRV_CLIENTS_SOCKET) // If limit is reached, accept and destroy
    {
        int32_t newSocket = -1;
        struct sockaddr_in sin;
        socklen_t sockaddr_size = sizeof(struct sockaddr_in);

        do
        {
            // Beyond MAX_SOCKET, only MAX_SOCKET + 1 can be accepted.
            // If beyond, yield then retry
            valueNbSockets = P_SOCKET_increment_nb_sockets();
            if ((MAX_LOG_SRV_CLIENTS_SOCKET + 2) >= valueNbSockets)
            {
                memset(&sin, 0, sizeof(struct sockaddr_in));
                newSocket = zsock_accept(pLogSrv->sockLogServer,  //
                                         (struct sockaddr*) &sin, //
                                         &sockaddr_size);         //

                if (newSocket >= 0)
                {
                    zsock_shutdown(newSocket, ZSOCK_SHUT_RDWR);
                    zsock_close(newSocket);
                    newSocket = -1;
                }
                P_SOCKET_decrement_nb_sockets();
            }
            else
            {
                P_SOCKET_decrement_nb_sockets();
                k_yield();
            }
            // While some process are processing this function
        } while (valueNbSockets > (MAX_LOG_SRV_CLIENTS_SOCKET + 2));

        // Indicates one more free socket
        P_SOCKET_decrement_nb_sockets();

        socketResult = -1;
    }
    else
    {
        // Accept new connection
        // then refuse it if out of memory (workaround zephyr)
        int32_t newSocket = -1;
        struct sockaddr_in sin;
        socklen_t sockaddr_size = sizeof(struct sockaddr_in);

        memset(&sin, 0, sizeof(struct sockaddr_in));
        newSocket = zsock_accept(pLogSrv->sockLogServer,  //
                                 (struct sockaddr*) &sin, //
                                 &sockaddr_size);         //

        // If connection valid, search empty server slot to register it.
        // Then realign event pointer for this client on oldest log event
        // in order to list an historic at the connection.
        if (newSocket < 0)
        {
            // Indicate that system socket not used
            P_SOCKET_decrement_nb_sockets();
            socketResult = -1;
        }
        else
        {
            bool bSlotFound = false;
            uint32_t indexNewClient = 0;
            for (uint32_t i = 0; i < LOGSRV_MAX_CLIENTS; i++)
            {
                // Check if empty slot
                if (SOPC_INVALID_SOCKET == pLogSrv->socketLogClt[i])
                {
                    // Register socket
                    pLogSrv->socketLogClt[i] = newSocket;

                    // Mark slot as well found
                    bSlotFound = true;
                    indexNewClient = i;
                    break;
                }
            }

            // If slot found, add to server read file descriptor
            // used by select function and set this socket as not blocking
            if (!bSlotFound)
            {
                zsock_shutdown(newSocket, ZSOCK_SHUT_RDWR);
                zsock_close(newSocket);
                newSocket = -1;

                // Indicate that system socket not used
                P_SOCKET_decrement_nb_sockets();
                socketResult = -1;
            }
            else
            {
                ZSOCK_FD_SET(pLogSrv->socketLogClt[indexNewClient], &pLogSrv->readfs);
                if (pLogSrv->socketLogClt[indexNewClient] >= pLogSrv->maxSock)
                {
                    pLogSrv->maxSock = pLogSrv->socketLogClt[indexNewClient] + 1;
                }

                return indexNewClient;
            } // Slot found
        }     // Socket accept with error
    }         // Not enough memory
    return UINT32_MAX;
}

// Write log to file
static inline void P_LOGSRV_LOGCHANNEL_Push(tLogChannel* pCh, const uint8_t* buffer, uint32_t size, bool bIncludeDate)
{
    if (size + LOGSRV_TIMESTAMP_SIZE < LOGSRV_CONFIG_MAX_DATA_CHANNEL)
    {
        uint32_t nextSize = 0;
        uint32_t nextWrite = pCh->evtWr;

        // Erase log oldest record if necessary
        while (((pCh->nbEvts + 1) > LOGSRV_CONFIG_MAX_EVENT_CHANNEL) ||
               ((pCh->nbData + size + LOGSRV_TIMESTAMP_SIZE) > LOGSRV_CONFIG_MAX_DATA_CHANNEL))
        {
            nextSize = pCh->event[nextWrite].size;
            if (nextSize > 0)
            {
                pCh->event[nextWrite].size = 0;
                pCh->event[nextWrite].offset = 0;
                pCh->nbData -= nextSize;
                pCh->nbEvts--;
            }
            nextWrite = (nextWrite + 1) % LOGSRV_CONFIG_MAX_EVENT_CHANNEL;
        }

        uint32_t sizeLocalTime = 0;
        if (bIncludeDate)
        {
            // Get timestamp string
            int64_t dt_100ns = SOPC_Time_GetCurrentTimeUTC();
            time_t seconds = 0;
            struct tm tm;

            SOPC_ReturnStatus resTime = SOPC_Time_ToTimeT(dt_100ns, &seconds);
            if (SOPC_STATUS_OK == resTime)
            {
                resTime = SOPC_Time_Breakdown_UTC(seconds, &tm);
            }

            if (SOPC_STATUS_OK == resTime)
            {
                sizeLocalTime = strftime(pCh->lastUTCTimeStamp, LOGSRV_TIMESTAMP_SIZE - 1, "%Y/%m/%d %H:%M:%S", &tm);
            }

            sizeLocalTime += snprintf(pCh->lastUTCTimeStamp + sizeLocalTime, LOGSRV_TIMESTAMP_SIZE - sizeLocalTime - 1,
                                      ".%03u ", (uint32_t)((dt_100ns / 10000) % 1000));
        }
        // Add new record info
        pCh->event[pCh->evtWr].size = sizeLocalTime + size;
        pCh->event[pCh->evtWr].offset = pCh->dataWr;

        pCh->evtWr = (pCh->evtWr + 1) % LOGSRV_CONFIG_MAX_EVENT_CHANNEL;
        pCh->nbEvts++;

        // Write record data
        for (uint32_t i = 0; i < sizeLocalTime; i++)
        {
            pCh->data[pCh->dataWr] = pCh->lastUTCTimeStamp[i];
            if (pCh->dataWr >= LOGSRV_CONFIG_MAX_DATA_CHANNEL)
            {
                pCh->data[pCh->dataWr % LOGSRV_CONFIG_MAX_DATA_CHANNEL] = pCh->lastUTCTimeStamp[i];
            }
            pCh->dataWr++;
        }

        for (uint32_t i = 0; i < size; i++)
        {
            pCh->data[pCh->dataWr] = buffer[i];
            if (pCh->dataWr >= LOGSRV_CONFIG_MAX_DATA_CHANNEL)
            {
                pCh->data[pCh->dataWr % LOGSRV_CONFIG_MAX_DATA_CHANNEL] = buffer[i];
            }
            pCh->dataWr++;
        }
        pCh->dataWr = pCh->dataWr % LOGSRV_CONFIG_MAX_DATA_CHANNEL;
        pCh->nbData += (size + sizeLocalTime);
    }
}

// Send data to client
static inline void P_LOGSRV_SendDataToClient(tLogServer* pLogSrv, uint32_t instanceClient)
{
    // Get read index associated with this socket
    uint32_t clientReadIndex = 0;
    clientReadIndex = pLogSrv->logCltRdIdx[instanceClient];

    int32_t socketByteSent = 0;

    // Check if at least one event exist. Zero size event can't be writed.
    if (pLogSrv->logChannel.nbEvts > 0)
    {
        // While diff between writer and reader and if no error during write to socket
        while (pLogSrv->logChannel.evtWr != clientReadIndex && socketByteSent >= 0)
        {
            // Get data size and offset
            uint32_t dataSize = pLogSrv->logChannel.event[clientReadIndex].size;
            uint32_t offset = pLogSrv->logChannel.event[clientReadIndex].offset;

            // Send data on socket
            if (dataSize > 0)
            {
                // Write socket operation in sync mode
                P_LOGSRV_SOCKET_SetBlocking(pLogSrv->socketLogClt[instanceClient], true);

                socketByteSent = 0;
                for (uint32_t j = 0; j < dataSize; j += socketByteSent)
                {
                    socketByteSent = send(pLogSrv->socketLogClt[instanceClient], //
                                          &pLogSrv->logChannel.data[offset + j], //
                                          dataSize - j,                          //
                                          0);                                    //

                    // If error during socket write, close client socket and remove
                    // from socket descriptor list to monitor
                    if (socketByteSent < 0)
                    {
                        P_LOGSRV_close_client_connection(pLogSrv, instanceClient);
                        break;
                    }
                }

                if (pLogSrv->socketLogClt[instanceClient] != SOPC_INVALID_SOCKET)
                {
                    P_LOGSRV_SOCKET_SetBlocking(pLogSrv->socketLogClt[instanceClient], false);
                }
            }

            // Update read index
            if (socketByteSent >= 0)
            {
                pLogSrv->logCltRdIdx[instanceClient] =
                    (pLogSrv->logCltRdIdx[instanceClient] + 1) % LOGSRV_CONFIG_MAX_EVENT_CHANNEL;

                clientReadIndex = pLogSrv->logCltRdIdx[instanceClient];
            }
        } // While no error and event log
    }     // At least one event
}

// Log server thread callback
static void* P_LOGSRV_ThreadMonitorCallback(void* pCtx)
{
    tLogServer* pLogSrv = (tLogServer*) pCtx;

    // Select timeout
    struct timeval timeout = {.tv_sec = 0, .tv_usec = LOGSRV_PERIOD_US};

    // Used to reajust timeout
    uint64_t timestamp = 0;
    uint32_t delta = 0;

    ZSOCK_FD_ZERO(&pLogSrv->readfs);
    ZSOCK_FD_ZERO(&pLogSrv->workfs);

    while (false == pLogSrv->bQuit)
    {
        switch (pLogSrv->status)
        {
        case E_LOG_SRV_BINDING:
        {
            int32_t socketResult = 0;
            socketResult = P_LOGSRV_create_server_socket(pLogSrv);
            if (socketResult < 0)
            {
                pLogSrv->status = E_LOG_SRV_BINDING;
                k_sleep(LOGSRV_CONFIG_PERIOD_MS);
            }
            else
            {
                timeout.tv_sec = 0;
                timeout.tv_usec = LOGSRV_PERIOD_US;
                timestamp = (uint64_t) k_uptime_get();

                pLogSrv->status = E_LOG_SRV_ONLINE;
            }
        }
        break;
        case E_LOG_SRV_ONLINE:
        {
            pLogSrv->workfs = pLogSrv->readfs;

            int32_t socketResult = 0;
            socketResult = select(pLogSrv->maxSock, &pLogSrv->workfs, NULL, NULL, &timeout);

            if (socketResult < 0)
            {
                if (pLogSrv->sockLogServer >= 0)
                {
                    P_LOGSRV_destroy_server_socket(pLogSrv);
                }

                for (uint32_t i = 0; i < LOGSRV_MAX_CLIENTS; i++)
                {
                    if (pLogSrv->socketLogClt[i] >= 0)
                    {
                        P_LOGSRV_close_client_connection(pLogSrv, i);
                    }
                }

                pLogSrv->status = E_LOG_SRV_BINDING;
            }
            else
            {
                if (0 == socketResult || delta >= LOGSRV_CONFIG_PERIOD_MS)
                {
                    // Periodic zone ------------------------------------------------
                    timestamp = (uint64_t) k_uptime_get();
                    timeout.tv_sec = 0;
                    timeout.tv_usec = (LOGSRV_PERIOD_US);
                    delta = 0;

                    // Parse socket list accepted by this server.
                    for (uint32_t i = 0; i < LOGSRV_MAX_CLIENTS; i++)
                    {
                        if (pLogSrv->socketLogClt[i] != -1)
                        {
                            // Enter log channel critical section
                            Mutex_Lock(&pLogSrv->lockLogChannel);
                            // Send data
                            P_LOGSRV_SendDataToClient(pLogSrv, i);
                            // Exit log channel critical section
                            Mutex_Unlock(&pLogSrv->lockLogChannel);
                        } // valid socket
                    }

                    // Compute delta spent by event log treatment for each client
                    // then reajust select timeout parameter

                    uint64_t timestamp_temp = timestamp;
                    delta = k_uptime_delta_32(&timestamp_temp);

                    if (delta <= LOGSRV_CONFIG_PERIOD_MS)
                    {
                        timeout.tv_sec = 0;
                        timeout.tv_usec = (LOGSRV_PERIOD_US - delta * 1000);
                    }
                }
                else
                {
                    // Event zone ------------------------------------------------
                    // Parse all existing sockets
                    for (int32_t iterSocket = 0; iterSocket <= pLogSrv->maxSock; iterSocket++)
                    {
                        // If socket has been registered by this server
                        if (FD_ISSET(iterSocket, &pLogSrv->workfs) != 0)
                        {
                            // If socket is server socket
                            if (iterSocket == pLogSrv->sockLogServer)
                            {
                                uint32_t indexNewClient = P_LOGSRV_accept_client_connection(pLogSrv);

                                // If indexNewClient >= MAX_CLIENTS, connection has not been accepted
                                if (indexNewClient < LOGSRV_MAX_CLIENTS)
                                {
                                    // Send data log historic
                                    // Log channel critical section
                                    Mutex_Lock(&pLogSrv->lockLogChannel);

                                    // Set index on potential oldest log
                                    uint32_t index = (pLogSrv->logChannel.evtWr + LOGSRV_CONFIG_MAX_EVENT_CHANNEL -
                                                      pLogSrv->logChannel.nbEvts) %
                                                     LOGSRV_CONFIG_MAX_EVENT_CHANNEL;

                                    if (LOGSRV_CONFIG_MAX_EVENT_CHANNEL == pLogSrv->logChannel.nbEvts)
                                    {
                                        index = (index + 1) % LOGSRV_CONFIG_MAX_EVENT_CHANNEL;
                                    }

                                    pLogSrv->logCltRdIdx[indexNewClient] = index;

                                    P_LOGSRV_SendDataToClient(pLogSrv, indexNewClient);

                                    Mutex_Unlock(&pLogSrv->lockLogChannel);
                                }
                            }
                            else
                            {
                                uint8_t buffer[0];
                                // Verify disconnection event
                                socketResult = recv(iterSocket, buffer, 1, 0);
                                if (socketResult <= 0)
                                {
                                    // If disconnection event, search into list,
                                    // remove from monitoring list then close and mark it as invalid
                                    for (uint32_t indexClient = 0; indexClient < LOGSRV_MAX_CLIENTS; indexClient++)
                                    {
                                        if (pLogSrv->socketLogClt[indexClient] == iterSocket)
                                        {
                                            P_LOGSRV_close_client_connection(pLogSrv, indexClient);
                                            break;
                                        }
                                    } // Parsing clients
                                }     // Client deconnexion
                            }         // Server or client socket
                        }             // Event - socket identification
                    }                 // Parse all kernel sockets

                    // Verify time elapsed from last periodic
                    uint64_t timestamp_temp = timestamp;
                    delta = k_uptime_delta_32(&timestamp_temp);

                    if (delta <= LOGSRV_CONFIG_PERIOD_MS)
                    {
                        timeout.tv_sec = 0;
                        timeout.tv_usec = (LOGSRV_PERIOD_US - delta * 1000);
                    }
                } // Event part
            }     // Select result
        }

        break;
        default:
            break;
        }

    } // While bQuit == false

    // Quit flag is set, close all active connections
    for (uint32_t indexClient = 0; indexClient < LOGSRV_MAX_CLIENTS; indexClient++)
    {
        if (pLogSrv->socketLogClt[indexClient] >= 0)
        {
            P_LOGSRV_close_client_connection(pLogSrv, indexClient);
        }
    }

    if (pLogSrv->sockLogServer >= 0)
    {
        P_LOGSRV_destroy_server_socket(pLogSrv);
    }

    return NULL;
}

//*** Internal function used by SOPC API ***

// Thread safe print log
static inline SOPC_ReturnStatus P_LOGSRV_Print(tLogServer* p, const uint8_t* buffer, uint32_t size, bool bIncludeDate)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == p)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    result = Mutex_Lock(&p->lockLogChannel);
    if (SOPC_STATUS_OK == result)
    {
        P_LOGSRV_LOGCHANNEL_Push(&p->logChannel, buffer, size, bIncludeDate);
        Mutex_Unlock(&p->lockLogChannel);
    }
    return result;
}

// Wait for ending log server thread,
// clear print log critical section
// and free log server instance
static inline void P_LOGSRV_Destroy(tLogServer** p)
{
    if (p != NULL)
    {
        tLogServer* pLogSrv = *p;
        if (pLogSrv != NULL)
        {
            pLogSrv->bQuit = true;
            SOPC_Thread_Join(pLogSrv->threadMonitor);
            pLogSrv->threadMonitor = (Thread) NULL;

            Mutex_Clear(&pLogSrv->lockLogChannel);
            SOPC_Free(pLogSrv);
            *p = NULL;
        }
    }
}

// Create log server instance and launch it
static inline tLogServer* P_LOGSRV_Create(uint32_t port)
{
    if (port < 60 || port > 120)
    {
        return NULL;
    }

    SOPC_Socket_Network_Initialize();

    tLogServer* pLogSrv = (tLogServer*) SOPC_Calloc(1, sizeof(tLogServer));

    if (NULL == pLogSrv)
    {
        return NULL;
    }

    // Set to invalid socket value all sockets
    pLogSrv->sockLogServer = SOPC_INVALID_SOCKET;

    for (uint32_t i = 0; i < LOGSRV_MAX_CLIENTS; i++)
    {
        pLogSrv->socketLogClt[i] = SOPC_INVALID_SOCKET;
    }

    // Clear file descriptor used by select socket function
    ZSOCK_FD_ZERO(&pLogSrv->readfs);
    ZSOCK_FD_ZERO(&pLogSrv->workfs);

    // Initialize critical section
    SOPC_ReturnStatus result = Mutex_Initialization(&pLogSrv->lockLogChannel);

    // Launch server thread
    if (SOPC_STATUS_OK == result)
    {
        pLogSrv->port = port;
        pLogSrv->status = E_LOG_SRV_BINDING;
        pLogSrv->bQuit = false;
        result = SOPC_Thread_Create(&pLogSrv->threadMonitor, (void*) P_LOGSRV_ThreadMonitorCallback, pLogSrv, NULL);
    }

    if (result != SOPC_STATUS_OK)
    {
        Mutex_Clear(&pLogSrv->lockLogChannel);
        SOPC_Free(pLogSrv);
        pLogSrv = NULL;
    }

    return pLogSrv;
}

// Used by public API, next call to SOPC_LogServer_Print will return error
static inline void P_LOGSRV_SYNC_STATUS_set_quit_flag(SOPC_LogServer_Handle handle)
{
    if (handle < LOGSRV_CONFIG_MAX_LOG_SRV)
    {
        tLogServerHanlde* pHandle = &gLogSrvHandles[handle];
        bool bTransition = false;
        eLogSrvSyncStatus currentStatus = 0;
        eLogSrvSyncStatus newStatus = 0;
        do
        {
            currentStatus = pHandle->status;
            newStatus = currentStatus | 0x80000000;
            bTransition = __sync_bool_compare_and_swap(&pHandle->status, currentStatus, newStatus);

        } while (!bTransition);
    }
    return;
}

// Used by public API, increment in use count if > INITIALIZED sync status
static inline eLogSrvSyncStatus P_LOGSRV_SYNC_STATUS_increment_in_use(SOPC_LogServer_Handle handle)
{
    eLogSrvSyncStatus currentStatus = 0;
    eLogSrvSyncStatus newStatus = 0;
    if (handle < LOGSRV_CONFIG_MAX_LOG_SRV)
    {
        tLogServerHanlde* pHandle = &gLogSrvHandles[handle];
        bool bTransition = false;

        do
        {
            currentStatus = pHandle->status;

            if ((currentStatus & (~0x80000000)) >= E_LOG_SRV_SYNC_INITIALIZED)
            {
                newStatus = currentStatus + 1;
            }
            else
            {
                newStatus = currentStatus;
            }

            bTransition = __sync_bool_compare_and_swap(&pHandle->status, currentStatus, newStatus);

        } while (!bTransition);
    }
    return newStatus;
}

// Used by public API, decrement in use count if > INITIALIZED sync status
static inline eLogSrvSyncStatus P_LOGSRV_SYNC_STATUS_decrement_in_use(SOPC_LogServer_Handle handle)
{
    eLogSrvSyncStatus currentStatus = 0;
    eLogSrvSyncStatus newStatus = 0;
    if (handle < LOGSRV_CONFIG_MAX_LOG_SRV)
    {
        tLogServerHanlde* pHandle = &gLogSrvHandles[handle];
        bool bTransition = false;

        do
        {
            // Load current status
            currentStatus = pHandle->status;

            if ((currentStatus & (~0x80000000)) > E_LOG_SRV_SYNC_INITIALIZED)
            {
                newStatus = currentStatus - 1;
            }
            else
            {
                newStatus = currentStatus;
            }

            bTransition = __sync_bool_compare_and_swap(&pHandle->status, currentStatus, newStatus);

        } while (!bTransition);
    }
    return newStatus;
}

// *** Public log server API ***

// Creation of new log server
SOPC_ReturnStatus SOPC_LogServer_Create(SOPC_LogServer_Handle* pHandle, // Returned log server instance handle
                                        uint32_t port)                  // TCP port between 60 and 120
{
    if (NULL == pHandle)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_LogServer_Handle handle = SOPC_LOGSRV_INVALID_HANDLE;

    eLogSrvSyncStatus status = E_LOG_SRV_SYNC_NOT_INITIALIZED;

    for (uint32_t i = 0; i < LOGSRV_CONFIG_MAX_LOG_SRV && SOPC_LOGSRV_INVALID_HANDLE == handle; i++)
    {
        status = __sync_val_compare_and_swap(&gLogSrvHandles[i].status,      //
                                             E_LOG_SRV_SYNC_NOT_INITIALIZED, //
                                             E_LOG_SRV_SYNC_INITIALIZING);   //

        if (E_LOG_SRV_SYNC_NOT_INITIALIZED == status)
        {
            tLogServer* pLogSrv = P_LOGSRV_Create(port);
            if (pLogSrv != NULL)
            {
                gLogSrvHandles[i].pLogServer = pLogSrv;
                handle = i;
                gLogSrvHandles[i].status = E_LOG_SRV_SYNC_INITIALIZED;
                result = SOPC_STATUS_OK;
            }
            else
            {
                gLogSrvHandles[i].status = E_LOG_SRV_SYNC_NOT_INITIALIZED;
                result = SOPC_STATUS_NOK;
            }
        }
    }

    *pHandle = handle;

    return result;
}

// Destruction of log server. Handle is set to invalid handle value
SOPC_ReturnStatus SOPC_LogServer_Destroy(SOPC_LogServer_Handle* pHandle)
{
    if (NULL == pHandle)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_LogServer_Handle handle = *pHandle;

    if (handle >= LOGSRV_CONFIG_MAX_LOG_SRV)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    eLogSrvSyncStatus fromStatus = E_LOG_SRV_SYNC_NOT_INITIALIZED;

    do
    {
        P_LOGSRV_SYNC_STATUS_set_quit_flag(handle);
        fromStatus = __sync_val_compare_and_swap(&gLogSrvHandles[handle].status,          //
                                                 E_LOG_SRV_SYNC_INITIALIZED | 0x80000000, //
                                                 E_LOG_SRV_SYNC_DEINITIALIZING);          //

        if (E_LOG_SRV_SYNC_INITIALIZED == (fromStatus & (~0x80000000)))
        {
            P_LOGSRV_Destroy(&gLogSrvHandles[handle].pLogServer);
            gLogSrvHandles[handle].status = E_LOG_SRV_SYNC_NOT_INITIALIZED;

            result = SOPC_STATUS_OK;
        }
        else if (E_LOG_SRV_SYNC_DEINITIALIZING == (fromStatus & (~0x80000000)) ||
                 E_LOG_SRV_SYNC_INITIALIZING == (fromStatus & (~0x80000000)) ||
                 (fromStatus & (~0x80000000)) > E_LOG_SRV_SYNC_INITIALIZED)
        {
            result = SOPC_STATUS_INVALID_STATE;
            k_yield();
        }
        else
        {
            result = SOPC_STATUS_NOK;
        }

    } while (SOPC_STATUS_INVALID_STATE == result);

    *pHandle = SOPC_LOGSRV_INVALID_HANDLE;

    return result;
}

// Log server print.
SOPC_ReturnStatus SOPC_LogServer_Print(SOPC_LogServer_Handle handle, // Server instance handle
                                       const uint8_t* value,         // Data to log
                                       uint32_t size,
                                       bool bIncludeDate) // Data size
{
    if (handle >= LOGSRV_CONFIG_MAX_LOG_SRV || NULL == value || 0 == size || size > LOGSRV_CONFIG_MAX_DATA_CHANNEL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    eLogSrvSyncStatus status = P_LOGSRV_SYNC_STATUS_increment_in_use(handle);

    if ((status & (~0x80000000)) > E_LOG_SRV_SYNC_INITIALIZED)
    {
        if (0x80000000 == (status & 0x80000000))
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            tLogServer* pLogSrv = gLogSrvHandles[handle].pLogServer;
            result = P_LOGSRV_Print(pLogSrv, value, size, bIncludeDate);
        }
        P_LOGSRV_SYNC_STATUS_decrement_in_use(handle);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}
