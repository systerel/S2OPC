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
    msg_read_request

SEES
    constants,
    address_space,
    message_in_bs

DEFINITIONS
    d_inv == (
        nb_ReadValue          : NAT                      &
        nb_ReadValue         <= k_n_read_resp_max        &
        ReadValue_NodeId      : t_ReadValue +-> t_NodeId &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId
    )

ABSTRACT_VARIABLES
    nb_ReadValue,
    ReadValue_NodeId,
    ReadValue_AttributeId

INVARIANT
    d_inv

INITIALISATION
    nb_ReadValue          := 0  ||
    ReadValue_NodeId      := {} ||
    ReadValue_AttributeId := {}

OPERATIONS

    /* Makes the ReadRequest available to the machine (if possible)
     * This "read" does not have the meaning of the other reads, which are accessors... */
    p_read_ok, p_nb_ReadValue <-- check_ReadRequest(p_msg) =
    PRE
        p_msg  : t_msg_i &
        p_msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req
    THEN
        p_read_ok,
        p_nb_ReadValue,
        nb_ReadValue,
        ReadValue_NodeId,
        ReadValue_AttributeId
        :(  p_read_ok      : BOOL & /* Error code, for now a simple bool */
            p_nb_ReadValue : INT  &
            d_inv          &
            (p_read_ok = TRUE
             =>
             p_nb_ReadValue = nb_ReadValue)
        )
    END
    ;

    p_isvalid, p_nid, p_aid <-- getall_ReadValue_NodeId_AttributeId(p_msg, p_rvi) =
    PRE
        p_msg    : t_msg_i                     &
        p_msg    = c_msg_in                    &
        c_msg_in : t_msg                       &
        a_msg_in_type = e_msg_session_read_req &
        p_rvi : t_ReadValue_i                  &
        p_rvi : t_ReadValue                    &
        p_rvi : cast_t_ReadValue[0..nb_ReadValue]
    THEN
       IF  p_rvi : dom(ReadValue_NodeId) &
           p_rvi : dom(ReadValue_AttributeId)
       THEN
           p_isvalid := TRUE ||
           p_nid,
           p_aid
           :(p_nid : t_NodeId_i                   &
             p_nid : t_NodeId                     &
             p_aid : t_AttributeId_i              &
             p_aid : t_AttributeId                &
             p_nid = ReadValue_NodeId(p_rvi)      &
             p_aid = ReadValue_AttributeId(p_rvi)
            )
       ELSE
           p_isvalid := FALSE           ||
           p_nid     :: t_NodeId_i      ||
           p_aid     :: t_AttributeId_i
       END
    END
    ;

    p_nb_ReadValue <-- get_nb_ReadValue =
    BEGIN
        p_nb_ReadValue := nb_ReadValue
    END
                 
END
