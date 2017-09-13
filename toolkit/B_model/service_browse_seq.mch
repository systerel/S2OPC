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
    service_browse_seq

SEES
    constants

OPERATIONS

    StatusCode_service <-- treat_browse_request_BrowseValues =
    /* TODOPAB: add PRE to ensure decode decode_browse_request is called before this one */
    /* ... and resets the variable at the end of the op */
    BEGIN
        StatusCode_service :: t_StatusCode_i
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

END
