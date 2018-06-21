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
