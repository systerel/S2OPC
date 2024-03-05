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

#include "util_event.h"

#include "constants.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "util_b2c.h"
#include "util_variant.h"

const SOPC_Server_Event_Types* initEventTypes = NULL;

static const SOPC_NodeId EventQueueOverflowTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_EventQueueOverflowEventType);
static const SOPC_NodeId ServerId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);
static const SOPC_NodeId BaseEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseEventType);

static const SOPC_Event* queueOverflowEvent = NULL;

bool init_event_types(SOPC_EndpointConfigIdx epIdx)
{
    if (NULL == initEventTypes)
    {
        SOPC_Endpoint_Config* endpointConfig = SOPC_ToolkitServer_GetEndpointConfig(epIdx);
        SOPC_ASSERT(NULL != endpointConfig && NULL != endpointConfig->serverConfigPtr);
        SOPC_Server_Config* serverConfig = endpointConfig->serverConfigPtr;
        initEventTypes = serverConfig->eventTypes;
    }
    // It should not be NULL when EventNotifier attribute is supported (necessary to reach this point).
    // But since it is only guaranteed to be initialized when using server wrapper we still have to check it.
    return NULL != initEventTypes;
}

bool util_event__alloc_event_field_list(uint32_t clientHandle,
                                        int32_t nbSelectClauses,
                                        OpcUa_EventFieldList** ppEventFieldList)
{
    SOPC_ASSERT(NULL != ppEventFieldList);
    bool res = false;
    OpcUa_EventFieldList* eventFieldList = NULL;
    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_EventFieldList_EncodeableType, (void**) &eventFieldList);
    if (SOPC_STATUS_OK == status)
    {
        eventFieldList->ClientHandle = clientHandle;
        eventFieldList->EventFields = SOPC_Calloc((size_t) nbSelectClauses, sizeof(*eventFieldList->EventFields));
        eventFieldList->NoOfEventFields = nbSelectClauses;
        if (NULL != eventFieldList->EventFields)
        {
            res = true;
            *ppEventFieldList = eventFieldList;
        }
        else
        {
            SOPC_Free(eventFieldList);
        }
    }
    return res;
}

static void applyDataValueTsRemoval(SOPC_DataValue* dv, bool removeSrc, bool removeSrv)
{
    SOPC_ASSERT(NULL != dv);
    if (removeSrc)
    {
        dv->SourceTimestamp = 0;
        dv->SourcePicoSeconds = 0;
    }
    if (removeSrv)
    {
        dv->ServerTimestamp = 0;
        dv->ServerPicoSeconds = 0;
    }
}

void util_event__set_event_field_list_elt(char** preferredLocalesIds,
                                          const constants__t_TimestampsToReturn_i timestampToReturn,
                                          bool userAccessGranted,
                                          int32_t selectClauseIdx,
                                          const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                          const SOPC_Event* pEvent,
                                          OpcUa_EventFieldList* pEventFieldList)
{
    SOPC_Variant* selectVal = &pEventFieldList->EventFields[selectClauseIdx];

    /*
     * From part 4 (v1.05) §7.22.3:
     * For example, the Server shall set the event field to Bad_UserAccessDenied
     * if the value is not accessible to the user associated with the Session.
     * If a Value Attribute has an uncertain or bad StatusCode associated with it then the Server
     * shall provide the StatusCode instead of the Value Attribute.
     */
    // eventNotifier attribute is considered not accessible, return a bad status instead of value
    if (!userAccessGranted)
    {
        SOPC_Variant_Initialize(selectVal);
        selectVal->BuiltInTypeId = SOPC_StatusCode_Id;
        selectVal->Value.Status = OpcUa_BadUserAccessDenied;
        return;
    }

    SOPC_ASSERT(NULL != pFilterCtx && !pFilterCtx->isDataFilter);
    char* qnPath = pFilterCtx->Filter.Event.qnPathStrSelectClauses[selectClauseIdx];
    SOPC_NumericRange* indexRange = pFilterCtx->Filter.Event.indexRangeSelectClauses[selectClauseIdx];
    const SOPC_Variant* eventVar = SOPC_Event_GetVariableFromStrPath(pEvent, qnPath);

    if (NULL != eventVar)
    {
        SOPC_ReturnStatus status =
            util_variant__copy_and_apply_locales_and_index_range(selectVal, eventVar, preferredLocalesIds, indexRange);

        /* Apply the TimestampToReturn in case of DataValue built it type:
         * "When monitoring Events, this applies only to Event fields that are of type DataValue.
         */
        if (SOPC_STATUS_OK == status && SOPC_DataValue_Id == selectVal->BuiltInTypeId)
        {
            bool removeSrc = false;
            bool removeSrv = false;
            switch (timestampToReturn)
            {
            case constants__e_ttr_source:
                removeSrv = true;
                break;
            case constants__e_ttr_server:
                removeSrc = true;
                break;
            case constants__e_ttr_neither:
                removeSrc = true;
                removeSrv = true;
                break;
            default:
                // Keep both in other cases
                break;
            }

            if (removeSrc || removeSrv)
            {
                if (SOPC_VariantArrayType_SingleValue == selectVal->ArrayType)
                {
                    applyDataValueTsRemoval(selectVal->Value.DataValue, removeSrc, removeSrv);
                }
                else
                {
                    int32_t length = SOPC_Variant_GetArrayOrMatrixLength(selectVal);
                    for (int32_t i = 0; i < length; i++)
                    {
                        applyDataValueTsRemoval(SOPC_VARIANT_GET_ARRAY_VALUES_PTR(selectVal, DataValue), removeSrc,
                                                removeSrv);
                    }
                }
            }
        }
    } // else: keep NULL variant selectVal content
}

static bool is_event_type_compatible_with_select_clause(const OpcUa_SimpleAttributeOperand* selectClause,
                                                        const SOPC_NodeId* eventTypeId)
{
    bool res = SOPC_NodeId_Equal(&BaseEventTypeId, &selectClause->TypeDefinitionId);
    if (!res)
    {
        res = SOPC_NodeId_Equal(&selectClause->TypeDefinitionId, eventTypeId);
    }
    return res;
}

bool util_event__alloc_and_fill_event_field_list(const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                                 uint32_t clientHandle,
                                                 char** preferredLocalesIds,
                                                 const constants__t_TimestampsToReturn_i timestampToReturn,
                                                 bool userAccessGranted,
                                                 const SOPC_Event* pEvent,
                                                 OpcUa_EventFieldList** ppEventFieldList)
{
    SOPC_ASSERT(NULL != pFilterCtx);
    SOPC_ASSERT(NULL != pEvent);
    SOPC_ASSERT(NULL != ppEventFieldList);
    SOPC_ASSERT(!pFilterCtx->isDataFilter);
    int32_t nbSelectClauses = pFilterCtx->Filter.Event.eventFilter->NoOfSelectClauses;
    bool res = nbSelectClauses > 0;
    bool setField = false;
    if (res)
    {
        res = util_event__alloc_event_field_list(clientHandle, nbSelectClauses, ppEventFieldList);
    }
    if (res)
    {
        const SOPC_NodeId* eventTypeId = SOPC_Event_GetEventTypeId(pEvent);
        for (int32_t i = 0; i < nbSelectClauses; i++)
        {
            setField = is_event_type_compatible_with_select_clause(
                &pFilterCtx->Filter.Event.eventFilter->SelectClauses[i], eventTypeId);
            if (setField)
            {
                util_event__set_event_field_list_elt(preferredLocalesIds, timestampToReturn, userAccessGranted, i,
                                                     pFilterCtx, pEvent, *ppEventFieldList);
            }
        }
    }

    return res;
}

static SOPC_Event* create_queueOverflowEvent(void)
{
    SOPC_ASSERT(NULL != initEventTypes);
    if (NULL == queueOverflowEvent)
    {
        SOPC_Event* QOfEvent = SOPC_EventManager_CreateEventInstance(initEventTypes, &EventQueueOverflowTypeId);
        if (NULL == QOfEvent)
        {
            // Impossible to instantiate the event type
            return false;
        }
        /**
         * From part 5 (v1.05) §6.4.34:
         * The SourceNode Property for Events of this type shall be assigned to the NodeId of the Server
         * Object. The SourceName for Events of this type shall be "Internal/EventQueueOverflow".
         */
        SOPC_ReturnStatus status = SOPC_Event_SetSourceNode(QOfEvent, &ServerId);
        SOPC_String sourceName;
        SOPC_String_Initialize(&sourceName);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_AttachFromCstring(&sourceName, "Internal/EventQueueOverflow");
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetSourceName(QOfEvent, &sourceName);
        }
        if (SOPC_STATUS_OK == status)
        {
            queueOverflowEvent = QOfEvent;
        }
        else
        {
            SOPC_Event_Delete(&QOfEvent);
        }
    }

    SOPC_Event* event = SOPC_Event_CreateCopy(queueOverflowEvent, true);
    if (NULL == event)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_Event_SetTime(event, SOPC_Time_GetCurrentTimeUTC());
    // We still want to generate the created queueOverflowEvent even if it fails
    SOPC_UNUSED_RESULT(status);
    return event;
}

bool util_event__gen_event_queue_overflow_notification(const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                                       uint32_t clientHandle,
                                                       OpcUa_EventFieldList** ppEventFieldList)
{
    SOPC_Event* queueOverflowEventInst = create_queueOverflowEvent();
    if (NULL == queueOverflowEventInst)
    {
        return false;
    }
    bool res = util_event__alloc_and_fill_event_field_list(pFilterCtx, clientHandle, NULL, constants__e_ttr_both, true,
                                                           queueOverflowEventInst, ppEventFieldList);
    SOPC_Event_Delete(&queueOverflowEventInst);
    return res;
}
