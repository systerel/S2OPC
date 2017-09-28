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

/** \file
 *
 * \brief Declares initialisable members of the AddressSpace
 *        and OPC-UA values of B constants.
 */


#ifndef address_space_impl_h_
#define address_space_impl_h_


#include "b2c.h"
#include "constants.h"
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
    e_aid_UserExecutable = 22
} AttributeId;

#define e_aid_min e_aid_NodeId
#define e_aid_max e_aid_UserExecutable


/* Attributes, and references */
extern int32_t address_space_bs__nNodeIds;
extern int32_t address_space_bs__nVariables;
extern int32_t address_space_bs__nVariableTypes;
extern int32_t address_space_bs__nObjectTypes;
extern int32_t address_space_bs__nReferenceTypes;
extern int32_t address_space_bs__nDataTypes;
extern int32_t address_space_bs__nMethods;
extern int32_t address_space_bs__nObjects;
extern int32_t address_space_bs__nViews;
extern SOPC_NodeId         **address_space_bs__a_NodeId;
extern OpcUa_NodeClass     *address_space_bs__a_NodeClass;
extern SOPC_QualifiedName  *address_space_bs__a_BrowseName;
extern SOPC_LocalizedText  *address_space_bs__a_DisplayName;
extern int32_t             *address_space_bs__a_DisplayName_begin;
extern int32_t             *address_space_bs__a_DisplayName_end;
extern SOPC_Variant        *address_space_bs__a_Value;
extern SOPC_StatusCode     *address_space_bs__a_Value_StatusCode;
extern SOPC_ExpandedNodeId **address_space_bs__HasTypeDefinition;
extern SOPC_NodeId         **address_space_bs__refs_ReferenceType;
extern SOPC_ExpandedNodeId **address_space_bs__refs_TargetNode;
extern bool                *address_space_bs__refs_IsForward;
extern int32_t             *address_space_bs__RefIndexBegin;
extern int32_t             *address_space_bs__RefIndexEnd;


#endif // address_space_impl_h_
