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
    address_space_local

ABSTRACT_VARIABLES
    local_service_treatment

INVARIANT
    local_service_treatment : BOOL

INITIALISATION
    local_service_treatment := FALSE

OPERATIONS

    p_res <-- is_local_service_treatment =
    BEGIN
        p_res := local_service_treatment
    END
    ;

    set_local_service_treatment(p_val) =
    PRE
        p_val : BOOL
    THEN
        local_service_treatment := p_val
    END

END
