/*
 *  Copyright (C) 2018 Systerel and others.
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

#ifndef SOPC_CONFIG_CONSTANTS_CHECK_H_
#define SOPC_CONFIG_CONSTANTS_CHECK_H_

#include <stdint.h>

#include "sopc_toolkit_config_constants.h"

/* Maximum value accepted in B model */
#if SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS > INT32_MAX
#error "Max number of endpoint descriptions cannot be more than INT32_MAX"
#endif

#if SOPC_MAX_SECURE_CONNECTIONS > SOPC_MAX_SOCKETS
#error "Max number of secure connections cannot be greater than max number of sockets"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SECURE_CONNECTIONS > INT32_MAX
#error "Max number of secure connections cannot be more than INT32_MAX"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SESSIONS > INT32_MAX
#error "Max number of sessions cannot be more than INT32_MAX"
#endif

/* Maximum session timeout accepted */
#if SOPC_MAX_SESSION_TIMEOUT > UINT32_MAX
#error "Maximum requested session timeout is > UINT32_MAX"
#endif
#if SOPC_MAX_SESSION_TIMEOUT < SOPC_MIN_SESSION_TIMEOUT
#error "Maximum requested session timeout is < MIN"
#endif

/* Minimum session timeout accepted */
#if SOPC_MIN_SESSION_TIMEOUT < 10000
#error "Minimum requested session timeout is < 10000"
#endif
#if SOPC_MIN_SESSION_TIMEOUT > SOPC_MAX_SESSION_TIMEOUT
#error "Minimum requested session timeout is > MAX"
#endif

/* Check use of uintptr_t is not an issue on the current platform */
#if UINTPTR_MAX < UINT32_MAX
#error "UINTPTR_MAX < UINT32_MAX whereas uintptr_t are used to store uint32_t values"
#endif

/* Check use of int64_t to store ssize_t values is valid */
#if SSIZE_MAX > INT64_MAX
#error "SSIZE_MAX > INT64_MAX whereas int64_t are used to store ssize_t values"
#endif

/* Check casts from uint32_t / int32_t to size_t are valid without verification */
#if SIZE_MAX < UINT32_MAX
#error "SIZE_MAX < UINT32_MAX whereas uint32_t are casted to size_t values"
#endif

#endif
