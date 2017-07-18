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
        nb_ReadValue : NAT &
        init_ok : BOOL &
        ReadValue_Node : t_ReadValue +-> t_Node &
        ReadValue_AttributeId : t_ReadValue +-> t_AttributeId
    )

ABSTRACT_VARIABLES
    init_ok,
    nb_ReadValue,
    ReadValue_Node,
    ReadValue_AttributeId

INVARIANT
    d_inv &
    (init_ok = TRUE
     =>
     nb_ReadValue <= k_n_read_resp_max)

INITIALISATION
    init_ok := FALSE ||
    nb_ReadValue := 0  ||
    ReadValue_Node := {} ||
    ReadValue_AttributeId := {}

OPERATIONS

    /* Makes the ReadRequest available to the machine (if possible)
     * This "read" does not have the meaning of the other reads, which are accessors... */
    rr, nb <-- read_ReadRequest(msg) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req
    THEN
       rr,
       nb,
       init_ok,
       nb_ReadValue,
       ReadValue_Node,
        ReadValue_AttributeId
       :(rr : BOOL & /* Error code, for now a simple bool */
         nb : NAT &
         d_inv &
         rr = init_ok &
         (rr = TRUE
          =>
          nb_ReadValue <= k_n_read_resp_max &
          nb = nb_ReadValue)
        )
    END
    ;

    rr <-- read_nb_ReadValue =
    PRE
        init_ok = TRUE
    THEN
        rr := nb_ReadValue
    END
    ;

    isvalid, node, aid <-- readall_ReadValue_Node_AttributeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue &
        init_ok = TRUE
    THEN
       IF rvi : dom(ReadValue_Node) &
          rvi : dom(ReadValue_AttributeId)
       THEN
          isvalid := TRUE ||
          node,
          aid
          :(node : t_Node_i &
            aid  : t_AttributeId_i &
            node = ReadValue_Node(rvi) &
            aid  = ReadValue_AttributeId(rvi)
           )
       ELSE
          isvalid := FALSE           ||
          node    :: t_Node_i        ||
          aid     :: t_AttributeId_i
       END
    END

END
