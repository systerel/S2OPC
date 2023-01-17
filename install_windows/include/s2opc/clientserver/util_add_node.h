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

#ifndef UTIL_ADD_NODE_H_
#define UTIL_ADD_NODE_H_

#include "constants_statuscodes_bs.h"

void util_add_node__check_constraints_addNode_AddressSpace_Variable(
    const SOPC_NodeId* parentNid,
    const SOPC_NodeId* refTypeId,
    const SOPC_QualifiedName* browseName,
    const SOPC_NodeId* typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* sc_addnode);

SOPC_ReturnStatus util_add_node__AddVariableNodeAttributes(SOPC_AddressSpace_Node* node,
                                                           OpcUa_VariableNode* varNode,
                                                           const SOPC_ExtensionObject* nodeAttributes,
                                                           constants_statuscodes_bs__t_StatusCode_i* sc_addnode);

SOPC_ReturnStatus util_add_node__AddRefChildToParentNode(const SOPC_NodeId* parentNodeId,
                                                         const SOPC_NodeId* childNodeId,
                                                         const SOPC_NodeId* refTypeId);

void util_add_node__RemLastRefInParentNode(const SOPC_NodeId* parentNodeId);

#endif /* UTIL_ADD_NODE_H_ */
