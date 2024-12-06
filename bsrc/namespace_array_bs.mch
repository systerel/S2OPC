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
    namespace_array_bs

SEES
    constants

ABSTRACT_VARIABLES
    a_variant,
    a_namespaceUris

INVARIANT
    a_variant : t_Variant &
    a_namespaceUris : seq(t_NamespaceUri)

INITIALISATION
    a_variant := c_Variant_indet ||
    a_namespaceUris :(a_namespaceUris = {})

OPERATIONS

    p_nb <-- read_variant_nb_namespaceUris(p_variant) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant
    THEN
        a_variant := p_variant ||
        a_namespaceUris :: seq(t_NamespaceUri) ||
        p_nb := size(a_namespaceUris)
    END
    ;

    p_namespaceUri <-- read_variant_namespaceUri(p_variant, p_index) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant &
        p_variant = a_variant &
        p_index : NAT1 &
        p_index <= size(a_namespaceUris)
    THEN
        p_namespaceUri :(
            p_namespaceUri : t_NamespaceUri &
            p_namespaceUri = a_namespaceUris(p_index)
        )
    END
END
