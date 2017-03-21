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

#ifndef SOPC_STACK_CSTS_H_
#define SOPC_STACK_CSTS_H_

#include <stdint.h>

/* High level API activation */
// Client API activation
#define OPCUA_HAVE_CLIENTAPI 1
// Client API activation
#define OPCUA_HAVE_SERVERAPI 1

/* Stack configuration options */

// Validate the server certificate provided by client application during treatment of the open secure channel response,
#define OPCUA_SECURECONNECTION_VALIDATE_SERVERCERT 1

// Maximum size of a TCP UA message (max representable in binary is UINT32_MAX),
#define OPCUA_ENCODER_MAXMESSAGELENGTH (uint32_t) UINT16_MAX

// Maximum lifetime of a secure channel security token (max representable in binary is UINT32_MAX),
#define OPCUA_SECURITYTOKEN_LIFETIME_MAX (uint32_t) INT32_MAX // ~24 days
// Minimum lifetime of a secure channel security token (max representable in binary is UINT32_MAX),
#define OPCUA_SECURITYTOKEN_LIFETIME_MIN (uint32_t) UINT16_MAX // ~1 minute

// Maximum time for activation of secure channel (time between hello message and secure channel request),
#define OPCUA_SECURELISTENER_CHANNELTIMEOUT 5000

// Default and maximum chunk size for send and receive buffers for a client (max in implem: UINT32_MAX),
#define OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE (uint32_t) UINT16_MAX
// Default and maximum chunk size for send and receive buffers for a server (max in implem: UINT32_MAX),
#define OPCUA_TCPLISTENER_DEFAULTCHUNKSIZE (uint32_t) UINT16_MAX

// Buffer size for the socket receive buffer,
#define OPCUA_P_TCPRCVBUFFERSIZE UINT16_MAX
// Buffer size for the socket send buffer,
#define OPCUA_P_TCPSNDBUFFERSIZE UINT16_MAX

// Default timeout for server sockets,
#define OPCUA_TCPLISTENER_TIMEOUT UINT16_MAX // ~18 hours

// Maximum number of threads managing sockets in a socket manager,
#define OPCUA_SOCKET_MAXMANAGERS 0 // Not implemented

// Maximum number of pending messages before the server starts to block.
#define OPCUA_SECURECONNECTION_MAXPENDINGMESSAGES 0// Not implemented

// Maximum number of simultaneous connections managed by stack
#define OPCUA_MAXCONNECTIONS 150

// Maximum number of simultaneous connections on 1 endpoint managed by stack
#define OPCUA_ENDPOINT_MAXCONNECTIONS 50

#endif /* SOPC_STACK_CSTS_H_ */
