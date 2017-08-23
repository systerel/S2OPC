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
                      e_sc_bad_secure_channel_id_invalid,
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
    t_byte_buffer_i;
    t_UserId_i;
    
    t_session_i;
    t_session_token_i;
    t_user_i;
    t_msg_header_i;
    t_msg_i;
    t_request_context_i;
    t_request_handle_i;
    t_channel_config_idx_i;
    t_channel_i;
    t_endpoint_config_idx_i;
    
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
    
    t_buffer_in_state_i = {c_buffer_in_state_indet,
        e_buffer_in_msg_not_read,
        e_buffer_in_msg_type_read,
        e_buffer_in_msg_header_read,
        e_buffer_in_msg_read};
    
    t_buffer_out_state_i = {c_buffer_out_state_indet,
        e_buffer_out_msg_written};
    
    /* OPC UA message type */
    t_msg_type_i = {c_msg_type_indet, e_msg_service_fault_resp,
        e_msg_get_endpoints_service_req, e_msg_get_endpoints_service_resp,
        e_msg_session_create_req, e_msg_session_create_resp,
        e_msg_session_activate_req, e_msg_session_activate_resp,
        e_msg_session_close_req, e_msg_session_close_resp,
        e_msg_session_read_req, e_msg_session_read_resp,
        e_msg_session_write_req, e_msg_session_write_resp};
    
    /* OPC UA message header type (request / response) */
    t_msg_header_type = {
        e_msg_request_type,
        e_msg_response_type
    };
    
    /* Service type: session management / service on a session / public service */
    t_msg_service_class = {
        e_msg_session_treatment_class,
        e_msg_session_service_class,
        e_msg_discovery_service_class
    }

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
    c_byte_buffer_indet,
    c_UserId_indet,

    c_endpoint_config_idx_indet,
    c_user_indet,
    c_session_indet,
    c_session_token_indet,
    c_msg_header_indet,
    c_msg_indet,
    c_request_context_indet,
    c_channel_config_idx_indet,
    c_channel_indet,
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
    t_byte_buffer,
    t_UserId,

    cast_t_ReadValue,
    cast_t_WriteValue,
    
    t_endpoint_config_idx,
    t_session,
    t_session_token,
    t_user,
    t_buffer_in_state,
    t_msg_header,
    t_msg,
    t_request_context,
    t_msg_type,
    t_channel_config_idx,
    t_channel,
    t_request_handle,
    
    s_service_treatment_reqs,
    s_service_treatment_resps,
    s_service_on_session_reqs,
    s_service_on_session_resps,
    s_discovery_service_reqs,
    s_discovery_service_resps,
    s_service_reqs,
    s_service_resps

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

    t_byte_buffer <: t_byte_buffer_i &
    c_byte_buffer_indet : t_byte_buffer_i &
    c_byte_buffer_indet /: t_byte_buffer &

    t_UserId <: t_UserId_i &
    c_UserId_indet : t_UserId_i &
    c_UserId_indet /: t_UserId &

    t_channel <: t_channel_i &
    c_channel_indet : t_channel_i &
    c_channel_indet /: t_channel &
    
    t_endpoint_config_idx <: t_endpoint_config_idx_i &
    c_endpoint_config_idx_indet : t_endpoint_config_idx_i & 
    c_endpoint_config_idx_indet /: t_endpoint_config_idx &
    
    t_session <: t_session_i &
    c_session_indet : t_session_i &
    c_session_indet /: t_session &

    t_session_token <: t_session_token_i &
    c_session_token_indet : t_session_token_i &
    c_session_token_indet /: t_session_token &

    t_user <: t_user_i &
    c_user_indet : t_user_i &
    c_user_indet /: t_user &
    
    t_msg_header <: t_msg_header_i &
    c_msg_header_indet : t_msg_header_i &
    c_msg_header_indet /: t_msg_header &
    
    t_msg <: t_msg_i &
    c_msg_indet : t_msg_i &
    c_msg_indet /: t_msg &

    t_request_context <: t_request_context_i &
    c_request_context_indet : t_request_context_i &
    c_request_context_indet /: t_request_context &

    t_request_handle <: t_request_handle_i &
    c_request_handle_indet : t_request_handle_i &
    c_request_handle_indet /: t_request_handle &
    
    t_channel_config_idx <: t_channel_config_idx_i &
    c_channel_config_idx_indet : t_channel_config_idx_i &
    c_channel_config_idx_indet /: t_channel_config_idx &
    
    t_buffer_in_state = { e_buffer_in_msg_not_read,
        e_buffer_in_msg_type_read,
        e_buffer_in_msg_header_read,
        e_buffer_in_msg_read } &
    
    s_service_treatment_reqs = {e_msg_session_create_req,
        e_msg_session_activate_req,
        e_msg_session_close_req} &

    s_service_treatment_resps = {e_msg_session_create_resp,
        e_msg_session_activate_resp,
        e_msg_session_close_resp } &
    
    s_service_on_session_reqs  = {e_msg_session_read_req,
        e_msg_session_write_req} &
    
    s_service_on_session_resps = {e_msg_session_read_resp,
        e_msg_session_write_resp} &
    
    s_discovery_service_reqs = {e_msg_get_endpoints_service_req} &
    s_discovery_service_resps = {e_msg_get_endpoints_service_resp} &
    
    s_service_reqs  = s_service_treatment_reqs \/ s_service_on_session_reqs \/ s_discovery_service_reqs &
    s_service_resps = s_service_on_session_resps \/ s_discovery_service_resps \/ {e_msg_service_fault_resp} &
    
    t_msg_type = s_service_reqs \/ s_service_resps

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
