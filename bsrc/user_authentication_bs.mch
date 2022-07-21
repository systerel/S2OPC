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
    user_authentication_bs

SEES
    constants,
    constants_statuscodes_bs

OPERATIONS

    p_user_token_type <-- get_user_token_type_from_token (p_user_token) =
    PRE
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        IF p_user_token : t_user_token
        THEN
            p_user_token_type :: t_user_token_type_i
        ELSE
            p_user_token_type := c_userTokenType_indet
        END
    END
    ;

    /* Check if there is a user token policy available in the endpoint config compatible with the channel config */
    p_user_token_policy_available <-- has_user_token_policy_available (p_channel_config_idx, p_endpoint_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user_token_policy_available :: BOOL
    END
    ;

    p_supported_user_token_type, p_user_security_policy <-- is_user_token_supported (p_user_token_type, p_user_token, p_channel_config_idx, p_endpoint_config_idx) =
    PRE
        p_user_token_type : t_user_token_type_i &
        p_user_token : t_user_token_i & /* Necessary to extract the policy URI */
        p_user_token : t_user_token &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        IF p_user_token_type /= c_userTokenType_indet
        THEN
            /* Check the endpoint has a user token type compatible for the secure channel security policy */
            p_supported_user_token_type :: BOOL ||
            p_user_security_policy :: t_SecurityPolicy
        ELSE
            p_supported_user_token_type := FALSE ||
            p_user_security_policy := e_secpol_None
        END
    END
    ;

    p_valid, p_user_token_copy <-- shallow_copy_user_token(p_token_type, p_user_token) =
    PRE
        p_token_type : t_user_token_type_i &
        p_token_type /= c_userTokenType_indet &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        CHOICE
            p_valid := TRUE ||
            p_user_token_copy :(p_user_token_copy : t_user_token_i & p_user_token_copy : t_user_token)
        OR
            p_valid := FALSE ||
            p_user_token_copy := c_user_token_indet
        END
    END
    ;

    p_valid, p_user_token_encrypted <-- encrypt_user_token(p_channel_config_idx, p_server_cert, p_server_nonce, p_user_secu_policy, p_token_type, p_user_token) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_server_cert : t_byte_buffer_i &
        p_server_cert : t_byte_buffer &
        p_server_nonce : t_Nonce_i &
        p_server_nonce : t_Nonce &
        p_user_secu_policy : t_SecurityPolicy &
        p_token_type : t_user_token_type_i &
        p_token_type = e_userTokenType_userName & /* Only encrypted identity token supported for now */
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        CHOICE
            p_valid := TRUE ||
            p_user_token_encrypted :(p_user_token_encrypted : t_user_token_i & p_user_token_encrypted : t_user_token)
        OR
            p_valid := FALSE ||
            p_user_token_encrypted := c_user_token_indet
        END
    END
    ;

    p_sc_valid_user_token, p_user_token_decrypted <-- decrypt_user_token (p_endpoint_config_idx, p_server_nonce, p_user_secu_policy, p_token_type, p_user_token) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_server_nonce : t_Nonce_i &
        p_server_nonce : t_Nonce &
        p_user_secu_policy : t_SecurityPolicy &
        p_token_type : t_user_token_type_i &
        p_token_type = e_userTokenType_userName & /* Only encrypted identity token supported for now */
        p_user_token : t_user_token_i & /* Necessary to extract the policy URI */
        p_user_token : t_user_token
    THEN
        CHOICE
            p_sc_valid_user_token := TRUE ||
            p_user_token_decrypted :(p_user_token_decrypted : t_user_token_i & p_user_token_decrypted : t_user_token)
        OR
            p_sc_valid_user_token := FALSE ||
            p_user_token_decrypted := c_user_token_indet
        END
    END;

    p_sc_valid_user <-- is_valid_user_authentication (p_endpoint_config_idx,  p_token_type, p_user_token) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_token_type : t_user_token_type_i &
        p_token_type : t_user_token_type_i - {c_userTokenType_indet, e_userTokenType_anonymous} &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        p_sc_valid_user :: {e_sc_ok, e_sc_bad_identity_token_invalid, e_sc_bad_identity_token_rejected, e_sc_bad_user_access_denied}
    END
    ;

    p_is_allocated_user, p_user <-- allocate_authenticated_user (p_endpoint_config_idx, p_user_token) =
    PRE
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        CHOICE
            p_is_allocated_user := TRUE ||
            p_user :(p_user : t_user_i & p_user : t_user)
        OR
            p_is_allocated_user := FALSE ||
            p_user := c_user_indet
        END
    END
    ;

    deallocate_user (p_user) =
    PRE
        p_user : t_user_i
    THEN
        skip
    END
    ;

    /* This operation does not require a call to deallocate.
     * It fetches the pointer to the local user, which is static but not const.
     * Hence, this operation shall be called between endpoint context switches. */
    p_user <-- get_local_user (endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user :( p_user : t_user_i &
                  p_user : t_user )
    END

END
