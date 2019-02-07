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

    gen_data_changed_event (p_nid, p_attribute, p_prev_dataValue, p_new_val, p_new_val_sc, p_new_val_ts_src, p_new_val_ts_srv) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_attribute : t_AttributeId_i &
        p_attribute : t_AttributeId &
        p_prev_dataValue : t_DataValue_i &
        p_prev_dataValue : t_DataValue &
        p_new_val : t_Variant_i &
        p_new_val : t_Variant &
        p_new_val_sc : t_RawStatusCode &
        p_new_val_ts_src : t_Timestamp &
        p_new_val_ts_srv : t_Timestamp
    THEN
        /* Generates a data changed event */
        skip
    END
    ;

    gen_data_changed_event_failed =
    BEGIN
        /* Failed to generate data event, generate trace */
        skip
    END

END
