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

/**
 *
 * \brief Holds the version of the toolkit.
 *
 */

#ifndef SOPC_VERSION_H_
#define SOPC_VERSION_H_

/** @brief Version of the toolkit */
#define SOPC_TOOLKIT_VERSION_MAJOR 1
#define SOPC_TOOLKIT_VERSION_MEDIUM 5
#define SOPC_TOOLKIT_VERSION_MINOR 0

#define SOPC_STRINGIFY(x) #x
#define SOPC_QUOTE(x) SOPC_STRINGIFY(x)

#define SOPC_TOOLKIT_VERSION               \
    SOPC_QUOTE(SOPC_TOOLKIT_VERSION_MAJOR) \
    "." SOPC_QUOTE(SOPC_TOOLKIT_VERSION_MEDIUM) "." SOPC_QUOTE(SOPC_TOOLKIT_VERSION_MINOR) "*"

#endif /* SOPC_VERSION_H_ */
