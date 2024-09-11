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
    roleset_references_it

SEES
    constants,
    address_space_itf

ABSTRACT_VARIABLES
    starting_node,
    a_Node_RefIndex_Reference,
    a_Node_References,
    references_to_iterate,
    references_iterated,
    next_reference_index

INVARIANT
    starting_node : t_Node_i &
    /* a_Node_RefIndex_Reference mimics RefIndex_Reference with a domain restricted to the starting_node.
       Note: RefIndex_Reference cannot be referenced in invariants.
    */
    a_Node_RefIndex_Reference : {starting_node} * t_RefIndex +-> t_Reference &
    a_Node_References <: t_Reference &
    a_Node_References = ran(a_Node_RefIndex_Reference) &
    references_to_iterate <: t_Reference &
    references_iterated   <: t_Reference &
    references_to_iterate /\ references_iterated = {}  &
    references_to_iterate \/ references_iterated = a_Node_References  &
    next_reference_index : NAT

INITIALISATION
    a_Node_RefIndex_Reference := {} ||
    a_Node_References := {} ||
    starting_node := c_Node_indet ||
    next_reference_index := 0 ||
    references_to_iterate := {} ||
    references_iterated := {}

OPERATIONS

    p_continue <-- init_iter_roleset_references(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : s_Node &
        c_Server_ServerCapabilities_RoleSet_NodeId = a_NodeId(p_node)
    THEN
        starting_node, next_reference_index,
        a_Node_RefIndex_Reference, a_Node_References,
        references_to_iterate, references_iterated,
        p_continue :(
            starting_node : t_Node_i & starting_node : t_Node &
            next_reference_index : NAT & next_reference_index : t_RefIndex &
            a_Node_RefIndex_Reference : {starting_node} * t_RefIndex +-> t_Reference &
            a_Node_References <: t_Reference &
            references_to_iterate <: t_Reference &
            references_iterated   <: t_Reference &
            p_continue : BOOL &
            starting_node = p_node &
            a_Node_RefIndex_Reference = {p_node} * (1.. Node_RefIndexEnd(p_node)) <| RefIndex_Reference &
            a_Node_References = ran(a_Node_RefIndex_Reference) &
            references_to_iterate = a_Node_References &
            references_iterated = {} &
            (references_to_iterate = {} =>
                p_continue = FALSE & next_reference_index = 0) &
            (references_to_iterate /= {} =>
                p_continue = TRUE & next_reference_index = 1)
        )
    END
    ;

    p_continue, p_ref <-- continue_iter_roleset_references =
    PRE
        references_to_iterate /= {} &
        a_NodeId(starting_node) = c_Server_ServerCapabilities_RoleSet_NodeId
    THEN
        references_iterated   := references_iterated   \/ {RefIndex_Reference(starting_node |-> next_reference_index)} ||
        references_to_iterate := references_to_iterate -  {RefIndex_Reference(starting_node |-> next_reference_index)} ||
        p_continue := bool(references_to_iterate - {RefIndex_Reference(starting_node |-> next_reference_index)} /= {}) ||
        p_ref, next_reference_index :(
            p_ref : t_Reference_i &
            next_reference_index : NAT & next_reference_index : t_RefIndex &
            p_ref = a_Node_RefIndex_Reference(starting_node |-> next_reference_index$0) &
            p_ref = RefIndex_Reference(starting_node |-> next_reference_index$0) &
            next_reference_index = next_reference_index$0 + 1
        )
    END

END

