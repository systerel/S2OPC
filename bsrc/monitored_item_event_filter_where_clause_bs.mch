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

MACHINE
    monitored_item_event_filter_where_clause_bs

SEES
    constants,
    constants_statuscodes_bs,
    monitored_item_event_filter_treatment_bs

ABSTRACT_VARIABLES
    c_filterResult,
    a_whereElementResultSet

INVARIANT
    c_filterResult : t_filterResult_i &
    a_whereElementResultSet <: NAT

INITIALISATION
    c_filterResult,
    a_whereElementResultSet
    :(
      c_filterResult = c_filterResult_indet &
      a_whereElementResultSet = {}
     )

OPERATIONS

    init_where_elt_results (p_filterResult) =
    PRE
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult
    THEN
        c_filterResult := p_filterResult ||
        a_whereElementResultSet := {}
    END
    ;

    set_where_element_result (p_filterResult, p_whereEltIdx, p_rawOperatorSc, p_rawOperandSc) =
    PRE
        p_filterResult : t_filterResult_i &
        p_filterResult : t_filterResult &
        c_filterResult = p_filterResult &
        p_whereEltIdx : NAT &
        p_whereEltIdx : 1..a_filter_NbWhereClauseElements &
        p_rawOperatorSc : t_RawStatusCode &
        p_rawOperandSc : t_RawStatusCode
    THEN
       a_whereElementResultSet := a_whereElementResultSet \/ {p_whereEltIdx}
    END

END
