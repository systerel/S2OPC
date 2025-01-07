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
 * \brief Internal interfaces to manage Alarm & Conditions (A&C) instances
 */

#ifndef LIBS2OPC_INTERNAL_ALARM_CONDITION_H_
#define LIBS2OPC_INTERNAL_ALARM_CONDITION_H_

#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_event.h"
#include "sopc_event_handler.h"
#include "sopc_mutexes.h"

#include "libs2opc_internal_eventid_list.h"
#include "libs2opc_server_alarm_conditions.h"

/*
 * Global variables to manage A&C
 */

// The singleton configuration structure
struct _SOPC_AlarmConditionConfig
{
    // Mutex to protect the global variables
    SOPC_Mutex g_mutex;
    // The looper used to execute internal event treatment in a dedicated thread for:
    // - Method call and service responses long treatment
    // - A&C states callback calls
    SOPC_EventHandler* g_alarmCondLooperEvtHdlr;
    // Dictionary of A&C instances: Condition NodeId -> AC
    SOPC_Dict* g_alarmConditionsDict;
    // Dictionary of latest event triggered by condition: Condition NodeId -> latest event triggered by condition
    SOPC_Dict* g_refreshEventsDict;
};

extern struct _SOPC_AlarmConditionConfig sopc_alarmConditionConfig;

/*
 * Internal structure to manage A&C instances
 */

// A&C instance structure

struct _SOPC_AlarmCondition
{
    SOPC_Mutex mut;              // Mutex to protect the A&C content during modification / access
    SOPC_NodeId notifierNode;    // Id of the node used to trigger events in address space
    SOPC_NodeId conditionNode;   // Id of the A&C node
    int32_t autoRetainFlag;      // Flag to manage Retain state automatically
    int32_t autoAckOnActiveFlag; // Flag to set automatically the acknowledgeable state on active state transition
    int32_t autoConfOnAckedFlag; // Flag to set automatically the confirmable state on acknowledged state transitions
    SOPC_Event* data; // Stores event field states of the A&C, those are synchronized with address space node variables
    SOPC_EventIdList* ackEventIds;    // List of EventIds recorded for AckedState since last change to False state
    SOPC_EventIdList* confEventIds;   // List of EventIds recorded for ConfirmedState since last change to False state
    SOPC_EventIdList* globalEventIds; // List of latest EventIds recorded

    SOPC_Event* nodeIds;    // Stores the node ids of corresponding nodes for each event fields
    SOPC_NodeId* methodIds; // Stores the nodes ids of the well-known and supported A&C methods
    SOPC_Dict*
        varChangedCbDict; // Stores the callback for variable changes (path -> SOPC_AlarmCondition_StateChanged_Fct_Ctx)
};

// A&C well-known methods
typedef enum SOPC_ServerAlarmCondMethods
{
    SOPC_AC_METHOD_DISABLE = 0,
    SOPC_AC_METHOD_ENABLE,
    SOPC_AC_METHOD_ADD_COMMENT,
    SOPC_AC_METHOD_ACKNOWLEDGE,
    SOPC_AC_METHOD_CONFIRM,
    SOPC_AC_METHOD_COUNT,
} SOPC_ServerAlarmCondMethods;

// A&C well-known methods qualified names
static const SOPC_QualifiedName acMethodsNames[SOPC_AC_METHOD_COUNT] = {
    SOPC_QUALIFIED_NAME(0, "Disable"),     SOPC_QUALIFIED_NAME(0, "Enable"),  SOPC_QUALIFIED_NAME(0, "AddComment"),
    SOPC_QUALIFIED_NAME(0, "Acknowledge"), SOPC_QUALIFIED_NAME(0, "Confirm"),
};

// A&C internal library events for dedicated looper treatment
typedef enum SOPC_ServerAlarmCondLibEvent
{
    SOPC_AC_EVT_TBP_RESP = 0xace0,
    SOPC_AC_EVT_WRITE_RESP,
    SOPC_AC_EVT_METHOD_CALL_SET_STATE,
    SOPC_AC_EVT_METHOD_CALL_COND_REFRESH,
    SOPC_AC_EVT_METHOD_CALL_FAILURE,
    SOPC_AC_EVT_STATE_CHANGED_CB,
} SOPC_ServerAlarmCondLibEvent;

// A&C internal context for state changed callback
typedef struct
{
    SOPC_AlarmCondition* ac;
    const char* qnPath;
    SOPC_AlarmCondition_StateChanged_Fct* callback;
    uintptr_t userCtx;
} SOPC_AlarmCondition_StateChanged_Fct_Ctx;

/* Well-known states fields config */

typedef struct
{
    // Note: Effective* fields are not managed !
    const char* self;               // State path (text representation of value)
    const char* id;                 // State/Id path (actual boolean value)
    const char* transitionTime;     // State/TransitionTime path (optional
    const char* falseTrueStates[2]; // {State/FalseState, State/TrueState}
} SOPC_InternalTwoStateVariableFieldsPaths;

extern const SOPC_InternalTwoStateVariableFieldsPaths cEnabledStatePaths;
extern const SOPC_InternalTwoStateVariableFieldsPaths cAckedStatePaths;
extern const SOPC_InternalTwoStateVariableFieldsPaths cConfirmedStatePaths;
extern const SOPC_InternalTwoStateVariableFieldsPaths cActiveStatePaths;

/*
 * A&C well-known fields paths
 */
extern const char* cSeverityIdPath;
extern const char* cLastSeverityIdPath;
extern const char* cQualityIdPath;
extern const char* cCommentIdPath;
extern const char* cClientUserIdPath;
extern const char* cEventIdPath;
extern const char* cRetainPath;

/*
 * Internal functions A&C on manager
 */

// Check if A&C manager is initialized
bool SOPC_InternalAlarmConditionMgr_IsInit(void);

// Get the flag value for automatic confirmable state on acked transitions
bool SOPC_InternalAlarmCondition_GetAutoConfOnAcked(SOPC_AlarmCondition* ac);

// Set the automatic confirmable state on acked transitions
void SOPC_InternalAlarmCondition_SetAutoConfOnAcked(SOPC_AlarmCondition* ac);

// Get the flag value for automatic acknowledgeable state on active transition to true
bool SOPC_InternalAlarmCondition_GetAutoAckableOnActive(SOPC_AlarmCondition* ac);

// Set the automatic acknowledgeable state on active transition to true
void SOPC_InternalAlarmCondition_SetAutoAckableOnActive(SOPC_AlarmCondition* ac);

// Get the flag value for automatic Retain state management
bool SOPC_InternalAlarmCondition_GetAutoRetain(SOPC_AlarmCondition* ac);

// Set the automatic Retain state management
void SOPC_InternalAlarmCondition_SetAutoRetain(SOPC_AlarmCondition* ac);

/*
 * Internal functions to manage A&C instances using mutex lock
 */

// Set A&C enabled state: see ::SOPC_AlarmCondition_SetEnabledState (additional triggerCb parameter only)
// - if triggerCb is true, the state changed callback is triggered
SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetEnabledState(SOPC_AlarmCondition* pAlarmCondition,
                                                              bool enabled,
                                                              bool setRetain,
                                                              const SOPC_LocalizedText* optComment,
                                                              bool triggerCb);

// Set A&C boolean state:
// - the following state fields are set: localized text Value, boolean Id and TransitionTime
// - if optComment is not NULL, the localized text Comment field is set
// - if optClientUserId is not NULL, the ClientUserId field is set
// - if updateNodeVal is true, the corresponding nodes in address space variables are updated consequently
// - if triggerEvent is true, an event is triggered at the containing new field values (if it has changed)
// - if triggerCb is true, the state changed callback is triggered
SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetBoolState(
    SOPC_AlarmCondition* pAlarmCondition,
    const SOPC_InternalTwoStateVariableFieldsPaths* stateVarPaths,
    bool newState,
    const SOPC_LocalizedText* optComment,
    const SOPC_String* optClientUserId,
    bool updateNodeVal,
    bool triggerEvent,
    bool triggerCb);

// Get A&C boolean value from field path: if variant is not a boolean, return false
bool SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(SOPC_AlarmCondition* ac, const char* strPath);

SOPC_ReturnStatus SOPC_InternalAlarmCondition_Acknowledge(SOPC_AlarmCondition* pAlarmCondition,
                                                          const SOPC_LocalizedText* optComment,
                                                          const SOPC_String* optClientUserId,
                                                          bool triggerCb);

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition,
                                                                 bool acknowledgeable,
                                                                 const SOPC_LocalizedText* optComment);

SOPC_ReturnStatus SOPC_InternalAlarmCondition_Confirm(SOPC_AlarmCondition* pAlarmCondition,
                                                      const SOPC_LocalizedText* optComment,
                                                      const SOPC_String* optClientUserId,
                                                      bool triggerCb);

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetConfirmable(SOPC_AlarmCondition* pAlarmCondition,
                                                             bool confirmable,
                                                             const SOPC_LocalizedText* optComment);

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetActiveState(SOPC_AlarmCondition* pAlarmCondition,
                                                             bool active,
                                                             const SOPC_LocalizedText* optComment,
                                                             const SOPC_String* optClientUserId,
                                                             bool triggerEvent);

// Set A&C comment: set ClientUserId, comment and trigger event
SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetComment(SOPC_AlarmCondition* pAlarmCondition,
                                                         const SOPC_LocalizedText* comment,
                                                         const SOPC_String* userId);

// Delete A&C instance
void SOPC_ServerAlarmCondition_Delete(SOPC_AlarmCondition** ppAlarmCond);

/*
 * Internal functions to manage A&C instances with mutex locked prior to call
 */

// Initialize new A&C instance fields:
// - EnabledState: set it to the Id boolean value, considered true if Id is NULL
// - Retain: set to false if NULL or state is disabled
// - Quality: set to Good if NULL
// - LastSeverity: set to 0 if NULL
// - AckedState: set it to the Id boolean value (if not NULL)
// - ConfirmedState: set it to the Id boolean value (if not NULL)
// - ActiveState: set it to the Id boolean value (if not NULL)
// - ClientUserId: set it to the ClientUserId empty string value if NULL
// Note: Set state fields uses ::SOPC_InternalAlarmCondition_SetBoolState without update of node (not known yet)
SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_InitFields(SOPC_AlarmCondition* ac);

// Get A&C variables node ids using TBPToNodeIds and then write address space variables values
void SOPC_Internal_GetAlarmCondition_Vars_NodeIds_And_Write_Values(SOPC_AlarmCondition* ac);

// Set A&C variable from field path:
// - if updateNodeVal is true, the corresponding nodes in address space variables are updated consequently
SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                                           const char* varPath,
                                                                           const SOPC_Variant* val,
                                                                           bool updateNodeVal);

// Set A&C condition variable from field path: ConditionVariable have a SourceTimestamp which is updated if present
// - if updateNodeVal is true, the corresponding nodes in address space variables are updated consequently
SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_SetConditionVariableFromStrPath(
    SOPC_AlarmCondition* pAlarmCondition,
    const char* varPath,
    const SOPC_Variant* val,
    bool updateNodeVal);

// Triggers A&C event (force in case of Disabled state)
SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition, bool force);

// Triggers A&C event with lock of the A&C instance
SOPC_ReturnStatus SOPC_InternalAlarmConditionWithLock_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition, bool traceLog);

/*
 * Internal functions to manage method calls
 */

// Configure method call manager
SOPC_ReturnStatus SOPC_Internal_ConfigureMethodCallManager(SOPC_MethodCallManager* mcm);

// Initialize A&C methods
void SOPC_InternalMethod_InitAlarmCondition_Methods(SOPC_MethodCallManager* mcm, SOPC_AlarmCondition* ac);

// Treats internal A&C library events
void onMethodCallLibEvt(SOPC_EventHandler* handler,
                        int32_t event,
                        uint32_t eltId,
                        uintptr_t params,
                        uintptr_t auxParam);

/*
 * Internal utility functions
 */

// Get conditionId C string representation (to be deallocated by caller) or NULL if not present
char* SOPC_InternalConditionIdToString(const SOPC_AlarmCondition* ac);

// Get variant C string representation (to be deallocated by caller) or NULL if not present
char* SOPC_InternalVarToString(const SOPC_Variant* var);

// Get EventId C string representation (to be deallocated by caller) or NULL if not present
char* SOPC_InternalGetEventIdString(const SOPC_Event* event);

// Get ByteString C string representation (to be deallocated by caller) or NULL if not present
char* SOPC_InternalByteStringToString(const SOPC_ByteString* bs);

#endif
