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
    constants

SETS
    /* The nodes of the Address Space \/ {indet} */
    t_Node_i;
    /* The node IDs. There might not be a NODE behind a node ID. */
    t_NodeId_i;

    /* Expanded Nodes Id : nodes ID local the the address space server or from distant address spaces */
    t_ExpandedNodeId_i;

    /* Address space references */
    t_Reference_i;

    /* The node classes */
    t_NodeClass_i = {c_NodeClass_indet,
                     e_ncl_Object, e_ncl_Variable, e_ncl_Method,
                     e_ncl_ObjectType, e_ncl_VariableType, e_ncl_ReferenceType, e_ncl_DataType,
                     e_ncl_View};
    t_Variant_i;
    t_AttributeId_i = {c_AttributeId_indet,
                       e_aid_NodeId,
                       e_aid_NodeClass,
                       e_aid_BrowseName,
                       e_aid_DisplayName,
                       e_aid_Description,
                       e_aid_Value,
                       e_aid_AccessLevel};
    t_StatusCode_i = {c_StatusCode_indet, e_sc_ok,
                      e_sc_bad_generic,
                      e_sc_uncertain_generic,
                      e_sc_bad_internal_error,
                      e_sc_bad_secure_channel_closed,
                      e_sc_bad_secure_channel_id_invalid,
                      e_sc_bad_connection_closed,
                      e_sc_bad_invalid_state,
                      e_sc_bad_session_id_invalid,
                      e_sc_bad_session_closed,
                      e_sc_bad_session_not_activated,
                      e_sc_bad_identity_token_invalid,
                      e_sc_bad_encoding_error,
                      e_sc_bad_decoding_error,
                      e_sc_bad_invalid_argument,
                      e_sc_bad_unexpected_error,
                      e_sc_bad_out_of_memory,
                      e_sc_bad_nothing_to_do,
                      e_sc_bad_too_many_ops,
                      e_sc_bad_timestamps_to_return_invalid,
                      e_sc_bad_max_age_invalid,
                      e_sc_bad_node_id_unknown,
                      e_sc_bad_node_id_invalid,
                      e_sc_bad_attribute_id_invalid,
                      e_sc_bad_service_unsupported,
                      e_sc_bad_write_not_supported};

    t_BrowseDirection_i = {e_bd_indet,
                           e_bd_forward,
                           e_bd_inverse,
                           e_bd_both};

    /*t_Timestamp;*/

    t_TimestampsToReturn_i = {c_TimestampsToReturn_indet,
                              e_ttr_source,
                              e_ttr_server,
                              e_ttr_both,
                              e_ttr_neither};

    /* A ReadValue of a ReadRequest. The size of that will define the nb_max_read_value. */
    t_ReadValue_i;

    /* A WriteValue of a WriteRequest. */
    t_WriteValue_i;

    /* A BrowseValue of a BrowseRequest. */
    t_BrowseValue_i;
    t_BrowseResult_i;

    t_byte_buffer_i;
    t_LocalizedText_i;
    t_QualifiedName_i;

    t_session_i;
    t_session_token_i;
    t_user_i;
    t_Nonce_i; /* void* */
    t_SignatureData_i; /* void* */
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

    t_SecurityPolicy = {
        e_secpol_None,
        e_secpol_B256,
        e_secpol_B256S256};

    t_buffer_in_state_i = {c_buffer_in_state_indet,
        e_buffer_in_msg_not_read,
        e_buffer_in_msg_type_read,
        e_buffer_in_msg_header_read,
        e_buffer_in_msg_read};

    t_buffer_out_state_i = {c_buffer_out_state_indet,
        e_buffer_out_msg_written};

    /* OPC UA message type */
    t_msg_type_i = {c_msg_type_indet, e_msg_service_fault_resp,
        e_msg_discovery_find_servers_req,
        e_msg_discovery_find_servers_resp,
        e_msg_discovery_find_servers_on_network_req,
        e_msg_discovery_find_servers_on_network_resp,
        e_msg_discovery_get_endpoints_req,
        e_msg_discovery_get_endpoints_resp,
        e_msg_discovery_register_server_req,
        e_msg_discovery_register_server_resp,
        e_msg_discovery_register_server2_req,
        e_msg_discovery_register_server2_resp,
        e_msg_session_create_req,
        e_msg_session_create_resp,
        e_msg_session_activate_req,
        e_msg_session_activate_resp,
        e_msg_session_close_req,
        e_msg_session_close_resp,
        e_msg_session_cancel_req,
        e_msg_session_cancel_resp,
        e_msg_node_add_nodes_req,
        e_msg_node_add_nodes_resp,
        e_msg_node_add_references_req,
        e_msg_node_add_references_resp,
        e_msg_node_delete_nodes_req,
        e_msg_node_delete_nodes_resp,
        e_msg_node_delete_references_req,
        e_msg_node_delete_references_resp,
        e_msg_view_browse_req,
        e_msg_view_browse_resp,
        e_msg_view_browse_next_req,
        e_msg_view_browse_next_resp,
        e_msg_view_translate_browse_paths_to_node_ids_req,
        e_msg_view_translate_browse_paths_to_node_ids_resp,
        e_msg_view_register_nodes_req,
        e_msg_view_register_nodes_resp,
        e_msg_view_unregister_nodes_req,
        e_msg_view_unregister_nodes_resp,
        e_msg_query_first_req,
        e_msg_query_first_resp,
        e_msg_query_next_req,
        e_msg_query_next_resp,
        e_msg_attribute_read_req,
        e_msg_attribute_read_resp,
        e_msg_attribute_history_read_req,
        e_msg_attribute_history_read_resp,
        e_msg_attribute_write_req,
        e_msg_attribute_write_resp,
        e_msg_attribute_history_update_req,
        e_msg_attribute_history_update_resp,
        e_msg_method_call_req,
        e_msg_method_call_resp,
        e_msg_monitored_items_create_req,
        e_msg_monitored_items_create_resp,
        e_msg_monitored_items_modify_req,
        e_msg_monitored_items_modify_resp,
        e_msg_monitored_items_set_monitoring_mode_req,
        e_msg_monitored_items_set_monitoring_mode_resp,
        e_msg_monitored_items_set_triggering_req,
        e_msg_monitored_items_set_triggering_resp,
        e_msg_monitored_items_delete_req,
        e_msg_monitored_items_delete_resp,
        e_msg_subscription_create_req,
        e_msg_subscription_create_resp,
        e_msg_subscription_modify_req,
        e_msg_subscription_modify_resp,
        e_msg_subscription_set_publishing_mode_req,
        e_msg_subscription_set_publishing_mode_resp,
        e_msg_subscription_publish_req,
        e_msg_subscription_publish_resp,
        e_msg_subscription_republish_req,
        e_msg_subscription_republish_resp,
        e_msg_subscription_transfer_subscriptions_req,
        e_msg_subscription_transfer_subscriptions_resp,
        e_msg_subscription_delete_subscriptions_req,
        e_msg_subscription_delete_subscriptions_resp};

    /* OPC UA message header type (request / response) */
    t_msg_header_type_i = { c_msg_header_type_indet,
        e_msg_request_type,
        e_msg_response_type
    };

    /* Service type: session management / service on a session / public service */
    t_msg_service_class_i = { c_msg_service_class_indet,
        e_msg_session_treatment_class,
        e_msg_session_service_class,
        e_msg_discovery_service_class,
        e_msg_service_fault_class
    }

CONCRETE_CONSTANTS
    /*AddS,*/
    k_n_Nodes,
    k_n_read_resp_max,
    k_n_WriteResponse_max,
    k_n_BrowseResponse_max,
    k_n_BrowseTarget_max,

    c_Variant_indet,
    c_NodeId_indet,
    c_ExpandedNodeId_indet,
    c_Reference_indet,
    c_Node_indet,
    c_ReadValue_indet,
    c_WriteValue_indet,
    c_BrowseValue_indet,
    c_BrowseResult_indet,
    c_byte_buffer_indet,
    c_LocalizedText_indet,
    c_QualifiedName_indet,

    c_endpoint_config_idx_indet,
    c_user_indet,
    c_session_indet,
    c_session_token_indet,
    c_Nonce_indet,
    c_SignatureData_indet,
    c_msg_header_indet,
    c_msg_indet,
    c_request_context_indet,
    c_channel_config_idx_indet,
    c_channel_indet,
    c_request_handle_indet

ABSTRACT_CONSTANTS
    conv_NodeId_Variant,
    conv_NodeClass_Variant,

    conv_ExpandedNodeId_NodeId,

    t_StatusCode,
    t_BrowseDirection,
    t_TimestampsToReturn,
    t_AttributeId,
    t_NodeClass,
    t_Variant,
    t_NodeId,
    t_ExpandedNodeId,
    t_Reference,
    t_Node,
    t_ReadValue,
    t_WriteValue,
    t_BrowseValue,
    t_BrowseResult,
    t_RefIndex,
    t_byte_buffer,
    t_LocalizedText,
    t_QualifiedName,

    cast_t_session,
    cast_t_channel,
    cast_t_ReadValue,
    cast_t_WriteValue,
    cast_t_BrowseValue,
    cast_t_BrowseResult,

    t_endpoint_config_idx,
    t_session,
    t_session_token,
    t_user,
    t_Nonce,
    t_SignatureData,
    t_buffer_in_state,
    t_msg_header,
    t_msg,
    t_request_context,
    t_msg_type,
    t_msg_header_type,
    t_msg_service_class,
    t_channel_config_idx,
    t_channel,
    t_request_handle,

    s_session_treatment_reqs,
    s_session_treatment_resps,
    s_service_on_session_reqs,
    s_service_on_session_resps,
    s_discovery_service_reqs,
    s_discovery_service_resps,
    s_service_reqs,
    s_service_resps,

    Is_SubType,
    Is_Dir_Forward_Compatible

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

    t_ExpandedNodeId <: t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet : t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet /: t_ExpandedNodeId &

    t_Reference <: t_Reference_i &
    c_Reference_indet : t_Reference_i &
    c_Reference_indet /: t_Reference &

    t_StatusCode = t_StatusCode_i - {c_StatusCode_indet} &

    t_BrowseDirection = {e_bd_forward,
                         e_bd_inverse,
                         e_bd_both} &

    t_TimestampsToReturn = {c_TimestampsToReturn_indet,
                            e_ttr_source,
                            e_ttr_server,
                            e_ttr_both,
                            e_ttr_neither} &

    t_AttributeId = {e_aid_NodeId,
                     e_aid_NodeClass,
                     e_aid_BrowseName,
                     e_aid_DisplayName,
                     e_aid_Description,
                     e_aid_Value,
                     e_aid_AccessLevel} &

    t_NodeClass = t_NodeClass_i - {c_NodeClass_indet} &

    conv_NodeId_Variant : t_NodeId --> t_Variant &
    conv_NodeClass_Variant : t_NodeClass --> t_Variant &

    conv_ExpandedNodeId_NodeId : t_ExpandedNodeId +-> t_NodeId &

    k_n_Nodes : NAT &
    k_n_read_resp_max : NAT &
    k_n_WriteResponse_max : NAT &
    k_n_BrowseResponse_max : NAT &
    k_n_BrowseTarget_max : NAT &

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

    t_BrowseValue <: t_BrowseValue_i &
    c_BrowseValue_indet : t_BrowseValue_i &
    c_BrowseValue_indet /: t_BrowseValue &
    cast_t_BrowseValue : NAT +-> t_BrowseValue_i &
    cast_t_BrowseValue : perm(t_BrowseValue) &

    t_BrowseResult <: t_BrowseResult_i &
    c_BrowseResult_indet : t_BrowseResult_i &
    c_BrowseResult_indet /: t_BrowseResult &
    cast_t_BrowseResult : NAT +-> t_BrowseResult_i &
    cast_t_BrowseResult : perm(t_BrowseResult) &

    t_RefIndex <: NAT &

    t_byte_buffer <: t_byte_buffer_i &
    c_byte_buffer_indet : t_byte_buffer_i &
    c_byte_buffer_indet /: t_byte_buffer &

    t_LocalizedText <: t_LocalizedText_i &
    c_LocalizedText_indet : t_LocalizedText_i &
    c_LocalizedText_indet /: t_LocalizedText &

    t_QualifiedName <: t_QualifiedName_i &
    c_QualifiedName_indet : t_QualifiedName_i &
    c_QualifiedName_indet /: t_QualifiedName &

    t_channel <: t_channel_i &
    c_channel_indet : t_channel_i &
    c_channel_indet /: t_channel &
    cast_t_channel : NAT +-> t_channel_i &
    cast_t_channel : perm(t_channel) &

    t_endpoint_config_idx <: t_endpoint_config_idx_i &
    c_endpoint_config_idx_indet : t_endpoint_config_idx_i &
    c_endpoint_config_idx_indet /: t_endpoint_config_idx &

    t_session <: t_session_i &
    c_session_indet : t_session_i &
    c_session_indet /: t_session &
    cast_t_session : NAT +-> t_session_i &
    cast_t_session : perm(t_session) &

    t_session_token <: t_session_token_i &
    c_session_token_indet : t_session_token_i &
    c_session_token_indet /: t_session_token &

    t_Nonce <: t_Nonce_i &
    c_Nonce_indet : t_Nonce_i &
    c_Nonce_indet /: t_Nonce &

    t_SignatureData <: t_SignatureData_i &
    c_SignatureData_indet : t_SignatureData_i &
    c_SignatureData_indet /: t_SignatureData &

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

    s_session_treatment_reqs = {e_msg_session_create_req,
        e_msg_session_activate_req,
        e_msg_session_close_req,
        e_msg_session_cancel_req } &

    s_session_treatment_resps = {e_msg_session_create_resp,
        e_msg_session_activate_resp,
        e_msg_session_close_resp,
        e_msg_session_cancel_resp } &

    s_service_on_session_reqs  = {
        e_msg_node_add_nodes_req,
        e_msg_node_add_references_req,
        e_msg_node_delete_nodes_req,
        e_msg_node_delete_references_req,
        e_msg_view_browse_req,
        e_msg_view_browse_next_req,
        e_msg_view_translate_browse_paths_to_node_ids_req,
        e_msg_view_register_nodes_req,
        e_msg_view_unregister_nodes_req,
        e_msg_query_first_req,
        e_msg_query_next_req,
        e_msg_attribute_read_req,
        e_msg_attribute_history_read_req,
        e_msg_attribute_write_req,
        e_msg_attribute_history_update_req,
        e_msg_method_call_req,
        e_msg_monitored_items_create_req,
        e_msg_monitored_items_modify_req,
        e_msg_monitored_items_set_monitoring_mode_req,
        e_msg_monitored_items_set_monitoring_mode_resp,
        e_msg_monitored_items_set_triggering_req,
        e_msg_monitored_items_delete_req,
        e_msg_subscription_create_req,
        e_msg_subscription_modify_req,
        e_msg_subscription_set_publishing_mode_req,
        e_msg_subscription_publish_req,
        e_msg_subscription_republish_req,
        e_msg_subscription_transfer_subscriptions_req,
        e_msg_subscription_delete_subscriptions_req
    } &

    s_service_on_session_resps = {
        e_msg_node_add_nodes_resp,
        e_msg_node_add_references_resp,
        e_msg_node_delete_nodes_resp,
        e_msg_node_delete_references_resp,
        e_msg_view_browse_resp,
        e_msg_view_browse_next_resp,
        e_msg_view_translate_browse_paths_to_node_ids_resp,
        e_msg_view_register_nodes_resp,
        e_msg_view_unregister_nodes_resp,
        e_msg_query_first_resp,
        e_msg_query_next_resp,
        e_msg_attribute_read_resp,
        e_msg_attribute_history_read_resp,
        e_msg_attribute_write_resp,
        e_msg_attribute_history_update_resp,
        e_msg_method_call_resp,
        e_msg_monitored_items_create_resp,
        e_msg_monitored_items_modify_resp,
        e_msg_monitored_items_set_monitoring_mode_resp,
        e_msg_monitored_items_set_triggering_resp,
        e_msg_monitored_items_delete_resp,
        e_msg_subscription_create_resp,
        e_msg_subscription_modify_resp,
        e_msg_subscription_set_publishing_mode_resp,
        e_msg_subscription_publish_resp,
        e_msg_subscription_republish_resp,
        e_msg_subscription_transfer_subscriptions_resp,
        e_msg_subscription_delete_subscriptions_resp} &

    s_discovery_service_reqs = {
        e_msg_discovery_find_servers_req,
        e_msg_discovery_find_servers_on_network_req,
        e_msg_discovery_get_endpoints_req,
        e_msg_discovery_register_server_req,
        e_msg_discovery_register_server2_req
    } &
    
    s_discovery_service_resps = {
        e_msg_discovery_find_servers_resp,
        e_msg_discovery_find_servers_on_network_resp,
        e_msg_discovery_get_endpoints_resp,
        e_msg_discovery_register_server_resp,
        e_msg_discovery_register_server2_resp} 
    &

    s_service_reqs  = s_session_treatment_reqs \/ s_service_on_session_reqs \/ s_discovery_service_reqs &
    s_service_resps = s_session_treatment_resps \/ s_service_on_session_resps \/ s_discovery_service_resps \/ {e_msg_service_fault_resp} &

    t_msg_type = s_service_reqs \/ s_service_resps &

    t_msg_header_type <: t_msg_header_type_i &
    c_msg_header_type_indet : t_msg_header_type_i &
    c_msg_header_type_indet /: t_msg_header_type &
    
    t_msg_service_class <: t_msg_service_class_i &
    c_msg_service_class_indet : t_msg_service_class_i &
    c_msg_service_class_indet /: t_msg_service_class &

    Is_SubType : t_NodeId * t_NodeId --> BOOL &

    Is_Dir_Forward_Compatible : t_BrowseDirection * BOOL --> BOOL &
    Is_Dir_Forward_Compatible = {e_bd_forward |-> FALSE |-> FALSE,
                                 e_bd_forward |-> TRUE  |-> TRUE,
                                 e_bd_inverse |-> FALSE |-> TRUE,
                                 e_bd_inverse |-> TRUE  |-> FALSE,
                                 e_bd_both    |-> FALSE |-> TRUE,
                                 e_bd_both    |-> TRUE  |-> TRUE}

OPERATIONS

    p_res <-- is_t_channel_config_idx(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i
    THEN
        p_res := bool(p_config_idx : t_channel_config_idx)
    END
    ;
        
    p_res <-- is_t_endpoint_config_idx(p_endpoint_config_idx) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i
    THEN
        p_res := bool(p_endpoint_config_idx : t_endpoint_config_idx)
    END
    ;
    
    p_card_session <-- get_card_t_session =
    BEGIN
        p_card_session := card(t_session)
    END
    ;

    p_session <-- get_cast_t_session(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_session)
    THEN
        p_session := cast_t_session(p_ind)
    END
    ;

    p_res <-- is_t_channel(p_channel) =
    PRE
        p_channel : t_channel_i
    THEN
        p_res := bool(p_channel : t_channel)
    END
    ;

    p_card_channel <-- get_card_t_channel =
    BEGIN
        p_card_channel := card(t_channel)
    END
    ;

    p_channel <-- get_cast_t_channel(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_channel)
    THEN
        p_channel := cast_t_channel(p_ind)
    END
    ;

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
    ;

    p_bvi <-- get_cast_t_BrowseValue(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_BrowseValue)
    THEN
        p_bvi := cast_t_BrowseValue(p_ind)
    END
    ;

    p_bri <-- get_cast_t_BrowseResult(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_BrowseResult)
    THEN
        p_bri := cast_t_BrowseResult(p_ind)
    END
    ;

    p_isvalid, p_nid <-- getall_conv_ExpandedNodeId_NodeId(p_expnid) =
    PRE
        p_expnid : t_ExpandedNodeId_i &
        p_expnid : t_ExpandedNodeId
    THEN
        IF  p_expnid : dom(conv_ExpandedNodeId_NodeId) THEN
            p_isvalid := TRUE ||
            p_nid :(p_nid : t_NodeId_i &
                    p_nid = conv_ExpandedNodeId_NodeId(p_expnid))
        ELSE
            p_isvalid := FALSE ||
            p_nid     :: t_NodeId_i
        END
    END
    ;

    p_dir_compat <-- get_Is_Dir_Forward_Compatible(p_dir, p_IsForward) =
    PRE
        p_dir       : t_BrowseDirection_i &
        p_dir       : t_BrowseDirection   &
        p_IsForward : BOOL
    THEN
        p_dir_compat := Is_Dir_Forward_Compatible(p_dir |-> p_IsForward)
    END
    ;

    p_res <-- get_Is_SubType(p_type1, p_type2) =
    PRE
        p_type1 : t_NodeId_i &
        p_type1 : t_NodeId   &
        p_type2 : t_NodeId_i &
        p_type2 : t_NodeId
    THEN
        p_res := Is_SubType(p_type1 |-> p_type2)
    END

END
