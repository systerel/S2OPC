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

/**
 * \file libs2opc_client_alarm_conditions.h
 *
 * \brief High level interface to monitor Alarm & Conditions (A&C) instances on client's side.
 *
 * This API provides functionality to monitor alarms (object derived from ConditionType events) from a given
 * notifier node using an OPC UA subscription. Alarms are organized into "monitored alarm groups", each
 * corresponding to a single MonitoredItem within the subscription. Events received for each alarm are
 * forwarded to a user-defined callback.
 *
 * Key features:
 * - Creation and management of monitored alarm groups;
 * - Synchronous calls to OPC UA server methods (Enable, Disable, Acknowledge, Confirm, AddComment, Refresh);
 * - Retrieval of the state (Enabled, Acked, Confirmed, Active) of monitored alarms;
 * - Enable / Disable an alarm using its ConditionId;
 * - Access to the latest event associated with an alarm.
 *
 * Constraints:
 * - Only one subscription parameters per secure connection (due to one subscription per connection limitation);
 * - Some parameters/features are not yet supported (e.g., optAlarmFields, ignoreRetain);
 * - Assumes the client and secure connection are already initialized using libs2opc_client.h mechanisms.
 *
 * Typical usage:
 * 1. Initialize the alarm manager using SOPC_MonitoredAlarmMgr_Initialize()
 * 2. Create a monitored alarms group with SOPC_MonitoredAlarm_CreateAlarmsGroup()
 * 3. Interact with alarms using *_Call_*, *_Get* functions using the callback provided at initialization
 * 4. Clean up with SOPC_MonitoredAlarmMgr_Clear() at the end of the session
 */

#ifndef LIBS2OPC_CLIENT_ALARM_CONDITIONS_H_
#define LIBS2OPC_CLIENT_ALARM_CONDITIONS_H_

#include <stdbool.h>
#include <stdint.h>

#include "libs2opc_client.h"
#include "sopc_builtintypes.h"
#include "sopc_event.h"

/**
 * \brief type representing an instantiated ::SOPC_MonitoredAlarm (ConditionType or subtype)
 */
typedef struct _SOPC_MonitoredAlarm SOPC_MonitoredAlarm;

/**
 * \brief type representing a group of monitored alarms created with
 *        ::SOPC_MonitoredAlarm_CreateAlarmsGroup
 *
 * \note It corresponds to a unique monitored item in the subscription
 */
typedef struct _SOPC_MonitoredAlarmsGroup SOPC_MonitoredAlarmsGroup;

/**
 * \brief Callback type for event notification received for given alarm
 * \param group  the monitored alarms group
 * \param alarmCond the monitored alarm condition for which an event has been received
 * \param rcvEvent  the event received (to be deleted)
 */
typedef void SOPC_MonitoredAlarm_Event_Fct(SOPC_MonitoredAlarmsGroup* group,
                                           SOPC_MonitoredAlarm* alarmCond,
                                           SOPC_Event* rcvEvent);

/**
 * \brief Initializes the module.
 * \param eventCb Callback function that will be called at event reception.
 * \param ignoreRetain (NOT MANAGED YET) A boolean flag indicating whether to ignore the retain property of the alarm.
 *                                       If true, the retain property will be ignored.
 */
SOPC_ReturnStatus SOPC_MonitoredAlarmMgr_Initialize(SOPC_MonitoredAlarm_Event_Fct* eventCb, bool ignoreRetain);

/**
 * \brief Clear the monitored alarm group and all its alarms associated.
 * \param group The monitored alarm group to clear.
 */
void SOPC_MonitoredAlarm_ClearAlarmsGroup(SOPC_MonitoredAlarmsGroup* group);

/**
 * \brief   Delete the remaining alarms / alarm groups, delete user-callback looper.
 * \warning No more monitored alarm group should be manipulated after this call.
 */
void SOPC_MonitoredAlarmMgr_Clear(void);

/**
 * \brief Creates monitored alarm instances (MonitoredItem) from the alarm notification on the given notifier node.
 *        Monitored alarms are automatically created on alarm notification received
 *        This function initializes a monitored alarm using the provided notifier and alarm NodeIds.
 * \param secureConnection The secureConnection to which monitored alarm will be added.
 * \param createSubReq The subscription request parameters.
 *                     May be NULL if you want to re-use an existing already used subscription.
 * \param notifierId The notifier NodeId to subscribe for events from.
 * \param optAlarmFields (NOT MANAGED YET) (optional) the alarm fields that will be requested to be notified.
 *                                         By default AlarmConditionType fields are received.
 * \warning 1 subscription parameters per connection max (because one subscription per connection max).
 * \return the created monitored alarms group or NULL if something failed.
 */
SOPC_MonitoredAlarmsGroup* SOPC_MonitoredAlarm_CreateAlarmsGroup(SOPC_ClientConnection* secureConnection,
                                                                 OpcUa_CreateSubscriptionRequest* createSubReq,
                                                                 const SOPC_NodeId* notifierId,
                                                                 const SOPC_Event* optAlarmFields);
/**
 * \brief Gets the created alarm group subscription parameters values revised by the server.
 *
 * \param alarmGroup                 The alarm group instance
 * \param revisedPublishingInterval  Pointer for the revised publishing interval output value (optional)
 * \param revisedLifetimeCount       Pointer for the revised lifetime count output value (optional)
 * \param revisedMaxKeepAliveCount   Pointer for the revised max keep alive count output value (optional)
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of invalid parameters,
 *         SOPC_STATUS_INVALID_STATE if the client or connection is not running.
 */
SOPC_ReturnStatus SOPC_MonitoredAlarm_GetSubscriptionRevisedParams(SOPC_MonitoredAlarmsGroup* alarmGroup,
                                                                   double* revisedPublishingInterval,
                                                                   uint32_t* revisedLifetimeCount,
                                                                   uint32_t* revisedMaxKeepAliveCount);

/**
 * \brief Synchronous call to the server method Enable / Disable of ConditionType, on specified alarm.
 * \param alarmCond The monitored alarm to call the method on.
 * \param enable true for calling method Enable, false for calling method Disable.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_Enable(SOPC_MonitoredAlarm* alarmCond, bool enable);

/**
 * \brief Synchronous call to the server method Acknowledge of AcknowledgeConditionType, on specified alarm.
 * \param alarmCond The monitored alarm to call the method on.
 * \param optComment (optionnal) Acknowledge comment that will be added in variable Comment of ConditionType.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_Acknowledge(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* optComment);

/**
 * \brief Synchronous call to the server method Confirm of AcknowledgeConditionType, on specified alarm.
 * \param alarmCond The monitored alarm to call the method on.
 * \param optComment (optionnal) Confirm comment that will be added in variable Comment of ConditionType.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_Confirm(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* optComment);

/**
 * \brief Synchronous call to the server method AddComment of ConditionType, on specified alarm.
 * \param alarmCond The monitored alarm to call the method on.
 * \param comment (optionnal) Comment that will be added in variable Comment of ConditionType.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_AddComment(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* comment);

/**
 * \brief Refresh the monitored alarms group to update the alarm instances with the latest values.
 * \param group The monitored alarm to call the method on.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 * \note Uses ConditionRefresh2 method if available, otherwise (if fail in creation / sending the request OR
 *       if OpcUa_BadMethodInvalid is returned by the server) ConditionRefresh method is used.
 *       In the latter case, other groups might be refreshed as well if they are using the same subscription.
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_Refresh(const SOPC_MonitoredAlarmsGroup* group);

/**
 * \brief Enable or Disable a monitored alarm from its ConditionId.
 *        This function makes synchronous call to the server method Enable / Disable with specified ConditionId.
 *        It should be used when a condition or alarm is disabled
 *        in order to enable it and receive the first event notification that
 *        will provide an ::SOPC_MonitoredAlarm instance.
 * \param group the alarm group the future (existing) alarm will be (is) associated with.
 * \param conditionId the ConditionId of the alarm to enable / disable.
 * \param enable true for enabling, false for disabling.
 * \note the group is only used to retrieve the connection for the method call,
 *       the monitored alarm will be added to all groups in which a notification is received.
 * \return
 * - OpcUa_BadInvalidArgument if bad argument provided to the function
 * - OpcUa_BadInternalError if fail at creation or at sending the request to the server
 * - OPC UA status code server response if the request has been successfully sent to the server
 *   (service result if service failed, item response result if service succeeded)
 */
SOPC_StatusCode SOPC_MonitoredAlarmCall_EnableFromId(const SOPC_MonitoredAlarmsGroup* group,
                                                     const SOPC_NodeId* conditionId,
                                                     bool enable);

/**
 * \brief Get the ConditionId of the alarm.
 * \param alarmCond the alarm we want the ConditionId from.
 * \return - NULL if bad argument provided to the function
 *         - the ConditionId NodeId on success.
 */
const SOPC_NodeId* SOPC_MonitoredAlarm_GetConditionId(SOPC_MonitoredAlarm* alarmCond);

/**
 * \brief Get the EventId of the last event related to the alarm.
 * \param alarmCond the alarm we want the last event EventId from.
 * \return
 * - NULL if bad argument provided to the function
 * - the ByteString EventId on success.
 */
const SOPC_ByteString* SOPC_MonitoredAlarm_GetLastEventId(SOPC_MonitoredAlarm* alarmCond);

/**
 * \brief Get the EnabledState of the alarm.
 * \param alarmCond the alarm we want to know the state of.
 * \param state the state result.
 * \return
 * - SOPC_STATUS_INVALID_PARAMETERS if bad argument provided to the function
 * - SOPC_STATUS_NOK if the state could not be retrieved from the alarm
 * - SOPC_STATUS_OK on success
 */
SOPC_ReturnStatus SOPC_MonitoredAlarm_GetEnabledState(SOPC_MonitoredAlarm* alarmCond, bool* state);

/**
 * \brief Get the AckedState of the alarm.
 * \param alarmCond the alarm we want to know the state of.
 * \param state the state result.
 * \return
 * - SOPC_STATUS_INVALID_PARAMETERS if bad argument provided to the function
 * - SOPC_STATUS_NOK if the state could not be retrieved from the alarm
 * - SOPC_STATUS_OK on success
 */
SOPC_ReturnStatus SOPC_MonitoredAlarm_GetAckedState(SOPC_MonitoredAlarm* alarmCond, bool* state);

/**
 * \brief Get the ConfirmedState of the alarm.
 * \param alarmCond the alarm we want to know the state of.
 * \param state the state result.
 * \return
 * - SOPC_STATUS_INVALID_PARAMETERS if bad argument provided to the function
 * - SOPC_STATUS_NOK if the state could not be retrieved from the alarm
 * - SOPC_STATUS_OK on success
 */
SOPC_ReturnStatus SOPC_MonitoredAlarm_GetConfirmedState(SOPC_MonitoredAlarm* alarmCond, bool* state);

/**
 * \brief Get the ActiveState of the alarm.
 * \param alarmCond the alarm we want to know the state of.
 * \param state the state result.
 * \note the group is only used to retrieve the connection for the method call,
 *       the monitored alarm will be added to all groups in which a notification is received.
 * \return
 * - SOPC_STATUS_INVALID_PARAMETERS if bad argument provided to the function
 * - SOPC_STATUS_NOK if the state could not be retrieved from the alarm
 * - SOPC_STATUS_OK on success
 */
SOPC_ReturnStatus SOPC_MonitoredAlarm_GetActiveState(SOPC_MonitoredAlarm* alarmCond, bool* state);

/**
 * \brief Returns a copy of the last event related to the alarm.
 * \param alarmCond the alarm we want to get the last related event from.
 * \return
 * - NULL if bad argument provided to the function
 * - the event on success
 */
SOPC_Event* SOPC_MonitoredAlarm_GetLastEvent(SOPC_MonitoredAlarm* alarmCond);

/**
 * \brief Returns the group the alarm is associated with.
 * \param alarmCond the alarm whose group we want to get.
 * \return
 * - NULL if bad argument provided to the function
 * - the alarm group on success
 */
SOPC_MonitoredAlarmsGroup* SOPC_MonitoredAlarm_GetGroup(SOPC_MonitoredAlarm* alarmCond);

#endif
