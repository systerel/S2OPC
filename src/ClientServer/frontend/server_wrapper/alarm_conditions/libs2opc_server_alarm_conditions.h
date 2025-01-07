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
 * \brief High level interface to manage Alarm & Conditions (A&C) instances
 *
 * This interface provides utilities to instantiate alarm and condition instances using the following principles:
 * - A&C instances are expected to be present in address space as nodes
 * - This module stores the values of associated variables into ::SOPC_Event provided instance at creation
 *   and manage its consistency with matching address space nodes content.
 * - A&C instances states might be changed using this module API or A&C instances method call (e.g. AddCom., Ack.,
 * Conf., etc.)
 * - A&C instances external states changes (through method calls) lead to API asynchronous callback call
 *   (after method call execution)
 *
 * Important note: the current scope of implementation is A&C Alarm Server Facet and includes the following types:
 * ConditionType, AcknowledgeableConditionType and AlarmConditionType/DiscreteAlarmType (Active state only).
 *
 * Details on states management:
 * - *State:
 *   - transition: any change of state value automatically triggers an Event on the notifier node
 *                (except for transition to un-acknowledgeable / un-confirmable)
 * - EnabledState:
 *   - default: if the initial event instance does not define value (NULL) for State/Id,
 *              TRUE is set as it is specified in case of AlarmManager restart (see part 9).
 *   - init: State value is set accordingly to State/Id value
 *   - Enabled -> Disabled: Retain value is set to FALSE as it is specified (see part 9).
 *
 * - AckedState:
 *   - init: State value is set accordingly to State/Id value if value is not NULL
 *   - Acknowledgeable: when AckedState value is True (or NULL), the State is locked for Acknowledge operation
 *                      until ::SOPC_AlarmCondition_SetAcknowledgeable is called to unlock it (AckedState=False).
 *                      It might be set again to un-acknowledgeable state (AckedState=True) with same function, in
 *                      this particular case no event is triggered automatically (but it might be triggered manually).
 *   - Acknowledge: when AckedState=False it is possible to acknowledge and make transition to AckedState=True
 *                  using the server API ::SOPC_AlarmCondition_Acknowledge or for a client using the Acknowledge()
 *                  method call (if executable for client user).
 *   - AutoAcknowledgeable: when ActiveState is defined, it is possible to make state acknowledgeable automatically
 *                          on ActiveState transition to TRUE, thus ActiveState=TRUE and AckedState=FALSE
 *                          are notified in same event.
 *
 * - ConfirmedState:
 *   - init: State value is set accordingly to State/Id value if value is not NULL
 *   - Confirmable: when ConfirmedState value is True (or NULL), the State is locked for Confirm operation
 *                  until ::SOPC_AlarmCondition_SetConfirmable is called to unlock it (ConfirmedState=False).
 *                  It might be set again to un-confirmable state (ConfirmedState=True) with same function,
 *                  in this particular case no event is triggered automatically (but it might be triggered manually).
 *   - Confirm:     when ConfirmedState=False it is possible to acknowledge and make transition to ConfirmedState=True
 *                  using the server API ::SOPC_AlarmCondition_Confirm or for a client using the Confirm()
 *                  method call (if available/executable for client user).
 *   - AutoConfirmable: it is possible to manage state confirmable automatically on AckedState transitions:
 *                      - On Acknowledgeable operation transition to acknowledgeable,
 *                        AckedState=FALSE and ConfirmedState=TRUE (non-confirmable) are set and notified in same event.
 *                        If un-acknowledgeable is set, both states AckedState / ConfirmedState
 *                        are set to TRUE but no event is automatically triggered.
 *                      - On Acknowledge transition, AckedState=TRUE and ConfirmedState=FALSE are
 *                        set and notified in same event.
 *
 * - ActiveState: no specific behavior defined except when ::SOPC_AlarmCondition_SetAutoAcknowledgeable is set
 *                (see AckedState:AutoAcknowledgeable description).
 *
 * - Retain property: except for the particular case of Disable transition where it is set to FALSE,
 *                    it shall be set manually to TRUE/FALSE when needed by application (see part 9).
 *
 * Specific constraints on Acknowledge()/Confirm()/AddComment() method calls:
 * - Acknowledge()/Confirm():
 *   accept the latest ::S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED eventIds
 *   since the latest State transition to FALSE, otherwise OpcUa_BadEventIdUnknown is returned.
 * - AddComment():
 *   accept the latest ::S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED eventIds, otherwise
 *   OpcUa_BadEventIdUnknown is returned.
 *
 * Details on variables management:
 * - ConditionVariable:
 *   - Quality/Comment: specific setters are defined and trigger event as specified
 *   - LastSeverity: specific setter ::SOPC_AlarmCondition_SetSeverity shall be used to change Severity,
 *                   it manages automatically LastSeverity when Severity change and triggers event
 *   - Others: ::SOPC_AlarmCondition_SetConditionVariable or ::SOPC_AlarmCondition_SetConditionVariableFromStrPath
 *             setters shall be used to manage the SourceTimestamp property and eventually trigger an event on change.
 *
 * \warning Application shall never access or modify Alarm & Conditions instances variables using other means than API
 *          of this module (no read/write local services on its nodes) and shall forbid any possible client modification
 *          (AccessLevel shall be RO).
 *
 * In order to use this module ::SOPC_ServerAlarmConditionMgr_Initialize shall be called during server configuration
 * phase and prior to any other call. The ::SOPC_ServerAlarmConditionMgr_Clear shall be called when server stopped.
 */

#ifndef LIBS2OPC_SERVER_ALARM_CONDITION_H_
#define LIBS2OPC_SERVER_ALARM_CONDITION_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_call_method_manager.h"
#include "sopc_event.h"

/**
 * \brief Initializes the Alarm & Conditions manager and configure it with the provided method call manager.
 *
 * This function configures the Alarm & Conditions manager to use the specified method call manager
 * for handling alarm-related method calls. It must be called during the server configuration phase,
 * before the server is started.
 *
 * \param mcm Pointer to the method call manager that will be used for alarm condition method calls.
 *            This must be the one configured for the server using ::SOPC_ServerConfigHelper_SetMethodCallManager.
 *
 * \return SOPC_STATUS_OK if initialization was successful,
 *         SOPC_STATUS_INVALID_PARAMETERS if mcm is NULL,
 *         SOPC_STATUS_INVALID_STATE if the server is not in configuring phase or manager is already initialized,
 *         SOPC_STATUS_OUT_OF_MEMORY in case of memory allocation failure.
 *
 * \warning This function must be called during server configuration phase, before the server is started.
 * \warning The provided method call manager \p mcm must be the one configured for the server using
 *          SOPC_ServerConfigHelper_SetMethodCallManager.
 */
SOPC_ReturnStatus SOPC_ServerAlarmConditionMgr_Initialize(SOPC_MethodCallManager* mcm);

/**
 * \brief Clears the Alarm & Conditions manager after server stopped.
 *
 * \warning The server shall be stopped otherwise this call is ignored
 * \warning Any previously created alarm condition instance using ::SOPC_ServerAlarmCondition_Create shall not be used
 * after this call, those are deallocated and become invalid.
 */
void SOPC_ServerAlarmConditionMgr_Clear(void);

/**
 * \brief type representing an instantiated ::SOPC_AlarmCondition
 */
typedef struct _SOPC_AlarmCondition SOPC_AlarmCondition;

/**
 * \brief Create new AlarmCondition instance based on existing AlarmCondition nodes in addresss space
 *        and an Event instance representing the initial AlarmCondition fields content.
 *        After instance creation, the condition node is automatically browsed to retrieve all NodeIds
 *        of the event fields and event fields values are written in the associated nodes.
 *        Those value might be modified later using the ::SOPC_AlarmCondition returned object
 *        and associated nodes will then be modified consequently.
 *
 * \param notifierNode       the NodeId of the node notifying the condition events triggered
 *                           (it might be the SourceCondition itself or its HasEventSource parent)
 * \param conditionNode      the NodeId of the node representing the AlarmCondition in address space
 * \param conditionInst      the event instance that contains inital data for the AlarmCondition fields,
 *                           it shall not be used anymore by caller after a successful call.
 * \param[out] outAlarmCond  the alarm condition instance created on successful call.
 *
 * \warning The server shall be started, thus ::SOPC_Server_Start or :: SOPC_Server_Serve shall have been called
 *
 * \note Some AlarmCondition fields are automatically initialized by this function when not set in event:
 *       - EnabledState: set it to the Id boolean value, and considered true if Id is NULL
 *       - Retain: set to false if NULL or state is disabled
 *       - Quality: set to Good if NULL
 *       - LastSeverity: set to 0 if NULL
 *       - AckedState: set it the Id boolean value (if not NULL)
 *       - ConfirmedState: set it to the Id boolean value (if not NULL)
 *       - ActiveState: set it to the Id boolean value (if not NULL)
 *       - ClientUserId: set it to the ClientUserId empty string value if NULL
 *         (it is then updated on supported method calls modifying Comment field)
 */
SOPC_ReturnStatus SOPC_AlarmCondition_CreateFromEvent(const SOPC_NodeId* notifierNode,
                                                      const SOPC_NodeId* conditionNode,
                                                      SOPC_Event* conditionInst,
                                                      SOPC_AlarmCondition** outAlarmCond);

// Dedicated operations to set well-known and supported state variables

/**
 * \brief Sets the EnabledState to the given value if transition is possible.
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *        An event is triggered on success.
 *
 *        On transition to FALSE state the Retain variable is always set to FALSE.
 *        On transition to TRUE state and if \p setRetain is set, the Retain variable is always set to TRUE.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param enabled the state value to set
 * \param setRetain also sets the Retain value to TRUE when \p enabled is set,
 *                  if ::SOPC_AlarmCondition_SetAutoRetain has been configured it SHALL be FALSE.
 * \param optComment the comment to set prior to transition (NULL otherwise)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_STATE if the current state is already in requested state,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetEnabledState(SOPC_AlarmCondition* pAlarmCondition,
                                                      bool enabled,
                                                      bool setRetain,
                                                      const SOPC_LocalizedText* optComment);

/**
 * \brief Returns true if the current EnabledState boolean value is true, false otherwise.
 *
 * \param pAlarmCondition the AlarmCondition instance requested state
 *
 * \return true if the EnabledState is true, false otherwise.
 */
bool SOPC_AlarmCondition_GetEnabledState(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Sets automatically the AckedState to FALSE when a successful transition of ActiveState to TRUE
 *        occurs for the given AlarmCondition instance.
 *
 * \note When this option is activated, AckedState=False and ActivateStated=True
 *       changes are notified in the same EventNotification on alarm Activation.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 *
 * \return SOPC_STATUS_OK if configuration succeeded,
 *         SOPC_STATUS_INVALID_STATE if module is not initialized,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance).
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Makes the AckedState activation possible (or impossible)
 *        for Acknowledge() method calls (if available) or ::SOPC_AlarmCondition_Acknowledge.
 *
 *        When it is un-acknowledgeable the AckedState is TRUE and when it is acknowledgeable it is FALSE.
 *
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *
 *        An event is triggered on success if \p acknowledgeable is set.
 *
 * \warning if ::SOPC_AlarmCondition_SetAutoAcknowledgeable has been configured this function SHALL NOT be called
 *          with \p acknowledgeable set as it is automatically done on alarm activation.
 *          If it is the case SOPC_INVALID_PARAMETERS is returned.
 *
 * \warning The list of EventId accepted by Acknowledge() method call is reset each time
 *          \p acknowledgeable is set and is limited to the
 *          ::S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED latest events.
 *
 * \note It is always possible to set the state as un-acknowledgeable even when EnabledState is FALSE
 *
 * \note if ::SOPC_AlarmCondition_SetAutoConfirmable has been configured,
 *       the ConfirmedState is automatically set to TRUE (un-confirmable) on successful call.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param acknowledgeable if flag is set the AckedState becomes FALSE, otherwise AckedState becomes TRUE
 * \param optComment the comment to set prior to transition (NULL otherwise)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully or state is unchanged,
 *         SOPC_STATUS_INVALID_STATE if the current EnabledState (FALSE) does not allow transition,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition,
                                                         bool acknowledgeable,
                                                         const SOPC_LocalizedText* optComment);

/**
 * \brief Acknowledges the alarm by setting the AckedState to TRUE if transition is possible.
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *        An event is triggered on success.
 *
 * \warning if ::SOPC_AlarmCondition_SetAutoConfirmable has been configured on the given
 *          AlarmCondition instance, the ConfirmedState is automatically set to FALSE (confirmable)
 *          in case of success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param optComment the comment to set prior to transition (NULL otherwise)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_STATE if the EnabledState (FALSE) or AckedState does not allow transition to given value,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_Acknowledge(SOPC_AlarmCondition* pAlarmCondition,
                                                  const SOPC_LocalizedText* optComment);

/**
 * \brief Returns true if the current AckedState boolean value is true, false otherwise.
 *
 * \param pAlarmCondition the AlarmCondition instance requested state
 *
 * \return true if the AckedState is true, false otherwise.
 */
bool SOPC_AlarmCondition_GetAckedState(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Sets automatically the ConfirmedState to FALSE (confirmable)
 *        when a successful Acknowledge transition of AckedState to TRUE
 *        occurs for the given AlarmCondition instance.
 *        It is also set automatically to ConfirmedState to TRUE (un-confirmable)
 *        when ::SOPC_AlarmCondition_SetAcknowledgeable (for both values cases).
 *
 * \note When this option is activated, the ConfirmedState=FALSE (resp. TRUE)
 *       and AckedState=TRUE (resp. FALSE) state changes are notified
 *       in the same EventNotification on successful acknowledgement
 *      (resp. acknowledgeable state transition).
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 *
 * \return SOPC_STATUS_OK if configuration succeeded,
 *         SOPC_STATUS_INVALID_STATE if module is not initialized,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance).
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoConfirmable(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Makes the ConfirmedState activation possible (or impossible) for
 *        Confirm() method calls (if available) or ::SOPC_AlarmCondition_Confirm.
 *
 *        When it is un-confirmable the ConfirmedState is TRUE and when it is confirmable it is FALSE.
 *
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *
 *        An event is triggered on success if \p confirmable is set.
 *
 * \warning if ::SOPC_AlarmCondition_SetAutoConfirmable has been configured this function SHALL NOT be called
 *          when alarm is enabled as it is automatically managed.
 *          If it is the case SOPC_INVALID_PARAMETERS is returned except when alarm is disabled.
 *
 * \warning The list of EventId accepted by Confirm() method call is reset each time
 *          \p confirmable is set and is limited to the
 *          ::S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED latest events.
 *
 * \note It is always possible to set the state as un-confirmable even when EnabledState is FALSE
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param confirmable if flag is set the ConfirmedState becomes FALSE, otherwise ConfirmedState becomes TRUE
 * \param optComment the comment to set prior to transition (NULL otherwise)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully or state is unchanged,
 *         SOPC_STATUS_INVALID_STATE if the current EnabledState (FALSE) does not allow transition,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetConfirmable(SOPC_AlarmCondition* pAlarmCondition,
                                                     bool confirmable,
                                                     const SOPC_LocalizedText* optComment);

/**
 * \brief Confirms the alarm by setting the ConfirmedState to TRUE if transition is possible.
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *        An event is triggered on success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param optComment the comment to set prior to transition (NULL otherwise)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_STATE if the EnabledState (FALSE)
 *         or ConfirmedState does not allow transition to given value,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_Confirm(SOPC_AlarmCondition* pAlarmCondition,
                                              const SOPC_LocalizedText* optComment);

/**
 * \brief Returns true if the current ConfirmedState boolean value is true, false otherwise.
 *
 * \param pAlarmCondition the AlarmCondition instance requested state
 *
 * \return true if the ConfirmedState is true, false otherwise.
 */
bool SOPC_AlarmCondition_GetConfirmedState(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Sets the ActiveState to the given value if transition is possible.
 *        Both State/Id and State values are updated and Comment is updated when provided.
 *        An event is triggered on success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param active          the state value to set
 * \param optComment      the comment to set prior to transition (NULL otherwise)
 *
 * \note if ::SOPC_AlarmCondition_SetAutoAcknowledgeable has been configured on the given
 *       AlarmCondition instance, the AckedState is automatically set to FALSE (acknowledgeable)
 *       when ActiveState is set to TRUE.
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_STATE if the EnabledState (FALSE) or ActiveState
 *                                   does not allow transition to given value,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetActiveState(SOPC_AlarmCondition* pAlarmCondition,
                                                     bool active,
                                                     const SOPC_LocalizedText* optComment);

/**
 * \brief Returns true if the current ActiveState boolean value is true, false otherwise.
 *
 * \param pAlarmCondition the AlarmCondition instance requested state
 *
 * \return true if the ActiveState is true, false otherwise.
 */
bool SOPC_AlarmCondition_GetActiveState(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Sets automatically the Retain state to TRUE
 *        (alarm activation or alarm enabled and either active, acknowledgeable or confirmable)
 *        and sets automatically the Retain state to FALSE
 *        (alarm inactive + acknowledged + confirmed or
 *         alarm inactive + acknowledged without confirmation needed or
 *         alarm inactive + without acknowledge nor confirmation needed).
 *        Some constraints shall be respected to set Retain to FALSE automatically (see warning section).
 *
 * \warning ::SOPC_AlarmCondition_SetAutoAcknowledgeable shall be set for an AcknowledgeableConditionType and
 *          ::SOPC_AlarmCondition_SetAutoConfirmable shall be set if it is confirmable.
 *          In case neither has been set, Retain is set to FALSE as soon as the alarm becomes inactive.
 *
 * \note The Retain state is ALWAYS set to FALSE automatically when the AlarmCondition is Disabled
 *       even when ::SOPC_AlarmCondition_SetAutoRetain has not been set.
 *
 * \note If the alarm does not have ActiveState variable, the state is ignored for Retain automatic management.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 *
 * \return SOPC_STATUS_OK if configuration succeeded,
 *         SOPC_STATUS_INVALID_STATE if module is not initialized,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance).
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoRetain(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Sets the Retain variable to the given value.
 *        An event is triggered if \p triggerEvent is set and retain state is true or has changed.
 *
 * \warning Retain variable is ALWAYS automatically set to FALSE when an alarm is Disabled,
 *          this is mandatory in OPC UA specification and does not depend on use of
 *          ::SOPC_AlarmCondition_SetAutoRetain.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param retain the Retain value to set
 * \param triggerEvent triggers an event on success (only if \p retain is true or state has changed)
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_INVALID_STATE if module is not initialized or alarm not enabled.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetRetain(SOPC_AlarmCondition* pAlarmCondition, bool retain, bool triggerEvent);

/**
 * \brief Sets the Quality condition variable to the given value.
 *        SourceTimestamp property is updated on success.
 *        An event is triggered on success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param quality the Quality value to set
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetQuality(SOPC_AlarmCondition* pAlarmCondition, SOPC_StatusCode quality);

/**
 * \brief Sets the Severity variable to the given value.
 *        LastSeverity variable (and its SourceTimestamp property) is updated on success with previous Severity value.
 *        An event is triggered on success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param severity the Severity value to set
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetSeverity(SOPC_AlarmCondition* pAlarmCondition, uint16_t severity);

/**
 * \brief Sets the Comment condition variable to the given value.
 *        SourceTimestamp property is updated on success.
 *        An event is triggered on success.
 *
 * \param pAlarmCondition the AlarmCondition instance to update
 * \param comment the Comment value to set
 *
 * \return SOPC_STATUS_OK if transition occurred successfully,
 *         SOPC_STATUS_INVALID_PARAMETERS if argument is invalid (NULL instance),
 *         SOPC_STATUS_NOK if module is not initialized,
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetComment(SOPC_AlarmCondition* pAlarmCondition,
                                                 const SOPC_LocalizedText* comment);

// Generic operation to set variables (all possible event condition type fields)

/**
 * \brief Sets the given variable value for the given browse path in the given alarm
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pAlarmCondition         pointer to the alarm condition for which variable value shall be set
 * \param nbQnPath                number of qualified name in the browse path
 * \param qualifiedNamePathArray  qualified name path array containing the \p nbQnPath path elements
 *                                (e.g.: ['0:EnabledState', '0:Id'])
 * \param var                     variable to set in the given alarm for the given browse path
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetVariable(SOPC_AlarmCondition* pAlarmCondition,
                                                  uint16_t nbQnPath,
                                                  const SOPC_QualifiedName* qualifiedNamePathArray,
                                                  const SOPC_Variant* var);

/**
 * \brief Sets the given variable value for the given browse path (as a string) in the given alarm
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pAlarmCondition  pointer to the alarm condition for which variable value shall be set
 * \param qnPath  qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 * \param var     variable to set in the given alarm for the given browse path
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                             const char* qnPath,
                                                             const SOPC_Variant* var);

/**
 * \brief Similar to :SOPC_AlarmCondition_SetVariable but for a ConditionVariable (i.e. of type ConditionVariableType).
 *        In addition to the value, the SourceTimestamp property value is set automatically.
 *        Changing a ConditionVariable value is considered important and supposed to trigger an Event,
 *        thus a dedicated parameter is provided to trigger an event.
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pAlarmCondition         pointer to the alarm condition for which variable value shall be set
 * \param nbQnPath                number of qualified name in the browse path
 * \param qualifiedNamePathArray  qualified name path array containing the \p nbQnPath path elements
 *                                (e.g.: ['0:EnabledState', '0:Id'])
 * \param var                     variable to set in the given alarm for the given browse path
 * \param triggerEvent            it should be set to true to trigger an event and to comply with
 *                                part 9 specification on ConditionVariable value changes
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetConditionVariable(SOPC_AlarmCondition* pAlarmCondition,
                                                           uint16_t nbQnPath,
                                                           const SOPC_QualifiedName* qualifiedNamePathArray,
                                                           const SOPC_Variant* var,
                                                           bool triggerEvent);

/**
 * \brief Similar to :SOPC_AlarmCondition_SetVariableFromStrPath but for a ConditionVariable
 *        (i.e. of type ConditionVariableType).
 *        In addition to the value, the SourceTimestamp property value is set automatically.
 *        Changing a ConditionVariable value is considered important and supposed to trigger an Event,
 *        thus a dedicated parameter is provided to trigger an event.
 *
 * \warning This API does not typecheck the variant value type,
 *          it is caller responsibility to be consistent with EventType description
 *
 * \param pAlarmCondition  pointer to the alarm condition for which variable value shall be set
 * \param qnPath           qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 * \param var              variable to set in the given alarm for the given browse path
 * \param triggerEvent     it should be set to true to trigger an event and to comply with
 *                         part 9 specification on ConditionVariable value changes
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID parameter or SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetConditionVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                                      const char* qnPath,
                                                                      const SOPC_Variant* var,
                                                                      bool triggerEvent);

/**
 * \brief Gets the variable value for the given browse path (as a string) in the given alarm
 *
 * \param pAlarmCondition  pointer to the alarm condition for which variable value shall be get
 * \param qnPath  qualified name path separated by '~' as a string (e.g.: '0:EnabledState~0:Id')
 *
 * \return The variable value for the given browse path in case of success, NULL otherwise
 *
 * \warning returned variable is a copy and shall be deallocated by caller
 *
 */
SOPC_Variant* SOPC_AlarmCondition_GetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition, const char* qnPath);

/**
 * \brief Gets the variable value for the given browse path (as a qualified name array) in the given alarm
 *
 * \param pAlarmCondition         pointer to the alarm condition for which variable value shall be get
 * \param nbQnPath                number of qualified name in the browse path
 * \param qualifiedNamePathArray  qualified name path array containing the \p nbQnPath path elements
 *                                (e.g.: ['0:EnabledState', '0:Id'])
 *
 * \return The variable value for the given browse path in case of success, NULL otherwise
 *
 * \warning returned variable is a copy and shall be deallocated by caller
 */
SOPC_Variant* SOPC_AlarmCondition_GetVariable(SOPC_AlarmCondition* pAlarmCondition,
                                              uint16_t nbQnPath,
                                              const SOPC_QualifiedName* qualifiedNamePathArray);

/**
 * \brief Triggers an event on the alarm:
 * - Checks the EnabledState is Enabled, otherwise fails with ::OpcUa_BadConditionDisabled
 * - Sets the newly created EventId in Condition
 * - Triggers an event for the condition on configured AlarmCondition notifier (and Server) node
 *
 * \param pAlarmCondition pointer to the alarm condition for which event shall be triggered
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_STATE if the EnabledState is not Enabled otherwise.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition);

/**
 * \brief Callback type for state changed event
 *
 * \param userCbCtx  the user context provided when recording the callback
 * \param alarmCond  the alarm condition in which state changed
 * \param qnPath     the State variable path
 * \param prevValue  the Variant value of the State/Id prior to state transition
 * \param newValue   the new and current Variant value of the State/Id
 */
typedef void SOPC_AlarmCondition_StateChanged_Fct(uintptr_t userCbCtx,
                                                  SOPC_AlarmCondition* alarmCond,
                                                  const char* qnPath,
                                                  const SOPC_Variant* prevValue,
                                                  const SOPC_Variant* newValue);

/**
 * \brief Sets a callback function to be called when a EnabledState variable is modified through
 *        Enable() or Disable() method calls.
 *
 * This function registers a callback that will be triggered whenever the alarm state variable changes.
 * The callback provides information about the state change, including the previous and new values.
 *
 * \param pAlarmCondition  Pointer to the alarm condition instance for which to set the callback
 * \param callback         Function pointer to the callback that will be called on state change
 * \param userCbCtx        User context that will be passed to the callback function
 *
 * \return SOPC_STATUS_OK if the callback was successfully registered,
 *         SOPC_STATUS_INVALID_PARAMETERS if any argument is invalid (NULL instance or callback),
 *         SOPC_STATUS_INVALID_STATE if the module is not initialized.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetEnabledStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                              SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                              uintptr_t userCbCtx);

/**
 * \brief Sets a callback function to be called when a AckedState variable is modified through
 *        Acknowledge() method calls.
 *
 * This function registers a callback that will be triggered whenever the alarm state variable changes.
 * The callback provides information about the state change, including the previous and new values.
 *
 * \param pAlarmCondition  Pointer to the alarm condition instance for which to set the callback
 * \param callback         Function pointer to the callback that will be called on state change
 * \param userCbCtx        User context that will be passed to the callback function
 *
 * \return SOPC_STATUS_OK if the callback was successfully registered,
 *         SOPC_STATUS_INVALID_PARAMETERS if any argument is invalid (NULL instance or callback),
 *         SOPC_STATUS_INVALID_STATE if the module is not initialized.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetAckedStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                            SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                            uintptr_t userCbCtx);

/**
 * \brief Sets a callback function to be called when a ConfirmedState variable is modified through
 *        Confirm() method calls.
 *
 * This function registers a callback that will be triggered whenever the alarm state variable changes.
 * The callback provides information about the state change, including the previous and new values.
 *
 * \param pAlarmCondition  Pointer to the alarm condition instance for which to set the callback
 * \param callback         Function pointer to the callback that will be called on state change
 * \param userCbCtx        User context that will be passed to the callback function
 *
 * \return SOPC_STATUS_OK if the callback was successfully registered,
 *         SOPC_STATUS_INVALID_PARAMETERS if any argument is invalid (NULL instance or callback),
 *         SOPC_STATUS_INVALID_STATE if the module is not initialized.
 */
SOPC_ReturnStatus SOPC_AlarmCondition_SetConfirmedStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                                SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                                uintptr_t userCbCtx);

#endif
