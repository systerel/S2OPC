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

#ifndef SOPC_ADDRESS_SPACE_ACCESS_INTERNAL_H_
#define SOPC_ADDRESS_SPACE_ACCESS_INTERNAL_H_

#include "sopc_address_space.h"
#include "sopc_address_space_access.h"

/**
 *
 * \privatesection
 *
 * \brief Internal API for AddressSpaceAccess
 *
 */

/**
 * \brief Create an AddressSpaceAccess from the given AddressSpace with a flag indicating
 *        if the (write) operations should be recorded.
 * \param addSpaceRef       The AddressSpace for which an AddressSpaceAccess will be created
 * \param recordOperations  Flag indicating if the (write) operations on address space should be recorded.
 *                          The recorded operations can be used to create Data/Node changes
 *                          necessary for subscription notifications.
 * \return the AddressSpaceAccess created
 */
SOPC_AddressSpaceAccess* SOPC_AddressSpaceAccess_Create(SOPC_AddressSpace* addSpaceRef, bool recordOperations);

/* \brief Get the operations that occurred during AddressSpaceAccess lifetime if recordOperations was set.
 *        It shall be called just prior to ::SOPC_AddressSpaceAccess_Delete and no further operations shall be done.
 *
 * \warning It is responsibility of caller to deallocate content and list once called.
 *
 * \return A SLinkedList containing prepended SOPC_AddressSpaceAccessOperation* operations since access creation.
 *         NULL if recordOperations == false or operations == NULL, operations becomes NULL after this call
 */
SOPC_SLinkedList* SOPC_AddressSpaceAccess_GetOperations(SOPC_AddressSpaceAccess* addSpaceAccess);

/**
 * \brief Delete an AddressSpaceAccess previously created with ::SOPC_AddressSpaceAccess_Create
 * \param addSpaceAccess    Pointer to the allocated AddressSpaceAccess to delete.
 *                          The remaining recorded operations are deleted.
 *                          Content will be set to NULL after call.
 */
void SOPC_AddressSpaceAccess_Delete(SOPC_AddressSpaceAccess** addSpaceAccess);

typedef enum
{
    SOPC_ADDSPACE_WRITE,      /* param1 = old OpcUa_WriteValue, param2 = new OpcUa_WriteValue * */
    SOPC_ADDSPACE_CHANGE_NODE /* param1 = bool (true if added, false if removed); param2 = SOPC_NodeId* */
} SOPC_AddressSpaceAccessOperationEnum;

typedef struct _SOPC_AddressSpaceAccessOperation
{
    SOPC_AddressSpaceAccessOperationEnum operation;
    void* param1;
    void* param2;
} SOPC_AddressSpaceAccessOperation;

#endif /* SOPC_ADDRESS_SPACE_ACCESS_INTERNAL_H_ */
