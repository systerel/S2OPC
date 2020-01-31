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

#ifndef SOPC_HELPER_URI_H_
#define SOPC_HELPER_URI_H_
#define TCP_UA_MAX_URL_LENGTH 4096

#include <stdbool.h>

#include "sopc_enums.h"

typedef enum SOPC_UriSwitch
{
    SOPC_URI_PREFIX = 0,
    SOPC_URI_PORT,
} SOPC_UriSwitch;

typedef enum SOPC_UriType
{
    SOPC_URI_UNDETERMINED = 0,
    SOPC_URI_TCPUA,
    SOPC_URI_UDPUA,
    SOPC_URI_ETHUA,
    SOPC_URI_MQTTUA
} SOPC_UriType;

/* set given type, hostname and prefix from given URI and return true. Return false in case of failure */
/* return false if at least one parameter is NULL */
/* in case of failure parameters are not modified */
SOPC_ReturnStatus SOPC_Helper_URI_SplitUri(const char* uri, SOPC_UriType* type, char** hostname, char** port);

#endif /* SOPC_HELPER_URI_H_ */
