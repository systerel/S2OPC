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

#ifndef XML_EXPAT_H_
#define XML_EXPAT_H_

#include <stdio.h>

#include "sopc_pubsub_conf.h"

/**
 *  /brief
 *      Reads the content of a XML file and extracts the ::SOPC_PubSubConfiguration
 *  /param fd
 *      A opened non-NULL file descriptor. (Text file, validating the XSD schema
 *      's2opc_pubsub_config.xsd'
 *  /return The PubSub configuration, or NULL in case of invalid parameter (or invalid file content)
 */
SOPC_PubSubConfiguration* SOPC_PubSubConfig_ParseXML(FILE* fd);

#endif /* XML_EXPAT_H_ */
