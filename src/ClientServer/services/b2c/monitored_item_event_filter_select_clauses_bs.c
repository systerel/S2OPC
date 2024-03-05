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

#include "monitored_item_event_filter_select_clauses_bs.h"

#include "monitored_item_pointer_impl.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "util_event.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_event_filter_select_clauses_bs__INITIALISATION(void) {}
/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_select_clauses_bs__alloc_event_field_list(
    const constants__t_client_handle_i monitored_item_event_filter_select_clauses_bs__p_clientHandle,
    const t_entier4 monitored_item_event_filter_select_clauses_bs__p_nbSelectClauses,
    t_bool* const monitored_item_event_filter_select_clauses_bs__bres,
    constants__t_eventFieldList_i* const monitored_item_event_filter_select_clauses_bs__p_eventFieldList)
{
    *monitored_item_event_filter_select_clauses_bs__bres =
        util_event__alloc_event_field_list(monitored_item_event_filter_select_clauses_bs__p_clientHandle,
                                           monitored_item_event_filter_select_clauses_bs__p_nbSelectClauses,
                                           monitored_item_event_filter_select_clauses_bs__p_eventFieldList);
}

void monitored_item_event_filter_select_clauses_bs__init_select_clause_results(
    const constants__t_filterResult_i monitored_item_event_filter_select_clauses_bs__p_filterResult)
{
    // Only for proof on preconditions
    SOPC_UNUSED_ARG(monitored_item_event_filter_select_clauses_bs__p_filterResult);
}

void monitored_item_event_filter_select_clauses_bs__set_event_field_list_elt(
    const constants__t_LocaleIds_i monitored_item_event_filter_select_clauses_bs__p_localeIds,
    const constants__t_TimestampsToReturn_i monitored_item_event_filter_select_clauses_bs__p_timestampToReturn,
    const t_bool monitored_item_event_filter_select_clauses_bs__p_userAccessGranted,
    const t_entier4 monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx,
    const constants__t_monitoringFilterCtx_i monitored_item_event_filter_select_clauses_bs__p_filterCtx,
    const constants__t_eventFieldList_i monitored_item_event_filter_select_clauses_bs__p_eventFieldList,
    const constants__t_Event_i monitored_item_event_filter_select_clauses_bs__p_event)
{
    util_event__set_event_field_list_elt(
        monitored_item_event_filter_select_clauses_bs__p_localeIds,
        monitored_item_event_filter_select_clauses_bs__p_timestampToReturn,
        monitored_item_event_filter_select_clauses_bs__p_userAccessGranted,
        monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx - 1,
        (SOPC_InternalMonitoredItemFilterCtx*) monitored_item_event_filter_select_clauses_bs__p_filterCtx,
        monitored_item_event_filter_select_clauses_bs__p_event,
        monitored_item_event_filter_select_clauses_bs__p_eventFieldList);
}

void monitored_item_event_filter_select_clauses_bs__set_event_field_list_elt_null(
    const t_entier4 monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx,
    const constants__t_eventFieldList_i monitored_item_event_filter_select_clauses_bs__p_eventFieldList)
{
    // Nothing to do: for proof only
    SOPC_UNUSED_ARG(monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx);
    SOPC_UNUSED_ARG(monitored_item_event_filter_select_clauses_bs__p_eventFieldList);
}

void monitored_item_event_filter_select_clauses_bs__set_select_clause_result(
    const constants__t_filterResult_i monitored_item_event_filter_select_clauses_bs__p_filterResult,
    const t_entier4 monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx,
    const constants__t_RawStatusCode monitored_item_event_filter_select_clauses_bs__p_rawSc)
{
    SOPC_ASSERT(monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx > 0);
    const t_entier4 clauseArrayIdx = monitored_item_event_filter_select_clauses_bs__p_selectClauseIdx - 1;

    monitored_item_event_filter_select_clauses_bs__p_filterResult->SelectClauseResults[clauseArrayIdx] =
        monitored_item_event_filter_select_clauses_bs__p_rawSc;
}
