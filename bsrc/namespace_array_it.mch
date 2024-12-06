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
    namespace_array_it

SEES
    constants

DEFINITIONS
    d_variables ==
    namespaceUrisIdx_to_iterate,
    namespaceUrisIdx_iterated,
    nb_namespaceUris
    ;

    d_inv ==
    (
    nb_namespaceUris : NAT &
    namespaceUrisIdx_to_iterate <: NAT1 &
    namespaceUrisIdx_iterated   <: NAT1 &
    namespaceUrisIdx_to_iterate /\ namespaceUrisIdx_iterated = {} &
    1..nb_namespaceUris = namespaceUrisIdx_to_iterate \/ namespaceUrisIdx_iterated
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_namespaceUris(p_nb_namespaceUris) =
    PRE
        p_nb_namespaceUris : NAT
    THEN
        namespaceUrisIdx_iterated,
        namespaceUrisIdx_to_iterate,
        nb_namespaceUris,
        p_continue
        :(nb_namespaceUris : NAT &
          namespaceUrisIdx_to_iterate <: NAT1 &
          namespaceUrisIdx_iterated   <: NAT1 &
          p_continue : BOOL &
          p_continue = bool(p_nb_namespaceUris > 0 & p_nb_namespaceUris : dom(cast_t_NamespaceIdx)) &
          (p_continue = TRUE =>
                namespaceUrisIdx_iterated = {} &
                namespaceUrisIdx_to_iterate = 1..p_nb_namespaceUris &
                nb_namespaceUris = p_nb_namespaceUris
          ) &
          (p_continue = FALSE =>
                namespaceUrisIdx_iterated = {} &
                namespaceUrisIdx_to_iterate = {} &
                nb_namespaceUris = 0
          )
        )
    END
    ;

    p_continue, p_namespaceUriIdx <-- continue_iter_namespaceUris =
    PRE
        namespaceUrisIdx_to_iterate /= {}
    THEN
        ANY l_namespaceUriIdx WHERE
            l_namespaceUriIdx : NAT &
            l_namespaceUriIdx : namespaceUrisIdx_to_iterate
        THEN
            namespaceUrisIdx_iterated   := namespaceUrisIdx_iterated   \/ {l_namespaceUriIdx} ||
            namespaceUrisIdx_to_iterate := namespaceUrisIdx_to_iterate -  {l_namespaceUriIdx} ||
            p_namespaceUriIdx           := l_namespaceUriIdx                                  ||
            p_continue                  := bool(namespaceUrisIdx_to_iterate /= {l_namespaceUriIdx})
        END
    END

END
