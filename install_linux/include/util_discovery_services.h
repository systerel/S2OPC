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

#ifndef util_discovery_services_h_
#define util_discovery_services_h_

#include "constants.h"
#include "sopc_builtintypes.h"
#include "sopc_types.h"

bool SOPC_Discovery_ContainsBinaryProfileURI(uint32_t nbOfProfileURI, SOPC_String* profileUris);

constants__t_StatusCode_i SOPC_Discovery_GetEndPointsDescriptions(
    const constants__t_endpoint_config_idx_i endpoint_config_idx,
    bool isCreateSessionResponse,
    SOPC_String* EndpointUrl,
    uint32_t* nbOfEndpointDescriptions,
    OpcUa_EndpointDescription** endpointDescriptions);

#endif /* util_discovery_services_h_ */
