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

#include <stddef.h>
#include <string.h>

#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_mem_alloc.h"

#define URI_PREFIX_SEP "://"
#define URI_HOSTNAME_SEP ":"
#define URI_PORT_SEP "/"

#define URI_OPEN_BRACKET '['
#define URI_CLOSE_BRACKET ']'

#define TCPUA_PREFIX ((const char*) "opc.tcp")
#define UDPUA_PREFIX ((const char*) "opc.udp")
#define ETHUA_PREFIX ((const char*) "opc.eth")
#define MQTTUA_PREFIX ((const char*) "MqttUa")



static bool getUriPortId(const char** ppCursor, char** ppPort)
{
    const char* start = *ppCursor;
    const char* pCursor = *ppCursor;
    bool res = true;
    bool match = false;
    char* resStr = NULL;
    size_t len = 0;

    while (!match)
    {
        match = URI_match(*pCursor, URI_Port_Sep);
        if (!*pCursor)
        {
            match = true;
        }
        if (!match)
        {
            ++len;
            ++pCursor;
        }
    }
    if (0 == len)
    {
        res = false;
    }
    if (res)
    {
        resStr = calloc(len + 1, sizeof(char));
        if (resStr == NULL)
        {
            res = false;
        }
    }
    if (res)
    {
        resStr = strncpy(resStr, start, len);
        *ppPort = resStr;
        *ppCursor = pCursor;
    }
    return (res);
}

static bool getUriHostname(const char** ppCursor, char** ppHostname)
{
    const char* start = *ppCursor;
    const char* pCursor = *ppCursor;
    bool res = true;
    bool match = false;
    char* resStr = NULL;
    size_t NbBracketOpen = URI_Default_Bracket_nb;
    size_t len = 0;

    while (!match && res)
    {
        if (NbBracketOpen <= 0)
        {
            match = URI_match(*pCursor, URI_HostName_Sep);
        }
        if (!match)
        {
            if (*pCursor == '[')
            {
                ++NbBracketOpen;
            }
            if (0 < NbBracketOpen && ']' == *pCursor)
            {
                --NbBracketOpen;
            }
            ++len;
            ++pCursor;
        }
        if (!*pCursor)
        {
            res = false;
        }
    }
    if (NbBracketOpen > 0 || 0 == len)
    {
        res = false;
    }
    if (res)
    {
        resStr = calloc(len + 1, sizeof(char));
        if (resStr == NULL)
        {
            res = false;
        }
    }
    if (res)
    {
        resStr = strncpy(resStr, start, len);
        *ppHostname = resStr;
        while (match)
        {
            match = URI_match(*(++pCursor), URI_HostName_Sep);
        }
        *ppCursor = pCursor;
    }
    return (res);
}

static bool getUriPrefix(const char** ppCursor, char** ppPrefix)
{
    const char* start = *ppCursor;
    const char* pCursor = *ppCursor;
    bool res = true;
    bool match = false;
    char* resStr = NULL;
    size_t len = 0;

    while (!match && res)
    {
        match = URI_match(*pCursor, URI_Prefix_Sep);
        if (!match)
        {
            ++len;
            ++pCursor;
        }
        else if (!*pCursor)
        {
            res = false;
        }
    }
    if (0 == len)
    {
        res = false;
    }
    if (res)
    {
        resStr = calloc(len + 1, sizeof(char));
        if (resStr == NULL)
        {
            res = false;
        }
    }
    if (res)
    {
        resStr = strncpy(resStr, start, len);
        *ppPrefix = resStr;
        while (match)
        {
            match = URI_match(*(++pCursor), URI_Prefix_Sep);
        }
        *ppCursor = pCursor;
    }
    return (res);
}

static bool getUriTypeFromEnum(char** prefix, SOPC_UriType* type)
{
    int i = 0;

    if (strncmp(*prefix, TCPUA_PREFIX, strlen(*prefix)) == 0)
    {
        *type = SOPC_URI_TcpUa;
        return (true);
    }
    else if (strncmp(*prefix, UDPUA_PREFIX, strlen(*prefix)) == 0)
    {
        *type = SOPC_URI_UdpUa;
        return (true);
    }
    else if (strncmp(*prefix, ETHUA_PREFIX, strlen(*prefix)) == 0)
    {
        *type = SOPC_URI_EthUa;
        return (true);
    }
    else if (strncmp(*prefix, MQTTUA_PREFIX, strlen(*prefix)) == 0)
    {
        *type = SOPC_URI_MqttUa;
        return (true);
    }
    return (false);
}

bool SOPC_Helper_URI_SplitUri(const char* uri, SOPC_UriType* type, char** hostname, char** port)
{
    if (NULL == uri || NULL == hostname || NULL == port)
    {
        return (false);
    }
    if (strlen(uri) + 4 > TCP_UA_MAX_URL_LENGTH) // Encoded value shall be less than 4096 byte
    {
        return (false);
    }

    const char* pCursor = uri;
    char* prefix = NULL;
    bool result = true;

    if (result)
    {
        result = getUriPrefix(&pCursor, &prefix);
    }
    if (result)
    {
        result = getUriHostname(&pCursor, hostname);
    }
    if (result)
    {
        result = getUriPortId(&pCursor, port);
    }
    if (result)
    {
        result = getUriTypeFromEnum(&prefix, type);
    }
    free(prefix);
    if (!result)
    {
        free(*hostname);
        free(*port);
    }
    return (result);
}
