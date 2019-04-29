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

#include "sopc_helper_uri.h"

#include <stdint.h>
#include <string.h>

#include "sopc_helper_string.h"

bool SOPC_Helper_URI_ParseTcpUaUri(const char* uri, size_t* hostnameLength, size_t* portIdx, size_t* portLength)
{
    bool result = false;
    size_t idx = 0;
    bool isPort = false;
    bool endOfPort = false;
    bool hasPort = false;
    bool hasName = false;
    bool invalid = false;
    bool startIPv6 = false;
    if (uri != NULL && hostnameLength != NULL && portLength != NULL)
    {
        *hostnameLength = 0;
        *portIdx = 0;
        *portLength = 0;
        if (strlen(uri) + 4 > TCP_UA_MAX_URL_LENGTH)
        {
            // Encoded value shall be less than 4096 bytes
        }
        else if (strlen(uri) > 10 && SOPC_strncmp_ignore_case(uri, (const char*) "opc.tcp://", 10) == 0)
        {
            // search for a ':' defining port for given IP
            // search for a '/' defining endpoint name for given IP => at least 1 char after it (len - 1)
            for (idx = 10; idx < strlen(uri) - 1; idx++)
            {
                if (false != isPort && false == endOfPort)
                {
                    if (uri[idx] >= '0' && uri[idx] <= '9')
                    {
                        if (false == hasPort)
                        {
                            // port definition
                            hasPort = true;
                            *portIdx = idx;
                        }
                    }
                    else if (uri[idx] == '/' && false == invalid)
                    {
                        // Name of the endpoint after port, invalid otherwise
                        if (false == hasPort)
                        {
                            invalid = true;
                        }
                        else
                        {
                            *portLength = idx - *portIdx;
                            hasName = true;
                            endOfPort = true; // End of port definition
                        }
                    }
                    else
                    {
                        if (false == hasPort || false == hasName)
                        {
                            // unexpected character: we do not expect a endpoint name
                            invalid = true;
                        }
                    }
                }
                else
                {
                    if (false == endOfPort)
                    {
                        // Treatment before the port parsing
                        if (uri[idx] == ':' && false == startIPv6)
                        {
                            *hostnameLength = idx - 10;
                            isPort = true;
                        }
                        else if (uri[idx] == '[')
                        {
                            startIPv6 = true;
                        }
                        else if (uri[idx] == ']')
                        {
                            if (false == startIPv6)
                            {
                                invalid = true;
                            }
                            else
                            {
                                startIPv6 = false;
                            }
                        }
                    }
                    else if (hasPort)
                    {
                        // Treatment after the port parsing
                        // TODO: check absence of forbidden characters
                    }
                }
            }

            if (hasPort != false && false == invalid)
            {
                result = true;
                if (*portLength == 0)
                {
                    // No endpoint name after port provided
                    *portLength = idx - *portIdx + 1;
                }
            }
        }
    }

    return result;
}

bool ParseURI(const char* uri, char** hostname, char** port)
{
    bool result = false;
    size_t hostnameLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;
    char* lHostname = NULL;
    char* lPort = NULL;

    if (uri != NULL && hostname != NULL && port != NULL)
    {
        result = SOPC_Helper_URI_ParseTcpUaUri(uri, &hostnameLength, &portIdx, &portLength);
    }

    if (result != false)
    {
        if (portIdx != 0 && hostnameLength != 0 && portLength != 0)
        {
            lHostname = calloc(1u + hostnameLength, sizeof(char));
            if (NULL == lHostname)
                return false;
            if (lHostname != memcpy(lHostname, &(uri[10]), hostnameLength))
            {
                free(lHostname);
                return false;
            }
            lHostname[hostnameLength] = '\0';

            lPort = calloc(1u + portLength, sizeof(char));
            if (NULL == lPort)
            {
                free(lHostname);
                return false;
            }
            if (lPort != memcpy(lPort, &(uri[portIdx]), portLength))
            {
                free(lHostname);
                free(lPort);
                return false;
            }
            lPort[portLength] = '\0';
            *hostname = lHostname;
            *port = lPort;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

