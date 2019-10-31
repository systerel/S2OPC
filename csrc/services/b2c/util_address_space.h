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

#ifndef UTIL_ADDRESS_SPACE_H_
#define UTIL_ADDRESS_SPACE_H_

bool is_type_definition(const OpcUa_ReferenceNode* ref);
bool is_component(const OpcUa_ReferenceNode* ref);
bool is_property(const OpcUa_ReferenceNode* ref);

void util_get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                             constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def);

#endif /* UTIL_ADDRESS_SPACE_H_ */
