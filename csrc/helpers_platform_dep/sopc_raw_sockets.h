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

#ifndef SOPC_RAW_SOCKETS_H_
#define SOPC_RAW_SOCKETS_H_

#include <stdbool.h>
#include "sopc_toolkit_constants.h"

// Platform dependent types
#include "p_sockets.h"

/**
 *  \brief Initialize the network communication allowing to use sockets
 */
bool Socket_Network_Initialize(void);

/**
 *  \brief Clear the network communication when sockets not used anymore
 */
bool Socket_Network_Clear(void);

/**
 *  \brief Provide a linked list of socket addressing information for establishing TCP connections over IPV4 and IPV6
 *  using the hostname and/or port if provided.
 *
 *  \param hostname    The hostname of the machine to connect or used to listen (optional for listening)
 *  \param port        The port number on which to connect or to listen to
 *  \param addrs       The addressing information to establish or listen a TCP connection over IPV4 and IPV6
 *
 *  \return            GOOD if operation succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_AddrInfo_Get(char* hostname, char* port, Socket_AddressInfo** addrs);

/**
 *  \brief Given a socket addressing information element of a linked list,
 *   provides the next addressing information or NULL if no more are available.
 *
 *  \param addr    A socket addressing information element
 *
 *  \return        Next socket adressing information element or NULL if no more are present.
 */
Socket_AddressInfo* Socket_AddrInfo_IterNext(Socket_AddressInfo* addr);

/**
 *  \brief Given a socket addressing information element,
 *   returns 0 if address is not IPV6 and not 0 value otherwise.
 *
 *  \param addr    A socket addressing information element
 *
 *  \return        0 if address is not IPV6 and not 0 value otherwise.
 */
uint8_t Socket_AddrInfo_IsIPV6(Socket_AddressInfo* addr);

/**
 *  \brief Deallocate a linked list of socket addressing information.
 *
 *  \param addrs   (In/Out) A linked list of socket addressing information to deallocate. Pointer set to NULL after
 * operation.
 *
 */
void Socket_AddrInfoDelete(Socket_AddressInfo** addrs);

/**
 *  \brief Clear socket state to an invalid socket
 *
 *  \param sock              (Out) Value pointed is set to invalid socket value
 */
void Socket_Clear(Socket* sock);

/**
 *  \brief Create a new socket using the addressing information provided
 *
 *  \param addr              The addressing information used to instantiate a TCP/IP socket
 *  \param setReuseAddr      If value is not false (0) the socket is configured to could be reused
 *  \param setNonBlocking    If value is not false (0) the socket is configured to be non blocking
 *  \param sock              (Out) Value pointed is set with the newly created socket
 *
 *  \return                  GOOD if operation succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_CreateNew(Socket_AddressInfo* addr, bool setReuseAddr, bool setNonBlocking, Socket* sock);

/**
 *  \brief Configure the socket to listen connections using the given addressing information
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *  \param sock    The socket used for binding and listening
 *  \param addr    The addressing information used to bind the socket for listening (IP and port)
 *
 *  \return        GOOD if operation succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_Listen(Socket sock, Socket_AddressInfo* addr);

/**
 *  \brief Operation to accept a connection on a listening socket
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *
 *  \param listeningSock    The listening socket on which a read event has been received.
 *  \param setNonBlocking   If value is not false (0) the connection socket is configured to be non blocking
 *  \param acceptedSock     (Out) Value pointed is set with the newly created socket for accepted connection
 *
 *  \return        GOOD if operation succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_Accept(Socket listeningSock, bool setNonBlocking, Socket* acceptedSock);

/**
 *  \brief Operation to establish a connection using the given socket and addressing information
 *  Connection establishment result must be detected when receiving a read event on the socket and
 *  then by calling the Socket_CheckAckConnect operation
 *
 *  \param addr    The addressing information used to establish connection (IP and port)
 *  \param sock    The socket used for establishing the connection
 *
 *  \return        GOOD if operation succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_Connect(Socket sock, Socket_AddressInfo* addr);

/**
 *  \brief Operation to check connection establishment result on a connecting socket
 *  After using Socket_Connect on a socket and receiving a write event on the socket this operation returns
 *  the failure or success of the connection
 *
 *  \param sock    The socket on which the first read event has been received after calling Socket_Connect operation.
 *
 *  \return        GOOD if connection succeeded, BAD otherwise.
 */
SOPC_ReturnStatus Socket_CheckAckConnect(Socket sock);

/**
 *  \brief Add a socket to the given socket set
 *
 *  \param sock       The socket to add to the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SocketSet_Add(Socket sock, SocketSet* sockSet);

/**
 *  \brief Returns if a socket is present in the given socket set
 *
 *  \param sock       The socket to search in the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 *
 *  \return           true (!= false) if present, false otherwise
 */
bool SocketSet_IsPresent(Socket sock, SocketSet* sockSet);
/**
 *  \brief Clear a socket set
 *
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SocketSet_Clear(SocketSet* sockSet);

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
int32_t Socket_WaitSocketEvents(SocketSet* readSet, SocketSet* writeSet, SocketSet* exceptSet, uint32_t waitMs);

/**
 *  \brief Write data through the socket
 *
 *  \param sock      The socket on which data must be written
 *  \param data      The data bytes to write on socket
 *  \param count     The number of bytes to write
 *  \param sentBytes Pointer to the number of bytes sent on socket after call
 *
 *  \return          SOPC_STATUS_OK if all bytes were written,
 *                   SOPC_STATUS_WOULD_BLOCK if socket write operation would block,
 *                   SOPC_STATUS_NOK if it failed and
 */
SOPC_ReturnStatus Socket_Write(Socket sock, uint8_t* data, uint32_t count, uint32_t* sentBytes);

/**
 *  \brief Read data through the socket
 *
 *  \param sock         The socket on which data must be read
 *  \param data         The data bytes to be set with read bytes
 *  \param dataSize     The number of bytes that can be set (or expected to be read)
 *  \param readCount    The number of bytes actually read on the socket
 *
 *  \return         GOOD if operation succeeded, SOPC_STATUS_CLOSED in case of disconnection and SOPC_STATUS_NOK
 * otherwise.
 */
SOPC_ReturnStatus Socket_Read(Socket sock, uint8_t* data, uint32_t dataSize, int32_t* readCount);

/**
 *  \brief Close the socket connection and/or clear the socket
 *
 *  \param sock     The socket to disconnect and/or clear
 */
void Socket_Close(Socket* sock);

#endif /* SOPC_RAW_SOCKETS_H_ */
