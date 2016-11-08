/*
 *  \file p_sockets.h
 *
 *  \brief Platform independent socket interface with a platform dependent implementation.
 *
 *  Created on: Oct 25, 2016
 *      Author: VMO (Systerel)
 */

#ifndef INGOPCS_PLATFORMS_LINUX_P_SOCKETS_H_
#define INGOPCS_PLATFORMS_LINUX_P_SOCKETS_H_

#include <netdb.h>
#include <sys/select.h>

#include "../../core_types/sopc_base_types.h"

/**
 *  \brief Socket base type
 */
typedef int Socket;

/**
 *  \brief Socket addressing information for listening or connecting operation type
 */
typedef struct addrinfo Socket_AddressInfo;

/**
 *  \brief Set of sockets type
 */
typedef struct {
    int    fdmax; /**< max of the set */
    fd_set set;   /**< set */
} SocketSet;

/**
 *  \brief Initialize the network communication allowing to use sockets
 */
SOPC_StatusCode Socket_Network_Initialize(void);

/**
 *  \brief Clear the network communication when sockets not used anymore
 */
SOPC_StatusCode Socket_Network_Clear(void);

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
SOPC_StatusCode Socket_AddrInfo_Get(char* hostname, char* port, Socket_AddressInfo** addrs);

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
 *  \brief Deallocate a linked list of socket addressing information.
 *
 *  \param addrs   (In/Out) A linked list of socket addressing information to deallocate. Pointer set to NULL after operation.
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
 *  \param setReuseAddr      If value is not zero the socket is configured to could be reused
 *  \param setNonBlocking    If value is not zero the socket is configured to be non blocking
 *  \param sock              (Out) Value pointed is set with the newly created socket
 *
 *  \return                  GOOD if operation succeeded, BAD otherwise.
 */
SOPC_StatusCode Socket_CreateNew(Socket_AddressInfo* addr,
                            uint8_t             setReuseAddr,
                            uint8_t             setNonBlocking,
                            Socket*             sock);

/**
 *  \brief Configure the socket to listen connections using the given addressing information
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *  \param sock    The socket used for binding and listening
 *  \param addr    The addressing information used to bind the socket for listening (IP and port)
 *
 *  \return        GOOD if operation succeeded, BAD otherwise.
 */
SOPC_StatusCode Socket_Listen(Socket              sock,
                         Socket_AddressInfo* addr);

/**
 *  \brief Operation to accept a connection on a listening socket
 *  Connection on a listening socket is detected when receiving a read event on the socket.
 *
 *  \param listeningSock    The listening socket on which a read event has been received.
 *  \param acceptedSock     (Out) Value pointed is set with the newly created socket for accepted connection
 *
 *  \return        GOOD if operation succeeded, BAD otherwise.
 */
SOPC_StatusCode Socket_Accept(Socket  listeningSock,
                         Socket* acceptedSock);

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
SOPC_StatusCode Socket_Connect(Socket              sock,
                          Socket_AddressInfo* addr);

/**
 *  \brief Operation to check connection establishment result on a connecting socket
 *  After using Socket_Connect on a socket and receiving a write event on the socket this operation returns
 *  the failure or success of the connection
 *
 *  \param sock    The socket on which the first read event has been received after calling Socket_Connect operation.
 *
 *  \return        GOOD if connection succeeded, BAD otherwise.
 */
SOPC_StatusCode Socket_CheckAckConnect(Socket  sock);

/**
 *  \brief Add a socket to the given socket set
 *
 *  \param sock       The socket to add to the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SocketSet_Add(Socket     sock,
                   SocketSet* sockSet);

/**
 *  \brief Returns if a socket is present in the given socket set
 *
 *  \param sock       The socket to search in the set (not NULL)
 *  \param sockSet    The socket set to use for the operation (not NULL)
 *
 *  \return           Not zero if present, zero otherwise
 */
int8_t SocketSet_IsPresent(Socket     sock,
                           SocketSet* sockSet);
/**
 *  \brief Clear a socket set
 *
 *  \param sockSet    The socket set to use for the operation (not NULL)
 */
void SocketSet_Clear(SocketSet* sockSet);

/**
 *  \brief Wait for events (read, write and exception) on the sockets in the given sets for a given duration.
 *  If events are received on sockets or waiting reached timeout it returns and the socket sets contain only the sockets on which events occurred
 *
 *  \param readSet      (In/Out) The set of sockets on which read events are awaited and as a result the set of sockets on which event occurred
 *  \param writeSet     (In/Out) The set of sockets on which write events are awaited and as a result the set of sockets on which event occurred
 *  \param exceptSet    (In/Out) The set of sockets on which exception events are awaited and as a result the set of sockets on which event occurred
 *  \param waitMs       The maximum duration in milliseconds waiting for events (0 means infinite)
 *
 *  \return             The number of sockets with events contained by sets or -1 if failed
 */
int32_t Socket_WaitSocketEvents(SocketSet* readSet,
                                SocketSet* writeSet,
                                SocketSet* exceptSet,
                                uint32_t   waitMs);

/**
 *  \brief Write data through the socket
 *
 *  \param sock     The socket on which data must be written
 *  \param data     The data bytes to write on socket
 *  \param count    The number of bytes to write
 *
 *  \return         The number of bytes really written (should always be = to count)
 */
int32_t Socket_Write(Socket   sock,
                     uint8_t* data,
                     uint32_t count);

/**
 *  \brief Read data through the socket
 *
 *  \param sock         The socket on which data must be read
 *  \param data         The data bytes to be set with read bytes
 *  \param dataSize     The number of bytes that can be set (or expected to be read)
 *  \param readCount    The number of bytes actually read on the socket
 *
 *  \return         GOOD if operation succeeded, BAD_DISCONNECT in case of disconnection and BAD otherwise.
 */
SOPC_StatusCode Socket_Read(Socket     sock,
                       uint8_t*   data,
                       uint32_t   dataSize,
                       uint32_t*  readCount);


/**
 *  \brief Close the socket connection and/or clear the socket
 *
 *  \param sock     The socket to disconnect and/or clear
 */
void Socket_Close(Socket*  sock);

#endif /* INGOPCS_PLATFORMS_LINUX_P_SOCKETS_H_ */
