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
    response_write_bs

SEES
    constants

ABSTRACT_VARIABLES
    ResponseWrite_StatusCode

INVARIANT
    ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode

INITIALISATION
    ResponseWrite_StatusCode :: {{}}

OPERATIONS
    ResponseWrite_allocated <-- alloc_write_request_responses_malloc(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_WriteResponse_max
    THEN
        ResponseWrite_StatusCode,
        ResponseWrite_allocated
        :(
            ResponseWrite_allocated  : BOOL                          &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
            (ResponseWrite_allocated = TRUE
             =>
             ResponseWrite_StatusCode = cast_t_WriteValue[1..nb_req] * {e_sc_nok}) &
            (ResponseWrite_allocated = FALSE
             =>
             ResponseWrite_StatusCode = ResponseWrite_StatusCode$0)
         )
    END
    ;

    reset_ResponseWrite =
    BEGIN
       ResponseWrite_StatusCode :: {{}}
    END
    ;

    set_ResponseWrite_StatusCode(wvi, sc) =
    PRE
        wvi : t_WriteValue_i &
        wvi : t_WriteValue &
        sc : t_StatusCode_i &
        sc : t_StatusCode
    THEN
        ResponseWrite_StatusCode(wvi) := sc
    END
    ;

    isvalid, sc <-- getall_ResponseWrite_StatusCode(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : t_WriteValue
    THEN
        isvalid := bool(wvi : dom(ResponseWrite_StatusCode)) ||
        IF wvi : dom(ResponseWrite_StatusCode) THEN
            sc
            :(
                sc : t_StatusCode_i &
                sc = ResponseWrite_StatusCode(wvi)
            )
        ELSE
            sc :: t_StatusCode_i
        END
    END
    ;

    write_WriteResponse_msg_out(msg_out) =
    PRE
        msg_out : t_msg_i &
        msg_out : t_msg
    THEN
        skip
    END

END
