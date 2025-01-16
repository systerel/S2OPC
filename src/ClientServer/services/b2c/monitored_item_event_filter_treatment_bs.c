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

#include "monitored_item_event_filter_treatment_bs.h"

#include "monitored_item_pointer_impl.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_event_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_event.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_treatment_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static void free_filter_ctx(SOPC_InternalMonitoredItemFilterCtx* filterCtx)
{
    if (NULL != filterCtx)
    {
        if (NULL != filterCtx->Filter.Event.eventFilter)
        {
            for (int32_t i = 0; i < filterCtx->Filter.Event.eventFilter->NoOfSelectClauses; i++)
            {
                SOPC_NumericRange_Delete(filterCtx->Filter.Event.indexRangeSelectClauses[i]);
                SOPC_Free(filterCtx->Filter.Event.qnPathStrSelectClauses[i]);
            }
            SOPC_ReturnStatus status = SOPC_EncodeableObject_Delete(filterCtx->Filter.Event.eventFilter->encodeableType,
                                                                    (void**) &filterCtx->Filter.Event.eventFilter);
            SOPC_UNUSED_RESULT(status);
        }
        SOPC_Free(filterCtx->Filter.Event.indexRangeSelectClauses);
        SOPC_Free(filterCtx->Filter.Event.qnPathStrSelectClauses);
        SOPC_NodeId_Clear(&filterCtx->Filter.Event.whereClauseTypeId);
        SOPC_Free(filterCtx);
    }
}

void monitored_item_event_filter_treatment_bs__check_events_supported(
    const constants__t_endpoint_config_idx_i monitored_item_event_filter_treatment_bs__p_endpoint_idx,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__scEventsSupported)
{
    bool eventTypesSupported = init_event_types(monitored_item_event_filter_treatment_bs__p_endpoint_idx);
    if (eventTypesSupported)
    {
        *monitored_item_event_filter_treatment_bs__scEventsSupported = constants_statuscodes_bs__e_sc_ok;
    }
    else
    {
        *monitored_item_event_filter_treatment_bs__scEventsSupported =
            constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
    }
}

void monitored_item_event_filter_treatment_bs__check_is_event_notifier(
    const constants__t_Variant_i monitored_item_event_filter_treatment_bs__p_value,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__scIsEventNotifier)
{
    SOPC_Variant* eventNotifierByte = monitored_item_event_filter_treatment_bs__p_value;
    SOPC_ASSERT(NULL != eventNotifierByte);
    SOPC_ASSERT(SOPC_Byte_Id == eventNotifierByte->BuiltInTypeId);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventNotifierByte->ArrayType);
    if (0 == (OpcUa_EventNotifierType_SubscribeToEvents & eventNotifierByte->Value.Byte))
    {
        *monitored_item_event_filter_treatment_bs__scIsEventNotifier =
            constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
    }
    else
    {
        *monitored_item_event_filter_treatment_bs__scIsEventNotifier = constants_statuscodes_bs__e_sc_ok;
    }
}

extern void monitored_item_event_filter_treatment_bs__get_event_type_id(
    const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
    constants__t_NodeId_i* const monitored_item_event_filter_treatment_bs__nodeId)
{
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *monitored_item_event_filter_treatment_bs__nodeId =
        (constants__t_NodeId_i) SOPC_Event_GetEventTypeId(monitored_item_event_filter_treatment_bs__p_event);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

extern void monitored_item_event_filter_treatment_bs__get_select_clause_type_id(
    const t_entier4 monitored_item_event_filter_treatment_bs__p_selectClauseIdx,
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
    constants__t_NodeId_i* const monitored_item_event_filter_treatment_bs__nodeId)
{
    SOPC_InternalMonitoredItemFilterCtx* filterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx;

    *monitored_item_event_filter_treatment_bs__nodeId =
        &filterCtx->Filter.Event.eventFilter
             ->SelectClauses[monitored_item_event_filter_treatment_bs__p_selectClauseIdx - 1]
             .TypeDefinitionId;
}

extern void monitored_item_event_filter_treatment_bs__get_where_elt_of_type_id(
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
    constants__t_NodeId_i* const monitored_item_event_filter_treatment_bs__nodeId)
{
    SOPC_InternalMonitoredItemFilterCtx* filterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx;

    *monitored_item_event_filter_treatment_bs__nodeId = &filterCtx->Filter.Event.whereClauseTypeId;
}

extern void monitored_item_event_filter_treatment_bs__event_check_filter_ctx(
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
    const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
    t_bool* const monitored_item_event_filter_treatment_bs__bres,
    t_entier4* const monitored_item_event_filter_treatment_bs__nbSelectClauses,
    t_entier4* const monitored_item_event_filter_treatment_bs__nbWhereClauseElements)
{
    *monitored_item_event_filter_treatment_bs__bres = false;
    SOPC_UNUSED_ARG(monitored_item_event_filter_treatment_bs__p_event);
    SOPC_InternalMonitoredItemFilterCtx* filterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx;
    if (!filterCtx->isDataFilter && filterCtx->Filter.Event.eventFilter->NoOfSelectClauses > 0)
    {
        OpcUa_EventFilter* eventFilter =
            ((SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx)
                ->Filter.Event.eventFilter;
        *monitored_item_event_filter_treatment_bs__nbSelectClauses = eventFilter->NoOfSelectClauses;
        *monitored_item_event_filter_treatment_bs__nbWhereClauseElements = eventFilter->WhereClause.NoOfElements;
        *monitored_item_event_filter_treatment_bs__bres = true;
    }
}

void monitored_item_event_filter_treatment_bs__init_and_check_is_event_filter(
    const constants__t_monitoringFilter_i monitored_item_event_filter_treatment_bs__p_filter,
    const constants__t_AttributeId_i monitored_item_event_filter_treatment_bs__p_aid,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__scIsEventFilter)
{
    SOPC_ASSERT(constants__e_aid_EventNotifier == monitored_item_event_filter_treatment_bs__p_aid);
    SOPC_ASSERT(NULL != monitored_item_event_filter_treatment_bs__p_filter);
    *monitored_item_event_filter_treatment_bs__scIsEventFilter = constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
    if (&OpcUa_EventFilter_EncodeableType == monitored_item_event_filter_treatment_bs__p_filter->Body.Object.ObjType)
    {
        *monitored_item_event_filter_treatment_bs__scIsEventFilter = constants_statuscodes_bs__e_sc_ok;
    }
}

void monitored_item_event_filter_treatment_bs__init_event(
    const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event)
{
    // init B model
    SOPC_ASSERT(NULL != monitored_item_event_filter_treatment_bs__p_event);
}

static bool may_value_rank_n_dimensions_be_compatible(size_t n_dimensions, int32_t valueRank)
{
    if ((valueRank > 0 && valueRank == (int64_t) n_dimensions) || (n_dimensions > 0 && 0 == valueRank) ||
        -2 == valueRank || (-3 == valueRank && 1 == n_dimensions) || (-1 == valueRank && 0 == n_dimensions))
    {
        // All valid results:
        // 1) Same number of dimensions
        // 2) OneOrMoreDimensions
        // 3) Any number of dimensions
        // 4) One dimensional array dimension
        // 5) Scalar ByteString/String (0 dimensions) => array of bytes
        return true;
    }
    return false;
}

static SOPC_ReturnStatus check_index_range_may_be_applicable(const SOPC_NumericRange* indexRange,
                                                             const SOPC_NodeId* dataType,
                                                             int32_t valueRank)
{
    SOPC_ReturnStatus status = SOPC_STATUS_WOULD_BLOCK;
    if (may_value_rank_n_dimensions_be_compatible(indexRange->n_dimensions, valueRank))
    {
        return SOPC_STATUS_OK;
    }
    if (0 == dataType->Namespace && SOPC_IdentifierType_Numeric == dataType->IdentifierType &&
        (OpcUaId_String == dataType->Data.Numeric || OpcUaId_ByteString == dataType->Data.Numeric))
    {
        if (indexRange->n_dimensions > 0 && valueRank == -1 &&
            may_value_rank_n_dimensions_be_compatible(indexRange->n_dimensions - 1, valueRank))
        {
            return SOPC_STATUS_OK;
        }
    }
    return status;
}

void monitored_item_event_filter_treatment_bs__check_select_clause_and_fill_ctx(
    const t_entier4 monitored_item_event_filter_treatment_bs__p_selectClauseIdx,
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__selectStatusCode,
    constants__t_RawStatusCode* const monitored_item_event_filter_treatment_bs__clauseRawSc)
{
    *monitored_item_event_filter_treatment_bs__selectStatusCode = constants_statuscodes_bs__e_sc_ok;
    *monitored_item_event_filter_treatment_bs__clauseRawSc = SOPC_GoodGenericStatus;

    OpcUa_EventFilter* eventFilter =
        ((SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx)
            ->Filter.Event.eventFilter;
    SOPC_ASSERT(monitored_item_event_filter_treatment_bs__p_selectClauseIdx > 0);
    const t_entier4 clauseArrayIdx = monitored_item_event_filter_treatment_bs__p_selectClauseIdx - 1;
    SOPC_ASSERT(clauseArrayIdx < eventFilter->NoOfSelectClauses);

    OpcUa_SimpleAttributeOperand* selectClause = &eventFilter->SelectClauses[clauseArrayIdx];
    // Accepts Value attribute
    if (selectClause->AttributeId != SOPC_AttributeId_Value)
    {
        // Note: make an exception to manage the mechanism to obtain the event (node) NodeId (see ConditionId in part9).
        // When the attributeId is NodeId with an empty path, we will return the NodeId defined in Event if defined.
        if (selectClause->AttributeId != SOPC_AttributeId_NodeId || selectClause->NoOfBrowsePath > 0)
        {
            *monitored_item_event_filter_treatment_bs__clauseRawSc = OpcUa_BadAttributeIdInvalid;
            *monitored_item_event_filter_treatment_bs__selectStatusCode =
                constants_statuscodes_bs__e_sc_bad_monitored_item_filter_unsupported;
            return;
        }
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (OPCUA_NAMESPACE_INDEX == selectClause->TypeDefinitionId.Namespace &&
        SOPC_IdentifierType_Numeric == selectClause->TypeDefinitionId.IdentifierType &&
        OpcUaId_BaseEventType == selectClause->TypeDefinitionId.Data.Numeric)
    {
        // Select clause will be evaluated for any event type
        status = SOPC_STATUS_OK;
    }
    else
    {
        // Check the event type id is a known type and browse path is valid for the given type
        // Note: if AddNodes is implemented/allowed for EventTypes (i.e. ObjectTypes) it might be evaluated
        // when event is triggered
        status = SOPC_EventManager_HasEventTypeAndBrowsePath(initEventTypes,
                                                             (const SOPC_NodeId*) &selectClause->TypeDefinitionId,
                                                             selectClause->NoOfBrowsePath, selectClause->BrowsePath);
    }

    SOPC_StatusCode badResultStatus = OpcUa_BadBrowseNameInvalid; // if previous status NOK
    SOPC_NumericRange* indexRange = NULL;
    SOPC_Event* exampleEvent = NULL;
    const SOPC_Variant* exampleVar = NULL;
    const SOPC_NodeId* dataType = NULL;
    int32_t valueRank = 0;

    if (SOPC_STATUS_OK == status && selectClause->IndexRange.Length > 0)
    {
        // Check index range is valid
        const char* indexRangeStr = SOPC_String_GetRawCString(&selectClause->IndexRange);
        status = SOPC_NumericRange_Parse(indexRangeStr, &indexRange);

        // Check index range might be applicable for the event DataType / ValueRank
        if (SOPC_STATUS_OK == status)
        {
            exampleEvent = SOPC_EventManager_CreateEventInstance(initEventTypes,
                                                                 (const SOPC_NodeId*) &selectClause->TypeDefinitionId);
            status = (NULL == exampleEvent ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        }
        if (SOPC_STATUS_OK == status)
        {
            // uint16_t cast success enforced by checks in SOPC_EventManager_HasEventTypeAndBrowsePath
            exampleVar = SOPC_Event_GetVariableAndType(exampleEvent, (uint16_t) selectClause->NoOfBrowsePath,
                                                       selectClause->BrowsePath, &dataType, &valueRank);
            status = (NULL == exampleVar ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = check_index_range_may_be_applicable(indexRange, dataType, valueRank);
        }
        if (SOPC_STATUS_OK != status)
        {
            if (SOPC_STATUS_WOULD_BLOCK == status)
            {
                /* From part 4 (v1.05) table 149:
                 * The indexRange is valid but the value of the Attribute is never an array.
                 * Note: we suppose it is also applicable to Value attribute here.
                 */
                badResultStatus = OpcUa_BadTypeMismatch;
            }
            else
            {
                badResultStatus = OpcUa_BadIndexRangeInvalid;
            }
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        SOPC_Event_Delete(&exampleEvent);
    }

    if (SOPC_STATUS_OK != status)
    {
        if (SOPC_STATUS_INVALID_PARAMETERS != status || selectClause->NoOfBrowsePath <= 0)
        {
            badResultStatus = OpcUa_BadTypeDefinitionInvalid;
            if (SOPC_EventManager_HasEventType(initEventTypes, (const SOPC_NodeId*) &selectClause->TypeDefinitionId))
            {
                // StatusCode may seems unadapted but the associated description in part 4 (v1.05) table 149 is the
                // following: The browsePath is specified but it will never exist in any Event.
                badResultStatus = OpcUa_BadNodeIdUnknown;
            }
        }
        SOPC_NumericRange_Delete(indexRange);
        *monitored_item_event_filter_treatment_bs__clauseRawSc = badResultStatus;
        *monitored_item_event_filter_treatment_bs__selectStatusCode =
            constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
    }
    else
    {
        SOPC_InternalMonitoredItemFilterCtx* filterCtx =
            (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx;
        // Note: uint16_t possible cast already checked by SOPC_EventManager_HasEventTypeAndBrowsePath success
        status =
            SOPC_EventManagerUtil_QnPathToCString((uint16_t) selectClause->NoOfBrowsePath, selectClause->BrowsePath,
                                                  &filterCtx->Filter.Event.qnPathStrSelectClauses[clauseArrayIdx]);
        if (SOPC_STATUS_OK == status)
        {
            filterCtx->Filter.Event.indexRangeSelectClauses[clauseArrayIdx] = indexRange;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_NumericRange_Delete(indexRange);
            *monitored_item_event_filter_treatment_bs__clauseRawSc = OpcUa_BadOutOfMemory;
            *monitored_item_event_filter_treatment_bs__selectStatusCode =
                constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }
    }
}

static bool checkAndGetNodeIdOperand(SOPC_ExtensionObject* operand, SOPC_NodeId** outNodeId)
{
    SOPC_ASSERT(NULL != outNodeId);
    SOPC_NodeId* result = NULL;
    if (SOPC_ExtObjBodyEncoding_Object == operand->Encoding)
    {
        if (&OpcUa_LiteralOperand_EncodeableType == operand->Body.Object.ObjType)
        {
            OpcUa_LiteralOperand* litOp = (OpcUa_LiteralOperand*) operand->Body.Object.Value;
            if (SOPC_VariantArrayType_SingleValue == litOp->Value.ArrayType &&
                SOPC_NodeId_Id == litOp->Value.BuiltInTypeId)
            {
                result = litOp->Value.Value.NodeId;
            }
        }
        else if (&OpcUa_AttributeOperand_EncodeableType == operand->Body.Object.ObjType)
        {
            // Part 4 examples are using AttributeOperand even if LiteralOperand seems more appropriate
            OpcUa_AttributeOperand* attrOp = (OpcUa_AttributeOperand*) operand->Body.Object.Value;
            if (SOPC_AttributeId_NodeId == attrOp->AttributeId && attrOp->BrowsePath.NoOfElements <= 0 &&
                attrOp->IndexRange.Length <= 0)
            {
                result = &attrOp->NodeId;
            }
        }
    }
    *outNodeId = result;
    return NULL != result;
}

void monitored_item_event_filter_treatment_bs__check_where_elt_and_fill_ctx(
    const t_entier4 monitored_item_event_filter_treatment_bs__p_whereEltIdx,
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__statusCode,
    constants__t_RawStatusCode* const monitored_item_event_filter_treatment_bs__operatorRawSc,
    constants__t_RawStatusCode* const monitored_item_event_filter_treatment_bs__operandRawSc)
{
    SOPC_ASSERT(monitored_item_event_filter_treatment_bs__p_whereEltIdx > 0);
    const t_entier4 eltArrayIdx = monitored_item_event_filter_treatment_bs__p_whereEltIdx - 1;

    *monitored_item_event_filter_treatment_bs__statusCode =
        constants_statuscodes_bs__e_sc_bad_monitored_item_filter_unsupported;
    *monitored_item_event_filter_treatment_bs__operatorRawSc = OpcUa_BadUnexpectedError;
    *monitored_item_event_filter_treatment_bs__operandRawSc = OpcUa_BadUnexpectedError;

    if (eltArrayIdx > 0)
    {
        // Unsupported expression using operators
        return;
    }

    SOPC_InternalMonitoredItemFilterCtx* filterCtx =
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx;
    OpcUa_EventFilter* eventFilter = filterCtx->Filter.Event.eventFilter;

    OpcUa_ContentFilterElement* elt = &eventFilter->WhereClause.Elements[eltArrayIdx];
    if (OpcUa_FilterOperator_OfType != elt->FilterOperator)
    {
        // Only OfType operator is supported
        *monitored_item_event_filter_treatment_bs__operatorRawSc = OpcUa_BadMonitoredItemFilterUnsupported;
        *monitored_item_event_filter_treatment_bs__operandRawSc = SOPC_GoodGenericStatus;
    }
    else if (elt->NoOfFilterOperands != 1)
    {
        *monitored_item_event_filter_treatment_bs__statusCode =
            constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;

        // Expecting 1 operand only
        *monitored_item_event_filter_treatment_bs__operatorRawSc = OpcUa_BadFilterOperandCountMismatch;
        *monitored_item_event_filter_treatment_bs__operandRawSc = SOPC_GoodGenericStatus;
    }
    else
    {
        SOPC_NodeId* operandNodeId = NULL;
        bool result = checkAndGetNodeIdOperand(&elt->FilterOperands[0], &operandNodeId);
        if (result)
        {
            // Check the event type id is a known type
            // Note: if AddNodes is implemented/allowed for EventTypes (i.e. ObjectTypes) it might be evaluated
            // when event is triggered
            result = SOPC_EventManager_HasEventType(initEventTypes, operandNodeId);
        }
        if (result)
        {
            filterCtx->Filter.Event.whereClauseTypeId = *operandNodeId;
            SOPC_NodeId_Initialize(operandNodeId);

            *monitored_item_event_filter_treatment_bs__operatorRawSc = SOPC_GoodGenericStatus;
            *monitored_item_event_filter_treatment_bs__operandRawSc = SOPC_GoodGenericStatus;
            *monitored_item_event_filter_treatment_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
        }
        else
        {
            *monitored_item_event_filter_treatment_bs__operatorRawSc = SOPC_GoodGenericStatus;
            // Expecting NodeId operand only
            *monitored_item_event_filter_treatment_bs__operandRawSc = OpcUa_BadNodeIdInvalid;
            *monitored_item_event_filter_treatment_bs__statusCode =
                constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
        }
    }
}

void monitored_item_event_filter_treatment_bs__delete_event_filter_context(
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_treatment_bs__p_filterCtx)
{
    if (NULL != monitored_item_event_filter_treatment_bs__p_filterCtx)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        free_filter_ctx((SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_treatment_bs__p_filterCtx);
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
}

void monitored_item_event_filter_treatment_bs__delete_event_filter_result(
    const constants__t_filterResult_i monitored_item_event_filter_treatment_bs__p_filterResult)
{
    if (NULL != monitored_item_event_filter_treatment_bs__p_filterResult)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_EncodeableObject_Delete(&OpcUa_EventFilterResult_EncodeableType,
                                     (void**) &monitored_item_event_filter_treatment_bs__p_filterResult);
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
}

void monitored_item_event_filter_treatment_bs__get_event_source_node(
    const constants__t_Event_i monitored_item_event_filter_treatment_bs__p_event,
    constants__t_NodeId_i* const monitored_item_event_filter_treatment_bs__p_nodeId)
{
    *monitored_item_event_filter_treatment_bs__p_nodeId =
        SOPC_Event_GetSourceNode(monitored_item_event_filter_treatment_bs__p_event);
}

void monitored_item_event_filter_treatment_bs__init_event_filter_ctx_and_result(
    const constants__t_monitoringFilter_i monitored_item_event_filter_treatment_bs__p_filter,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_event_filter_treatment_bs__p_sc,
    constants__t_monitoringFilterCtx_i* const monitored_item_event_filter_treatment_bs__p_filterCtx,
    constants__t_filterResult_i* const monitored_item_event_filter_treatment_bs__p_filterResult,
    t_entier4* const monitored_item_event_filter_treatment_bs__p_nbSelectClauses,
    t_entier4* const monitored_item_event_filter_treatment_bs__p_nbWhereClausesElements)
{
    *monitored_item_event_filter_treatment_bs__p_filterResult = NULL;
    *monitored_item_event_filter_treatment_bs__p_filterCtx = NULL;
    *monitored_item_event_filter_treatment_bs__p_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    *monitored_item_event_filter_treatment_bs__p_nbSelectClauses = 0;
    *monitored_item_event_filter_treatment_bs__p_nbWhereClausesElements = 0;

    SOPC_ASSERT(NULL != monitored_item_event_filter_treatment_bs__p_filter);
    SOPC_ASSERT(&OpcUa_EventFilter_EncodeableType ==
                monitored_item_event_filter_treatment_bs__p_filter->Body.Object.ObjType);
    OpcUa_EventFilter* eventFilter =
        (OpcUa_EventFilter*) monitored_item_event_filter_treatment_bs__p_filter->Body.Object.Value;
    OpcUa_EventFilterResult* eventFilterResult = NULL;
    SOPC_InternalMonitoredItemFilterCtx* filterCtx = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (eventFilter->NoOfSelectClauses <= 0)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
        *monitored_item_event_filter_treatment_bs__p_sc =
            constants_statuscodes_bs__e_sc_bad_monitored_item_filter_invalid;
    }
    else
    {
        filterCtx = SOPC_Calloc(1, sizeof(*filterCtx));
        status = (NULL == filterCtx ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Create(&OpcUa_EventFilter_EncodeableType,
                                                  (void**) &filterCtx->Filter.Event.eventFilter);
        }
        if (SOPC_STATUS_OK == status)
        {
            filterCtx->isDataFilter = false;
            status = SOPC_EncodeableObject_Copy(eventFilter->encodeableType, filterCtx->Filter.Event.eventFilter,
                                                eventFilter);
            if (SOPC_STATUS_OK == status)
            {
                filterCtx->Filter.Event.qnPathStrSelectClauses = SOPC_Calloc(
                    (size_t) eventFilter->NoOfSelectClauses, sizeof(*filterCtx->Filter.Event.qnPathStrSelectClauses));
                status = (NULL == filterCtx->Filter.Event.qnPathStrSelectClauses ? SOPC_STATUS_OUT_OF_MEMORY
                                                                                 : SOPC_STATUS_OK);
            }
            if (SOPC_STATUS_OK == status)
            {
                filterCtx->Filter.Event.indexRangeSelectClauses = SOPC_Calloc(
                    (size_t) eventFilter->NoOfSelectClauses, sizeof(*filterCtx->Filter.Event.indexRangeSelectClauses));
                status = (NULL == filterCtx->Filter.Event.indexRangeSelectClauses ? SOPC_STATUS_OUT_OF_MEMORY
                                                                                  : SOPC_STATUS_OK);
            }
            if (SOPC_STATUS_OK == status)
            {
                SOPC_NodeId_Initialize(&filterCtx->Filter.Event.whereClauseTypeId);
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Create(&OpcUa_EventFilterResult_EncodeableType, (void**) &eventFilterResult);
        }
        if (SOPC_STATUS_OK == status)
        {
            eventFilterResult->SelectClauseResults =
                SOPC_Calloc((size_t) eventFilter->NoOfSelectClauses, sizeof(*eventFilterResult->SelectClauseResults));
            if (NULL != eventFilterResult->SelectClauseResults)
            {
                eventFilterResult->NoOfSelectClauseResults = eventFilter->NoOfSelectClauses;
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (SOPC_STATUS_OK == status && eventFilter->WhereClause.NoOfElements > 0)
        {
            eventFilterResult->WhereClauseResult.ElementResults =
                SOPC_Calloc((size_t) eventFilter->WhereClause.NoOfElements,
                            sizeof(*eventFilterResult->WhereClauseResult.ElementResults));
            if (NULL != eventFilterResult->WhereClauseResult.ElementResults)
            {
                eventFilterResult->WhereClauseResult.NoOfElementResults = eventFilter->WhereClause.NoOfElements;
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            if (SOPC_STATUS_OK == status)
            {
                for (int32_t i = 0;
                     SOPC_STATUS_OK == status && i < eventFilterResult->WhereClauseResult.NoOfElementResults; i++)
                {
                    OpcUa_ContentFilterElementResult* eltResult =
                        &eventFilterResult->WhereClauseResult.ElementResults[i];
                    OpcUa_ContentFilterElementResult_Initialize(eltResult);
                    eltResult->OperandStatusCodes =
                        SOPC_Calloc((size_t) eventFilter->WhereClause.Elements[i].NoOfFilterOperands,
                                    sizeof(*eltResult->OperandStatusCodes));
                    if (NULL != eltResult->OperandStatusCodes)
                    {
                        eltResult->NoOfOperandStatusCodes = eventFilter->WhereClause.Elements[i].NoOfFilterOperands;
                    }
                }
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *monitored_item_event_filter_treatment_bs__p_sc = constants_statuscodes_bs__e_sc_ok;
        *monitored_item_event_filter_treatment_bs__p_filterCtx = filterCtx;
        *monitored_item_event_filter_treatment_bs__p_filterResult = eventFilterResult;
        *monitored_item_event_filter_treatment_bs__p_nbSelectClauses = eventFilter->NoOfSelectClauses;
        *monitored_item_event_filter_treatment_bs__p_nbWhereClausesElements =
            (eventFilter->WhereClause.NoOfElements > 0 ? eventFilter->WhereClause.NoOfElements : 0);
    }
    else
    {
        free_filter_ctx(filterCtx);
        SOPC_EncodeableObject_Delete(&OpcUa_EventFilterResult_EncodeableType, (void**) &eventFilterResult);
    }
}
