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
    address_space_typing

SEES
    constants

DEFINITIONS
    d_variables ==

        /* Typing */
        Type_IsTransitiveSubtype,
        Type_IsIncludedValueRank
    ;

    d_inv ==
    (
        /* Typing */
        Type_IsTransitiveSubtype : t_NodeId <-> t_NodeId &
        Type_IsIncludedValueRank : t_ConcreteValueRank <-> t_ValueRank
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables
    :(
        d_inv
    )

OPERATIONS

    /* Types */
    bres <-- is_transitive_subtype(p_subtype, p_parent_type) =
    PRE
        p_subtype : t_NodeId_i &
        p_subtype : t_NodeId &
        p_parent_type : t_NodeId_i &
        p_parent_type : t_NodeId
    THEN
        bres := bool(p_subtype |-> p_parent_type : Type_IsTransitiveSubtype)
    END
    ;

    bres <-- is_included_ValueRank(p_concValueRank, p_expValueRank) =
    PRE
        p_concValueRank : t_ConcreteValueRank &
        p_expValueRank : t_ValueRank
    THEN
        bres :(bres : BOOL &
               bres = bool((p_expValueRank = c_ValueRank_ScalarOrOneDimension & p_concValueRank : {c_ValueRank_Scalar,c_ValueRank_OneDimension})
                           or
                           (p_expValueRank = c_ValueRank_Any)
                           or
                           (p_expValueRank = c_ValueRank_Scalar & p_concValueRank = c_ValueRank_Scalar)
                           or
                           (p_expValueRank = c_ValueRank_OneOrMoreDimensions  & p_concValueRank >= c_ValueRank_OneDimension)
                           or
                           (p_expValueRank >= c_ValueRank_OneDimension & p_expValueRank = p_concValueRank)) &
               (bres = TRUE <=> ((p_concValueRank |-> p_expValueRank) : Type_IsIncludedValueRank))
              )
    END

END
