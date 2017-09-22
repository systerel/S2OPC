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

#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"

#include "add.h"
#include "address_space_impl.h"

#include "util_b2c.h"


SOPC_StatusCode _ToolkitServer_Initialize_AddressSpace()
{
    /* Glue the address_space_bs machine content to the generated address space content */
    int32_t i;

    /* Number of nodes by nodeclass */
     /* TODO: fixme when defines are available */
    address_space_bs__nNodeIds = 55;
    address_space_bs__nVariables = 33;
    address_space_bs__nVariableTypes = 0;
    address_space_bs__nObjectTypes = 0;
    address_space_bs__nReferenceTypes = 0;
    address_space_bs__nDataTypes = 0;
    address_space_bs__nMethods = 0;
    address_space_bs__nObjects = 22;
    address_space_bs__nViews = 0;

    /* Attributes */
    address_space_bs__a_NodeId = (constants__t_NodeId_i *)NodeId;
    /* Converts NodeClasses */
    address_space_bs__a_NodeClass = (constants__t_NodeClass_i *)NodeClass;
    for(i=1; i<=address_space_bs__nNodeIds; ++i)
        assert(util_NodeClass__C_to_B(NodeClass[i], &address_space_bs__a_NodeClass[i]));
    address_space_bs__a_BrowseName = (constants__t_QualifiedName_i *)BrowseName;
    address_space_bs__a_DisplayName = (constants__t_LocalizedText_i *)DisplayName;
    address_space_bs__a_DisplayName_begin = DisplayName_begin;
    address_space_bs__a_DisplayName_end = DisplayName_end;
    address_space_bs__a_Value = (constants__t_Variant_i *)Value;
    /* Converts status codes */
    address_space_bs__a_Value_StatusCode = status_code;
    for(i=1; i<=address_space_bs__nVariables; ++i)
        assert(util_status_code__C_to_B(status_code[i], &address_space_bs__a_Value_StatusCode[i]));
    address_space_bs__HasTypeDefinition = NULL;

    /* References */
    address_space_bs__refs_ReferenceType = (constants__t_NodeId_i *)reference_type;
    address_space_bs__refs_TargetNode = (constants__t_ExpandedNodeId_i *)reference_target;
    address_space_bs__refs_IsForward = reference_isForward;
    address_space_bs__RefIndexBegin = reference_begin;
    address_space_bs__RefIndexEnd = reference_end;

    return STATUS_OK;
}


SOPC_StatusCode SOPC_ToolkitServer_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    SOPC_StatusCode sc = _ToolkitServer_Initialize_AddressSpace();
    if(STATUS_OK != sc)
        return sc;

    return SOPC_Internal_Toolkit_Initialize(pAppFct);
}


