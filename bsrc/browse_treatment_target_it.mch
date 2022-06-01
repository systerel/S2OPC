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
    browse_treatment_target_it

SEES
    constants,
    address_space_itf

ABSTRACT_VARIABLES
    starting_node,
    references_to_iterate,
    references_iterated,
    next_reference_index

INVARIANT
    starting_node : t_Node_i &
    references_to_iterate <: NAT &
    references_iterated   <: NAT &
    references_to_iterate /\ references_iterated = {} &
    next_reference_index : NAT

INITIALISATION
    starting_node := c_Node_indet ||
    next_reference_index := 0 ||
    references_to_iterate := {} ||
    references_iterated := {}

OPERATIONS

    p_continue <-- init_iter_reference(p_node, p_startIndex) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node   &
        p_startIndex : NAT &
        p_startIndex : t_RefIndex &
        p_startIndex > 0
    THEN
        starting_node := p_node ||
        next_reference_index := p_startIndex ||
        references_to_iterate := p_startIndex..Node_RefIndexEnd(p_node) ||
        references_iterated := {} ||
        p_continue := bool(Node_RefIndexEnd(p_node) > 0 &  p_startIndex <= Node_RefIndexEnd(p_node))
    END
    ;

    p_continue, p_ref, p_nextRefIndex <-- continue_iter_reference =
    PRE
        references_to_iterate /= {} &
        next_reference_index /= 0
    THEN
        references_iterated   := references_iterated   \/ {next_reference_index} ||
        references_to_iterate := references_to_iterate -  {next_reference_index} ||
        p_continue := bool(references_to_iterate - {next_reference_index} /= {}) ||
        p_ref :(p_ref : t_Reference_i & p_ref = RefIndex_Reference(starting_node |-> next_reference_index)) ||
        next_reference_index,
        p_nextRefIndex :(next_reference_index : NAT & next_reference_index : t_RefIndex &
                         next_reference_index = next_reference_index$0 + 1 &
                         p_nextRefIndex = next_reference_index)
    END

END
