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
 * Utils to add node in address space
 */

#ifndef SOPC_NODE_MGT_HELPER_INTERNAL_H_
#define SOPC_NODE_MGT_HELPER_INTERNAL_H_

#include "sopc_address_space.h"
#include "sopc_builtintypes.h"
#include "sopc_types.h"

/* NodeClass handled by the function: Variable and Object */
SOPC_StatusCode SOPC_NodeMgtHelperInternal_CheckConstraints_AddNode(OpcUa_NodeClass nodeclass,
                                                                    SOPC_AddressSpace* addSpace,
                                                                    const SOPC_ExpandedNodeId* parentNid,
                                                                    const SOPC_NodeId* refTypeId,
                                                                    const SOPC_QualifiedName* browseName,
                                                                    const SOPC_ExpandedNodeId* typeDefId);

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes(SOPC_AddressSpace* addSpace,
                                                                       SOPC_AddressSpace_Node* node,
                                                                       OpcUa_VariableNode* varNode,
                                                                       const OpcUa_VariableAttributes* varAttributes,
                                                                       SOPC_StatusCode* scAddNode);

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddObjectNodeAttributes(OpcUa_ObjectNode* objNode,
                                                                     const OpcUa_ObjectAttributes* objAttributes,
                                                                     SOPC_StatusCode* scAddNode);

SOPC_StatusCode SOPC_NodeMgtHelperInternal_CopyDataInNode(OpcUa_Node* node,
                                                          const SOPC_ExpandedNodeId* parentNodeId,
                                                          const SOPC_NodeId* newNodeId,
                                                          const SOPC_NodeId* refTypeId,
                                                          const SOPC_QualifiedName* browseName,
                                                          const SOPC_ExpandedNodeId* typeDefId);

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddRefChildToParentNode(SOPC_AddressSpace* addSpace,
                                                                     const SOPC_NodeId* parentNodeId,
                                                                     const SOPC_NodeId* childNodeId,
                                                                     const SOPC_NodeId* refTypeId);

bool SOPC_NodeMgtHelperInternal_RemoveLastRefInParentNode(SOPC_AddressSpace* addSpace, const SOPC_NodeId* parentNodeId);

#endif /* SOPC_NODE_MGT_HELPER_INTERNAL_H_ */
