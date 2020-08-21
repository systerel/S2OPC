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
    time_reference_bs

SEES
    constants

ABSTRACT_VARIABLES
    TimeReference_IsLessThan
INVARIANT
    TimeReference_IsLessThan : t_timeref <-> t_timeref
INITIALISATION
    TimeReference_IsLessThan :(TimeReference_IsLessThan : t_timeref <-> t_timeref)
OPERATIONS

    p_timeref <-- get_current_TimerReference =
    BEGIN
        p_timeref :(p_timeref : t_timeref_i & p_timeref : t_timeref)
    END
    ;

    p_is_oldest <-- is_less_than_TimeReference (p_left, p_right) =
    PRE
        p_left : t_timeref_i &
        p_left : t_timeref &
        p_right : t_timeref_i &
        p_right : t_timeref
    THEN
        p_is_oldest := bool(p_left |-> p_right : TimeReference_IsLessThan)
    END
END
