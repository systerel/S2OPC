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
    service_read

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    StatusCode_service <-- treat_read_request(p_user, p_locales, p_request_msg, p_response_msg) =
    PRE
        p_user         : t_user_i                 &
        p_user         : t_user                   &
        p_locales      : t_LocaleIds_i            &
        p_locales      : t_LocaleIds              &
        p_request_msg  : t_msg_i                  &
        p_request_msg  = c_msg_in                 &
        c_msg_in       : t_msg                    &
        a_msg_in_type  = e_msg_attribute_read_req &
        p_response_msg : t_msg_i                  &
        p_response_msg = c_msg_out                &
        c_msg_out      : t_msg
    THEN
        StatusCode_service
        :(
          StatusCode_service    : t_StatusCode_i &
          StatusCode_service    : {e_sc_ok, e_sc_bad_too_many_ops, e_sc_bad_nothing_to_do,
                                   e_sc_bad_timestamps_to_return_invalid, e_sc_bad_max_age_invalid,
                                   e_sc_bad_out_of_memory}
         )
    END

END
