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
    call_method_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    request_handle_bs

ABSTRACT_VARIABLES

    exec_result

INVARIANT

    exec_result : seq(t_Variant_i)

INITIALISATION

    exec_result
    :(
        exec_result = {}
    )

OPERATIONS

    rawStatusCode <-- exec_callMethod(p_req_msg, p_callMethod, p_endpoint_config_idx) =
    PRE
        p_req_msg : t_msg_i   &
        p_req_msg = c_msg_out &
        p_callMethod : t_CallMethod_i &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        exec_result = {} /* Do not call before the memory is freed */
    THEN
        rawStatusCode,
        exec_result
        :(
            rawStatusCode : t_RawStatusCode &
            exec_result : seq(t_Variant_i)
        )
    END
    ;

    free_exec_result =
    BEGIN
        exec_result :( exec_result = {} )
    END
    ;

    nb <-- read_nb_exec_result =
    BEGIN
        nb
        :(
            nb : NAT &
            nb = size(exec_result)
        )
    END
    ;

    value <-- read_exec_result(index) =
    PRE
        index : NAT1 &
        index : dom(exec_result)
    THEN
        value
        :(
            value : t_Variant_i &
            value = exec_result(index)
        )
    END
END
