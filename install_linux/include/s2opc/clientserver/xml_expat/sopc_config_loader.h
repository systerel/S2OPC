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

#ifndef SOPC_CONFIG_LOADER_H_
#define SOPC_CONFIG_LOADER_H_

#include <stdio.h>

#include "sopc_user_app_itf.h"

/* Parse the XML configuration file
 *
 * \param fd      Path to XML file compliant with s2opc_clientserver_config.xsd schema
 * \param config  (ouput) Server configuration structure to be filled
 *
 * \return        true if the parsing succeeded, false otherwise
 * */
bool SOPC_Config_Parse(FILE* fd, SOPC_S2OPC_Config* config);

#endif /* SOPC_CONFIG_LOADER_H_ */
