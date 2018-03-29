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

/******************************************************************************

 File Name            : constants.h

 Date                 : 29/03/2018 14:46:08

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _constants_h
#define _constants_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "constants_bs.h"

/*-----------------------------
   SETS Clause: deferred sets
  -----------------------------*/
#define constants__t_ExpandedNodeId_i constants_bs__t_ExpandedNodeId_i
#define constants__t_LocalizedText_i constants_bs__t_LocalizedText_i
#define constants__t_NodeId_i constants_bs__t_NodeId_i
#define constants__t_Node_i constants_bs__t_Node_i
#define constants__t_Nonce_i constants_bs__t_Nonce_i
#define constants__t_QualifiedName_i constants_bs__t_QualifiedName_i
#define constants__t_Reference_i constants_bs__t_Reference_i
#define constants__t_SignatureData_i constants_bs__t_SignatureData_i
#define constants__t_Variant_i constants_bs__t_Variant_i
#define constants__t_WriteValuePointer_i constants_bs__t_WriteValuePointer_i
#define constants__t_application_context_i constants_bs__t_application_context_i
#define constants__t_byte_buffer_i constants_bs__t_byte_buffer_i
#define constants__t_channel_config_idx_i constants_bs__t_channel_config_idx_i
#define constants__t_channel_i constants_bs__t_channel_i
#define constants__t_endpoint_config_idx_i constants_bs__t_endpoint_config_idx_i
#define constants__t_msg_header_i constants_bs__t_msg_header_i
#define constants__t_msg_i constants_bs__t_msg_i
#define constants__t_request_context_i constants_bs__t_request_context_i
#define constants__t_request_handle_i constants_bs__t_request_handle_i
#define constants__t_session_i constants_bs__t_session_i
#define constants__t_session_token_i constants_bs__t_session_token_i
#define constants__t_user_i constants_bs__t_user_i

#define constants__t_ReadValue_i t_entier4
#define constants__t_WriteValue_i t_entier4
#define constants__t_BrowseValue_i t_entier4
#define constants__t_BrowseResult_i t_entier4

/*-------------------------------
   SETS Clause: enumerated sets
  -------------------------------*/
typedef enum {
    constants__c_AttributeId_indet,
    constants__e_aid_NodeId,
    constants__e_aid_NodeClass,
    constants__e_aid_BrowseName,
    constants__e_aid_DisplayName,
    constants__e_aid_Description,
    constants__e_aid_Value,
    constants__e_aid_AccessLevel
} constants__t_AttributeId_i;
typedef enum {
    constants__e_bd_indet,
    constants__e_bd_forward,
    constants__e_bd_inverse,
    constants__e_bd_both
} constants__t_BrowseDirection_i;
typedef enum {
    constants__c_NodeClass_indet,
    constants__e_ncl_Object,
    constants__e_ncl_Variable,
    constants__e_ncl_Method,
    constants__e_ncl_ObjectType,
    constants__e_ncl_VariableType,
    constants__e_ncl_ReferenceType,
    constants__e_ncl_DataType,
    constants__e_ncl_View
} constants__t_NodeClass_i;
typedef enum {
    constants__e_secpol_None,
    constants__e_secpol_B256,
    constants__e_secpol_B256S256
} constants__t_SecurityPolicy;
typedef enum {
    constants__c_StatusCode_indet,
    constants__e_sc_ok,
    constants__e_sc_bad_generic,
    constants__e_sc_uncertain_generic,
    constants__e_sc_bad_internal_error,
    constants__e_sc_bad_secure_channel_closed,
    constants__e_sc_bad_secure_channel_id_invalid,
    constants__e_sc_bad_connection_closed,
    constants__e_sc_bad_invalid_state,
    constants__e_sc_bad_session_id_invalid,
    constants__e_sc_bad_session_closed,
    constants__e_sc_bad_session_not_activated,
    constants__e_sc_bad_identity_token_invalid,
    constants__e_sc_bad_encoding_error,
    constants__e_sc_bad_decoding_error,
    constants__e_sc_bad_invalid_argument,
    constants__e_sc_bad_unexpected_error,
    constants__e_sc_bad_out_of_memory,
    constants__e_sc_bad_nothing_to_do,
    constants__e_sc_bad_too_many_ops,
    constants__e_sc_bad_timestamps_to_return_invalid,
    constants__e_sc_bad_max_age_invalid,
    constants__e_sc_bad_node_id_unknown,
    constants__e_sc_bad_node_id_invalid,
    constants__e_sc_bad_view_id_unknown,
    constants__e_sc_bad_attribute_id_invalid,
    constants__e_sc_bad_browse_direction_invalid,
    constants__e_sc_bad_service_unsupported,
    constants__e_sc_bad_write_not_supported
} constants__t_StatusCode_i;
typedef enum {
    constants__c_TimestampsToReturn_indet,
    constants__e_ttr_source,
    constants__e_ttr_server,
    constants__e_ttr_both,
    constants__e_ttr_neither
} constants__t_TimestampsToReturn_i;
typedef enum {
    constants__c_buffer_in_state_indet,
    constants__e_buffer_in_msg_not_read,
    constants__e_buffer_in_msg_type_read,
    constants__e_buffer_in_msg_header_read,
    constants__e_buffer_in_msg_read
} constants__t_buffer_in_state_i;
typedef enum {
    constants__c_buffer_out_state_indet,
    constants__e_buffer_out_msg_written
} constants__t_buffer_out_state_i;
typedef enum {
    constants__c_msg_header_type_indet,
    constants__e_msg_request_type,
    constants__e_msg_response_type
} constants__t_msg_header_type_i;
typedef enum {
    constants__c_msg_service_class_indet,
    constants__e_msg_session_treatment_class,
    constants__e_msg_session_service_class,
    constants__e_msg_discovery_service_class,
    constants__e_msg_service_fault_class
} constants__t_msg_service_class_i;
typedef enum {
    constants__c_msg_type_indet,
    constants__e_msg_service_fault_resp,
    constants__e_msg_discovery_find_servers_req,
    constants__e_msg_discovery_find_servers_resp,
    constants__e_msg_discovery_find_servers_on_network_req,
    constants__e_msg_discovery_find_servers_on_network_resp,
    constants__e_msg_discovery_get_endpoints_req,
    constants__e_msg_discovery_get_endpoints_resp,
    constants__e_msg_discovery_register_server_req,
    constants__e_msg_discovery_register_server_resp,
    constants__e_msg_discovery_register_server2_req,
    constants__e_msg_discovery_register_server2_resp,
    constants__e_msg_session_create_req,
    constants__e_msg_session_create_resp,
    constants__e_msg_session_activate_req,
    constants__e_msg_session_activate_resp,
    constants__e_msg_session_close_req,
    constants__e_msg_session_close_resp,
    constants__e_msg_session_cancel_req,
    constants__e_msg_session_cancel_resp,
    constants__e_msg_node_add_nodes_req,
    constants__e_msg_node_add_nodes_resp,
    constants__e_msg_node_add_references_req,
    constants__e_msg_node_add_references_resp,
    constants__e_msg_node_delete_nodes_req,
    constants__e_msg_node_delete_nodes_resp,
    constants__e_msg_node_delete_references_req,
    constants__e_msg_node_delete_references_resp,
    constants__e_msg_view_browse_req,
    constants__e_msg_view_browse_resp,
    constants__e_msg_view_browse_next_req,
    constants__e_msg_view_browse_next_resp,
    constants__e_msg_view_translate_browse_paths_to_node_ids_req,
    constants__e_msg_view_translate_browse_paths_to_node_ids_resp,
    constants__e_msg_view_register_nodes_req,
    constants__e_msg_view_register_nodes_resp,
    constants__e_msg_view_unregister_nodes_req,
    constants__e_msg_view_unregister_nodes_resp,
    constants__e_msg_query_first_req,
    constants__e_msg_query_first_resp,
    constants__e_msg_query_next_req,
    constants__e_msg_query_next_resp,
    constants__e_msg_attribute_read_req,
    constants__e_msg_attribute_read_resp,
    constants__e_msg_attribute_history_read_req,
    constants__e_msg_attribute_history_read_resp,
    constants__e_msg_attribute_write_req,
    constants__e_msg_attribute_write_resp,
    constants__e_msg_attribute_history_update_req,
    constants__e_msg_attribute_history_update_resp,
    constants__e_msg_method_call_req,
    constants__e_msg_method_call_resp,
    constants__e_msg_monitored_items_create_req,
    constants__e_msg_monitored_items_create_resp,
    constants__e_msg_monitored_items_modify_req,
    constants__e_msg_monitored_items_modify_resp,
    constants__e_msg_monitored_items_set_monitoring_mode_req,
    constants__e_msg_monitored_items_set_monitoring_mode_resp,
    constants__e_msg_monitored_items_set_triggering_req,
    constants__e_msg_monitored_items_set_triggering_resp,
    constants__e_msg_monitored_items_delete_req,
    constants__e_msg_monitored_items_delete_resp,
    constants__e_msg_subscription_create_req,
    constants__e_msg_subscription_create_resp,
    constants__e_msg_subscription_modify_req,
    constants__e_msg_subscription_modify_resp,
    constants__e_msg_subscription_set_publishing_mode_req,
    constants__e_msg_subscription_set_publishing_mode_resp,
    constants__e_msg_subscription_publish_req,
    constants__e_msg_subscription_publish_resp,
    constants__e_msg_subscription_republish_req,
    constants__e_msg_subscription_republish_resp,
    constants__e_msg_subscription_transfer_subscriptions_req,
    constants__e_msg_subscription_transfer_subscriptions_resp,
    constants__e_msg_subscription_delete_subscriptions_req,
    constants__e_msg_subscription_delete_subscriptions_resp
} constants__t_msg_type_i;
typedef enum {
    constants__e_session_init,
    constants__e_session_creating,
    constants__e_session_created,
    constants__e_session_userActivating,
    constants__e_session_userActivated,
    constants__e_session_scActivating,
    constants__e_session_scOrphaned,
    constants__e_session_closing,
    constants__e_session_closed
} constants__t_sessionState;

/*--------------------------
   Added by the Translator
  --------------------------*/
#define constants__t_ExpandedNodeId_i_max constants_bs__t_ExpandedNodeId_i_max
#define constants__t_LocalizedText_i_max constants_bs__t_LocalizedText_i_max
#define constants__t_NodeId_i_max constants_bs__t_NodeId_i_max
#define constants__t_Node_i_max constants_bs__t_Node_i_max
#define constants__t_Nonce_i_max constants_bs__t_Nonce_i_max
#define constants__t_QualifiedName_i_max constants_bs__t_QualifiedName_i_max
#define constants__t_Reference_i_max constants_bs__t_Reference_i_max
#define constants__t_SignatureData_i_max constants_bs__t_SignatureData_i_max
#define constants__t_Variant_i_max constants_bs__t_Variant_i_max
#define constants__t_WriteValuePointer_i_max constants_bs__t_WriteValuePointer_i_max
#define constants__t_application_context_i_max constants_bs__t_application_context_i_max
#define constants__t_byte_buffer_i_max constants_bs__t_byte_buffer_i_max
#define constants__t_channel_config_idx_i_max constants_bs__t_channel_config_idx_i_max
#define constants__t_channel_i_max constants_bs__t_channel_i_max
#define constants__t_endpoint_config_idx_i_max constants_bs__t_endpoint_config_idx_i_max
#define constants__t_msg_header_i_max constants_bs__t_msg_header_i_max
#define constants__t_msg_i_max constants_bs__t_msg_i_max
#define constants__t_request_context_i_max constants_bs__t_request_context_i_max
#define constants__t_request_handle_i_max constants_bs__t_request_handle_i_max
#define constants__t_session_i_max constants_bs__t_session_i_max
#define constants__t_session_token_i_max constants_bs__t_session_token_i_max
#define constants__t_user_i_max constants_bs__t_user_i_max
#define constants__t_ReadValue_i_max constants__k_n_read_resp_max
#define constants__t_WriteValue_i_max constants__k_n_WriteResponse_max
#define constants__t_BrowseValue_i_max constants__k_n_BrowseResponse_max
#define constants__t_BrowseResult_i_max constants__k_n_BrowseTarget_max

/*------------------------------------------------
   CONCRETE_CONSTANTS Clause: scalars and arrays
  ------------------------------------------------*/
#define constants__c_ExpandedNodeId_indet constants_bs__c_ExpandedNodeId_indet
#define constants__c_LocalizedText_indet constants_bs__c_LocalizedText_indet
#define constants__c_NodeId_indet constants_bs__c_NodeId_indet
#define constants__c_Node_indet constants_bs__c_Node_indet
#define constants__c_Nonce_indet constants_bs__c_Nonce_indet
#define constants__c_QualifiedName_indet constants_bs__c_QualifiedName_indet
#define constants__c_Reference_indet constants_bs__c_Reference_indet
#define constants__c_SignatureData_indet constants_bs__c_SignatureData_indet
#define constants__c_Variant_indet constants_bs__c_Variant_indet
#define constants__c_byte_buffer_indet constants_bs__c_byte_buffer_indet
#define constants__c_channel_config_idx_indet constants_bs__c_channel_config_idx_indet
#define constants__c_channel_indet constants_bs__c_channel_indet
#define constants__c_endpoint_config_idx_indet constants_bs__c_endpoint_config_idx_indet
#define constants__c_msg_header_indet constants_bs__c_msg_header_indet
#define constants__c_msg_indet constants_bs__c_msg_indet
#define constants__c_no_application_context constants_bs__c_no_application_context
#define constants__c_request_context_indet constants_bs__c_request_context_indet
#define constants__c_request_handle_indet constants_bs__c_request_handle_indet
#define constants__c_session_indet constants_bs__c_session_indet
#define constants__c_session_token_indet constants_bs__c_session_token_indet
#define constants__c_user_indet constants_bs__c_user_indet
#define constants__k_n_Nodes constants_bs__k_n_Nodes
#define constants__k_n_read_resp_max (5000)
#define constants__k_n_WriteResponse_max (5000)
#define constants__k_n_BrowseResponse_max (5000)
#define constants__k_n_BrowseTarget_max (5000)
#define constants__c_ReadValue_indet (0)
#define constants__c_WriteValue_indet (0)
#define constants__c_BrowseValue_indet (0)
#define constants__c_BrowseResult_indet (0)

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define constants__get_Is_SubType constants_bs__get_Is_SubType
#define constants__get_card_t_channel constants_bs__get_card_t_channel
#define constants__get_card_t_session constants_bs__get_card_t_session
#define constants__get_cast_t_channel constants_bs__get_cast_t_channel
#define constants__get_cast_t_session constants_bs__get_cast_t_session
#define constants__getall_conv_ExpandedNodeId_NodeId constants_bs__getall_conv_ExpandedNodeId_NodeId
#define constants__is_t_channel constants_bs__is_t_channel
#define constants__is_t_channel_config_idx constants_bs__is_t_channel_config_idx
#define constants__is_t_endpoint_config_idx constants_bs__is_t_endpoint_config_idx

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void constants__get_Is_Dir_Forward_Compatible(const constants__t_BrowseDirection_i constants__p_dir,
                                                     const t_bool constants__p_IsForward,
                                                     t_bool* const constants__p_dir_compat);
extern void constants__get_cast_t_BrowseResult(const t_entier4 constants__p_ind,
                                               constants__t_BrowseResult_i* const constants__p_bri);
extern void constants__get_cast_t_BrowseValue(const t_entier4 constants__p_ind,
                                              constants__t_BrowseValue_i* const constants__p_bvi);
extern void constants__get_cast_t_WriteValue(const t_entier4 constants__ii,
                                             constants__t_WriteValue_i* const constants__wvi);
extern void constants__read_cast_t_ReadValue(const t_entier4 constants__ii,
                                             constants__t_ReadValue_i* const constants__rvi);

#endif
