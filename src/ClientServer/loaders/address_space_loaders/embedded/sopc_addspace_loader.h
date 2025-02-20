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

#ifndef SOPC_ADDSPACE_LOADER_H_
#define SOPC_ADDSPACE_LOADER_H_

#include "sopc_address_space.h"

/**
 * \brief Load the embedded address space defined as a C structure.
 *        If the C structure is defined as a const address space,
 *        \p allocNodes should be set to false and will be ignored.
 *
 * \param allocNodes true if the nodes shall be allocated and copied to fill the address space, false otherwise.
 *
 * \return The loaded address space in case of success, NULL otherwise.
 *
 * \note \p allocNodes shall be set to true to allocate and copy the nodes,
 *       it makes then possible to use AddNode and DeleteNode services when S2OPC_NODE_MANAGEMENT is defined.
 *
 * \warning If the C structure is defined as a const address space, \p allocNodes will be ignored.
 */
SOPC_AddressSpace* SOPC_Embedded_AddressSpace_LoadWithAlloc(bool allocNodes);

/**
 * \deprecated This function is deprecated and will be removed in future releases.
 *             Use SOPC_Embedded_AddressSpace_LoadWithAlloc(false) instead for same behavior.
 *
 * \brief Load the embedded address space defined as a C structure.
 *        The address space resulting uses the nodes of the provided structure
 *        without copying them even when those are not const.
 *        As a result AddNode and DeleteNode services will not be available.
 *
 * \return The loaded address space in case of success, NULL otherwise.
 */
SOPC_AddressSpace* SOPC_Embedded_AddressSpace_Load(void);

#endif /* SOPC_ADDSPACE_LOADER_H_ */
