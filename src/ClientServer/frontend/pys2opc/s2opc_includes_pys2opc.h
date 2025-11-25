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

#ifndef S2OPC_INCLUDES_pys2opc_H_
#define S2OPC_INCLUDES_pys2opc_H_

#ifdef S2OPC_AUTOPXD
#include "sopc_stdint_autopxd.h"
#endif
#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_logger.h"
#include "sopc_service_call_context.h"
#include "sopc_user_app_itf.h"

// All structures used in PYX files must be defined as follows, because :
// 1. It allows autopxd to generate this structure in the PXD
//    (necessary for function declarations using these structures)
// 2. Allows compilation of PYX files to have the symbol in a `.h` (otherwise there's a compile warning).
typedef struct SOPC_CallContext
{
    bool placeholder;
} SOPC_CallContext;

typedef struct SOPC_ClientConnection
{
    bool placeholder;
} SOPC_ClientConnection;

typedef struct SOPC_ClientHelper_Subscription
{
    bool placeholder;
} SOPC_ClientHelper_Subscription;

#endif /* S2OPC_INCLUDES_pys2opc_H_ */
