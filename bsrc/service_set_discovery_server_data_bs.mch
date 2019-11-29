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
    service_set_discovery_server_data_bs

SEES
    constants

ABSTRACT_VARIABLES
    RegisteredServer_ServerUri
INVARIANT
    RegisteredServer_ServerUri : t_RegisteredServer --> t_ServerUri
INITIALISATION
    RegisteredServer_ServerUri :(RegisteredServer_ServerUri : t_RegisteredServer --> t_ServerUri)

OPERATIONS

    p_server_uri <-- get_RegisteredServer_ServerUri(p_reg_server) =
    PRE
        p_reg_server : t_RegisteredServer_i &
        p_reg_server : t_RegisteredServer
    THEN
        p_server_uri := RegisteredServer_ServerUri(p_reg_server)
    END
    ;

    p_bool <-- is_equal_ServerUri (p_left, p_right) =
    PRE
        p_left : t_ServerUri &
        p_right : t_ServerUri
    THEN
        p_bool := bool(p_left = p_right)
    END
    ;

    p_bool <-- is_empty_ServerUri (p_server_uri) =
    PRE
        p_server_uri : t_ServerUri
    THEN
        p_bool := bool(p_server_uri = c_ServerUri_empty)
    END

END
