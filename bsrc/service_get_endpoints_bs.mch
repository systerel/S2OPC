/*
 *  Copyright (C) 2017 Systerel and others.
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
    service_get_endpoints_bs
    
SEES
    constants,
    message_in_bs,
    message_out_bs

OPERATIONS
    
    ret <-- treat_get_endpoints_request (req_msg, resp_msg, endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_get_endpoints_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_get_endpoints_resp
    THEN
        ret :(ret : t_StatusCode_i & ret : t_StatusCode)
    END

END
