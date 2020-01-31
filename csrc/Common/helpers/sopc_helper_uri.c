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

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

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

/**
 * Match prefix with SOPC existing prefix set the type with a SOPC_UriType and return true otherwise return false
 */
static SOPC_ReturnStatus getUriTypeFromEnum(char** prefix, SOPC_UriType* type);

/**
 * Extract Hostname from given URI
 * Must take a *pCursor which strictly start at the beginning of the sequence and a hostname with *hostname pointing to
 * NULL.
 * In case of failure parameters are not modified
 */
static SOPC_ReturnStatus getUriHostname(const char** ppCursor, char** ppHostname);

/**
 * Extract prefix or port from given URI depending on sep_match
 * Must take a *pCursor which strictly start at the beginning of the sequence and a port or prefix with *port or *prefix
 * pointing to NULL.
 * uriSwitch specify if we are looking for a port or a prefix.
 * In case of failure parameters are not modified
 */
static SOPC_ReturnStatus getUriPrefixOrPort(const char** ppCursor,
                                            char** ppFind,
                                            const char* sep_match,
                                            SOPC_UriSwitch uriSwitch);

static SOPC_ReturnStatus getUriHostname(const char** ppCursor, char** ppHostname)
{
    if (NULL == ppCursor || NULL == *ppCursor || NULL == ppHostname || NULL != *ppHostname)
    {
        return (SOPC_STATUS_INVALID_PARAMETERS);
    }

    const char* start = *ppCursor;
    const char* pCursor = *ppCursor;
    SOPC_ReturnStatus res = SOPC_STATUS_OK;
    bool match = false;
    char* resStr = NULL;
    size_t len = 0;
    size_t NbBracketOpen = 0;

    while (!match && res == SOPC_STATUS_OK)
    {
        if (0 == NbBracketOpen)
        {
            match = strchr(URI_HOSTNAME_SEP, *pCursor) != NULL;
        }
        if (!match)
        {
            if (*pCursor == URI_OPEN_BRACKET)
            {
                ++NbBracketOpen;
            }
            if (NbBracketOpen > 0 && URI_CLOSE_BRACKET == *pCursor)
            {
                --NbBracketOpen;
            }
            ++len;
            ++pCursor;
        }
        if ('\0' == *pCursor)
        {
            res = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (NbBracketOpen > 0 || 0 == len)
    {
        res = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == res)
    {
        resStr = SOPC_Calloc(len + 1, sizeof(char));
        if (NULL == resStr)
        {
            res = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == res)
    {
        resStr = strncpy(resStr, start, len);
        *ppHostname = resStr;
        pCursor += strlen(URI_HOSTNAME_SEP);
        *ppCursor = pCursor;
    }
    return (res);
}

static SOPC_ReturnStatus getUriPrefixOrPort(const char** ppCursor,
                                            char** ppFind,
                                            const char* sep_match,
                                            SOPC_UriSwitch uriSwitch)
{
    if (NULL == ppCursor || NULL == *ppCursor || NULL == ppFind || NULL != *ppFind || NULL == sep_match)
    {
        return (SOPC_STATUS_INVALID_PARAMETERS);
    }

    const char* start = *ppCursor;
    const char* pCursor = *ppCursor;
    SOPC_ReturnStatus res = SOPC_STATUS_OK;
    char* resStr = NULL;
    size_t len = 0;

    pCursor = strstr(start, sep_match);
    if (SOPC_URI_PREFIX == uriSwitch)
    {
        if (NULL == pCursor)
        {
            res = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (SOPC_URI_PORT == uriSwitch)
    {
        if (NULL == pCursor)
        {
            pCursor = start + strlen(start);
        }
    }
    else
    {
        assert(false && "Unknown uriSwitch");
    }

    if (SOPC_STATUS_OK == res)
    {
        if (pCursor > start)
        {
            len = (size_t)(pCursor - start);
        }
        else
        {
            res = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == res)
    {
        resStr = SOPC_Calloc(len + 1, sizeof(char));
        if (NULL == resStr)
        {
            res = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == res)
    {
        resStr = strncpy(resStr, start, len);
        pCursor += strlen(sep_match);
        *ppFind = resStr;
        *ppCursor = pCursor;
    }
    return (res);
}

static SOPC_ReturnStatus getUriTypeFromEnum(char** prefix, SOPC_UriType* type)
{
    if (strncmp(*prefix, TCPUA_PREFIX, (strlen(*prefix) + 1)) == 0)
    {
        *type = SOPC_URI_TCPUA;
        return (SOPC_STATUS_OK);
    }
    else if (strncmp(*prefix, UDPUA_PREFIX, (strlen(*prefix) + 1)) == 0)
    {
        *type = SOPC_URI_UDPUA;
        return (SOPC_STATUS_OK);
    }
    else if (strncmp(*prefix, ETHUA_PREFIX, (strlen(*prefix) + 1)) == 0)
    {
        *type = SOPC_URI_ETHUA;
        return (SOPC_STATUS_OK);
    }
    else if (strncmp(*prefix, MQTTUA_PREFIX, (strlen(*prefix) + 1)) == 0)
    {
        *type = SOPC_URI_MQTTUA;
        return (SOPC_STATUS_OK);
    }
    return (SOPC_STATUS_INVALID_PARAMETERS);
}

SOPC_ReturnStatus SOPC_Helper_URI_SplitUri(const char* uri, SOPC_UriType* type, char** hostname, char** port)
{
    if (NULL == uri || NULL == hostname || NULL == port || NULL != *port || NULL != *hostname)
    {
        return (SOPC_STATUS_INVALID_PARAMETERS);
    }
    if (strlen(uri) + 4 > TCP_UA_MAX_URL_LENGTH) // Encoded value shall be less than 4096 byte
    {
        return (SOPC_STATUS_INVALID_PARAMETERS);
    }

    /* *pCursor is a tmp copy to the uri. its purpose is to run through the URI
     * Its pos must be at the beginning of each sequence and must be strictly after the last separator found */
    const char* pCursor = uri;
    char* prefix = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == result)
    {
        result = getUriPrefixOrPort(&pCursor, &prefix, URI_PREFIX_SEP, SOPC_URI_PREFIX);
    }
    if (SOPC_STATUS_OK == result)
    {
        result = getUriHostname(&pCursor, hostname);
    }
    if (SOPC_STATUS_OK == result)
    {
        result = getUriPrefixOrPort(&pCursor, port, URI_PORT_SEP, SOPC_URI_PORT);
    }
    if (SOPC_STATUS_OK == result)
    {
        result = getUriTypeFromEnum(&prefix, type);
    }
    SOPC_Free(prefix);
    if (SOPC_STATUS_OK != result)
    {
        SOPC_Free(*hostname);
        SOPC_Free(*port);
        *hostname = NULL;
        *port = NULL;
    }
    return (result);
}
