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

#ifndef SOPC_BASE_TYPES_H_
#define SOPC_BASE_TYPES_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
# define BEGIN_EXTERN_C extern "C" {
# define END_EXTERN_C }
#else
# define BEGIN_EXTERN_C
# define END_EXTERN_C
#endif

BEGIN_EXTERN_C

typedef uint32_t SOPC_StatusCode;
#define STATUS_OK 0x0 // TODO: change values
#define STATUS_OK_INCOMPLETE 0x00000001
#define STATUS_NOK 0x80000000//0x10000000
#define STATUS_INVALID_PARAMETERS 0x80760001//0x20000000
#define STATUS_INVALID_STATE 0x80760002//0x30000000
#define STATUS_INVALID_RCV_PARAMETER 0x80000003//0x40000000

#define FALSE 0

typedef void* P_Timer;

END_EXTERN_C

#endif /* SOPC_BASE_TYPES_H_ */
