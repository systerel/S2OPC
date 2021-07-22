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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/
/** \file Provides a cache for data values.
 *
 * GetSource and SetTarget callback will get and set their values in the cache.
 */

#ifndef SOPC_UAM_CACHE_H_
#define SOPC_UAM_CACHE_H_

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/** Initializes the cache from the configuration using default zero-values (or null strings)
 *  */
bool UAM_Cache_Initialize(SOPC_PubSubConfiguration* config);

/**
 * \brief Get a value in the cache
 *
 * \warning The returned pointer is a direct access to the cached value and should not be freed
 *
 * \return A pointer to the value in cache, or NULL if not found
 */
SOPC_DataValue* UAM_Cache_Get(const SOPC_NodeId* nid);

/**
 * \brief Set a value in the cache
 *
 * \warning The ownership of given \p dv is taken by the cache, so you should not reuse or free it
 */
bool UAM_Cache_Set(SOPC_NodeId* nid, SOPC_DataValue* dv);

/**
 * \brief Show content of cache
 */
void UAM_Cache_List(void);

/**
 * \brief Show a spcific entry of the cache. Nothing is printed out if the entry does not exist
 */
void UAM_Cache_Show(const SOPC_NodeId* nid);

/*
 * \brief S2OPC calls this function when some NodeId have to be read from the cache.
 * \param  nodesToRead The list of NodeId to be read. They are freed by this call.
 * \param  nbValues The number of elements in nodesToRead.
 * \return An array of DataValue matching the array of NodeId in input. They shall be freed by the caller
 * */
SOPC_DataValue* UAM_Cache_GetSourceVariables(OpcUa_ReadValueId* nodesToRead, int32_t nbValues);

/*
 * \brief S2OPC calls this function when some NodeId have to be written into the cache.
 * \param  nodesToWrite The list of Nodes to be written. They are freed by this call.
 * \param  nbValues The number of elements in nodesToWrite.
 * \return True in case of success.
 * */
bool UAM_Cache_SetTargetVariables(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);

/** The Cache shall be locked before accessing data (and its content, to prevent it from being freed) */
void UAM_Cache_Lock(void);
void UAM_Cache_Unlock(void);

/**
 * TODO
 */
typedef void (*UAM_Cache_Notify_CB)(const SOPC_NodeId* const pNid, const SOPC_DataValue* const pDv);

/**
 * TODO
 */
void UAM_Cache_SetNotify(UAM_Cache_Notify_CB pfNotify);

/**
 * Module cleanup function. Shall be called before software exit
 */
void UAM_Cache_Clear(void);

#endif /* SOPC_UAM_CACHE_H_ */
