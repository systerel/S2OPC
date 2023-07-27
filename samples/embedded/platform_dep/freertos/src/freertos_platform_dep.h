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

#ifndef FREE_RTOS_PLATFORM_DEP_H
#define FREE_RTOS_PLATFORM_DEP_H

const char* get_IP_str(void);
const char* get_EP_str(void);

#define CONFIG_SOPC_ENDPOINT_ADDRESS get_EP_str()

#endif //  FREE_RTOS_PLATFORM_DEP_H
