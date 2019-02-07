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
    local_service_treatment : NAT

INITIALISATION
    local_service_treatment := 0

OPERATIONS

    set_local_service_treatment =
    BEGIN
        local_service_treatment := local_service_treatment + 1
    END
    ;

    unset_local_service_treatment =
    BEGIN
        IF local_service_treatment /= 0 THEN
            local_service_treatment := local_service_treatment - 1
        END
    END
    ;

    bres <-- is_local_service_treatment =
    BEGIN
        bres := bool(local_service_treatment /= 0)
    END

END
