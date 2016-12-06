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

#ifndef STUB_SERVER_STUB_SERVER_H_
#define STUB_SERVER_STUB_SERVER_H_

#include "sopc_base_types.h"
#include "sopc_endpoint.h"

typedef struct {
    int Stub;
} StubServer_Endpoint;

SOPC_StatusCode StubServer_EndpointEvent_Callback(SOPC_Endpoint      endpoint,
											      void*              cbData,
											      SOPC_EndpointEvent event,
											      SOPC_StatusCode    status,
											      uint32_t           secureChannelId,
											      const Certificate* clientCertificate,
											      const SOPC_String* securityPolicy);

#endif /* STUB_SERVER_STUB_SERVER_H_ */
