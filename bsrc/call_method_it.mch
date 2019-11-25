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
    call_method_it

SEES
    constants,
    msg_call_method_bs

DEFINITIONS
    d_variables ==
    callMethods_to_iterate,
    callMethods_iterated
    ;

    d_inv ==
    (
    callMethods_to_iterate <: t_CallMethod &
    callMethods_iterated   <: t_CallMethod &
    callMethods_to_iterate /\ callMethods_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_callMethods (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg   &
        CallMethod_Msg = p_req_msg
    THEN
        callMethods_iterated   := {}        ||
        callMethods_to_iterate := CallMethods ||
        p_continue             := bool(callMethods_to_iterate /= {})
    END
    ;

    p_continue, p_callMethod <-- continue_iter_callMethod =
    PRE
        callMethods_to_iterate /= {}
    THEN
        ANY l_callMethod WHERE
            l_callMethod : t_CallMethod_i &
            l_callMethod : t_CallMethod &
            l_callMethod : callMethods_to_iterate
        THEN
            callMethods_iterated   := callMethods_iterated   \/ {l_callMethod} ||
            callMethods_to_iterate := callMethods_to_iterate -  {l_callMethod} ||
            p_callMethod           := l_callMethod                          ||
            p_continue             := bool(callMethods_to_iterate /= {l_callMethod})
        END
    END

END
