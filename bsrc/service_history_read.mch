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
    service_history_read

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    p_StatusCode <-- treat_history_read_request(p_req_msg, p_resp_msg, p_user) =
    PRE
        p_req_msg      : t_msg_i                           &
        p_req_msg      = c_msg_in                          &
        c_msg_in       : t_msg                             &
        a_msg_in_type  = e_msg_attribute_history_read_req  &
        p_resp_msg     : t_msg_i                           &
        p_resp_msg     = c_msg_out                         &
        c_msg_out      : t_msg                             &
        a_msg_out_type = e_msg_attribute_history_read_resp &
        p_user         : t_user_i                          &
        p_user         : t_user
    THEN
        p_StatusCode :( p_StatusCode : t_StatusCode_i &
                        p_StatusCode : {e_sc_ok, e_sc_bad_out_of_memory,
                                        e_sc_bad_history_operation_invalid,
                                        e_sc_bad_history_operation_unsupported,
                                        e_sc_bad_timestamps_to_return_invalid,
                                        e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops})
    END

END
