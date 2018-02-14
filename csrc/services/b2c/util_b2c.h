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

#ifndef UTIL_B2C_H_
#define UTIL_B2C_H_

#include <stdbool.h>

#include "address_space_impl.h"
#include "constants.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_types.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest);

void util_message__get_message_type(SOPC_EncodeableType* encType, constants__t_msg_type_i* message__msg_type);

void util_status_code__B_to_C(constants__t_StatusCode_i bstatus, SOPC_StatusCode* status);

void util_status_code__C_to_B(SOPC_StatusCode status, constants__t_StatusCode_i* bstatus);

SOPC_ReturnStatus util_status_code__B_to_return_status_C(constants__t_StatusCode_i bstatus);

bool util_channel__SecurityPolicy_C_to_B(const char* uri, constants__t_SecurityPolicy* secpol);

/* Returns true or false upon failure (e_bd_indet or invalid cdir) */
bool util_BrowseDirection__B_to_C(constants__t_BrowseDirection_i bdir, OpcUa_BrowseDirection* cdir);

/* Returns true or false upon failure (invalid bdir) */
bool util_BrowseDirection__C_to_B(OpcUa_BrowseDirection cdir, constants__t_BrowseDirection_i* bdir);

/* Returns true or false upon failure (c_NodeClass_indet or invalid cncl) */
bool util_NodeClass__B_to_C(constants__t_NodeClass_i bncl, OpcUa_NodeClass* cncl);

/* Returns true or false upon failure (invalid bncl) */
bool util_NodeClass__C_to_B(OpcUa_NodeClass cncl, constants__t_NodeClass_i* bncl);

/* Returns true or false upon failure (c_TimestampsToReturn_indet or invalid pcttr) */
bool util_TimestampsToReturn__B_to_C(constants__t_TimestampsToReturn_i bttr, OpcUa_TimestampsToReturn* pcttr);

/* Returns true or false upon failure (invalid pbttr) */
bool util_TimestampsToReturn__C_to_B(OpcUa_TimestampsToReturn cttr, constants__t_TimestampsToReturn_i* pbttr);

/* Returns true or false upon failure (c_AttributeId_indet or invalid pcaid) */
bool util_AttributeId__B_to_C(constants__t_AttributeId_i baid, uint32_t* pcaid);

/* Returns true or false upon failure (invalid caid or invalid pbaid) */
bool util_AttributeId__C_to_B(uint32_t caid, constants__t_AttributeId_i* pbaid);

#endif /* UTIL_B2C_H */
