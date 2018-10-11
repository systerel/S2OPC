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
    gen_subscription_event_bs

SEES
    constants,
    message_in_bs,
    message_out_bs

OPERATIONS

    gen_data_changed_event (p_prev_dataValue, p_new_value_pointer) =
    PRE
        p_prev_dataValue : t_DataValue_i &
        p_prev_dataValue : t_DataValue &
        p_new_value_pointer : t_WriteValuePointer_i &
        p_new_value_pointer : t_WriteValuePointer
    THEN
        /* Generates a data changed event */
        skip
    END
    ;

    gen_data_changed_event_failed (p_prev_dataValue) =
    PRE
        p_prev_dataValue : t_DataValue_i &
        p_prev_dataValue : t_DataValue_i
    THEN
        /* Failed to generate data event, clear the prev value and generate trace */
        skip
    END

END
