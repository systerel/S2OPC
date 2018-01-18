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
    address_space_it

SEES
    constants

DEFINITIONS
    d_variables ==
        wreqs_nb_reqs,
        wreqs_to_iterate,
        wreqs_iterated
    ;

    d_inv ==
    (
        wreqs_nb_reqs : NAT &
        wreqs_nb_reqs <= k_n_WriteResponse_max &

        wreqs_to_iterate <: t_WriteValue &
        wreqs_iterated <: t_WriteValue &
        wreqs_to_iterate /\ wreqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS
    continue <-- init_iter_write_request(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_WriteResponse_max
    THEN
        wreqs_nb_reqs := nb_req ||
        wreqs_iterated := {} ||
        wreqs_to_iterate := cast_t_WriteValue[1..nb_req] ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, wvi <-- continue_iter_write_request =
    PRE
        wreqs_to_iterate /= {}
    THEN
        ANY l_wvi WHERE
            l_wvi : t_WriteValue_i &
            l_wvi : t_WriteValue &
            l_wvi : wreqs_to_iterate
        THEN
            wreqs_iterated := wreqs_iterated \/ {l_wvi} ||
            wreqs_to_iterate := wreqs_to_iterate - {l_wvi} ||
            wvi := l_wvi ||
            continue := bool(wreqs_to_iterate /= {l_wvi})
        END
    END

END
