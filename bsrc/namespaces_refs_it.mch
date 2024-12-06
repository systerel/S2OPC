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
    namespaces_refs_it

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    ns_starting_node,
    ns_a_Node_RefIndex_Reference,
    ns_a_Node_References,
    ns_references_to_iterate,
    ns_references_iterated,
    ns_next_reference_index

INVARIANT
    ns_starting_node : t_Node_i &
    /* ns_a_Node_RefIndex_Reference mimics RefIndex_Reference with a domain restricted to the starting_node.
       Note: RefIndex_Reference cannot be referenced in invariants.
    */
    ns_a_Node_RefIndex_Reference : {ns_starting_node} * t_RefIndex >+> t_Reference &
    ns_a_Node_References <: t_Reference &
    ns_a_Node_References = ran(ns_a_Node_RefIndex_Reference) &
    ns_references_to_iterate <: t_Reference &
    ns_references_iterated   <: t_Reference &
    ns_references_to_iterate /\ ns_references_iterated = {}  &
    ns_references_to_iterate \/ ns_references_iterated = ns_a_Node_References  &
    ns_next_reference_index : NAT

INITIALISATION
    ns_a_Node_RefIndex_Reference := {} ||
    ns_a_Node_References := {} ||
    ns_starting_node := c_Node_indet ||
    ns_next_reference_index := 0 ||
    ns_references_to_iterate := {} ||
    ns_references_iterated := {}

OPERATIONS

    p_continue <-- init_iter_namespaces_refs(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : s_Node &
        c_Server_Namespaces_NodeId = a_NodeId(p_node)
    THEN
        ns_starting_node, ns_next_reference_index,
        ns_a_Node_RefIndex_Reference, ns_a_Node_References,
        ns_references_to_iterate, ns_references_iterated,
        p_continue :(
            ns_starting_node : t_Node_i & ns_starting_node : t_Node &
            ns_next_reference_index : NAT & ns_next_reference_index : t_RefIndex &
            ns_a_Node_RefIndex_Reference : {ns_starting_node} * t_RefIndex +-> t_Reference &
            ns_a_Node_References <: t_Reference &
            ns_references_to_iterate <: t_Reference &
            ns_references_iterated   <: t_Reference &
            p_continue : BOOL &
            ns_starting_node = p_node &
            ns_a_Node_RefIndex_Reference = {p_node} * (1.. Node_RefIndexEnd(p_node)) <| RefIndex_Reference &
            ns_a_Node_References = ran(ns_a_Node_RefIndex_Reference) &
            ns_references_to_iterate = ns_a_Node_References &
            ns_references_iterated = {} &
            (ns_references_to_iterate = {} =>
                p_continue = FALSE & ns_next_reference_index = 0) &
            (ns_references_to_iterate /= {} =>
                p_continue = TRUE & ns_next_reference_index = 1)
        )
    END
    ;

    p_continue, p_ref <-- continue_iter_namespaces_refs =
    PRE
        ns_references_to_iterate /= {} &
        ns_next_reference_index /= 0 &
        a_NodeId(ns_starting_node) = c_Server_Namespaces_NodeId
    THEN
        ns_references_iterated   := ns_references_iterated   \/ {RefIndex_Reference(ns_starting_node |-> ns_next_reference_index)} ||
        ns_references_to_iterate := ns_references_to_iterate -  {RefIndex_Reference(ns_starting_node |-> ns_next_reference_index)} ||
        p_continue := bool(ns_references_to_iterate - {RefIndex_Reference(ns_starting_node |-> ns_next_reference_index)} /= {}) ||
        p_ref, ns_next_reference_index :(
            p_ref : t_Reference_i &
            ns_next_reference_index : NAT & ns_next_reference_index : t_RefIndex &
            p_ref = ns_a_Node_RefIndex_Reference(ns_starting_node |-> ns_next_reference_index$0) &
            p_ref = RefIndex_Reference(ns_starting_node |-> ns_next_reference_index$0) &
            ns_next_reference_index = ns_next_reference_index$0 + 1
        )
    END

END
