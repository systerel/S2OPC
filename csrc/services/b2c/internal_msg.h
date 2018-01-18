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

#ifndef _internal_msg_h
#define _internal_msg_h

#include "b2c.h"
#include "sopc_encodeable.h"
#include "sopc_types.h"

typedef struct message__request_message
{
    OpcUa_RequestHeader requestHeader;
    // ...
} message__request_message;

typedef struct message__response_message
{
    OpcUa_ResponseHeader responseHeader;
    // ...
} message__response_message;

#endif
