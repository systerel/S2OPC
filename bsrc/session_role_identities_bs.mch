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
    session_role_identities_bs

SEES
    constants

ABSTRACT_VARIABLES
    a_variant,
    a_identities

INVARIANT
    a_variant : t_Variant &
    a_identities : seq(t_Identity)

INITIALISATION
    a_variant := c_Variant_indet ||
    a_identities :(a_identities = {})

OPERATIONS

    p_nb <-- read_variant_nb_identities(p_variant, p_node) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant &
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        a_variant := p_variant ||
        a_identities :: seq(t_Identity) ||
        p_nb := size(a_identities)
    END
    ;

    p_identity <-- read_variant_identity(p_variant, p_index) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant &
        p_variant = a_variant &
        p_index : NAT1 &
        p_index <= size(a_identities)
    THEN
        p_identity :(
            p_identity : t_Identity_i &
            p_identity = a_identities(p_index)
        )
    END
END
