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

#include "monitored_item_event_filter_where_clause_bs.h"

#include "monitored_item_pointer_impl.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

void monitored_item_event_filter_where_clause_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void monitored_item_event_filter_where_clause_bs__init_where_elt_results(
    const constants__t_filterResult_i monitored_item_event_filter_where_clause_bs__p_filterResult)
{
    // Only for proof on preconditions
    SOPC_UNUSED_ARG(monitored_item_event_filter_where_clause_bs__p_filterResult);
}

void monitored_item_event_filter_where_clause_bs__set_where_element_result(
    const constants__t_filterResult_i monitored_item_event_filter_where_clause_bs__p_filterResult,
    const t_entier4 monitored_item_event_filter_where_clause_bs__p_whereEltIdx,
    const constants__t_RawStatusCode monitored_item_event_filter_where_clause_bs__p_rawOperatorSc,
    const constants__t_RawStatusCode monitored_item_event_filter_where_clause_bs__p_rawOperandSc)
{
    SOPC_ASSERT(monitored_item_event_filter_where_clause_bs__p_whereEltIdx > 0);
    const t_entier4 eltArrayIdx = monitored_item_event_filter_where_clause_bs__p_whereEltIdx - 1;

    monitored_item_event_filter_where_clause_bs__p_filterResult->WhereClauseResult.ElementResults[eltArrayIdx]
        .StatusCode = monitored_item_event_filter_where_clause_bs__p_rawOperatorSc;
    if (!SOPC_IsGoodStatus(monitored_item_event_filter_where_clause_bs__p_rawOperandSc))
    {
        monitored_item_event_filter_where_clause_bs__p_filterResult->WhereClauseResult.ElementResults[eltArrayIdx]
            .OperandStatusCodes[0] = monitored_item_event_filter_where_clause_bs__p_rawOperandSc;
    }
    else
    {
        // from part 4 table 119 (v1.05): operandStatusCodes list is empty if no operand error occurred
        // => make the list empty
        SOPC_Free(
            monitored_item_event_filter_where_clause_bs__p_filterResult->WhereClauseResult.ElementResults[eltArrayIdx]
                .OperandStatusCodes);
        monitored_item_event_filter_where_clause_bs__p_filterResult->WhereClauseResult.ElementResults[eltArrayIdx]
            .OperandStatusCodes = NULL;
        monitored_item_event_filter_where_clause_bs__p_filterResult->WhereClauseResult.ElementResults[eltArrayIdx]
            .NoOfOperandStatusCodes = 0;
    }
}
