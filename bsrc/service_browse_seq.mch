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
    service_browse_seq

SEES
    constants,
    message_in_bs

OPERATIONS

    StatusCode_service <-- treat_browse_request_BrowseValues =
    /* TODOPAB: add PRE to ensure decode decode_browse_request is called before this one */
    /* ... and resets the variable at the end of the op */
    BEGIN
        StatusCode_service :: t_StatusCode_i
    END
    ;

    /* Promoted from service_browse_decode_bs */
    StatusCode_service <-- decode_browse_request(req_payload) =
    PRE
        req_payload : t_msg_i &
        req_payload = c_msg_in
    THEN
        StatusCode_service
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    /* Promoted from msg_browse_response_bs */
    p_isvalid <-- write_BrowseResponse_msg_out(p_msg_out) =
    PRE
        p_msg_out : t_msg_i &
        p_msg_out : t_msg
    THEN
        p_isvalid :: BOOL
    END
    ;

    /* Promoted from msg_browse_response_bs */
    free_browse_result =
    BEGIN
        skip
    END
    ;

    /* Promoted from msg_browse_response_bs */
    set_ResponseBrowse_BrowseStatus(p_bvi, p_sc) =
    PRE
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        skip
    END

END
