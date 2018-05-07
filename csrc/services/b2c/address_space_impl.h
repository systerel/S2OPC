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

/** \file
 *
 * \brief Declares initialisable members of the AddressSpace
 *        and OPC-UA values of B constants.
 */

#ifndef ADDRESS_SPACE_IMPL_H_
#define ADDRESS_SPACE_IMPL_H_

#include "b2c.h"
#include "constants.h"
#include "sopc_address_space.h"
#include "sopc_types.h"

/* AttributeIds: they are #defined by the SDK... */
/* TODO: move them to util_*.*, and make conversion functions */
typedef enum {
    c_aid_indet = 0,
    e_aid_NodeId,
    e_aid_NodeClass,
    e_aid_BrowseName,
    e_aid_DisplayName,
    e_aid_Description,
    e_aid_Value = 13,
    e_aid_AccessLevel = 17,
    e_aid_UserExecutable = 22
} util__AttributeId;

#define e_aid_min e_aid_NodeId
#define e_aid_max e_aid_UserExecutable

/* Access levels, taken from Part 3 §5.6.2 Table 8 */
#define SOPC_AccessLevelMask_CurrentRead 1
#define SOPC_AccessLevelMask_CurrentWrite 2

/* Attributes, and references */
extern SOPC_AddressSpace* address_space_bs__nodes;

/* Address space configured */
extern bool sopc_addressSpace_configured;

#endif /* ADDRESS_SPACE_IMPL_H_ */
