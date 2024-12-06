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
    namespace_metadata_refs_it

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    nsMeta_starting_node,
    nsMeta_a_Node_RefIndex_Reference,
    nsMeta_a_Node_References,
    nsMeta_references_to_iterate,
    nsMeta_references_iterated,
    nsMeta_next_reference_index

INVARIANT
    nsMeta_starting_node : t_Node_i &
    /* nsMeta_a_Node_RefIndex_Reference mimics RefIndex_Reference with a domain restricted to the starting_node.
       Note: RefIndex_Reference cannot be referenced in invariants.
    */
    nsMeta_a_Node_RefIndex_Reference : {nsMeta_starting_node} * t_RefIndex >+> t_Reference &
    nsMeta_a_Node_References <: t_Reference &
    nsMeta_a_Node_References = ran(nsMeta_a_Node_RefIndex_Reference) &
    nsMeta_references_to_iterate <: t_Reference &
    nsMeta_references_iterated   <: t_Reference &
    nsMeta_references_to_iterate /\ nsMeta_references_iterated = {}  &
    nsMeta_references_to_iterate \/ nsMeta_references_iterated = nsMeta_a_Node_References  &
    nsMeta_next_reference_index : NAT

INITIALISATION
    nsMeta_a_Node_RefIndex_Reference := {} ||
    nsMeta_a_Node_References := {} ||
    nsMeta_starting_node := c_Node_indet ||
    nsMeta_next_reference_index := 0 ||
    nsMeta_references_to_iterate := {} ||
    nsMeta_references_iterated := {}

OPERATIONS

    p_continue <-- init_iter_namespacemetadata_refs(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : s_Node &
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(p_node)) = c_NamespaceMetadataType_NodeId
    THEN
        nsMeta_starting_node, nsMeta_next_reference_index,
        nsMeta_a_Node_RefIndex_Reference, nsMeta_a_Node_References,
        nsMeta_references_to_iterate, nsMeta_references_iterated,
        p_continue :(
            nsMeta_starting_node : t_Node_i & nsMeta_starting_node : t_Node &
            nsMeta_next_reference_index : NAT & nsMeta_next_reference_index : t_RefIndex &
            nsMeta_a_Node_RefIndex_Reference : {nsMeta_starting_node} * t_RefIndex +-> t_Reference &
            nsMeta_a_Node_References <: t_Reference &
            nsMeta_references_to_iterate <: t_Reference &
            nsMeta_references_iterated   <: t_Reference &
            p_continue : BOOL &
            nsMeta_starting_node = p_node &
            nsMeta_a_Node_RefIndex_Reference = {p_node} * (1.. Node_RefIndexEnd(p_node)) <| RefIndex_Reference &
            nsMeta_a_Node_References = ran(nsMeta_a_Node_RefIndex_Reference) &
            nsMeta_references_to_iterate = nsMeta_a_Node_References &
            nsMeta_references_iterated = {} &
            (nsMeta_references_to_iterate = {} =>
                p_continue = FALSE & nsMeta_next_reference_index = 0) &
            (nsMeta_references_to_iterate /= {} =>
                p_continue = TRUE & nsMeta_next_reference_index = 1)
        )
    END
    ;

    p_continue, p_ref <-- continue_iter_namespacemetadata_refs =
    PRE
        nsMeta_references_to_iterate /= {} &
        nsMeta_next_reference_index /= 0 &
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(nsMeta_starting_node)) = c_NamespaceMetadataType_NodeId
    THEN
        nsMeta_references_iterated   := nsMeta_references_iterated   \/ {RefIndex_Reference(nsMeta_starting_node |-> nsMeta_next_reference_index)} ||
        nsMeta_references_to_iterate := nsMeta_references_to_iterate -  {RefIndex_Reference(nsMeta_starting_node |-> nsMeta_next_reference_index)} ||
        p_continue := bool(nsMeta_references_to_iterate - {RefIndex_Reference(nsMeta_starting_node |-> nsMeta_next_reference_index)} /= {}) ||
        p_ref, nsMeta_next_reference_index :(
            p_ref : t_Reference_i &
            nsMeta_next_reference_index : NAT & nsMeta_next_reference_index : t_RefIndex &
            p_ref = nsMeta_a_Node_RefIndex_Reference(nsMeta_starting_node |-> nsMeta_next_reference_index$0) &
            p_ref = RefIndex_Reference(nsMeta_starting_node |-> nsMeta_next_reference_index$0) &
            nsMeta_next_reference_index = nsMeta_next_reference_index$0 + 1
        )
    END

END
