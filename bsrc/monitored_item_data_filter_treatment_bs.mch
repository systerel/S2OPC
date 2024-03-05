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
    monitored_item_data_filter_treatment_bs

SEES
    constants,
    constants_statuscodes_bs

ABSTRACT_VARIABLES
    a_monitoredItemNodeDataFilterValid

INVARIANT
    a_monitoredItemNodeDataFilterValid : t_Node <-> t_monitoringFilter

INITIALISATION
    a_monitoredItemNodeDataFilterValid :(a_monitoredItemNodeDataFilterValid : t_Node <-> t_monitoringFilter)

OPERATIONS

    StatusCode, filterCtx <-- check_monitored_item_data_filter_valid (p_node, p_filter) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node &
        p_filter : t_monitoringFilter_i &
        p_filter : t_monitoringFilter
    THEN
        /* Check filter is a DataChangeFilter and check validity regarding node DataType is Number (absolute) / Variable AnalogItem type EU range (percent)
           and returns an absolute deadband context for any deadband type (pre-computed from EURange if PercentDeadband) */
        IF (p_node |-> p_filter) : a_monitoredItemNodeDataFilterValid
        THEN
            filterCtx :(filterCtx : t_monitoringFilterCtx_i &
                        filterCtx : t_monitoringFilterCtx) ||
            StatusCode := e_sc_ok
        ELSE
            filterCtx := c_monitoringFilterCtx_indet ||
            StatusCode :(StatusCode : {e_sc_bad_out_of_memory, e_sc_bad_filter_not_allowed})
        END
    END

END
