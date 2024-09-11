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
    session_role_identity_bs

SEES
    constants

DEFINITIONS
    d_variables ==
        a_identity_criteriaType,
        a_identity_criteria
        ;

    d_inv ==
    (
        a_identity_criteriaType : t_Identity --> t_CriteriaType_i &
        a_identity_criteria : t_Identity --> t_Criteria
    );

    d_init ==
    d_inv

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_init )

OPERATIONS

    p_criteriaType <-- read_identity_criteriaType(p_identity) =
    PRE
        p_identity : t_Identity_i &
        p_identity : t_Identity
    THEN
        p_criteriaType :(p_criteriaType = a_identity_criteriaType(p_identity))
    END
    ;

    p_criteria <-- read_identity_criteria(p_identity) =
    PRE
        p_identity : t_Identity_i &
        p_identity : t_Identity
    THEN
        p_criteria :(p_criteria : t_Criteria_i & p_criteria = a_identity_criteria(p_identity))
    END

END
