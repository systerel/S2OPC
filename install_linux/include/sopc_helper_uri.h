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

#ifndef SOPC_HELPER_URI_H_
#define SOPC_HELPER_URI_H_

#include <stdbool.h>
#include <stddef.h>

#define TCP_UA_MAX_URL_LENGTH 4096

/** @brief: return true and output parameters if parsed with success. false otherwise */
bool SOPC_Helper_URI_ParseTcpUaUri(const char* uri, size_t* hostnameLength, size_t* portIdx, size_t* portLength);

#endif /* SOPC_HELPER_URI_H_ */
