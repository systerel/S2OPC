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
    e_aid_Value = 13,
    e_aid_UserExecutable = 22
} AttributeId;

#define e_aid_min e_aid_NodeId
#define e_aid_max e_aid_UserExecutable


/* Attributes, and references */
/* TODO: Today NB_NODES is defined in gen_addspace.h */
#include "gen_addspace.h"
extern constants__t_NodeId_i a_NodeId[NB_NODES];
extern constants__t_NodeClass_i a_NodeClass[NB_NODES];
extern constants__t_Variant_i a_Value[NB_NODES];
extern constants__t_StatusCode_i a_Value_StatusCode[NB_NODES];
extern size_t address_space_bs__nNodeId;
extern constants__t_QualifiedName_i address_space_bs__a_BrowseName;
extern constants__t_LocalizedText_i address_space_bs__a_DisplayName;
extern constants__t_ExpandedNodeId_i address_space_bs__HasTypeDefinition;
extern constants__t_NodeId_i address_space_bs__refs_ReferenceType;
extern constants__t_ExpandedNodeId_i address_space_bs__refs_TargetNode;
extern t_bool *address_space_bs__refs_IsForward;
extern size_t *address_space_bs__RefIndexBegin;
extern size_t *address_space_bs__RefIndexEnd;


#endif // address_space_impl_h_
