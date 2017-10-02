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

#ifndef SOPC_P_SOCKETS_H_
#define SOPC_P_SOCKETS_H_

#include <netdb.h>
#include <sys/select.h>

#define SOPC_INVALID_SOCKET -1

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

#endif /* SOPC_P_SOCKETS_H_ */
