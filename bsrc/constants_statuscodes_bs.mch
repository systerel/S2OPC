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
    constants_statuscodes_bs

SEES
    constants

SETS
    t_StatusCode_i = {c_StatusCode_indet, e_sc_ok,
                      e_sc_bad_generic,
                      e_sc_uncertain_generic,
                      e_sc_bad_internal_error,
                      e_sc_bad_secure_channel_closed,
                      e_sc_bad_secure_channel_id_invalid,
                      e_sc_bad_connection_closed,
                      e_sc_bad_invalid_state,
                      e_sc_bad_session_id_invalid,
                      e_sc_bad_session_closed,
                      e_sc_bad_session_not_activated,
                      e_sc_bad_too_many_sessions,
                      e_sc_bad_identity_token_invalid,
                      e_sc_bad_identity_token_rejected,
                      e_sc_bad_application_signature_invalid,
                      e_sc_bad_encoding_error,
                      e_sc_bad_decoding_error,
                      e_sc_bad_request_too_large,
                      e_sc_bad_response_too_large,
                      e_sc_bad_arguments_missing,
                      e_sc_bad_too_many_arguments,
                      e_sc_bad_invalid_argument,
                      e_sc_bad_method_invalid,
                      e_sc_bad_not_implemented,
                      e_sc_bad_not_executable,
                      e_sc_bad_unexpected_error,
                      e_sc_bad_out_of_memory,
                      e_sc_bad_nothing_to_do,
                      e_sc_bad_too_many_ops,
                      e_sc_bad_timestamps_to_return_invalid,
                      e_sc_bad_max_age_invalid,
                      e_sc_bad_node_id_unknown,
                      e_sc_bad_node_id_invalid,
                      e_sc_bad_view_id_unknown,
                      e_sc_bad_attribute_id_invalid,
                      e_sc_bad_browse_direction_invalid,
                      e_sc_bad_browse_name_invalid,
                      e_sc_bad_reference_type_id_invalid,
                      e_sc_bad_continuation_point_invalid,
                      e_sc_bad_no_continuation_points,
                      e_sc_bad_service_unsupported,
                      e_sc_bad_write_not_supported,
                      e_sc_bad_timeout,
                      e_sc_bad_too_many_subscriptions,
                      e_sc_bad_no_subscription,
                      e_sc_bad_subscription_id_invalid,
                      e_sc_bad_sequence_number_unknown,
                      e_sc_bad_too_many_monitored_items,
                      e_sc_bad_monitoring_mode_invalid,
                      e_sc_bad_monitored_item_filter_unsupported,
                      e_sc_bad_too_many_publish_requests,
                      e_sc_bad_message_not_available,
                      e_sc_bad_index_range_invalid,
                      e_sc_bad_index_range_no_data,
                      e_sc_bad_user_access_denied,
                      e_sc_bad_certificate_uri_invalid,
                      e_sc_bad_security_checks_failed,
                      e_sc_bad_request_interrupted,
                      e_sc_bad_data_unavailable,
                      e_sc_bad_not_writable,
                      e_sc_bad_not_readable,
                      e_sc_bad_type_mismatch,
                      e_sc_uncertain_reference_out_of_server,
                      e_sc_bad_too_many_matches,
                      e_sc_bad_query_too_complex,
                      e_sc_bad_no_match,
                      e_sc_bad_data_encoding_invalid,
                      e_sc_bad_server_uri_invalid,
                      e_sc_bad_server_name_missing,
                      e_sc_bad_discovery_url_missing,
                      e_sc_bad_semaphore_file_missing,
                      e_sc_bad_not_supported,
                      e_sc_bad_nonce_invalid,
                      e_sc_bad_encoding_limits_exceeded,
                      e_sc_bad_not_found,
                      e_sc_bad_security_mode_insufficient,
                      e_sc_bad_no_communication,
                      e_sc_bad_resource_unavailable,
                      e_sc_bad_out_of_range}

ABSTRACT_CONSTANTS
    c_RawStatusCode_BadInvalidState,
    c_RawStatusCode_Good,

    conv_StatusCode_To_RawStatusCode,
    conv_RawStatusCode_To_StatusCode,

    t_StatusCode

PROPERTIES

    c_RawStatusCode_BadInvalidState : t_RawStatusCode &
    c_RawStatusCode_Good : t_RawStatusCode &

    t_StatusCode = t_StatusCode_i - {c_StatusCode_indet} &

    conv_StatusCode_To_RawStatusCode : t_StatusCode --> t_RawStatusCode &
    conv_RawStatusCode_To_StatusCode : t_RawStatusCode --> t_StatusCode & /* Possible precision loss */

    conv_StatusCode_To_RawStatusCode(e_sc_ok)                = c_RawStatusCode_Good

OPERATIONS

    p_raw_sc <-- getall_conv_StatusCode_To_RawStatusCode(p_sc) =
    PRE
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        p_raw_sc := conv_StatusCode_To_RawStatusCode(p_sc)
    END
    ;

    p_sc <-- getall_conv_RawStatusCode_To_StatusCode(p_raw_sc) =
    PRE
        p_raw_sc : t_RawStatusCode
    THEN
        p_sc :(p_sc : t_StatusCode_i & p_sc = conv_RawStatusCode_To_StatusCode(p_raw_sc))
    END
    ;

    p_raw_sc <-- get_const_RawStatusCode_Good =
    BEGIN
        p_raw_sc := conv_StatusCode_To_RawStatusCode(e_sc_ok)
    END
    ;

    p_raw_sc <-- get_const_RawStatusCode_BadInvalidState =
    BEGIN
        p_raw_sc := conv_StatusCode_To_RawStatusCode(e_sc_bad_invalid_state)
    END

END
