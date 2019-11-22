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
    /* Machine is used to set current internal context available to applicative callbacks.
       No variables are defined in B to be able to set context anywhere (no proof obligation generated)
    */
    app_cb_call_context_bs
SEES
    constants
OPERATIONS

    /* For now only set on received message treatment and used only for server service request treatment */
    set_app_call_context_channel_config(p_channel_config, p_endpoint_config) =
    PRE
        p_channel_config : t_channel_config_idx_i &
        p_channel_config : t_channel_config_idx   &
        p_endpoint_config : t_endpoint_config_idx_i /* c_endpoint_config_idx_indet <=> client */
    THEN
        skip
    END
    ;

    /* For now only set on server service request on activated session treatment */
    set_app_call_context_session(p_user) =
    PRE
        p_user    : t_user_i    &
        p_user    : t_user
    THEN
        skip
    END
    ;

    clear_app_call_context =
    BEGIN
        skip
    END

END
