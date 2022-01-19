/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * This file is an excerpt from sopc_toolkit_config.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

//#include "sopc_address_space.h"
//#include "sopc_user_app_itf.h"
//#include "sopc_builtintypes.h"
/* #include "sopc_common_build_info.h"  Unused */
//#include "sopc_types.h"

SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct);
SOPC_ReturnStatus SOPC_ToolkitServer_Configured(void);
void SOPC_Toolkit_Clear(void);

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct);

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);
uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* config);

SOPC_Toolkit_Build_Info SOPC_ToolkitConfig_GetBuildInfo(void);

void SOPC_ToolkitClient_ClearAllSCs(void);
