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
    user_authentication

SEES
    constants,
    constants_statuscodes_bs

OPERATIONS

    p_valid, p_user_token_encrypted <-- may_encrypt_user_token(p_channel_config_idx, p_user_server_cert, p_server_nonce, p_user_secu_policy, p_user_token) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_user_server_cert : t_byte_buffer_i &
        p_user_server_cert : t_byte_buffer &
        p_server_nonce : t_Nonce_i &
        p_server_nonce : t_Nonce &
        p_user_secu_policy : t_SecurityPolicy &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        p_valid,
        p_user_token_encrypted
        :(p_valid : BOOL &
          p_user_token_encrypted : t_user_token_i &
          (p_valid = TRUE => p_user_token_encrypted : t_user_token) &
          (p_valid = FALSE => p_user_token_encrypted = c_user_token_indet))
    END
    ;

    /* TODO: add user token signature for X509 */
    p_sc_valid_user, p_user <-- allocate_valid_and_authenticated_user (p_user_token, p_server_nonce, p_channel_config_idx, p_endpoint_config_idx) =
    PRE
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        p_server_nonce : t_Nonce_i &
        p_server_nonce : t_Nonce &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_sc_valid_user,
        p_user
        :(p_sc_valid_user : t_StatusCode_i &
          p_sc_valid_user : t_StatusCode &
          p_user : t_user_i &
          (p_sc_valid_user = e_sc_ok => p_user : t_user) &
          (p_sc_valid_user /= e_sc_ok => /* TODO: add possible status code returned */
              p_user = c_user_indet))
    END
    ;

    deallocate_user (p_user) =
    PRE
        p_user : t_user_i
    THEN
        skip
    END
    ;

    p_user <-- get_local_user (endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user :( p_user : t_user_i &
                  p_user : t_user )
    END
    ;

    p_user_token_policy_available <-- has_user_token_policy_available (p_channel_config_idx, p_endpoint_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user_token_policy_available :: BOOL
    END

END
