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
    translate_browse_path_it

SEES
    constants,
    translate_browse_path

DEFINITIONS
    d_variables ==
    browsePaths_to_iterate,
    browsePaths_iterated
    ;

    d_inv ==
    (
    browsePaths_to_iterate <: t_BrowsePath &
    browsePaths_iterated   <: t_BrowsePath &
    browsePaths_to_iterate /\ browsePaths_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browsePaths =
    BEGIN
        browsePaths_iterated   := {}        ||
        browsePaths_to_iterate := BrowsePaths ||
        p_continue             := bool(browsePaths_to_iterate /= {})
    END
    ;

    p_continue, p_browsePath <-- continue_iter_browsePath =
    PRE
        browsePaths_to_iterate /= {}
    THEN
        ANY l_browsePath WHERE
            l_browsePath : t_BrowsePath_i &
            l_browsePath : t_BrowsePath &
            l_browsePath : browsePaths_to_iterate
        THEN
            browsePaths_iterated   := browsePaths_iterated   \/ {l_browsePath} ||
            browsePaths_to_iterate := browsePaths_to_iterate -  {l_browsePath} ||
            p_browsePath           := l_browsePath                          ||
            p_continue             := bool(browsePaths_to_iterate /= {l_browsePath})
        END
    END

END
