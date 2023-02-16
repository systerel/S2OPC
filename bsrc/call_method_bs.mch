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
    constants_statuscodes_bs

OPERATIONS

    p_rawStatusCode, p_nb_out, p_out_arguments <-- exec_callMethod(p_endpoint_config_idx, p_call_method_pointer) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_call_method_pointer : t_CallMethodPointer_i &
        p_call_method_pointer : t_CallMethodPointer
    THEN
        p_rawStatusCode,
        p_nb_out,
        p_out_arguments
        :(
            p_rawStatusCode : t_RawStatusCode &
            p_nb_out : NAT &
            p_out_arguments : t_ArgumentsPointer_i &
            (p_rawStatusCode /= c_RawStatusCode_Good =>
                p_nb_out = 0 &
                p_out_arguments = c_ArgumentsPointer_indet
            ) &
            (p_rawStatusCode /= c_RawStatusCode_Good =>
                (p_nb_out > 0 =>
                 p_out_arguments : t_ArgumentsPointer)
            )
        )
    END

END
