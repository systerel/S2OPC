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
    msg_call_method_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    request_handle_bs


DEFINITIONS
    d_variables_in ==
    CallMethods,
    CallMethods_Msg,
    CallMethod_Pointer,
    CallMethod_ObjectId,
    CallMethod_MethodId,
    CallMethod_InputArguments
    ;

    d_variables_out ==
    CallMethodRes_Msg,
    CallMethodRes_status,
    CallMethodRes_AllocatedInputArgumentResult,
    CallMethodRes_InputArgumentResult,
    CallMethodRes_NbOutputArguments,
    CallMethodRes_OutputArguments
    ;

    d_inv_in ==
    CallMethods_Msg     : t_msg_i         &
    CallMethods               <: t_CallMethod   &
    CallMethods               = cast_t_CallMethod[1..card(CallMethods)] &
    CallMethod_Pointer        : CallMethods --> t_CallMethodPointer &
    CallMethod_ObjectId       : CallMethods --> t_NodeId_i &
    CallMethod_MethodId       : CallMethods --> t_NodeId_i &
    CallMethod_InputArguments : CallMethods --> seq(t_Variant_i)
    ;

    d_inv_out ==
    CallMethodRes_Msg                    : t_msg_i &
    CallMethodRes_status                 : CallMethods +-> t_RawStatusCode      &
    CallMethodRes_AllocatedInputArgumentResult : CallMethods +-> NAT                & /* Allocated memory */
    CallMethodRes_InputArgumentResult    : CallMethods +-> seq(t_StatusCode) & /* Effective data */
    CallMethodRes_NbOutputArguments      : CallMethods +-> NAT                & /* Number of arguments */
    CallMethodRes_OutputArguments        : CallMethods +-> seq(t_Variant)    & /* Output arguments */
    dom(CallMethodRes_status) = dom(CallMethodRes_InputArgumentResult)          &
    dom(CallMethodRes_status) = dom(CallMethodRes_OutputArguments)

ABSTRACT_VARIABLES
    d_variables_in,
    d_variables_out


INVARIANT

    d_inv_in &
    d_inv_out

INITIALISATION

    d_variables_in,
    d_variables_out
    :
    (
        CallMethods_Msg = c_msg_indet &
        CallMethods = {} &
        CallMethod_ObjectId = {} &
        CallMethod_MethodId = {} &
        CallMethod_InputArguments = {} &

        CallMethodRes_Msg = c_msg_indet &
        CallMethodRes_status = {} &
        CallMethodRes_AllocatedInputArgumentResult = {} &
        CallMethodRes_InputArgumentResult = {} &
        CallMethodRes_NbOutputArguments = {} &
        CallMethodRes_OutputArguments = {}
    )
OPERATIONS

    Status, p_nb <-- read_call_method_request (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_method_call_req
    THEN
        Status,
        p_nb,
        d_variables_in
        :(
            Status : t_StatusCode    &
            p_nb : NAT               &
            d_inv_in                 &
            p_nb = card(CallMethods) &
            (Status = e_sc_ok
                =>
                CallMethods_Msg = p_req_msg
            ) &
            (Status /= e_sc_ok
                =>
                CallMethods_Msg = c_msg_indet &
                CallMethods = {}
            )
        )
    END
    ;

    p_nb <-- read_nb_CallMethods (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg
    THEN
        p_nb :(p_nb : NAT & p_nb = card(CallMethods))
    END
    ;

    p_callMethodPointer <-- read_CallMethod_Pointer (p_req_msg, p_callMethod) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg &
        p_callMethod   : t_CallMethod_i &
        p_callMethod   : CallMethods
    THEN
        p_callMethodPointer :( p_callMethodPointer : t_CallMethodPointer_i & p_callMethodPointer = CallMethod_Pointer(p_callMethod))
    END
    ;

    p_objectid <-- read_CallMethod_Objectid (p_req_msg, p_callMethod) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg &
        p_callMethod   : t_CallMethod_i &
        p_callMethod   : CallMethods
    THEN
        p_objectid :( p_objectid : t_NodeId_i & p_objectid = CallMethod_ObjectId(p_callMethod))
    END
    ;

    p_methodid <-- read_CallMethod_MethodId (p_req_msg, p_callMethod) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg &
        p_callMethod   : t_CallMethod_i &
        p_callMethod   : CallMethods
    THEN
        p_methodid :( p_methodid : t_NodeId_i & p_methodid = CallMethod_MethodId(p_callMethod))
    END
    ;

    p_nb <-- read_CallMethod_Nb_InputArguments (p_req_msg, p_callMethod) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg &
        p_callMethod   : t_CallMethod_i &
        p_callMethod   : CallMethods
    THEN
        p_nb :(p_nb = size(CallMethod_InputArguments(p_callMethod)))
    END
    ;

    p_arg <-- read_CallMethod_InputArguments (p_req_msg, p_callMethod, p_index_arg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        p_req_msg = CallMethods_Msg &
        p_callMethod : t_CallMethod_i &
        p_callMethod : CallMethods  &
        p_index_arg : NAT1 &
        p_index_arg : 1..(size(CallMethod_InputArguments(p_callMethod)))
    THEN
        p_arg :(p_arg = CallMethod_InputArguments(p_callMethod)(p_index_arg))
    END
    ;

    statusCode <-- alloc_CallMethod_Result(p_res_msg, nb) =
    PRE
        p_res_msg : t_msg_i   &
        p_res_msg = c_msg_out &
        a_msg_out_type = e_msg_method_call_resp &
        card(CallMethods) > 0 &
        nb : NAT &
        nb = card(CallMethods)
    THEN
        d_variables_out,
        statusCode
        :(
            statusCode : t_StatusCode_i &
            statusCode : t_StatusCode   &
            d_inv_out &
            (statusCode = e_sc_ok =>
                CallMethodRes_Msg = p_res_msg &
                CallMethodRes_status                 : CallMethods --> t_RawStatusCode   &
                CallMethodRes_AllocatedInputArgumentResult : CallMethods --> NAT         &
                CallMethodRes_InputArgumentResult    : CallMethods --> seq(t_StatusCode) &
                CallMethodRes_NbOutputArguments      : CallMethods --> NAT               &
                CallMethodRes_OutputArguments        : CallMethods --> seq(t_Variant)    &
                /* Input argument results are not allocated yet */
                ran(CallMethodRes_AllocatedInputArgumentResult) = {0} &
                ran(CallMethodRes_InputArgumentResult) = {} &
                /* Output results are not allocated yet */
                ran(CallMethodRes_NbOutputArguments) = {0} &
                ran(CallMethodRes_OutputArguments) = {}
            ) &
            (statusCode /= e_sc_ok =>
                CallMethodRes_Msg = c_msg_indet &
                dom(CallMethodRes_status) = {}
            )
        )
    END
    ;

    write_CallMethod_Res_Status(p_res_msg, callMethod, rawStatusCode) =
    PRE
        p_res_msg : t_msg_i &
        p_res_msg : t_msg   &
        p_res_msg = CallMethodRes_Msg &
        card(CallMethods) > 0     &
        callMethod : t_CallMethod_i &
        callMethod : CallMethods    &
        callMethod : dom(CallMethodRes_status) &
        rawStatusCode : t_RawStatusCode
    THEN
        CallMethodRes_status(callMethod) := rawStatusCode
    END
    ;

    statusCode <-- alloc_CallMethod_Res_InputArgumentResult(p_res_msg, callMethod, nb) =
    PRE
        p_res_msg : t_msg_i   &
        p_res_msg = c_msg_out &
        p_res_msg = CallMethodRes_Msg &
        card(CallMethods) > 0       &
        callMethod : t_CallMethod_i &
        callMethod : CallMethods    &
        callMethod : dom(CallMethodRes_InputArgumentResult) &
        nb         : NAT1
    THEN
        CallMethodRes_AllocatedInputArgumentResult,
        CallMethodRes_InputArgumentResult,
        statusCode
        :(
            statusCode : t_StatusCode_i &
            statusCode : { e_sc_ok, e_sc_bad_out_of_memory } &
            (statusCode = e_sc_ok =>
                {callMethod} <<| CallMethodRes_AllocatedInputArgumentResult = {callMethod} <<| CallMethodRes_AllocatedInputArgumentResult$0 &
                {callMethod} <<| CallMethodRes_InputArgumentResult = {callMethod} <<| CallMethodRes_InputArgumentResult$0 &
                CallMethodRes_AllocatedInputArgumentResult(callMethod) = nb &
                CallMethodRes_InputArgumentResult(callMethod) = {} ) &
            (statusCode = e_sc_bad_out_of_memory =>
                CallMethodRes_AllocatedInputArgumentResult = CallMethodRes_AllocatedInputArgumentResult$0 &
                CallMethodRes_InputArgumentResult = CallMethodRes_InputArgumentResult
            )
        )
    END
    ;

    free_CallMethod_Res_InputArgument(p_res_msg, callMethod) =
    PRE
        p_res_msg : t_msg_i   &
        p_res_msg = c_msg_out &
        card(CallMethods) > 0       &
        callMethod : t_CallMethod_i &
        callMethod : CallMethods    &
        callMethod : dom(CallMethodRes_InputArgumentResult)
    THEN
        CallMethodRes_AllocatedInputArgumentResult(callMethod) := 0 ||
        CallMethodRes_InputArgumentResult(callMethod) := {}
    END
    ;

    write_CallMethod_Res_InputArgumentResult(p_res_msg, callMethod, index, statusCode) =
    PRE
        p_res_msg : t_msg_i &
        p_res_msg : t_msg   &
        p_res_msg = CallMethodRes_Msg &
        card(CallMethods) > 0       &
        callMethod : t_CallMethod_i &
        callMethod : CallMethods    &
        callMethod : dom(CallMethodRes_InputArgumentResult) &
        index      : NAT1           &
        index      = size(CallMethodRes_InputArgumentResult(callMethod)) + 1 &
        index <= CallMethodRes_AllocatedInputArgumentResult(callMethod) &
        statusCode : t_StatusCode_i &
        statusCode : t_StatusCode
    THEN
        CallMethodRes_InputArgumentResult(callMethod)(index) := statusCode
    END
    ;

    /* Set OutputArguments result */
    set_CallMethod_Res_OutputArgument(p_res_msg, callMethod, nb_out_args, out_args) =
    PRE
        p_res_msg : t_msg_i   &
        p_res_msg = c_msg_out &
        card(CallMethods) > 0       &
        callMethod : t_CallMethod_i &
        callMethod : CallMethods    &
        callMethod : dom(CallMethodRes_OutputArguments) &
        nb_out_args : NAT1 &
        out_args : t_ArgumentsPointer_i &
        out_args : t_ArgumentsPointer
    THEN
        CallMethodRes_NbOutputArguments,
        CallMethodRes_OutputArguments
        :(
            {callMethod} <<| CallMethodRes_NbOutputArguments = {callMethod} <<| CallMethodRes_NbOutputArguments$0 &
            {callMethod} <<| CallMethodRes_OutputArguments = {callMethod} <<| CallMethodRes_OutputArguments$0 &
            CallMethodRes_NbOutputArguments(callMethod) = nb_out_args &
            callMethod : dom(CallMethodRes_OutputArguments) /* => value pointed by out_args */
        )
    END

END
