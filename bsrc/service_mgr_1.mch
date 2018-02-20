/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    service_mgr_1

ABSTRACT_VARIABLES
    local_service_treatment

INVARIANT
    local_service_treatment : BOOL

INITIALISATION
    local_service_treatment := FALSE

OPERATIONS

    set_local_service_treatment =
    BEGIN
        local_service_treatment := TRUE
    END
    ;

    unset_local_service_treatment =
    BEGIN
        local_service_treatment := FALSE
    END
    ;

    bres <-- is_local_service_treatment =
    BEGIN
        bres := local_service_treatment
    END

END
