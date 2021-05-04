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
    argument_pointer_bs

SEES
    constants,
    constants_statuscodes_bs

DEFINITIONS
    d_variables ==
    Variant_Arguments,
    Argument_DataType,
    Argument_ValueRank
    ;

    d_inv ==
    (
        Variant_Arguments : t_Variant +-> seq(t_Argument) &
        Argument_DataType : t_Argument --> t_NodeId & /* ExpandedNodeId from different server returns Structure <=> NodeId(i=22)
        / Null value return NodeId(i=0) */
        Argument_ValueRank : t_Argument --> t_ConcreteValueRank
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

    p_nb, p_bres <-- read_variant_nb_argument(p_variant, p_node) =
    PRE
        p_variant: t_Variant_i &
        p_variant: t_Variant &
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        IF p_variant : dom(Variant_Arguments)
        THEN
            p_nb := size(Variant_Arguments(p_variant)) ||
            p_bres := TRUE
        ELSE
            p_nb := 0 ||
            p_bres := FALSE
        END
    END
    ;

    p_arg <-- read_variant_argument(p_variant, p_index) =
    PRE
        p_variant: t_Variant_i &
        p_variant: t_Variant &
        p_variant : dom(Variant_Arguments) &
        p_index : NAT1 &
        p_index <= size(Variant_Arguments(p_variant))
    THEN
        p_arg :(
            p_arg : t_Argument_i &
            p_arg = Variant_Arguments(p_variant)(p_index)
        )
    END
    ;

    p_type <-- read_argument_type(p_arg)=
    PRE
        p_arg : t_Argument_i &
        p_arg : t_Argument
    THEN
        p_type :(p_type : t_NodeId_i & p_type = Argument_DataType(p_arg))

    END
    ;
    p_vr <-- read_argument_valueRank(p_arg)=
    PRE
        p_arg : t_Argument_i &
        p_arg : t_Argument
    THEN
        p_vr := Argument_ValueRank(p_arg)

    END

END
