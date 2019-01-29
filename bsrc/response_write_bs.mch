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
    response_write_bs

SEES
    constants,
    constants_statuscodes_bs

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
             ResponseWrite_StatusCode = cast_t_WriteValue[1..nb_req] * {e_sc_bad_internal_error}) &
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
        sc : t_StatusCode &
        wvi : dom(ResponseWrite_StatusCode)
    THEN
        ResponseWrite_StatusCode(wvi) := sc
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
