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

/**
 *  \file
 *
 *  \brief A platform independent API to use sockets
 */

#ifndef SOPC_RAW_SOCKETS_H_
#define SOPC_RAW_SOCKETS_H_

#include <stdbool.h>
#include <stdint.h>
#include "sopc_enums.h"

/**
 * \brief Provides a socket implementation
 * \note Each platform must provide the implementation of \ref SOPC_Socket_Impl and all related functions
 * in \ref sopc_raw_sockets.h */
typedef struct SOPC_Socket_Impl SOPC_Socket_Impl;
typedef SOPC_Socket_Impl* SOPC_Socket;
#define SOPC_INVALID_SOCKET NULL

/**
 * \brief Provides a socket address info implementation
 * \note Each platform must provide the implementation of \ref SOPC_Socket_AddressInfo and all related functions
 * in \ref sopc_raw_sockets.h */
typedef struct SOPC_Socket_AddressInfo SOPC_Socket_AddressInfo;

/**
 * \brief Provides a socket address implementation
 * \note Each platform must provide the implementation of \ref SOPC_Socket_Address and all related functions
 * in \ref sopc_raw_sockets.h */
typedef struct SOPC_Socket_Address SOPC_Socket_Address;

/**
 * \brief Provides a socket set implementation
 * \note Each platform must provide the implementation of \ref SOPC_SocketSet and all related functions
 * in \ref sopc_raw_sockets.h */
typedef struct SOPC_SocketSet SOPC_SocketSet;

/**
 *  \brief Activate the keep alive mechanism. According to RFC1122, this mechanism shall be used with care:
 *      deactivated by default, ability to enable or disable it run time for each connection at runtime.
 *
 * \param sock      The socket to activate keep alive probes
 * \param firstProbeDelay      The time (in seconds) the connection needs to remain idle
 * before TCP starts sendingkeepalive probes
 * \param interval      The time (in seconds) between individual keepalive probes
 * \param counter      The maximum number of keepalive probes TCP should send
 * before dropping the connection
 *
 *  \return            SOPC_STATUS_OK if operation succeeded,
 *                     SOPC_INVALID_PARAMETERS if parameters are not valid
 *                     SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Network_Enable_Keepalive(SOPC_Socket sock,
                                                       unsigned int firstProbeDelay,
                                                       unsigned int interval,
                                                       unsigned int counter);

/**
 *  \brief Deactivate the keep alive mechanism.
 *
 * \param sock      The socket to activate keep alive probes
 *
 *  \return            SOPC_STATUS_OK if operation succeeded,
 *                     SOPC_INVALID_PARAMETERS if parameters are not valid
 *                     SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Network_Disable_Keepalive(SOPC_Socket sock);

/**
 *  \brief Initialize the network communication allowing to use sockets
 */
bool SOPC_Socket_Network_Initialize(void);

/**
 *  \brief Clear the network communication when sockets not used anymore
 */
bool SOPC_Socket_Network_Clear(void);

/**
 *  \brief Provide a linked list of socket addressing information for establishing TCP connections over IPV4 and IPV6
 *  using the hostname and/or port if provided.
 *
 *  \param hostname    The hostname of the machine to connect or used to listen (optional for listening)
 *  \param port        The port number on which to connect or to listen to (optional for listening if hostname set)
 *  \param addrs       The addressing information to establish or listen a TCP connection over IPV4 and IPV6
 *
 *  \return            SOPC_STATUS_OK if operation succeeded,
 *                     SOPC_INVALID_PARAMETERS if parameters are not valid
 *                     SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(const char* hostname, const char* port, SOPC_Socket_AddressInfo** addrs);

/**
 *  \brief Given a socket addressing information element of a linked list,
 *   provides the next addressing information or NULL if no more are available.
 *
 *  \param addr    A socket addressing information element
 *
 *  \return        Next socket addressing information element or NULL if no more are present.
 */
SOPC_Socket_AddressInfo* SOPC_Socket_AddrInfo_IterNext(SOPC_Socket_AddressInfo* addr);

/**
 *  \brief Given a socket addressing information element,
 *   returns 0 if address is not IPV6 and not 0 value otherwise.
 *
 *  \param addr    A socket addressing information element
 *
 *  \return        0 if address is not IPV6 and not 0 value otherwise.
 */
uint8_t SOPC_Socket_AddrInfo_IsIPV6(const SOPC_Socket_AddressInfo* addr);

/**
 *  \brief Deallocate a linked list of socket addressing information.
 *
 *  \param addrs   (In/Out) A linked list of socket addressing information to deallocate. Pointer set to NULL after
 * operation.
 *
 */
void SOPC_Socket_AddrInfoDelete(SOPC_Socket_AddressInfo** addrs);

/**
 *  \brief Given a socket get the address information associated to the peer connected to the socket
 *
 *  \param sock    The socket connected to a peer
 *
 *  \return        The newly allocated peer socket address or NULL in case of error.
 *                 Caller is responsible to free the result using ::SOPC_SocketAddress_Delete.
 */
SOPC_Socket_Address* SOPC_Socket_GetPeerAddress(SOPC_Socket sock);

/**
 * \brief Copy the socket address from a socket address information
 *
 * \param addr  The socket address information to copy into socket address structure
 *
 * \return      The newly allocated socket address created from the \p addr or NULL in case of failures.
 *              Caller is responsible to free the result using ::SOPC_SocketAddress_Delete.
 */
SOPC_Socket_Address* SOPC_Socket_CopyAddress(SOPC_Socket_AddressInfo* addr);

/**
 * \brief Get address information as C strings from the given socket address
 *
 * \param      addr    The socket address to use to get information as C string
 * \param[out] host    (optional) The host information of the address as C string (i.e. IP).
 *                     Must be deallocated by caller (if non-NULL)
 * \param[out] service (optional) The service information of the address as C string (i.e. port).
 *                     Must be deallocated by caller (if non-NULL)
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS if parameters are invalid (NULL for addr or both NULL for outputs)
 *         or SOPC_STATUS_NOK if it failed.
 */
SOPC_ReturnStatus SOPC_SocketAddress_GetNameInfo(const SOPC_Socket_Address* addr, char** host, char** service);

/**
 *  \brief Deallocate a socket address obtained with ::SOPC_Socket_GetPeerAddress or ::SOPC_Socket_CopyAddress
 *
 *  \param addr (In/Out) An address information to deallocate.
 *                       Pointer set to NULL after operation.
 *
 */
void SOPC_SocketAddress_Delete(SOPC_Socket_Address** addr);

/**
 *  \brief Clear socket state to an invalid socket
 *
 *  \param[out] sock  Value pointed is set to invalid socket value
 */
void SOPC_Socket_Clear(SOPC_Socket* sock);

/**
 *  \brief Create a new socket using the addressing information provided
 *
 *  \param addr              The addressing information used to instantiate a TCP/IP socket
 *  \param setReuseAddr      If value is not false (0) the socket is configured to be reused
 *  \param setNonBlocking    If value is not false (0) the socket is configured to be non blocking
 *  \param[out] sock         Value pointed is set with the newly created socket
 *
 *  \return                  SOPC_STATUS_OK if operation succeeded,
 *                           SOPC_INVALID_PARAMETERS if parameters are not valid
 *                           SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_CreateNew(SOPC_Socket_AddressInfo* addr,
                                        bool setReuseAddr,
                                        bool setNonBlocking,
                                        SOPC_Socket* sock);

/**
 *  \brief Configure the socket to listen connections using the given addressing information
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *  \param sock    The socket used for binding and listening
 *  \param addr    The addressing information used to bind the socket for listening (IP and port)
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_INVALID_PARAMETERS if parameters are not valid
 *                 SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Listen(SOPC_Socket sock, SOPC_Socket_AddressInfo* addr);

/**
 *  \brief Operation to accept a connection on a listening socket
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *
 *  \param listeningSock      The listening socket on which a read event has been received.
 *  \param setNonBlocking     If value is not false (0) the connection socket is configured to be non blocking
 *  \param[out] acceptedSock  Value pointed is set with the newly created socket for accepted connection
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_INVALID_PARAMETERS if parameters are not valid
 *                 SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Accept(SOPC_Socket listeningSock, bool setNonBlocking, SOPC_Socket* acceptedSock);

/**
 *  \brief Operation to establish a connection using the given socket and addressing information
 *  Connection establishment result must be detected when receiving a read event on the socket and
 *  then by calling the SOPC_Socket_CheckAckConnect operation
 *
 *  \param addr    The addressing information used to establish connection (IP and port)
 *  \param sock    The socket used for establishing the connection
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_INVALID_PARAMETERS if parameters are not valid
 *                 SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Connect(SOPC_Socket sock, SOPC_Socket_AddressInfo* addr);

/**
 * \brief Operation to establish a connection using the given socket to the given local socket.
 *  Both sockets shall have been created with same addressing information protocol.
 *  The address to connected to is extracted from the target socket.
 *
 *  \param from    The socket used for establishing the connection
 *  \param to      The socket to connect to
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_INVALID_PARAMETERS if parameters are not valid
 *                 SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_ConnectToLocal(SOPC_Socket from, SOPC_Socket to);

/**
 *  \brief Operation to check connection establishment result on a connecting socket
 *  After using SOPC_Socket_Connect on a socket and receiving a write event on the socket this operation returns
 *  the failure or success of the connection
 *
 *  \param sock    The socket on which the first read event has been received after calling SOPC_Socket_Connect
 * operation.
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_INVALID_PARAMETERS if parameters are not valid
 *                 SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_CheckAckConnect(SOPC_Socket sock);

/**
 *  \brief Create a socket set
 *
 *  \return A socket set or NULL in case of failure.
 */
SOPC_SocketSet* SOPC_SocketSet_Create(void);

/**
 *  \brief Delete the pointed socket set and set the pointer to NULL
 *
 */
void SOPC_SocketSet_Delete(SOPC_SocketSet** set);

/**
 *  \brief Add a socket to the given socket set
 *
 *  \param sock       The socket to add to the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SOPC_SocketSet_Add(SOPC_Socket sock, SOPC_SocketSet* sockSet);

/**
 *  \brief Returns if a socket is present in the given socket set
 *
 *  \param sock       The socket to search in the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 *
 *  \return           true (!= false) if present, false otherwise
 */
bool SOPC_SocketSet_IsPresent(SOPC_Socket sock, SOPC_SocketSet* sockSet);
/**
 *  \brief Clear a socket set
 *
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SOPC_SocketSet_Clear(SOPC_SocketSet* sockSet);

/**
 *  \brief Wait for events (read, write and exception) on the sockets in the given sets for a given duration.
 *  If events are received on sockets or waiting reached timeout it returns and the socket sets contain only the sockets
 * on which events occurred
 *
 *  \param readSet      (In/Out) The set of sockets on which read events are awaited and as a result the set of sockets
 * on which event occurred \param writeSet     (In/Out) The set of sockets on which write events are awaited and as a
 * result the set of sockets on which event occurred \param exceptSet    (In/Out) The set of sockets on which exception
 * events are awaited and as a result the set of sockets on which event occurred \param waitMs       The maximum
 * duration in milliseconds waiting for events (0 means infinite)
 *
 *  \return             The number of sockets with events contained by sets or -1 if failed
 */
int32_t SOPC_Socket_WaitSocketEvents(SOPC_SocketSet* readSet,
                                     SOPC_SocketSet* writeSet,
                                     SOPC_SocketSet* exceptSet,
                                     uint32_t waitMs);

/**
 *  \brief Write data through the socket
 *
 *  \param sock      The socket on which data must be written
 *  \param data      The data bytes to write on socket
 *  \param count     The number of bytes to write
 *  \param sentBytes Pointer to the number of bytes sent on socket after call
 *                   (only significant when SOPC_STATUS_OK returned)
 *
 *  \return          SOPC_STATUS_OK if bytes were written,
 *                   SOPC_STATUS_INVALID_PARAMETERS if parameters are not valid,
 *                   SOPC_STATUS_WOULD_BLOCK if socket write operation would block,
 *                   SOPC_STATUS_NOK if it failed
 */
SOPC_ReturnStatus SOPC_Socket_Write(SOPC_Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes);

/**
 *  \brief Read data through the socket
 *
 *  \param sock         The socket on which data must be read
 *  \param data         The data bytes to be set with read bytes
 *  \param dataSize     The number of bytes that can be set (or expected to be read)
 *  \param readCount    Pointer to the number of bytes actually written on the socket
 *                      (only significant when SOPC_STATUS_OK returned)
 *
 *  \return         SOPC_STATUS_OK if operation succeeded,
 *                  SOPC_STATUS_WOULD_BLOCK if socket read operation would block,
 *                  SOPC_STATUS_CLOSED in case of disconnection and
 *                  SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_Read(SOPC_Socket sock, uint8_t* data, uint32_t dataSize, uint32_t* readCount);

/**
 *  \brief Retrieve number of bytes available to read on the socket
 *
 *  \param sock              The socket on which data might be available to read
 *  \param[out] bytesToRead  Pointer to the number of bytes available to read on the socket.
 *                           To be considered only if returned status is SOPC_STATUS_OK
 *
 *  \return             SOPC_STATUS_OK if operation succeeded,
 *                      SOPC_STATUS_INVALID_PARAMETERS if parameters are not valid, SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Socket_BytesToRead(SOPC_Socket sock, uint32_t* bytesToRead);

/**
 *  \brief Close the socket connection and/or clear the socket
 *
 *  \param sock     The socket to disconnect and/or clear
 */
void SOPC_Socket_Close(SOPC_Socket* sock);

#endif /* SOPC_RAW_SOCKETS_H_ */
