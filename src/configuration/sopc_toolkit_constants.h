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

#ifndef SOPC_TOOLKIT_CONSTANTS_H_
#define SOPC_TOOLKIT_CONSTANTS_H_

#include <stdbool.h>

/* COMMON CONFIGURATION */

/** @brief Maximum Message Length used */
#ifndef SOPC_MAX_MESSAGE_LENGTH
# define SOPC_MAX_MESSAGE_LENGTH  UINT16_MAX
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/* TCP SOCKETS CONFIGURATION */

/** @brief Maximum number of TCP sockets (listeners and connections) */
#ifndef SOPC_MAX_SOCKETS
# define SOPC_MAX_SOCKETS  150
#endif /* SOPC_MAX_SOCKETS */

/** @brief Maximum number of TCP sockets connections on a socket listener */
#ifndef SOPC_MAX_SOCKETS_CONNECTIONS
# define SOPC_MAX_SOCKETS_CONNECTIONS  50
#endif /* SOPC_MAX_SOCKETS_CONNECTIONS */

/* SECURE CHANNEL CONFIGURATION */

/** @brief Maximum number of endpoint description configured */
#ifndef SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
# define SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS  10
#endif /* SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS */

#ifndef SOPC_LISTENER_LISTEN_ALL_INTERFACES
# define SOPC_LISTENER_LISTEN_ALL_INTERFACES  true
#endif /* SOPC_LISTENER_LISTEN_ALL_INTERFACES */


/** @brief Maximum number of secure channel connections established (should be < SOPC_MAX_SOCKETS) */
#ifndef SOPC_MAX_SECURE_CONNECTIONS
# define SOPC_MAX_SECURE_CONNECTIONS  20
#endif /* SOPC_MAX_SECURE_CONNECTIONS */


#endif /* SOPC_TOOLKIT_CONSTANTS_H_ */
