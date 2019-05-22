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
    data_value_pointer_bs

SEES
    constants,
    constants_statuscodes_bs

DEFINITIONS

    d_variables ==
        DataValue_Variant,
        DataValue_StatusCode,
        DataValue_SourceTimestamp,
        DataValue_ServerTimestamp,

        DataValue_LocalDataType,
        DataValue_ValueRank
    ;

    d_inv ==
    (
        DataValue_Variant : t_DataValue --> t_Variant &
        DataValue_StatusCode : t_DataValue --> t_RawStatusCode &
        DataValue_SourceTimestamp : t_DataValue --> t_Timestamp &
        DataValue_ServerTimestamp : t_DataValue --> t_Timestamp &

        DataValue_LocalDataType : t_DataValue --> t_NodeId & /* ExpandedNodeId from different server returns Structure <=> NodeId(i=22)
                                                                / Null value return NodeId(i=0) */
        DataValue_ValueRank : t_DataValue --> t_ConcreteValueRank
    );

    d_init ==
        d_inv

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_init )

OPERATIONS

    p_variant <-- get_conv_DataValue_Variant(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue
    THEN
        p_variant :(p_variant : t_Variant_i & p_variant = DataValue_Variant(p_dataValue))
    END
    ;

    p_sc <-- get_conv_DataValue_Status(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue
    THEN
        p_sc := DataValue_StatusCode(p_dataValue)
    END
    ;

    p_st <-- get_conv_DataValue_SourceTimestamp(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue
    THEN
        p_st := DataValue_SourceTimestamp(p_dataValue)
    END
    ;

    p_dt <-- get_conv_DataValue_LocalDataType(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue
    THEN
        p_dt :(p_dt : t_NodeId_i & p_dt = DataValue_LocalDataType(p_dataValue))
    END
    ;

    p_vr <-- get_conv_DataValue_ValueRank(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue
    THEN
        p_vr := DataValue_ValueRank(p_dataValue)
    END

END
