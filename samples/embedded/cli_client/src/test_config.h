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

/** \file
 *
 * \brief Provides a cache for data values.
 *
 * This cached is based on a SOPC_Dict whose keys are SOPC_NodeId and values are SOPC_DataVariant.
 * Functions Cache_GetSourceVariables and Cache_SetTargetVariables are callback that interface PubSub with the cache.
 *
 * For now, this cache uses a single Mutex to assert that only one thread writes or read.
 * This could be enhanced with the use of a read/write double buffer for instance.
 */

#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_

/***************************************************/
/**              DEMO CONFIGURATION                */
/***************************************************/
// The server endpoint address
#ifndef CONFIG_SOPC_ENDPOINT_ADDRESS
#define CONFIG_SOPC_ENDPOINT_ADDRESS "opc.tcp://localhost:4841"
#endif

/***************************************************/
/**              CONFIGURATION FOR READ WRITE      */
/***************************************************/
#define SAFE_STRING(s) ((NULL == (s)) ? "<NULL>" : (s))

#ifndef MAX_CONFIG
#define MAX_CONFIG 16
#endif
#endif /* TEST_CONFIG_H_ */
