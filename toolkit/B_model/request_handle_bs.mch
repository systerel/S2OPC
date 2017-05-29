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
    request_handle_bs
    
SEES
    constants
    
ABSTRACT_VARIABLES
    s_request_handle
    
INVARIANT
    s_request_handle <: t_request_handle
    
INITIALISATION
    s_request_handle :: {{}}
    
OPERATIONS  
    
    request_handle <-- fresh_req_handle =
    BEGIN
        IF s_request_handle = t_request_handle
        THEN
            request_handle := c_request_handle_indet
        ELSE
            ANY l_request_handle WHERE
                l_request_handle : t_request_handle_i &
                l_request_handle : t_request_handle &
                l_request_handle /: s_request_handle
            THEN 
                request_handle := l_request_handle ||
                s_request_handle := s_request_handle \/ {l_request_handle}
            END
        END
    END
    ;
    
    ret <-- is_valid_req_handle (req_handle) =
    PRE
        req_handle : t_request_handle_i 
    THEN
        ret := bool (req_handle : s_request_handle)
    END
    ;
    
    remove_req_handle (req_handle) =
    PRE
        req_handle : t_request_handle_i
    THEN
        IF req_handle : t_request_handle
        THEN
            s_request_handle := s_request_handle - {req_handle}
        END
    END
    
END
