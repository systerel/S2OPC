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

#ifndef SOPC_WRAPPER_PROXYSTUB_H_
#define SOPC_WRAPPER_PROXYSTUB_H_

#include "sopc_base_types.h"
#include "sopc_encodeabletype.h"

BEGIN_EXTERN_C

SOPC_StatusCode OpcUa_ProxyStub_Initialize(void* pCalltable,
                                           void* pConfig);

SOPC_StatusCode OpcUa_ProxyStub_ReInitialize(void* pConfig);

void OpcUa_ProxyStub_Clear(void);

SOPC_StatusCode OpcUa_ProxyStub_AddTypes(SOPC_EncodeableType** types);

SOPC_StatusCode OpcUa_ProxyStub_SetNamespaceUris(char** namespaceUris);

char* OpcUa_ProxyStub_GetVersion();

char* OpcUa_ProxyStub_GetConfigString();

char* OpcUa_ProxyStub_GetStaticConfigString();

END_EXTERN_C

#endif /* SOPC_WRAPPER_PROXYSTUB_H_ */
