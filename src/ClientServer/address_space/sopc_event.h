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
 * \brief Interface to manipulate and customize OPC UA event instances.
 *
 *        The initial instance should be created using ::SOPC_ServerHelper_CreateEvent
 *        using an existing event type id in the configured address space.
 *        Once the event has been customized, it might be triggered using ::SOPC_ServerHelper_TriggerEvent.
 *
 *        This module provides dedicated functions to access the variables of base event type easily,
 *        including EventId, EventType, Time, Severity and Source.
 *        For other variables of known event subtypes, the generic access functions should be used
 *        to set variable value (::SOPC_Event_SetVariable or ::SOPC_Event_SetVariableFromStrPath)
 *        or to get variable value (::SOPC_Event_GetVariableAndType or ::SOPC_Event_GetVariableAndTypeFromStrPath).
 *
 * \note It is only possible to access variables defined in the event type of the event instance,
 *       otherwise access will fail when using generic access functions.
 */

#ifndef SOPC_EVENT_H_
#define SOPC_EVENT_H_

#include "sopc_types.h"

/**
 * \brief The abstract structure type for an OpcUa event instance that might be triggered from nodes.
 */
typedef struct _SOPC_Event SOPC_Event;

/**
 * \brief Clears the content of an Event
 *
 * \param pEvent pointer to the event to clear
 */
void SOPC_Event_Clear(SOPC_Event* pEvent);

/**
 * \brief Deletes the content of an Event
 *
 * \param ppEvent reference on the pointer to the event to clear
 */
void SOPC_Event_Delete(SOPC_Event** ppEvent);

/**
 * \brief Copies the provided event in a newly created event
 *
 * \param pEvent    pointer to the event to copy
 * \param genNewId  flag to set to indicate a new eventId shall be generated
 *
 * \return a newly allocated event which is a copy of the provided \p pEvent
 *         or NULL in case of invalid parameter or allocation failure.
 */
SOPC_Event* SOPC_Event_CreateCopy(const SOPC_Event* pEvent, bool genNewId);

/**
 * \brief Get the EventTypeId for the given event
 *
 * \param pEvent pointer to the event from which EventTypeId shall be retrieved
 *
 * \return the event type NodeId or NULL in case of error
 */
const SOPC_NodeId* SOPC_Event_GetEventTypeId(const SOPC_Event* pEvent);

/**
 * \brief Get the EventId for the given event
 *
 * \param pEvent pointer to the event from which EventId shall be retrieved
 *
 * \return the event type NodeId or NULL in case of error
 */
const SOPC_ByteString* SOPC_Event_GetEventId(const SOPC_Event* pEvent);

/**
 * \brief Sets the given EventId for the given event
 *
 * \note Events created using ::SOPC_EventManager_CreateEventInstance have
 *       an already defined unique identifier.
 *
 * \param pEvent pointer to the event for which EventId shall be set
 * \param pEventId pointer to the byte string to set as EventId content
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetEventId(SOPC_Event* pEvent, const SOPC_ByteString* pEventId);

/**
 * \brief Sets the given SourceName for the given event indicating the name of the event source.
 *
 * \param pEvent      pointer to the event for which SourceName shall be set
 * \param pSourceName pointer to the string to set as source name
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetSourceName(SOPC_Event* pEvent, const SOPC_String* pSourceName);

/**
 * \brief Sets the given SourceNode for the given event indicating the node of the event source.
 *
 * \param pEvent      pointer to the event for which SourceNode shall be set
 * \param pSourceNode pointer to the node id to set as source node
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetSourceNode(SOPC_Event* pEvent, const SOPC_NodeId* pSourceNode);

/**
 * \brief Get the Time for the given event
 *
 * \param pEvent pointer to the event from which Time shall be retrieved
 *
 * \return the event type Time or -1 in case of error
 */
SOPC_DateTime SOPC_Event_GetTime(const SOPC_Event* pEvent);

/**
 * \brief Sets the given Time for the given event indicating the time the event occurred.
 *
 * \note If it is not set by user application, it will have the same value as the ReceiveTime (time it is triggered).
 *
 *
 * \param pEvent  pointer to the event for which Time shall be set
 * \param time    time value of the event to set
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetTime(SOPC_Event* pEvent, SOPC_DateTime time);

/**
 * \brief (Internal purpose only) Sets the given ReceiveTime for the given event indicating the time
 *        the event was received by server (time it is triggered).
 *
 * \note It should not be set by user application, its value will be overwritten by server anyway.
 *
 *
 * \param pEvent       pointer to the event for which Time shall be set
 * \param receiveTime  receive time value of the event by the server
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetReceiveTime(SOPC_Event* pEvent, SOPC_DateTime receiveTime);

/**
 * \brief Sets the given LocalTime for the given event indicating the Offset and the DaylightSavingInOffset of the
 * event.
 *
 * \param pEvent      pointer to the event for which SourceNode shall be set
 * \param pLocalTime  pointer to the time zone data to set as local time
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetLocalTime(SOPC_Event* pEvent, const OpcUa_TimeZoneDataType* pLocalTime);

/**
 * \brief Sets the given Message for the given event indicating a human-readable
 *        and localizable text description of the event.
 *
 * \param pEvent      pointer to the event for which Message shall be set
 * \param pMessage    pointer to the message to set as message description
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetMessage(SOPC_Event* pEvent, const SOPC_LocalizedText* pMessage);

/**
 * \brief Sets the given Severity for the given event indicating the urgency of the event.
 *
 * \param pEvent    pointer to the event for which SourceNode shall be set
 * \param severity  severity value to set for the given event
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetSeverity(SOPC_Event* pEvent, uint16_t severity);

/**
 * \brief Sets the given variable value for the given browse path in the given event
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pEvent                  pointer to the event for which variable value shall be set
 * \param nbQnPath                number of qualified name in the browse path
 * \param qualifiedNamePathArray  qualified name path array containing the \p nbQnPath path elements
 *                                (e.g.: ['0:EnabledState', '0:Id'])
 * \param var                     variable to set in the given event for the given browse path
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetVariable(SOPC_Event* pEvent,
                                         uint16_t nbQnPath,
                                         SOPC_QualifiedName* qualifiedNamePathArray,
                                         const SOPC_Variant* var);

/**
 * \brief Sets the given variable value for the given browse path (as a string) in the given event
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pEvent  pointer to the event for which variable value shall be set
 * \param qnPath  qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 * \param var     variable to set in the given event for the given browse path
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_Event_SetVariableFromStrPath(SOPC_Event* pEvent, const char* qnPath, const SOPC_Variant* var);

/**
 * \brief Gets the variable value for the given browse path (as a string) in the given event
 *
 * \param pEvent  pointer to the event for which variable value shall be set
 * \param qnPath  qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 *
 * \return The variable value for the given browse path in case of success, NULL otherwise
 */
const SOPC_Variant* SOPC_Event_GetVariableFromStrPath(const SOPC_Event* pEvent, const char* qnPath);

/**
 * \brief Gets the variable value and type information for the given browse path in the given event
 *
 * \param pEvent                  pointer to the event for which variable value shall be set
 * \param nbQnPath                number of qualified name in the browse path
 * \param qualifiedNamePathArray  qualified name path array containing the \p nbQnPath path elements
 *                                (e.g.: ['0:EnabledState', '0:Id'])
 * \param[out] outDataType  (optional) the pointer is set with the data type of the returned value (if pointer provided)
 * \param[out] outValueRank (optional) the pointer is set with the value rank of the returned value
 *             (if pointer provided)
 *
 * \return The variable value for the given browse path in case of success, NULL otherwise
 */
const SOPC_Variant* SOPC_Event_GetVariableAndType(const SOPC_Event* pEvent,
                                                  uint16_t nbQnPath,
                                                  SOPC_QualifiedName* qualifiedNamePathArray,
                                                  const SOPC_NodeId** outDataType,
                                                  int32_t* outValueRank);

/**
 * \brief Gets the variable value and type information for the given browse path (as a string) in the given event
 *
 * \param pEvent  pointer to the event for which variable value shall be set
 * \param qnPath  qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 * \param[out] outDataType  (optional) the pointer is set with the data type of the returned value (if pointer provided)
 * \param[out] outValueRank (optional) the pointer is set with the value rank of the returned value
 *             (if pointer provided)
 *
 * \return The variable value for the given browse path in case of success, NULL otherwise
 */
const SOPC_Variant* SOPC_Event_GetVariableAndTypeFromStrPath(const SOPC_Event* pEvent,
                                                             const char* qnPath,
                                                             const SOPC_NodeId** outDataType,
                                                             int32_t* outValueRank);

/**
 * \brief Type of callback functions for \ref SOPC_Event.
 * The value of \p user_data is set when calling \ref SOPC_Event_ForEachVar.
 */
typedef void SOPC_Event_ForEachVar_Fct(const char* qnPath,
                                       SOPC_Variant* var,
                                       const SOPC_NodeId* dataType,
                                       int32_t valueRank,
                                       uintptr_t user_data);

/**
 * \brief Iterates over the event variables, calling the given function for each event variable.
 *
 * \param event     The event to iterate on
 * \param func      The function to call on each event variable
 * \param user_data A user chose pointer to pass as last parameter to the
 *                  callback function.
 *
 * \note The order of the iteration is implementation defined, and should not be relied on
 */
void SOPC_Event_ForEachVar(SOPC_Event* event, SOPC_Event_ForEachVar_Fct* func, uintptr_t user_data);

/**
 * \brief Returns the number of event variables in the given event
 *
 * \param event  The event to evaluate
 *
 * \return The number of variables of the given event or 0 in case of invalid parameter
 */
size_t SOPC_Event_GetNbVariables(SOPC_Event* event);

#endif // SOPC_EVENT_H_
