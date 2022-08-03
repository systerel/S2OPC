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
    subscription_absent_node
SEES
    constants,
    constants_statuscodes_bs,
    address_space_itf

DEFINITIONS
    d_vars ==
        a_Node_availableInFuture
    ;

    d_inv == (
    /* Represents nodeId not present in address space but available in the future.
    MI is created in this case if the attribute is available for this node. */
    a_Node_availableInFuture : t_NodeId +-> t_NodeClass
    )

ABSTRACT_VARIABLES
    d_vars

INVARIANT
    d_inv

INITIALISATION
    d_vars : d_inv
OPERATIONS

    bres_absent_knownNode, knownNodeClass, valueSc <-- if_not_present_is_Node_known(p_node_is_present, p_endpoint_config_idx, p_nid) =
    PRE
        p_node_is_present : BOOL &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        bres_absent_knownNode, knownNodeClass, valueSc :(
            (bres_absent_knownNode : BOOL & knownNodeClass : t_NodeClass_i & valueSc : t_RawStatusCode) &
            (p_node_is_present = TRUE => (bres_absent_knownNode = FALSE & knownNodeClass = c_NodeClass_indet)) &
            (p_node_is_present = FALSE => (bres_absent_knownNode = bool(p_nid : dom(a_Node_availableInFuture)
                                           & bres_absent_knownNode = TRUE =>
                                            (knownNodeClass : t_NodeClass
                                             & conv_RawStatusCode_To_StatusCode(valueSc) /= e_sc_ok)))))
    END
    ;

    sc, value, val_ts_src, val_ts_srv <-- eval_knownNode_requested_properties(p_nid, p_knownNodeClass, p_aid, p_indexRange) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : dom(a_Node_availableInFuture) &
        p_knownNodeClass : t_NodeClass_i &
        p_knownNodeClass = a_Node_availableInFuture(p_nid) &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_indexRange : t_IndexRange_i
    THEN
        sc, value, val_ts_src, val_ts_srv :(
            sc : t_StatusCode_i & sc : {e_sc_bad_attribute_id_invalid, e_sc_bad_index_range_no_data, e_sc_ok} &
            value = c_Variant_indet &
            val_ts_src : t_Timestamp & val_ts_src = c_Timestamp_null &
            val_ts_srv : t_Timestamp & val_ts_srv = c_Timestamp_null &
            (not(a_NodeClass_Attribute(p_knownNodeClass |-> p_aid) = TRUE)
                => sc = e_sc_bad_attribute_id_invalid) &
            ((a_NodeClass_Attribute(p_knownNodeClass |-> p_aid) = TRUE)
                =>
                (p_aid /= e_aid_Value & IsRangeDefined(p_indexRange) = TRUE
                    => sc = e_sc_bad_index_range_no_data) &
                (not (p_aid /= e_aid_Value & IsRangeDefined(p_indexRange) = TRUE)
                    => sc = e_sc_ok))
        )
    END
END
