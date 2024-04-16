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
 * \brief Contains the interface to configure events in the server.
 *        This interface is intended to be used by the server frontend wrapper only.
 */

#ifndef SOPC_EVENT_MANAGER_H_
#define SOPC_EVENT_MANAGER_H_

#include "sopc_address_space.h"
#include "sopc_event.h"

/**
 * \brief OPC UA server events types configuration
 */
typedef SOPC_Dict SOPC_Server_Event_Types;

/**
 * \brief Create the server event types based on given server address space.
 *        The BaseEventType is used to find and create reference event types that will be allowed to
 *        be instantiated using ::SOPC_EventManager_CreateEventInstance.
 *        BaseEventTypes and all subtypes of BaseEventTypes are then instantiable.
 *        Each event variable of each type are referenced and could be modified using
 *        ::SOPC_Event_SetVariable or ::SOPC_Event_SetVariableFromStrPath.
 *
 * \note If BaseEventType cannot be found, the BaseEventType of 1.04 Nodeset is used as referenced
 *       and is the only event type available for instantiation.
 *
 * \param addSpace            The server address space containing the reference event types for events instantiation.
 * \param[out] outEventTypes  The reference event types created by this function in case of success.
 *
 * \return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus SOPC_EventManager_CreateEventTypes(SOPC_AddressSpace* addSpace,
                                                     SOPC_Server_Event_Types** outEventTypes);

/**
 *  \brief Check if the given event type id is configured in the server event types provided
 *
 *  \param eventTypes   The reference event types that might contain the event type corresponding to the given \p
 *                      eventTypeId
 *  \param eventTypeId  The NodeId of the event type to instantiate
 *
 *  \return true if \p eventTypes contains \p eventTypeId, false otherwise (or in case of invalid parameter)
 */
bool SOPC_EventManager_HasEventType(const SOPC_Server_Event_Types* eventTypes, const SOPC_NodeId* eventTypeId);

/**
 *  \brief Check both if the given event type id is configured in the server event types
 *         and if the associated browse path is valid for this event type
 *
 *  \param eventTypes   The reference event types that might contain the event type corresponding to the given \p
 *                      eventTypeId
 *  \param eventTypeId  The NodeId of the event type to instantiate
 *
 *  \param nbQNamePath  The number of qualified names in the relative browse path to a node
 *
 *  \param qNamePath    The array of \p nbQNamePath qualified names in the relative browse path to a node
 *
 *  \return SOPC_STATUS_OK in case of success, otherwise
 *          SOPC_STATUS_INVALID_PARAMETERS (invalid parameters or NULL browse name)
 *          or SOPC_STATUS_NOK (absent event type or browse path)
 */
SOPC_ReturnStatus SOPC_EventManager_HasEventTypeAndBrowsePath(const SOPC_Server_Event_Types* eventTypes,
                                                              const SOPC_NodeId* eventTypeId,
                                                              int32_t nbQNamePath,
                                                              const SOPC_QualifiedName* qNamePath);

/**
 *  \brief Create an event instance based on the given reference event types.
 *         The EventId value is set to a unique value by this function prior to returning the new event instance.
 *         Caller is responsible for event deallocation.
 *
 *  \param eventTypes   The reference event types that shall contain the event type corresponding to the given \p
 *                      eventTypeId
 *  \param eventTypeId  The NodeId of the event type to instantiate
 *
 *  \return The created event instance or NULL in case of error
 *
 */
SOPC_Event* SOPC_EventManager_CreateEventInstance(const SOPC_Server_Event_Types* eventTypes,
                                                  const SOPC_NodeId* eventTypeId);

/**
 * \brief Clear the given event types content, the pointed configuration is deallocated and pointer is set to NULL.
 *
 * \param eventTypes  The event types configuration to delete
 */
void SOPC_EventManager_Delete(SOPC_Server_Event_Types** eventTypes);

/**
 * \brief Utility function to generate a C string version of a browse path composed of qualified names
 *        and separated by '~':
 *        "<SOPC_QualifiedName_ToCString(qNamePath[0])>~<SOPC_QualifiedName_ToCString(qNamePath[1])>~[...]<SOPC_QualifiedName_ToCString(qNamePath[nbQnPath-1])>"
 *
 *
 * \param      nbQnPath   Number of qualified names in the browse path contained in \p qNamePath
 * \param      qNamePath  Array of qualified names of length \p qNamePath representing the browse path
 * \param[out] qnPathStr  The resulting C string representing the browse path as a string
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS (invalid path size or empty QualifiedName, etc.)
 *         or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_EventManagerUtil_QnPathToCString(uint16_t nbQnPath,
                                                        const SOPC_QualifiedName* qNamePath,
                                                        char** qnPathStr);

/**
 * \brief Utility function to parse a C string version of a browse path composed of qualified names
 *        and separated by '~':
 *        "<SOPC_QualifiedName_ToCString(qNamePath[0])>~<SOPC_QualifiedName_ToCString(qNamePath[1])>~[...]<SOPC_QualifiedName_ToCString(qNamePath[nbQnPath-1])>"
 *
 * \param qnPathSep       The character to use as separator between path elements in \p qnPathStr, '\0' is forbidden
 * \param qnPathStr       The qualified name path separated by \p qnPathSep separator as a string.
 *                        E.g. with sep='~' for path qn0=(nsIdx=0,"EnabledState"), qn1=(0,"Id"):
 *                        "0:EnabledState~0:Id".
 *                        The escape character '\' might be used to un-specialize separator
 * \param[out] nbQnPath   Number of qualified names in the browse path contained in \p qNamePath
 * \param[out] qNamePath  Array of qualified names of length \p qNamePath representing the browse path
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_PARAMETERS (invalid path size or NULL input, etc.)
 *         or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_EventManagerUtil_cStringPathToQnPath(char qnPathSep,
                                                            const char* qnPathStr,
                                                            int32_t* nbQnPath,
                                                            SOPC_QualifiedName** qNamePath);

#endif // SOPC_EVENT_MANAGER_H_
