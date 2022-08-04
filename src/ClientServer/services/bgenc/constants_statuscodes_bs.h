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

/******************************************************************************

 File Name            : constants_statuscodes_bs.h

 Date                 : 04/08/2022 14:53:31

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _constants_statuscodes_bs_h
#define _constants_statuscodes_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*-------------------------------
   SETS Clause: enumerated sets
  -------------------------------*/
typedef enum {
   constants_statuscodes_bs__c_StatusCode_indet,
   constants_statuscodes_bs__e_sc_ok,
   constants_statuscodes_bs__e_sc_bad_generic,
   constants_statuscodes_bs__e_sc_uncertain_generic,
   constants_statuscodes_bs__e_sc_bad_internal_error,
   constants_statuscodes_bs__e_sc_bad_secure_channel_closed,
   constants_statuscodes_bs__e_sc_bad_secure_channel_id_invalid,
   constants_statuscodes_bs__e_sc_bad_connection_closed,
   constants_statuscodes_bs__e_sc_bad_invalid_state,
   constants_statuscodes_bs__e_sc_bad_session_id_invalid,
   constants_statuscodes_bs__e_sc_bad_session_closed,
   constants_statuscodes_bs__e_sc_bad_session_not_activated,
   constants_statuscodes_bs__e_sc_bad_too_many_sessions,
   constants_statuscodes_bs__e_sc_bad_identity_token_invalid,
   constants_statuscodes_bs__e_sc_bad_identity_token_rejected,
   constants_statuscodes_bs__e_sc_bad_application_signature_invalid,
   constants_statuscodes_bs__e_sc_bad_encoding_error,
   constants_statuscodes_bs__e_sc_bad_decoding_error,
   constants_statuscodes_bs__e_sc_bad_request_too_large,
   constants_statuscodes_bs__e_sc_bad_response_too_large,
   constants_statuscodes_bs__e_sc_bad_arguments_missing,
   constants_statuscodes_bs__e_sc_bad_too_many_arguments,
   constants_statuscodes_bs__e_sc_bad_invalid_argument,
   constants_statuscodes_bs__e_sc_bad_method_invalid,
   constants_statuscodes_bs__e_sc_bad_not_implemented,
   constants_statuscodes_bs__e_sc_bad_not_executable,
   constants_statuscodes_bs__e_sc_bad_unexpected_error,
   constants_statuscodes_bs__e_sc_bad_out_of_memory,
   constants_statuscodes_bs__e_sc_bad_nothing_to_do,
   constants_statuscodes_bs__e_sc_bad_too_many_ops,
   constants_statuscodes_bs__e_sc_bad_timestamps_to_return_invalid,
   constants_statuscodes_bs__e_sc_bad_max_age_invalid,
   constants_statuscodes_bs__e_sc_bad_node_id_unknown,
   constants_statuscodes_bs__e_sc_bad_node_id_invalid,
   constants_statuscodes_bs__e_sc_bad_view_id_unknown,
   constants_statuscodes_bs__e_sc_bad_attribute_id_invalid,
   constants_statuscodes_bs__e_sc_bad_browse_direction_invalid,
   constants_statuscodes_bs__e_sc_bad_browse_name_invalid,
   constants_statuscodes_bs__e_sc_bad_reference_type_id_invalid,
   constants_statuscodes_bs__e_sc_bad_continuation_point_invalid,
   constants_statuscodes_bs__e_sc_bad_no_continuation_points,
   constants_statuscodes_bs__e_sc_bad_service_unsupported,
   constants_statuscodes_bs__e_sc_bad_write_not_supported,
   constants_statuscodes_bs__e_sc_bad_timeout,
   constants_statuscodes_bs__e_sc_bad_too_many_subscriptions,
   constants_statuscodes_bs__e_sc_bad_no_subscription,
   constants_statuscodes_bs__e_sc_bad_subscription_id_invalid,
   constants_statuscodes_bs__e_sc_bad_sequence_number_unknown,
   constants_statuscodes_bs__e_sc_bad_too_many_monitored_items,
   constants_statuscodes_bs__e_sc_bad_monitoring_mode_invalid,
   constants_statuscodes_bs__e_sc_bad_monitored_item_filter_unsupported,
   constants_statuscodes_bs__e_sc_bad_too_many_publish_requests,
   constants_statuscodes_bs__e_sc_bad_message_not_available,
   constants_statuscodes_bs__e_sc_bad_index_range_invalid,
   constants_statuscodes_bs__e_sc_bad_index_range_no_data,
   constants_statuscodes_bs__e_sc_bad_user_access_denied,
   constants_statuscodes_bs__e_sc_bad_certificate_uri_invalid,
   constants_statuscodes_bs__e_sc_bad_security_checks_failed,
   constants_statuscodes_bs__e_sc_bad_request_interrupted,
   constants_statuscodes_bs__e_sc_bad_data_unavailable,
   constants_statuscodes_bs__e_sc_bad_not_writable,
   constants_statuscodes_bs__e_sc_bad_not_readable,
   constants_statuscodes_bs__e_sc_bad_type_mismatch,
   constants_statuscodes_bs__e_sc_uncertain_reference_out_of_server,
   constants_statuscodes_bs__e_sc_bad_too_many_matches,
   constants_statuscodes_bs__e_sc_bad_query_too_complex,
   constants_statuscodes_bs__e_sc_bad_no_match,
   constants_statuscodes_bs__e_sc_bad_data_encoding_invalid,
   constants_statuscodes_bs__e_sc_bad_server_uri_invalid,
   constants_statuscodes_bs__e_sc_bad_server_name_missing,
   constants_statuscodes_bs__e_sc_bad_discovery_url_missing,
   constants_statuscodes_bs__e_sc_bad_semaphore_file_missing,
   constants_statuscodes_bs__e_sc_bad_not_supported,
   constants_statuscodes_bs__e_sc_bad_nonce_invalid,
   constants_statuscodes_bs__e_sc_bad_encoding_limits_exceeded,
   constants_statuscodes_bs__e_sc_bad_not_found,
   constants_statuscodes_bs__e_sc_bad_security_mode_insufficient,
   constants_statuscodes_bs__e_sc_bad_no_communication,
   constants_statuscodes_bs__e_sc_bad_resource_unavailable,
   constants_statuscodes_bs__e_sc_bad_out_of_range
} constants_statuscodes_bs__t_StatusCode_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants_statuscodes_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void constants_statuscodes_bs__get_const_RawStatusCode_BadInvalidState(
   constants__t_RawStatusCode * const constants_statuscodes_bs__p_raw_sc);
extern void constants_statuscodes_bs__get_const_RawStatusCode_Good(
   constants__t_RawStatusCode * const constants_statuscodes_bs__p_raw_sc);
extern void constants_statuscodes_bs__getall_conv_RawStatusCode_To_StatusCode(
   const constants__t_RawStatusCode constants_statuscodes_bs__p_raw_sc,
   constants_statuscodes_bs__t_StatusCode_i * const constants_statuscodes_bs__p_sc);
extern void constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(
   const constants_statuscodes_bs__t_StatusCode_i constants_statuscodes_bs__p_sc,
   constants__t_RawStatusCode * const constants_statuscodes_bs__p_raw_sc);

#endif
