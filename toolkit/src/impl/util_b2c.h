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

#ifndef _util_b2c_h
#define _util_b2c_h

#include "sopc_encodeable.h"
#include "opcua_statuscodes.h"
#include "constants.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest);

void util_message__get_message_type(SOPC_EncodeableType* encType,
                                    constants__t_msg_type_i* message__msg_type);

void util_status_code__B_to_C(constants__t_StatusCode_i bstatus,
                              SOPC_StatusCode* status);

t_bool util_status_code__C_to_B(SOPC_StatusCode status,
                                constants__t_StatusCode_i* bstatus);

#endif /* _util_b2c_h */
