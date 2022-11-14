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
 * \brief Provides a synchronization mechanism between cache and AddressSpace
 * There are different ways to provide this mechanism, and they can be
 * switched using the CONFIG_CACHE_SYNCH or CONFIG_CACHE_ASYNCH parameter
 */

#ifndef CACHE_SYNCH_H_
#define CACHE_SYNCH_H_

#include <stdbool.h>

// S2OPC includes
#include "sopc_builtintypes.h"
#include "sopc_types.h"

#if CONFIG_DEMO_CACHE_SYNCH
#error "Not implemented"
#else

#define ASYNCH_CONTEXT_CACHE_SYNC 0xCAC3E51Cu

/**
 * @brief Event called when the subscriber receives new values
 * @param nodesToWrite An array of write events
 * @param nbValues The number of elements in nodesToWrite
 */
bool cacheSync_SetTargetVariables(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);

/**
 * @brief Provide a new value to store in the PubSubCache
 * This function should be called each time the Server AddressSpace is modified

 * @param pNid The modified NodeId in AddressSpace
 * @param pDv The new value
 */
void cacheSync_WriteToCache(const SOPC_NodeId* pNid, const SOPC_DataValue* pDv);

#endif

#endif // CACHE_SYNCH_H_
