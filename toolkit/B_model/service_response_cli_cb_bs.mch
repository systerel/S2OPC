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
    service_response_cli_cb_bs
    
SEES
    constants,
    message_in_bs
    
OPERATIONS
    
    cli_service_response (resp_msg, status) = 
    PRE
        resp_msg   : t_msg_i &
        resp_msg   = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : { e_msg_session_read_resp, e_msg_session_write_resp } &
        status : t_StatusCode_i &
        status : t_StatusCode
    THEN
        skip
    END
    
END
