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
    address_space_namespaces_indexes

SEES
    constants

ABSTRACT_VARIABLES
    a_nsIdxMax

INVARIANT
    a_nsIdxMax : t_NamespaceIdx

INITIALISATION
    a_nsIdxMax := cast_t_NamespaceIdx(0)
OPERATIONS

    update_max_namespace_idx =
    BEGIN
        a_nsIdxMax :(a_nsIdxMax : t_NamespaceIdx)
    END
    ;

    bres <-- has_NamespaceIndex_index(p_idx) =
    PRE
        p_idx : t_NamespaceIdx
    THEN
        bres := bool(cast_t_NamespaceIdx~(p_idx) <= cast_t_NamespaceIdx~(a_nsIdxMax))
    END

END
