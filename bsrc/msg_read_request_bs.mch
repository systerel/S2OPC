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
    msg_read_request_bs

SEES
    constants,
    message_in_bs

ABSTRACT_VARIABLES
    ReadValue_NodeId,
    ReadValue_AttributeId

INVARIANT
    ReadValue_NodeId      : t_ReadValue +-> t_NodeId &
    ReadValue_AttributeId : t_ReadValue +-> t_AttributeId

INITIALISATION
    ReadValue_NodeId,
    ReadValue_AttributeId
    :(
        ReadValue_NodeId      = {} &
        ReadValue_AttributeId = {}
     )

OPERATIONS
    nb <-- read_req_nb_ReadValue(msg) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        nb :: INT
    END
    ;

    p_tsToReturn <-- read_req_TimestampsToReturn(p_msg) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_tsToReturn :: t_TimestampsToReturn_i
    END
    ;
    
    p_maxAge_valid <-- read_req_MaxAge(p_msg) = 
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_maxAge_valid :: BOOL
    END
    ;

    /* Returns the NodeId (pointer or hash_id, or...)
     * Does not check if there is a node in the AddressSpace corresponding to this nid */
    isvalid, nid <-- getall_req_ReadValue_NodeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue
    THEN
        IF  rvi : dom(ReadValue_NodeId) THEN
            isvalid := TRUE ||
            nid :(nid : t_NodeId_i &
                  nid : t_NodeId &
                  nid = ReadValue_NodeId(rvi))
        ELSE
            isvalid := FALSE ||
            nid     :: t_NodeId_i
        END
    END
    ;

    /* Returns the AttributeId if it is valid */
    /* ... Could have merged them */
    isvalid, aid <-- getall_req_ReadValue_AttributeId(msg, rvi) =
    PRE
        msg  : t_msg_i &
        msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req &
        rvi : t_ReadValue_i &
        rvi : t_ReadValue
    THEN
        IF  rvi : dom(ReadValue_AttributeId) THEN
            isvalid := TRUE ||
            aid :(aid : t_AttributeId_i &
                  aid = ReadValue_AttributeId(rvi))
        ELSE
            isvalid := FALSE ||
            aid     :: t_AttributeId_i
        END
    END

END
