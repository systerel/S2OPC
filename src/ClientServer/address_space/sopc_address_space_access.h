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
 * \brief Add a Variable node into the AddressSpace with given attributes and references to its parent and type.
 *
 * \param addSpaceAccess  The AddressSpace Access used for write operation
 * \param parentNodeId    The ExpandedNodeId of the parent node in AddressSpace for the variable to add.
 *                        Only "local" NodeId are supported for now (ServerIndex shall be 0 and NamespaceUri ignored).
 *                        Parent node characteristics shall be compliant to add the new Variable node as a child.
 *                        See OPC UA specifications part 3 for constraints and returned error code for details.
 * \param refTypeId       The NodeId of the reference type used for reference between parent node and new variable node.
 *                        E.g. Organizes, HasComponent, etc.
 *                        ReferenceType shall be compliant with parent node and variable node characteristics.
 *                        See OPC UA specifications part 3 for constraints and returned error code for details.
 * \param newNodeId       The fresh NodeId for the new variable to add. It shall not already exist in AddressSpace.
 * \param browseName      The QualifiedName used when browsing the AddressSpace for the new variable node.
 *                        It shall be unique in the parent node children.
 * \param varAttributes   The attributes defined for the new variable node.
 *                        The following attributes combination are not supported and will make addition fail:
 *                        WriteMask or UserWriteMask,
 *                        UserAccessLevel,
 *                        NoOfArrayDimensions without ArrayDimensions,
 *                        Historizing = true,
 *                        MinimumSamplingInterval != 0,
 *
 * \param typeDefId       The ExpandedNodeId of the type definition node in AddressSpace for the variable to add.
 *                        E.g. BaseDataVariable, PropertyType, etc.
 *                        Only "local" NodeId are supported for now (ServerIndex shall be 0 and NamespaceUri ignored).
 *                        Type characteristics shall be compliant to add the Variable into the parent node indicated.
 *                        See OPC UA specifications part 3 for constraints and returned error code for details.
 *
 * \return SOPC_GoodGenericStatus in case of success, otherwise:
 *         - OpcUa_BadInvalidArgument: if provided parameters are invalid (NULL)
 *         - OpcUa_BadServiceUnsupported: if the AddressSpace does not support to add Variable node dynamically.
 *                                        Note: XML loaded AddressSpace supports this operation.
 *         - OpcUa_BadNodeIdExists: if \p newNodeId already exists in AddressSpace
 *         - OpcUa_BadParentNodeIdInvalid: if \p parentNodeId is unknown
 *         - OpcUa_BadReferenceNotAllowed: the \p refTypeId (Organizes, HasComponent, etc.) is not compliant regarding
 *                                         the parent node characteristics.
 *                                         See OPC UA specifications part 3 for constraints
 *                                         and logs for detail in case of error.
 *         - OpcUa_BadTypeDefinitionInvalid: the \p typeDefId (BaseDataVariable, PropertyType, etc.) is unknown
 *                                           or is not compliant regarding the Variable characteristics and
 *                                           its relation to parent node.
                                             See OPC UA specifications part 3 for constraints
 *                                           and logs for detail in case of error.
 *         - OpcUa_BadBrowseNameDuplicated: if \p browseName is not unique in parent node.
 *         - OpcUa_BadNodeAttributesInvalid: if \p varAttributes contains unsupported attributes.
 *                                           See logs for detail in case of error.
 *         - OpcUa_BadOutOfMemory: if an allocation failed during add node operation
 *
 * \note: Add variable node operation includes creation of mutual references with parent / type.
 *
 * \warning Children of the variable that might declared as mandatory in the \p typeDefId
 *          are not generated automatically by this operation for now.
 */
SOPC_StatusCode SOPC_AddressSpaceAccess_AddVariableNode(SOPC_AddressSpaceAccess* addSpaceAccess,
                                                        const SOPC_ExpandedNodeId* parentNodeId,
                                                        const SOPC_NodeId* refTypeId,
                                                        const SOPC_NodeId* newNodeId,
                                                        const SOPC_QualifiedName* browseName,
                                                        const OpcUa_VariableAttributes* varAttributes,
                                                        const SOPC_ExpandedNodeId* typeDefId);

#endif /* SOPC_ADDRESS_SPACE_ACCESS_H_ */
