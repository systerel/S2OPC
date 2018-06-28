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
    p_read_ok, p_nb_ReadValue, p_tsToReturn, p_statusCode <-- check_ReadRequest(p_msg) =
    PRE
        p_msg  : t_msg_i &
        p_msg  = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_read_req
    THEN
        p_read_ok,
        p_nb_ReadValue,
        p_tsToReturn,
        p_statusCode,
        nb_ReadValue,
        ReadValue_NodeId,
        ReadValue_AttributeId
        :(  p_read_ok      : BOOL & /* Error code, for now a simple bool */
            p_nb_ReadValue : INT  &
            p_tsToReturn   : t_TimestampsToReturn_i &
            p_statusCode   : t_StatusCode &
            d_inv          &
            (p_read_ok = TRUE
             =>
             p_nb_ReadValue = nb_ReadValue &
             p_tsToReturn : t_TimestampsToReturn &
             p_statusCode = e_sc_ok)
        )
    END
    ;

    p_isvalid, p_nid, p_aid <-- getall_ReadValue_NodeId_AttributeId(p_msg, p_rvi) =
    PRE
        p_msg    : t_msg_i                     &
        p_msg    = c_msg_in                    &
        c_msg_in : t_msg                       &
        a_msg_in_type = e_msg_attribute_read_req &
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
