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

#include "sopc_toolkit_async_api.h"

#include "sopc_services_api.h"

void SOPC_ToolkitServer_AsyncOpenEndpoint(uint32_t endpointDescriptionIdx)
{
    // TODO: check valid config and return bool
    SOPC_Services_EnqueueEvent(APP_TO_SE_OPEN_ENDPOINT, endpointDescriptionIdx, NULL, 0);
}

void SOPC_ToolkitServer_AsyncCloseEndpoint(uint32_t endpointDescriptionIdx)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_ENDPOINT, endpointDescriptionIdx, NULL, 0);
}

void SOPC_ToolkitClient_AsyncActivateSession(uint32_t endpointConnectionIdx)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_ACTIVATE_SESSION, endpointConnectionIdx, NULL,
                               1); // TODO: adapt B model to manage no user value at all since we treat only anonymous
}

void SOPC_ToolkitClient_AsyncSendRequestOnSession(uint32_t sessionId, void* requestStruct)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_SEND_SESSION_REQUEST, sessionId, requestStruct, 0);
}

void SOPC_ToolkitClient_AsyncCloseSession(uint32_t sessionId)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_SESSION, sessionId, NULL, 0);
}
