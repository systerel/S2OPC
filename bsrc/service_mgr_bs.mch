/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    service_mgr_bs

SEES
    constants,
    channel_mgr

DEFINITIONS
    d_variables ==
    a_client_discovery_req_to_send
    ;

    d_typing ==
	a_client_discovery_req_to_send : t_channel_config_idx +-> t_msg

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    d_variables :(d_typing)

OPERATIONS
    
    /* Needs UNINIT to deallocate non sent messages on uninit */
    UNINITIALISATION =
    BEGIN
        skip
    END
    ;
    
    /* Discovery request without connected channel: keep request to send it later */
    bres <-- client_async_discovery_request_without_channel (channel_config_idx, req_msg, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        req_msg            : t_msg_i &
        req_msg            : t_msg &
        app_context        : t_application_context_i        
    THEN
        bres :: BOOL
    END
    ;
    
    /* Generates an event to send the discovery request */
    client_channel_connected_event_discovery (channel_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END

END
