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

#include "libs2opc_server_internal.h"
#include "libs2opc_server_runtime_variables.h"

#include "sopc_event_handler.h"
#include "sopc_macros.h"

static SOPC_EventHandler* serverDiagnosticsEventHandler = NULL;

void SOPC_ServerInternal_UpdateServerDiagnostics(const SOPC_Server_RuntimeVariablesDiagnostics* diagnostics)
{
    OpcUa_WriteRequest* writeRequest = SOPC_RuntimeVariables_BuildUpdateServerDiagnosticsWriteRequest(diagnostics);

    if (NULL != writeRequest)
    {
        bool res = SOPC_ServerInternal_LocalServiceAsync(
            SOPC_HelperInternal_RuntimeVariableSetResponseCb, writeRequest, (uintptr_t) NULL,
            "Updating server diagnostics runtime variables of server information nodes failed."
            " Please check address space content includes necessary diagnostic information nodes.");

        SOPC_UNUSED_RESULT(res);
    }
}

void SOPC_ServerInternal_SetDiagnosticsEventHandler(SOPC_EventHandler* eventHandler)
{
    serverDiagnosticsEventHandler = eventHandler;
}

SOPC_EventHandler* SOPC_ServerInternal_GetDiagnosticsEventHandler(void)
{
    return serverDiagnosticsEventHandler;
}
