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

/******************************************************************************

 File Name            : constants.h

 Date                 : 05/08/2022 15:45:54

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _constants_h
#define _constants_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   EXTENDS Clause
  -----------------*/
#include "constants_bs.h"

/*-----------------------------
   SETS Clause: deferred sets
  -----------------------------*/
#define constants__t_ApplicationDescription_i constants_bs__t_ApplicationDescription_i
#define constants__t_Argument_i constants_bs__t_Argument_i
#define constants__t_ArrayDimensions_i constants_bs__t_ArrayDimensions_i
#define constants__t_BrowseNodeClassMask_i constants_bs__t_BrowseNodeClassMask_i
#define constants__t_BrowsePath_i constants_bs__t_BrowsePath_i
#define constants__t_BrowseResultMask_i constants_bs__t_BrowseResultMask_i
#define constants__t_BrowseResultReferences_i constants_bs__t_BrowseResultReferences_i
#define constants__t_Byte constants_bs__t_Byte
#define constants__t_CallMethod_i constants_bs__t_CallMethod_i
#define constants__t_ContinuationPointId_i constants_bs__t_ContinuationPointId_i
#define constants__t_ContinuationPoint_i constants_bs__t_ContinuationPoint_i
#define constants__t_DataValue_i constants_bs__t_DataValue_i
#define constants__t_ExpandedNodeId_i constants_bs__t_ExpandedNodeId_i
#define constants__t_IndexRange_i constants_bs__t_IndexRange_i
#define constants__t_Int32 constants_bs__t_Int32
#define constants__t_LocaleIds_i constants_bs__t_LocaleIds_i
#define constants__t_LocalizedText_i constants_bs__t_LocalizedText_i
#define constants__t_MdnsDiscoveryConfig_i constants_bs__t_MdnsDiscoveryConfig_i
#define constants__t_NodeId_i constants_bs__t_NodeId_i
#define constants__t_Node_i constants_bs__t_Node_i
#define constants__t_Nonce_i constants_bs__t_Nonce_i
#define constants__t_QualifiedName_i constants_bs__t_QualifiedName_i
#define constants__t_RawStatusCode constants_bs__t_RawStatusCode
#define constants__t_Reference_i constants_bs__t_Reference_i
#define constants__t_RegisteredServer2Info_i constants_bs__t_RegisteredServer2Info_i
#define constants__t_RegisteredServer_i constants_bs__t_RegisteredServer_i
#define constants__t_RelativePathElt_i constants_bs__t_RelativePathElt_i
#define constants__t_RelativePath_i constants_bs__t_RelativePath_i
#define constants__t_ServerCapabilities constants_bs__t_ServerCapabilities
#define constants__t_ServerUri constants_bs__t_ServerUri
#define constants__t_ServerUris constants_bs__t_ServerUris
#define constants__t_SignatureData_i constants_bs__t_SignatureData_i
#define constants__t_Timestamp constants_bs__t_Timestamp
#define constants__t_Variant_i constants_bs__t_Variant_i
#define constants__t_WriteValuePointer_i constants_bs__t_WriteValuePointer_i
#define constants__t_access_level constants_bs__t_access_level
#define constants__t_application_context_i constants_bs__t_application_context_i
#define constants__t_byte_buffer_i constants_bs__t_byte_buffer_i
#define constants__t_channel_config_idx_i constants_bs__t_channel_config_idx_i
#define constants__t_channel_i constants_bs__t_channel_i
#define constants__t_client_handle_i constants_bs__t_client_handle_i
#define constants__t_client_request_handle_i constants_bs__t_client_request_handle_i
#define constants__t_endpoint_config_idx_i constants_bs__t_endpoint_config_idx_i
#define constants__t_monitoredItemId_i constants_bs__t_monitoredItemId_i
#define constants__t_monitoredItemPointer_i constants_bs__t_monitoredItemPointer_i
#define constants__t_monitoredItemQueueIterator_i constants_bs__t_monitoredItemQueueIterator_i
#define constants__t_monitoredItemQueue_i constants_bs__t_monitoredItemQueue_i
#define constants__t_msg_header_i constants_bs__t_msg_header_i
#define constants__t_msg_i constants_bs__t_msg_i
#define constants__t_notifRepublishQueueIterator_i constants_bs__t_notifRepublishQueueIterator_i
#define constants__t_notifRepublishQueue_i constants_bs__t_notifRepublishQueue_i
#define constants__t_notif_msg_i constants_bs__t_notif_msg_i
#define constants__t_notificationQueue_i constants_bs__t_notificationQueue_i
#define constants__t_opcua_duration_i constants_bs__t_opcua_duration_i
#define constants__t_publishReqQueue_i constants_bs__t_publishReqQueue_i
#define constants__t_request_context_i constants_bs__t_request_context_i
#define constants__t_server_request_handle_i constants_bs__t_server_request_handle_i
#define constants__t_session_application_context_i constants_bs__t_session_application_context_i
#define constants__t_session_i constants_bs__t_session_i
#define constants__t_session_token_i constants_bs__t_session_token_i
#define constants__t_sub_seq_num_i constants_bs__t_sub_seq_num_i
#define constants__t_subscription_i constants_bs__t_subscription_i
#define constants__t_timer_id_i constants_bs__t_timer_id_i
#define constants__t_timeref_i constants_bs__t_timeref_i
#define constants__t_user_i constants_bs__t_user_i
#define constants__t_user_token_i constants_bs__t_user_token_i

#define constants__t_ReadValue_i t_entier4
#define constants__t_WriteValue_i t_entier4
#define constants__t_BrowseValue_i t_entier4

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
   constants__e_aid_WriteMask,
   constants__e_aid_UserWriteMask,
   constants__e_aid_IsAbstract,
   constants__e_aid_Symmetric,
   constants__e_aid_InverseName,
   constants__e_aid_ContainsNoLoops,
   constants__e_aid_EventNotifier,
   constants__e_aid_Value,
   constants__e_aid_DataType,
   constants__e_aid_ValueRank,
   constants__e_aid_ArrayDimensions,
   constants__e_aid_AccessLevel,
   constants__e_aid_UserAccessLevel,
   constants__e_aid_MinimumSamplingInterval,
   constants__e_aid_Historizing,
   constants__e_aid_Executable,
   constants__e_aid_UserExecutable
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
   constants__e_secpol_B256S256,
   constants__e_secpol_Aes128Sha256RsaOaep,
   constants__e_secpol_Aes256Sha256RsaPss
} constants__t_SecurityPolicy;
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
   constants__c_monitoringMode_indet,
   constants__e_monitoringMode_disabled,
   constants__e_monitoringMode_sampling,
   constants__e_monitoringMode_reporting
} constants__t_monitoringMode_i;
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
   constants__c_operation_type_indet,
   constants__e_operation_type_read,
   constants__e_operation_type_write,
   constants__e_operation_type_executable
} constants__t_operation_type_i;
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
typedef enum {
   constants__c_subscriptionState_indet,
   constants__e_subscriptionState_normal,
   constants__e_subscriptionState_late,
   constants__e_subscriptionState_keepAlive
} constants__t_subscriptionState_i;
typedef enum {
   constants__c_userTokenType_indet,
   constants__e_userTokenType_anonymous,
   constants__e_userTokenType_userName,
   constants__e_userTokenType_x509,
   constants__e_userTokenType_issued
} constants__t_user_token_type_i;

/*--------------------------
   Added by the Translator
  --------------------------*/
#define constants__t_ApplicationDescription_i_max constants_bs__t_ApplicationDescription_i_max
#define constants__t_Argument_i_max constants_bs__t_Argument_i_max
#define constants__t_ArrayDimensions_i_max constants_bs__t_ArrayDimensions_i_max
#define constants__t_BrowseNodeClassMask_i_max constants_bs__t_BrowseNodeClassMask_i_max
#define constants__t_BrowsePath_i_max constants_bs__t_BrowsePath_i_max
#define constants__t_BrowseResultMask_i_max constants_bs__t_BrowseResultMask_i_max
#define constants__t_BrowseResultReferences_i_max constants_bs__t_BrowseResultReferences_i_max
#define constants__t_Byte_max constants_bs__t_Byte_max
#define constants__t_CallMethod_i_max constants_bs__t_CallMethod_i_max
#define constants__t_ContinuationPointId_i_max constants_bs__t_ContinuationPointId_i_max
#define constants__t_ContinuationPoint_i_max constants_bs__t_ContinuationPoint_i_max
#define constants__t_DataValue_i_max constants_bs__t_DataValue_i_max
#define constants__t_ExpandedNodeId_i_max constants_bs__t_ExpandedNodeId_i_max
#define constants__t_IndexRange_i_max constants_bs__t_IndexRange_i_max
#define constants__t_Int32_max constants_bs__t_Int32_max
#define constants__t_LocaleIds_i_max constants_bs__t_LocaleIds_i_max
#define constants__t_LocalizedText_i_max constants_bs__t_LocalizedText_i_max
#define constants__t_MdnsDiscoveryConfig_i_max constants_bs__t_MdnsDiscoveryConfig_i_max
#define constants__t_NodeId_i_max constants_bs__t_NodeId_i_max
#define constants__t_Node_i_max constants_bs__t_Node_i_max
#define constants__t_Nonce_i_max constants_bs__t_Nonce_i_max
#define constants__t_QualifiedName_i_max constants_bs__t_QualifiedName_i_max
#define constants__t_RawStatusCode_max constants_bs__t_RawStatusCode_max
#define constants__t_Reference_i_max constants_bs__t_Reference_i_max
#define constants__t_RegisteredServer2Info_i_max constants_bs__t_RegisteredServer2Info_i_max
#define constants__t_RegisteredServer_i_max constants_bs__t_RegisteredServer_i_max
#define constants__t_RelativePathElt_i_max constants_bs__t_RelativePathElt_i_max
#define constants__t_RelativePath_i_max constants_bs__t_RelativePath_i_max
#define constants__t_ServerCapabilities_max constants_bs__t_ServerCapabilities_max
#define constants__t_ServerUri_max constants_bs__t_ServerUri_max
#define constants__t_ServerUris_max constants_bs__t_ServerUris_max
#define constants__t_SignatureData_i_max constants_bs__t_SignatureData_i_max
#define constants__t_Timestamp_max constants_bs__t_Timestamp_max
#define constants__t_Variant_i_max constants_bs__t_Variant_i_max
#define constants__t_WriteValuePointer_i_max constants_bs__t_WriteValuePointer_i_max
#define constants__t_access_level_max constants_bs__t_access_level_max
#define constants__t_application_context_i_max constants_bs__t_application_context_i_max
#define constants__t_byte_buffer_i_max constants_bs__t_byte_buffer_i_max
#define constants__t_channel_config_idx_i_max constants_bs__t_channel_config_idx_i_max
#define constants__t_channel_i_max constants_bs__t_channel_i_max
#define constants__t_client_handle_i_max constants_bs__t_client_handle_i_max
#define constants__t_client_request_handle_i_max constants_bs__t_client_request_handle_i_max
#define constants__t_endpoint_config_idx_i_max constants_bs__t_endpoint_config_idx_i_max
#define constants__t_monitoredItemId_i_max constants_bs__t_monitoredItemId_i_max
#define constants__t_monitoredItemPointer_i_max constants_bs__t_monitoredItemPointer_i_max
#define constants__t_monitoredItemQueueIterator_i_max constants_bs__t_monitoredItemQueueIterator_i_max
#define constants__t_monitoredItemQueue_i_max constants_bs__t_monitoredItemQueue_i_max
#define constants__t_msg_header_i_max constants_bs__t_msg_header_i_max
#define constants__t_msg_i_max constants_bs__t_msg_i_max
#define constants__t_notifRepublishQueueIterator_i_max constants_bs__t_notifRepublishQueueIterator_i_max
#define constants__t_notifRepublishQueue_i_max constants_bs__t_notifRepublishQueue_i_max
#define constants__t_notif_msg_i_max constants_bs__t_notif_msg_i_max
#define constants__t_notificationQueue_i_max constants_bs__t_notificationQueue_i_max
#define constants__t_opcua_duration_i_max constants_bs__t_opcua_duration_i_max
#define constants__t_publishReqQueue_i_max constants_bs__t_publishReqQueue_i_max
#define constants__t_request_context_i_max constants_bs__t_request_context_i_max
#define constants__t_server_request_handle_i_max constants_bs__t_server_request_handle_i_max
#define constants__t_session_application_context_i_max constants_bs__t_session_application_context_i_max
#define constants__t_session_i_max constants_bs__t_session_i_max
#define constants__t_session_token_i_max constants_bs__t_session_token_i_max
#define constants__t_sub_seq_num_i_max constants_bs__t_sub_seq_num_i_max
#define constants__t_subscription_i_max constants_bs__t_subscription_i_max
#define constants__t_timer_id_i_max constants_bs__t_timer_id_i_max
#define constants__t_timeref_i_max constants_bs__t_timeref_i_max
#define constants__t_user_i_max constants_bs__t_user_i_max
#define constants__t_user_token_i_max constants_bs__t_user_token_i_max
#define constants__t_ReadValue_i_max constants_bs__k_n_read_resp_max
#define constants__t_WriteValue_i_max constants_bs__k_n_WriteResponse_max
#define constants__t_BrowseValue_i_max constants_bs__k_n_BrowseResponse_max
#define constants__t_BrowsePathResPerElt_i_max constants_bs__k_n_BrowsePathResPerElt_max

/*------------------------------------------------
   CONCRETE_CONSTANTS Clause: scalars and arrays
  ------------------------------------------------*/
#define constants__c_ApplicationDescription_indet constants_bs__c_ApplicationDescription_indet
#define constants__c_Argument_indet constants_bs__c_Argument_indet
#define constants__c_ArrayDimensions_indet constants_bs__c_ArrayDimensions_indet
#define constants__c_BrowseNodeClassMask_indet constants_bs__c_BrowseNodeClassMask_indet
#define constants__c_BrowseResultMask_all constants_bs__c_BrowseResultMask_all
#define constants__c_BrowseResultMask_indet constants_bs__c_BrowseResultMask_indet
#define constants__c_BrowseResultReferences_indet constants_bs__c_BrowseResultReferences_indet
#define constants__c_ByteString_Type_NodeId constants_bs__c_ByteString_Type_NodeId
#define constants__c_Byte_Type_NodeId constants_bs__c_Byte_Type_NodeId
#define constants__c_CallMethod_indet constants_bs__c_CallMethod_indet
#define constants__c_ContinuationPointId_indet constants_bs__c_ContinuationPointId_indet
#define constants__c_ContinuationPoint_indet constants_bs__c_ContinuationPoint_indet
#define constants__c_DataValue_indet constants_bs__c_DataValue_indet
#define constants__c_ExpandedNodeId_indet constants_bs__c_ExpandedNodeId_indet
#define constants__c_IndexRange_indet constants_bs__c_IndexRange_indet
#define constants__c_LocaleIds_empty constants_bs__c_LocaleIds_empty
#define constants__c_LocaleIds_indet constants_bs__c_LocaleIds_indet
#define constants__c_LocalizedText_indet constants_bs__c_LocalizedText_indet
#define constants__c_MdnsDiscoveryConfig_indet constants_bs__c_MdnsDiscoveryConfig_indet
#define constants__c_NodeId_indet constants_bs__c_NodeId_indet
#define constants__c_Node_indet constants_bs__c_Node_indet
#define constants__c_Nonce_indet constants_bs__c_Nonce_indet
#define constants__c_Null_Type_NodeId constants_bs__c_Null_Type_NodeId
#define constants__c_QualifiedName_indet constants_bs__c_QualifiedName_indet
#define constants__c_Reference_indet constants_bs__c_Reference_indet
#define constants__c_RegisteredServer2Info_indet constants_bs__c_RegisteredServer2Info_indet
#define constants__c_RegisteredServer_indet constants_bs__c_RegisteredServer_indet
#define constants__c_RelativePathElt_indet constants_bs__c_RelativePathElt_indet
#define constants__c_RelativePath_indet constants_bs__c_RelativePath_indet
#define constants__c_ServerUri_empty constants_bs__c_ServerUri_empty
#define constants__c_Server_Nano_Extended constants_bs__c_Server_Nano_Extended
#define constants__c_SignatureData_indet constants_bs__c_SignatureData_indet
#define constants__c_Timestamp_null constants_bs__c_Timestamp_null
#define constants__c_Variant_indet constants_bs__c_Variant_indet
#define constants__c_WriteValuePointer_indet constants_bs__c_WriteValuePointer_indet
#define constants__c_browsePath_indet constants_bs__c_browsePath_indet
#define constants__c_byte_buffer_indet constants_bs__c_byte_buffer_indet
#define constants__c_channel_config_idx_indet constants_bs__c_channel_config_idx_indet
#define constants__c_channel_indet constants_bs__c_channel_indet
#define constants__c_client_request_handle_indet constants_bs__c_client_request_handle_indet
#define constants__c_endpoint_config_idx_indet constants_bs__c_endpoint_config_idx_indet
#define constants__c_max_channels_connected constants_bs__c_max_channels_connected
#define constants__c_monitoredItemId_indet constants_bs__c_monitoredItemId_indet
#define constants__c_monitoredItemPointer_indet constants_bs__c_monitoredItemPointer_indet
#define constants__c_monitoredItemQueueIterator_indet constants_bs__c_monitoredItemQueueIterator_indet
#define constants__c_monitoredItemQueue_indet constants_bs__c_monitoredItemQueue_indet
#define constants__c_msg_header_indet constants_bs__c_msg_header_indet
#define constants__c_msg_indet constants_bs__c_msg_indet
#define constants__c_no_application_context constants_bs__c_no_application_context
#define constants__c_notifRepublishQueueIterator_indet constants_bs__c_notifRepublishQueueIterator_indet
#define constants__c_notifRepublishQueue_indet constants_bs__c_notifRepublishQueue_indet
#define constants__c_notif_msg_indet constants_bs__c_notif_msg_indet
#define constants__c_notificationQueue_indet constants_bs__c_notificationQueue_indet
#define constants__c_opcua_duration_indet constants_bs__c_opcua_duration_indet
#define constants__c_opcua_duration_zero constants_bs__c_opcua_duration_zero
#define constants__c_publishReqQueue_indet constants_bs__c_publishReqQueue_indet
#define constants__c_request_context_indet constants_bs__c_request_context_indet
#define constants__c_server_request_handle_any constants_bs__c_server_request_handle_any
#define constants__c_session_indet constants_bs__c_session_indet
#define constants__c_session_token_indet constants_bs__c_session_token_indet
#define constants__c_sub_seq_num_indet constants_bs__c_sub_seq_num_indet
#define constants__c_sub_seq_num_init constants_bs__c_sub_seq_num_init
#define constants__c_subscription_indet constants_bs__c_subscription_indet
#define constants__c_timer_id_indet constants_bs__c_timer_id_indet
#define constants__c_timeref_indet constants_bs__c_timeref_indet
#define constants__c_user_indet constants_bs__c_user_indet
#define constants__c_user_token_indet constants_bs__c_user_token_indet
#define constants__k_n_BrowsePathResPerElt_max constants_bs__k_n_BrowsePathResPerElt_max
#define constants__k_n_BrowsePathResPerPath_max constants_bs__k_n_BrowsePathResPerPath_max
#define constants__k_n_BrowseResponse_max constants_bs__k_n_BrowseResponse_max
#define constants__k_n_BrowseTarget_max constants_bs__k_n_BrowseTarget_max
#define constants__k_n_IndexRange_max constants_bs__k_n_IndexRange_max
#define constants__k_n_WriteResponse_max constants_bs__k_n_WriteResponse_max
#define constants__k_n_genericOperationPerReq_max constants_bs__k_n_genericOperationPerReq_max
#define constants__k_n_monitoredItemNotif_max constants_bs__k_n_monitoredItemNotif_max
#define constants__k_n_monitoredItem_max constants_bs__k_n_monitoredItem_max
#define constants__k_n_publishRequestPerSub_max constants_bs__k_n_publishRequestPerSub_max
#define constants__k_n_read_resp_max constants_bs__k_n_read_resp_max
#define constants__k_n_registerNodes_max constants_bs__k_n_registerNodes_max
#define constants__k_n_republishNotifPerSub_max constants_bs__k_n_republishNotifPerSub_max
#define constants__k_n_unregisterNodes_max constants_bs__k_n_unregisterNodes_max
#define constants__c_ReadValue_indet (0)
#define constants__c_WriteValue_indet (0)
#define constants__c_BrowseValue_indet (0)
#define constants__c_ValueRank_OneDimension (1)
#define constants__c_ValueRank_OneOrMoreDimensions (0)
#define constants__c_ValueRank_Scalar (-1)
#define constants__c_ValueRank_Any (-2)
#define constants__c_ValueRank_ScalarOrOneDimension (-3)

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define constants__free_ExpandedNodeId constants_bs__free_ExpandedNodeId
#define constants__free_LocaleIds constants_bs__free_LocaleIds
#define constants__get_CurrentTimestamp constants_bs__get_CurrentTimestamp
#define constants__get_SupportedLocales constants_bs__get_SupportedLocales
#define constants__get_card_t_channel constants_bs__get_card_t_channel
#define constants__get_card_t_session constants_bs__get_card_t_session
#define constants__get_card_t_subscription constants_bs__get_card_t_subscription
#define constants__get_cast_t_BrowsePath constants_bs__get_cast_t_BrowsePath
#define constants__get_cast_t_CallMethod constants_bs__get_cast_t_CallMethod
#define constants__get_cast_t_channel constants_bs__get_cast_t_channel
#define constants__get_cast_t_session constants_bs__get_cast_t_session
#define constants__get_cast_t_subscription constants_bs__get_cast_t_subscription
#define constants__get_copy_ExpandedNodeId constants_bs__get_copy_ExpandedNodeId
#define constants__getall_conv_ExpandedNodeId_NodeId constants_bs__getall_conv_ExpandedNodeId_NodeId
#define constants__is_QualifiedNames_Empty constants_bs__is_QualifiedNames_Empty
#define constants__is_QualifiedNames_Equal constants_bs__is_QualifiedNames_Equal
#define constants__is_t_access_level_currentRead constants_bs__is_t_access_level_currentRead
#define constants__is_t_access_level_currentWrite constants_bs__is_t_access_level_currentWrite
#define constants__is_t_access_level_statusWrite constants_bs__is_t_access_level_statusWrite
#define constants__is_t_access_level_timestampWrite constants_bs__is_t_access_level_timestampWrite
#define constants__is_t_channel constants_bs__is_t_channel
#define constants__is_t_channel_config_idx constants_bs__is_t_channel_config_idx
#define constants__is_t_endpoint_config_idx constants_bs__is_t_endpoint_config_idx

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void constants__get_Is_Dir_Forward_Compatible(
   const constants__t_BrowseDirection_i constants__p_dir,
   const t_bool constants__p_IsForward,
   t_bool * const constants__p_dir_compat);
extern void constants__get_cast_t_BrowseValue(
   const t_entier4 constants__p_ind,
   constants__t_BrowseValue_i * const constants__p_bvi);
extern void constants__get_cast_t_WriteValue(
   const t_entier4 constants__ii,
   constants__t_WriteValue_i * const constants__wvi);
extern void constants__read_cast_t_ReadValue(
   const t_entier4 constants__ii,
   constants__t_ReadValue_i * const constants__rvi);

#endif
