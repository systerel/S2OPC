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
    service_write_1

SEES
    constants,
    constants_statuscodes_bs,
    service_write_decode_bs /* TODO: move to refinment */

DEFINITIONS
    "address_space.def";

    d_variables ==
        ResponseWrite_allocated,
        ResponseWrite_StatusCode
    ;

    d_inv ==
    (
        ResponseWrite_allocated : BOOL &
        ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
        (ResponseWrite_allocated = FALSE
         =>
         ResponseWrite_StatusCode = {})
    )

ABSTRACT_VARIABLES
    d_adsp_variables,
    d_variables

INVARIANT
    d_adsp_invariant &
    d_inv

INITIALISATION
    d_adsp_variables,
    d_variables
    :(
        d_adsp_invariant &
        d_inv
    )

OPERATIONS

    d_adsp_operations;

    StatusCode_service <-- treat_write_request_WriteValues (p_user, p_locales) =
    PRE
        p_user : t_user_i &
        p_user : t_user   &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds   &
        ResponseWrite_allocated = TRUE
    THEN
        StatusCode_service,
        a_Value,
        a_Value_StatusCode,
        a_Value_SourceTimestamp,
        ResponseWrite_StatusCode
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &

            a_Value : t_Node +-> t_Variant &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            a_Value_StatusCode : t_Node +-> t_RawStatusCode &
            dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &
            a_Value_SourceTimestamp : t_Node +-> t_Timestamp &
            dom(a_Value_SourceTimestamp) = dom(a_NodeClass|>{e_ncl_Variable}) &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode
        )
    END
    ;

    bret <-- alloc_write_request_responses(nb_req) =
    PRE
        nb_req : NAT &
        ResponseWrite_allocated = FALSE
    THEN
        bret,
        ResponseWrite_StatusCode,
        ResponseWrite_allocated
        :(
            bret : BOOL &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
            ResponseWrite_allocated : BOOL &
            (ResponseWrite_allocated = FALSE
             =>
             ResponseWrite_StatusCode = {}) &
            bret = ResponseWrite_allocated
        )
    END
    ;

    dealloc_write_request_responses =
    PRE
        ResponseWrite_allocated = TRUE
    THEN
        ResponseWrite_allocated  := FALSE ||
        ResponseWrite_StatusCode := {}
    END
    ;

    /* Promoted */
    write_WriteResponse_msg_out(msg_out) =
    PRE
        msg_out : t_msg_i &
        msg_out : t_msg
    THEN
        skip
    END

END
