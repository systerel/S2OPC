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
    service_browse_seq_it

SEES
    constants

DEFINITIONS
    d_variables ==
        breqs_nb_reqs,
        breqs_to_iterate,
        breqs_iterated
    ;

    d_inv ==
    (
        breqs_nb_reqs : NAT &
        breqs_nb_reqs <= k_n_BrowseResponse_max &

        breqs_to_iterate <: t_BrowseValue &
        breqs_iterated <: t_BrowseValue &
        breqs_to_iterate /\ breqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browse_request(p_nb_req) =
    PRE
        p_nb_req : NAT &
        p_nb_req <= k_n_BrowseResponse_max
    THEN
        breqs_nb_reqs    := p_nb_req                        ||
        breqs_iterated   := {}                              ||
        breqs_to_iterate := cast_t_BrowseValue[1..p_nb_req] ||
        p_continue       := bool(0 < p_nb_req)
    END
    ;

    p_continue, p_bvi <-- continue_iter_browse_request =
    PRE
        breqs_to_iterate /= {}
    THEN
        ANY l_bvi WHERE
            l_bvi : t_BrowseValue_i &
            l_bvi : t_BrowseValue &
            l_bvi : breqs_to_iterate
        THEN
            breqs_iterated   := breqs_iterated \/ {l_bvi} ||
            breqs_to_iterate := breqs_to_iterate - {l_bvi} ||
            p_bvi            := l_bvi ||
            p_continue       := bool(breqs_to_iterate /= {l_bvi})
        END
    END
    ;
   
    p_continue <-- init_iter_browse_result(p_nb_bri) =
    PRE
        p_nb_bri : NAT
    THEN
        p_continue :: BOOL
    END
    ;
   
    p_continue, p_bri <-- continue_iter_browse_result =
    BEGIN
       p_continue :: BOOL ||
       p_bri
       :(  p_bri : t_BrowseResult_i &
           p_bri : t_BrowseResult
        )
    END
    ;
    
    p_continue <-- init_iter_reference(p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        p_continue :: BOOL
    END
    ;
    
    p_continue, p_ref <-- continue_iter_reference =
    BEGIN
       p_continue :: BOOL ||
       p_ref
       :(  p_ref : t_Reference_i &
           p_ref : t_Reference
        )
    END
    
END
