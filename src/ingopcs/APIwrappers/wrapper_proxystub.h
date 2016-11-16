/*
 * wrapper_proxystub.h
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#ifndef SOPC_WRAPPER_PROXYSTUB_H_
#define SOPC_WRAPPER_PROXYSTUB_H_

#include "sopc_base_types.h"
#include "sopc_encodeable.h"

BEGIN_EXTERN_C

SOPC_StatusCode OpcUa_ProxyStub_Initialize(void* pCalltable,
                                           void* pConfig);

SOPC_StatusCode OpcUa_ProxyStub_ReInitialize(void* pConfig);

void OpcUa_ProxyStub_Clear(void);

SOPC_StatusCode OpcUa_ProxyStub_AddTypes(SOPC_EncodeableType** types);

SOPC_StatusCode OpcUa_ProxyStub_SetNamespaceUris(char** namespaceUris);

END_EXTERN_C

#endif /* SOPC_WRAPPER_PROXYSTUB_H_ */
