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

#ifndef SOPC_ADDRESS_SPACE_ACCESS_H_
#define SOPC_ADDRESS_SPACE_ACCESS_H_

#include <stdbool.h>

#include "sopc_builtintypes.h"
#include "sopc_types.h"

/**
 * \brief AddressSpace Access module provides controlled access to address space.
 *        This might be used to access address space during configuration phase
 *        or during a method call.
 *
 * \note  AddressSpace is managed by services layer and shall not be accessed unless
 *        an AddressSpaceAccess instance is provided by the S2OPC API.
 *
 * \warning AddressSpaceAccess operations are not thread-safe,
 *          concurrent access shall be managed by caller if caller want to do some.
 */

typedef struct _SOPC_AddressSpaceAccess SOPC_AddressSpaceAccess;

/**
 * \brief Read an attribute and retrieve its value as a Variant.
 *
 * \param addSpaceAccess  The AddressSpace Access used for read operation
 * \param nodeId          The NodeId of a node in the AddressSpace
 * \param attribId        The AttributeId to read in the node
 * \param[out] outValue   The pointer in which the Variant containing the read result will be returned
 *
 * \return SOPC_GoodGenericStatus in case of success with an allocated \p outValue, otherwise:
 *         - OpcUa_BadInvalidArgument: if provided parameters are invalid (NULL)
 *         - OpcUa_BadNodeIdUnknown: if provided \p nodeId is not present in AddressSpace
 *         - OpcUa_BadNotImplemented: if the requested attribute is not implemented
 *                                    (it might also be invalid for concerned node in this case)
 *         - OpcUa_BadOutOfMemory: if Variant allocation failed
 *
 * \warning The following attributes are not supported and will lead to return an OpcUa_BadNotImplemented status:
 *          - ContainsNoLoops
 *          - InverseName
 *          - Symmetric
 *          - EventNotifier
 *          - MinimumSamplingInterval
 *          - Historizing
 *          - UserAccessLevel
 *          - UserExecutable
 *          User related attributes will never be provided since behavior is dynamic.
 *
 * \warning For supported attributes, accessing an invalid attribute for the class of the corresponding node
 *          will lead to raise an assertion.
 *          TODO: returns OpcUa_BadInvalidAttributeId !!!
 */
SOPC_StatusCode SOPC_AddressSpaceAccess_ReadAttribute(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                      const SOPC_NodeId* nodeId,
                                                      SOPC_AttributeId attribId,
                                                      SOPC_Variant** outValue);

/**
 * \brief Read Value attribute content with Status and Source Timestamp metadata.
 *
 * \param addSpaceAccess     The AddressSpace Access used for read operation
 * \param nodeId             The NodeId of a Variable/VariableType node in the AddressSpace
 * \param optNumRange        (Optional) The numeric range to use to read value, it shall be NULL if no range requested.
 * \param[out] outDataValue  The pointer in which the DataValue containing the read Value result will be returned.
 *                           It contains the Value as a Variant and the associated StatusCode and Source Timestamp.
 *
 * \return SOPC_GoodGenericStatus in case of success with an allocated \p outValue, otherwise:
 *         - OpcUa_BadInvalidArgument: if provided parameters are invalid (NULL except if optional)
 *         - OpcUa_BadNodeIdUnknown: if provided \p nodeId is not present in AddressSpace
 *         - OpcUa_BadNotImplemented: if the requested attribute is not implemented
 *                                    (it might also be invalid for concerned node in this case)
 *         - OpcUa_BadOutOfMemory: if DataValue/Variant allocation failed
 *         - OpcUa_BadIndexRangeInvalid: if numeric range provided is invalid
 *         - OpcUa_BadIndexRangeNoData: if there is no data for the numeric range provided
 *
 * \note Server Timestamp is never set in returned value since current date on read is used for OPC UA service.
 */
SOPC_StatusCode SOPC_AddressSpaceAccess_ReadValue(const SOPC_AddressSpaceAccess* addSpaceAccess,
                                                  const SOPC_NodeId* nodeId,
                                                  const SOPC_NumericRange* optNumRange,
                                                  SOPC_DataValue** outDataValue);

/**
 * \brief Write Value attribute content with Status and Source Timestamp metadata.
 *
 * \param addSpaceAccess       The AddressSpace Access used for write operation
 * \param nodeId               The NodeId of a Variable/VariableType node in the AddressSpace
 * \param optNumRange          (Optional) The numeric range to write in the targeted node with the given value,
 *                                        it shall be NULL if no range requested (complete write of target value).
 * \param value                The value to write into the Variable/VariableType node.
 * \param optStatus            (Optional) The status code to associate with the value written,
 *                                        NULL if previous value shall be kept.
 * \param optSourceTimestamp   (Optional) The source timestamp as OPC UA DateTime to associate with the value written,
 *                                        NULL if previous source timestamp shall be kept,
 *                                        0 if the current time shall be set.
 * \param optSourcePicoSeconds (Optional) The source timestamp picoseconds part to associate with the value written,
 *                                        it shall be NULL if \p optSourceTimestamp is NULL,
 *                                        it might be NULL or set to 0 if the current time shall be set.
 *
 * \return SOPC_GoodGenericStatus in case of success, otherwise:
 *         - OpcUa_BadInvalidArgument: if provided parameters are invalid (NULL, incoherent parameters)
 *         - OpcUa_BadNodeIdUnknown: if provided \p nodeId is not present in AddressSpace
 *         - OpcUa_BadWriteNotSupported: if status code and/or timestamp provided but are read-only in AddressSpace
 *         - OpcUa_BadNotImplemented: if the requested attribute is not implemented
 *                                    (it might also be invalid for concerned node in this case)
 *         - OpcUa_BadOutOfMemory: if an allocation failed during write operation
 *         - OpcUa_BadIndexRangeInvalid: if numeric range provided is invalid
 *         - OpcUa_BadIndexRangeNoData: if there is no data for the numeric range provided
 *
 *
 * \warning No typechecking is implemented in this operation for now (value write will succeed),
 *          it is responsibility of caller to check the Value complies with the Variable DataType.
 */
SOPC_StatusCode SOPC_AddressSpaceAccess_WriteValue(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                   const SOPC_NodeId* nodeId,
                                                   const SOPC_NumericRange* optNumRange,
                                                   const SOPC_Variant* value,
                                                   const SOPC_StatusCode* optStatus,
                                                   const SOPC_DateTime* optSourceTimestamp,
                                                   const uint16_t* optSourcePicoSeconds);

/**
 * \note: Add variable node operation includes creation of mutual references with parent / type
 *        and might include type sub-nodes creation
 */
SOPC_StatusCode SOPC_AddressSpaceAccess_AddVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                        const SOPC_ExpandedNodeId* parentNodeId,
                                                        const SOPC_NodeId* refTypeId,
                                                        const SOPC_NodeId* newNodeId,
                                                        const SOPC_QualifiedName* browseName,
                                                        const OpcUa_VariableAttributes* varAttributes,
                                                        const SOPC_ExpandedNodeId* typeDefId);

#endif /* SOPC_ADDRESS_SPACE_ACCESS_H_ */
