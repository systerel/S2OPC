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
    constants

SETS
    /* The nodes of the Address Space \/ {indet} */
    t_Node_i;
    /* The node IDs. There might not be a NODE behind a node ID. */
    t_NodeId_i;
    /* The node classes */
    t_NodeClass_i = {c_NodeClass_indet,
                     e_ncl_Object, e_ncl_Variable, e_ncl_Method,
                     e_ncl_ObjectType, e_ncl_VariableType, e_ncl_ReferenceType, e_ncl_DataType,
                     e_ncl_View};
    t_Variant_i;
    t_AttributeId_i = {c_AttributeId_indet, e_aid_NodeId, e_aid_NodeClass, e_aid_Value};
    t_StatusCode_i = {c_StatusCode_indet, e_sc_ok, e_sc_nok,
                      e_sc_bad_secure_channel_closed,
                      e_sc_bad_connection_closed,
                      e_sc_bad_invalid_state,
                      e_sc_bad_session_id_invalid,
                      e_sc_bad_session_closed,
                      e_sc_bad_identity_token_invalid,
                      e_sc_bad_encoding_error,
                      e_sc_bad_invalid_argument,
                      e_sc_bad_unexpected_error,
                      e_sc_bad_out_of_memory};
    /*t_Timestamp;*/

    /* A ReadValue of a ReadRequest. The size of that will define the nb_max_read_value. */
    t_ReadValue_i;
    /* A WriteValue of a WriteRequest. */
    t_WriteValue_i;
    t_ByteString_i;
    t_UserId_i;

    t_channel_i;
    t_session_i;
    t_session_token_i;
    t_user_i;
    t_msg_i;
    t_request_handle_i;
    t_channel_config_idx_i;
    
    t_sessionState = {
        e_session_init,
        e_session_creating, 
        e_session_created, 
        e_session_userActivating, 
        e_session_userActivated, 
        e_session_scActivating, 
        e_session_scOrphaned, 
        e_session_closing, 
        e_session_closed};

    t_msg_type = {
        e_msg_tcpua_hello, e_msg_tcpua_ack,
        e_msg_sc_open_channel_req, e_msg_sc_open_channel_resp,
        e_msg_sc_close_channel_req, e_msg_sc_close_channel_resp,
        e_msg_public_service_req, e_msg_public_service_resp,
        e_msg_session_create_req, e_msg_session_create_resp,
        e_msg_session_activate_req, e_msg_session_activate_resp,
        e_msg_session_close_req, e_msg_session_close_resp,
        e_msg_session_read_req, e_msg_session_read_resp,
        e_msg_session_write_req, e_msg_session_write_resp}

CONCRETE_CONSTANTS
    /*AddS,*/
    k_n_Nodes,
    k_n_read_resp_max,
    k_n_WriteResponse_max,

    c_Variant_indet,
    c_NodeId_indet,
    c_Node_indet,
    c_ReadValue_indet,
    c_WriteValue_indet,
    c_ByteString_indet,
    c_UserId_indet,

    c_channel_indet,
    c_user_indet,
    c_session_indet,
    c_session_token_indet,
    c_msg_indet,
    c_channel_config_idx_indet,
    c_request_handle_indet

ABSTRACT_CONSTANTS
    conv_NodeId_Variant,
    conv_NodeClass_Variant,

    t_StatusCode,
    t_AttributeId,
    t_NodeClass,
    t_Variant,
    t_NodeId,
    t_Node,
    t_ReadValue,
    t_WriteValue,
    t_ByteString,
    t_UserId,

    cast_t_ReadValue,
    cast_t_WriteValue,

    t_channel,
    t_session,
    t_session_token,
    t_user,
    t_msg,
    t_channel_config_idx,
    t_request_handle,

    s_service_reqs,
    s_service_resps,
    s_req_msgs,
    s_resp_msgs

PROPERTIES
    /*AddS <: t_Node &*/
    t_Variant <: t_Variant_i &
    c_Variant_indet : t_Variant_i &
    c_Variant_indet /: t_Variant &

    t_Node <: t_Node_i &
    c_Node_indet : t_Node_i &
    c_Node_indet /: t_Node &

    t_NodeId <: t_NodeId_i &
    c_NodeId_indet : t_NodeId_i &
    c_NodeId_indet /: t_NodeId &

    t_StatusCode = {e_sc_ok, e_sc_nok,
                      e_sc_bad_secure_channel_closed,
                      e_sc_bad_connection_closed,
                      e_sc_bad_invalid_state,
                      e_sc_bad_session_id_invalid,
                      e_sc_bad_session_closed,
                      e_sc_bad_identity_token_invalid,
                      e_sc_bad_encoding_error,
                      e_sc_bad_invalid_argument,
                      e_sc_bad_unexpected_error,
                      e_sc_bad_out_of_memory} &
    t_AttributeId = {e_aid_NodeId, e_aid_NodeClass, e_aid_Value} &

    t_NodeClass = t_NodeClass_i - {c_NodeClass_indet} &

    conv_NodeId_Variant : t_NodeId --> t_Variant &
    conv_NodeClass_Variant : t_NodeClass --> t_Variant &

    k_n_Nodes : NAT &
    k_n_read_resp_max : NAT &
    k_n_WriteResponse_max : NAT &

    t_Node <: t_Node_i &
    c_Node_indet : t_Node_i &
    c_Node_indet /: t_Node &

    t_ReadValue <: t_ReadValue_i &
    c_ReadValue_indet : t_ReadValue_i &
    c_ReadValue_indet /: t_ReadValue &
    cast_t_ReadValue : NAT +-> t_ReadValue_i &
    cast_t_ReadValue : perm(t_ReadValue) &

    t_WriteValue <: t_WriteValue_i &
    c_WriteValue_indet : t_WriteValue_i &
    c_WriteValue_indet /: t_WriteValue &
    cast_t_WriteValue : NAT +-> t_WriteValue_i &
    cast_t_WriteValue : perm(t_WriteValue) &

    t_ByteString <: t_ByteString_i &
    c_ByteString_indet : t_ByteString_i &
    c_ByteString_indet /: t_ByteString &

    t_UserId <: t_UserId_i &
    c_UserId_indet : t_UserId_i &
    c_UserId_indet /: t_UserId &

    t_channel <: t_channel_i &
    c_channel_indet : t_channel_i &
    c_channel_indet /: t_channel &

    t_session <: t_session_i &
    c_session_indet : t_session_i &
    c_session_indet /: t_session &

    t_session_token <: t_session_token_i &
    c_session_token_indet : t_session_token_i &
    c_session_token_indet /: t_session_token &

    t_user <: t_user_i &
    c_user_indet : t_user_i &
    c_user_indet /: t_user &

    t_msg <: t_msg_i &
    c_msg_indet : t_msg_i &
    c_msg_indet /: t_msg &

    t_request_handle <: t_request_handle_i &
    c_request_handle_indet : t_request_handle_i &
    c_request_handle_indet /: t_request_handle &
    
    t_channel_config_idx <: t_channel_config_idx_i &
    c_channel_config_idx_indet : t_channel_config_idx_i &
    c_channel_config_idx_indet /: t_channel_config_idx &
    
    s_service_reqs  = {e_msg_session_read_req} &
    s_service_resps = {e_msg_session_read_resp} &
    s_req_msgs = {e_msg_tcpua_hello,
        e_msg_sc_open_channel_req,
        e_msg_sc_close_channel_req,
        e_msg_public_service_req,
        e_msg_session_create_req,
        e_msg_session_activate_req,
        e_msg_session_close_req,
        e_msg_session_read_req} &
    s_resp_msgs = {e_msg_tcpua_ack,
        e_msg_sc_open_channel_resp,
        e_msg_sc_close_channel_resp,
        e_msg_public_service_resp,
        e_msg_session_create_resp,
        e_msg_session_activate_resp,
        e_msg_session_close_resp,
        e_msg_session_read_resp}

OPERATIONS
    rvi <-- read_cast_t_ReadValue(ii) =
    PRE
        ii : NAT &
        ii : dom(cast_t_ReadValue)
    THEN
        rvi := cast_t_ReadValue(ii)
    END
    ;

    wvi <-- get_cast_t_WriteValue(ii) =
    PRE
        ii : NAT &
        ii : dom(cast_t_WriteValue)
    THEN
        wvi := cast_t_WriteValue(ii)
    END

END
