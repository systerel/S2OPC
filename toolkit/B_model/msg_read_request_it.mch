/*
 *  Copyright (C) 2017 Systerel and others.
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
    msg_read_request_it

SEES
    constants

DEFINITIONS
    d_inv ==
    (
        nb_reqs : NAT &
        nb_reqs <= k_n_read_resp_max &

        reqs_to_iterate <: t_ReadValue &
        reqs_iterated <: t_ReadValue &
        reqs_to_iterate /\ reqs_iterated = {}
    )
    /*;
    d_inv_uninit ==
    (
        nb_reqs = 0 &
        reqs_to_iterate = {} &
        reqs_iterated = {}
    )
    ;
    d_inv_init ==
    (
        0 < nb_reqs &
        nb_reqs <= k_n_read_resp_max &

        reqs_to_iterate /\ reqs_iterated = {} &
        card(reqs_to_iterate \/ reqs_iterated) = nb_reqs
    )*/

ABSTRACT_VARIABLES
    nb_reqs,
    reqs_to_iterate,
    reqs_iterated

INVARIANT
    d_inv /* & (d_inv_uninit or d_inv_init) */

INITIALISATION
    /* TODO: rename rreqs_nb_reqs, ... */
    nb_reqs, reqs_to_iterate, reqs_iterated :(d_inv)

OPERATIONS
    continue <-- init_iter_reqs(a_nb_reqs) =
    PRE
        a_nb_reqs : NAT &
        a_nb_reqs <= k_n_read_resp_max
    THEN
        nb_reqs := a_nb_reqs ||
        reqs_iterated := {} ||
        reqs_to_iterate := cast_t_ReadValue[1..a_nb_reqs] ||
        continue := bool(0 < a_nb_reqs)
    END
    ;

    continue, rvi <-- continue_iter_reqs =
    PRE
        reqs_to_iterate /= {}
    THEN
        ANY l_rvi WHERE
            l_rvi : t_ReadValue_i &
            l_rvi : t_ReadValue &
            l_rvi : reqs_to_iterate
        THEN
            reqs_iterated := reqs_iterated \/ {l_rvi} ||
            reqs_to_iterate := reqs_to_iterate - {l_rvi} ||
            rvi := l_rvi ||
            continue := bool(reqs_to_iterate /= {l_rvi})
        END
    END

END
