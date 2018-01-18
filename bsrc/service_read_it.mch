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
    service_read_it

SEES
    constants

DEFINITIONS
    d_variables ==
        rreqs_nb_reqs,
        rreqs_to_iterate,
        rreqs_iterated
    ;

    d_inv ==
    (
        rreqs_nb_reqs : NAT &
        rreqs_nb_reqs <= k_n_read_resp_max &

        rreqs_to_iterate <: t_ReadValue &
        rreqs_iterated <: t_ReadValue &
        rreqs_to_iterate /\ rreqs_iterated = {}
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
        nb_req <= k_n_read_resp_max
    THEN
        rreqs_nb_reqs := nb_req ||
        rreqs_iterated := {} ||
        rreqs_to_iterate := cast_t_ReadValue[1..nb_req] ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, rvi <-- continue_iter_write_request =
    PRE
        rreqs_to_iterate /= {}
    THEN
        ANY l_rvi WHERE
            l_rvi : t_ReadValue_i &
            l_rvi : t_ReadValue &
            l_rvi : rreqs_to_iterate
        THEN
            rreqs_iterated := rreqs_iterated \/ {l_rvi} ||
            rreqs_to_iterate := rreqs_to_iterate - {l_rvi} ||
            rvi := l_rvi ||
            continue := bool(rreqs_to_iterate /= {l_rvi})
        END
    END

END
