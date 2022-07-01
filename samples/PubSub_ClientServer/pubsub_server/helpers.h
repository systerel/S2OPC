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

#ifndef HELPERS_H_
#define HELPERS_H_

#include "sopc_builtintypes.h"

typedef void Helpers_WriteValue_Callback(OpcUa_WriteValue* pwv);

/**
 * Copy the NodeIds, the AttributeIds, and the DataValues in an array of WriteValues,
 * which is then encapsulated in an OpcUa_WriteRequest,
 * which is finally sent to the toolkit.
 *
 * wvNotifier is (sort-of) compatible with Address Space notification callback and can be NULL.
 * It is called on each WriteValue of the WriteRequest.
 * It is called *before* the WriteRequest is processed by the toolkit.
 */
SOPC_ReturnStatus Helpers_AsyncLocalWrite(uint32_t endpointConfigIdx,
                                          SOPC_NodeId** lNid,
                                          uint32_t* lAttrId,
                                          SOPC_DataValue** lpDv,
                                          size_t nItems,
                                          Helpers_WriteValue_Callback* wvNotifier);

#endif /* HELPERS_H_ */
