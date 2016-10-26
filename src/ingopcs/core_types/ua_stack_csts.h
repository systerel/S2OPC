/*
 * ua_stack_csts.h
 *
 *  Created on: Oct 4, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_STACK_CSTS_H_
#define INGOPCS_UA_STACK_CSTS_H_

#include <stdint.h>

/** High level API activation */
// Client API activation
#define OPCUA_HAVE_CLIENTAPI 1
// Client API activation
#define OPCUA_HAVE_SERVERAPI 1

/** Stack configuration options */

// Validate the server certificate provided by client application during treatment of the open secure channel response,
#define UA_SECURECONNECTION_VALIDATE_SERVERCERT 1

// Run in multi thread mode. Allow to manage sockets with several threads when activated,
#define UA_MULTITHREADED 0

// Maximum size of a TCP UA message (max representable in binary is UINT32_MAX),
#define UA_ENCODER_MAXMESSAGELENGTH (uint32_t) UINT16_MAX

// Maximum lifetime of a secure channel security token (max representable in binary is UINT32_MAX),
#define UA_SECURITYTOKEN_LIFETIME_MAX (uint32_t) INT32_MAX // ~24 days
// Minimum lifetime of a secure channel security token (max representable in binary is UINT32_MAX),
#define UA_SECURITYTOKEN_LIFETIME_MIN (uint32_t) UINT16_MAX // ~1 minute

// Maximum time for activation of secure channel (time between hello message and secure channel request),
#define UA_SECURELISTENER_CHANNELTIMEOUT 5000

// Default and maximum chunk size for send and receive buffers for a client (max in implem: UINT32_MAX),
#define UA_TCPCONNECTION_DEFAULTCHUNKSIZE (uint32_t) UINT16_MAX
// Default and maximum chunk size for send and receive buffers for a server (max in implem: UINT32_MAX),
#define UA_TCPLISTENER_DEFAULTCHUNKSIZE (uint32_t) UINT16_MAX

// Buffer size for the socket receive buffer,
#define UA_P_TCPRCVBUFFERSIZE UINT16_MAX
// Buffer size for the socket send buffer,
#define UA_P_TCPSNDBUFFERSIZE UINT16_MAX

// Default timeout for server sockets,
#define UA_TCPLISTENER_TIMEOUT UINT16_MAX // ~18 hours

// Maximum number of threads managing sockets in a socket manager,
#define UA_SOCKET_MAXMANAGERS 0 // Not implemented

// Maximum number of pending messages before the server starts to block.
#define UA_SECURECONNECTION_MAXPENDINGMESSAGES 0// Not implemented

// Maximum number of simultaneous connections managed by stack
#define UA_MAXCONNECTIONS 150

#endif /* INGOPCS_UA_STACK_CSTS_H_ */
