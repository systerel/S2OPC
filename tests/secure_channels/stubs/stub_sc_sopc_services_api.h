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

#ifndef STUBS_SC_SOPC_SERVICES_API_H_
#define STUBS_SC_SOPC_SERVICES_API_H_

#include "sopc_async_queue.h"
#include "sopc_services_api.h"

void SOPC_Services_Initialize(void);

void SOPC_Services_Clear(void);

typedef struct SOPC_StubSC_ServicesEventParams {
    SOPC_Services_Event event;
    uint32_t            eltId;
    void*               params;
    uint32_t            auxParam;
} SOPC_StubSC_ServicesEventParams;

// Async queue simulating the services dispatcher and containing SOPC_StubSC_ServicesEventParams elements
extern SOPC_AsyncQueue* servicesEvents;

#endif /* STUBS_SC_SOPC_SERVICES_API_H_ */
