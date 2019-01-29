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
    constants
SETS
    /* The nodes of the Address Space \/ {indet} */
    t_Node_i;
    /* The node IDs. There might not be a NODE behind a node ID. */
    t_NodeId_i; /* (SOPC_NodeId*) */

    /* Expanded Nodes Id : nodes ID local the the address space server or from distant address spaces */
    t_ExpandedNodeId_i; /* (SOPC_ExpandedNodeId*) */

    /* Address space references */
    t_Reference_i;

    /* The node classes */
    t_NodeClass_i = {c_NodeClass_indet,
                     e_ncl_Object, e_ncl_Variable, e_ncl_Method,
                     e_ncl_ObjectType, e_ncl_VariableType, e_ncl_ReferenceType, e_ncl_DataType,
                     e_ncl_View};
    t_Variant_i; /* (SOPC_Variant*) */
    t_DataValue_i; /* (SOPC_DataValue*) */
    t_AttributeId_i = {c_AttributeId_indet,
                       e_aid_NodeId,
                       e_aid_NodeClass,
                       e_aid_BrowseName,
                       e_aid_DisplayName,
                       e_aid_Description,
                       e_aid_WriteMask,
                       e_aid_UserWriteMask,
                       e_aid_IsAbstract,
                       e_aid_Symmetric,
                       e_aid_InverseName,
                       e_aid_ContainsNoLoop,
                       e_aid_EventNotifier,
                       e_aid_Value,
                       e_aid_DataType,
                       e_aid_ValueRank,
                       e_aid_ArrayDimensions,
                       e_aid_AccessLevel,
                       e_aid_UserAccessLevel,
                       e_aid_MinimumSamplingInterval,
                       e_aid_Historizing,
                       e_aid_Executable,
                       e_aid_UserExecutable};

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

    /* INDEXES */

    /* A ReadValue of a ReadRequest. The size of that will define the nb_max_read_value. */
    t_ReadValue_i;

    /* A WriteValue of a WriteRequest. */
    t_WriteValue_i;
    t_WriteValuePointer_i; /* (OpcUa_WriteValue*) */

    /* A BrowseValue of a BrowseRequest. */
    t_BrowseValue_i;
    t_BrowseResult_i;


    t_byte_buffer_i; /* (SOPC_Buffer*) */
    t_LocalizedText_i; /* (SOPC_LocalizedText*) */
    t_QualifiedName_i; /* (SOPC_QualifiedName*) */
    t_Timestamp; /* SOPC_Internal_Timestamp */
    t_Int32; /* int32_t */
    t_RawStatusCode; /* uint32_t */

    t_access_level; /* SOPC_Byte */

    t_session_i;
    t_session_token_i; /* (SOPC_NodeId*) */
    t_user_i; /* Server-side user */
    t_user_token_i; /* Raw OPC UA user token */
    t_user_token_type_i = {c_userTokenType_indet,
                           e_userTokenType_anonymous,
                           e_userTokenType_userName,
                           e_userTokenType_x509,
                           e_userTokenType_issued
                          };
    t_Nonce_i; /* (SOPC_ByteString*) */
    t_SignatureData_i; /* (OpcUa_SignatureData*) */
    t_msg_header_i; /* (OpcUa_RequestHeader*) OR (OpcUa_ResponseHeader*) */
    t_msg_i;        /* (OpcUa_<message>*) */
    t_request_context_i; /* uint32_t */
    t_client_request_handle_i; /* uint32_t */
    t_server_request_handle_i; /* uint32_t */
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


    /* SUBSCRIPTION TYPES */
    t_subscription_i;

    t_subscriptionState_i = {
        c_subscriptionState_indet, /* includes e_subscription_closed */
        /* e_subscriptionState_creating, => not necesssary */
        e_subscriptionState_normal,
        e_subscriptionState_late,
        e_subscriptionState_keepAlive
    };

    /* Subscription (creation) */
    t_notif_msg_i;         /* (OpcUa_NotificationMessage*) */
    t_sub_seq_num_i;       /* uint32_t */
    t_timeref_i;           /* SOPC_TimeReference */
    t_opcua_duration_i;    /* double */
    t_notificationQueue_i; /* (SOPC_SLinkedList*) */
    t_publishReqQueue_i;   /* (SOPC_SLinkedList*) */
    t_notifRepublishQueue_i; /* (SOPC_SLinkedList*) */
    t_notifRepublishQueueIterator_i; /* (SOPC_SLinkedListIterator*) */
    t_monitoredItemQueue_i;  /* (SOPC_SLinkedList*) */
    t_monitoredItemQueueIterator_i; /* (SOPC_SLinkedListIterator*) */

    /* Subscription (monitored item) */
    t_monitoringMode_i = {
        c_monitoringMode_indet,
        e_monitoringMode_disabled,
        e_monitoringMode_sampling,
        e_monitoringMode_reporting
    };

    t_monitoredItemPointer_i;
    t_monitoredItemId_i;

    t_client_handle_i; /* uint32_t */

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
    };

    /* Context provided by application on request and to provide with notifications */
    t_application_context_i; /* uintptr_t */
    t_timer_id_i; /* uint32_t */
    t_IndexRange_i;

    /* User authorization operation type, see SOPC_UserAuthorization_Manager */
    t_operation_type_i = {c_operation_type_indet,
                          e_operation_type_read,
                          e_operation_type_write}

CONCRETE_CONSTANTS
    /*AddS,*/
    k_n_read_resp_max,
    k_n_WriteResponse_max,
    k_n_BrowseResponse_max,
    k_n_BrowseTarget_max,
    k_n_monitoredItem_max,
    k_n_monitoredItemNotif_max,
    k_n_publishRequestPerSub_max,
    k_n_republishNotifPerSub_max,
    k_n_genericOperationPerReq_max,
    k_n_IndexRange_max,
    k_n_registerNodes_max,
    k_n_unregisterNodes_max,

    c_Null_Type_NodeId,
    c_ByteString_Type_NodeId,
    c_Byte_Type_NodeId,

    c_Variant_indet,
    c_DataValue_indet,
    c_NodeId_indet,
    c_ExpandedNodeId_indet,
    c_Reference_indet,
    c_Node_indet,
    c_ReadValue_indet,
    c_WriteValue_indet,
    c_WriteValuePointer_indet,
    c_BrowseValue_indet,
    c_BrowseResult_indet,
    c_byte_buffer_indet,
    c_LocalizedText_indet,
    c_QualifiedName_indet,

    c_endpoint_config_idx_indet,
    c_user_indet,
    c_user_token_indet,
    c_session_indet,
    c_session_token_indet,
    c_Nonce_indet,
    c_SignatureData_indet,
    c_subscription_indet,
    c_notif_msg_indet,
    c_sub_seq_num_indet,
    c_sub_seq_num_init,
    c_timeref_indet,
    c_opcua_duration_indet,
    c_opcua_duration_zero,
    c_notificationQueue_indet,
    c_publishReqQueue_indet,
    c_notifRepublishQueue_indet,
    c_notifRepublishQueueIterator_indet,
    c_monitoredItemQueue_indet,
    c_monitoredItemQueueIterator_indet,
    c_monitoredItemPointer_indet,
    c_monitoredItemId_indet,
    c_msg_header_indet,
    c_msg_indet,
    c_request_context_indet,
    c_channel_config_idx_indet,
    c_channel_indet,
    c_client_request_handle_indet,
    c_server_request_handle_any,
    c_no_application_context,
    c_timer_id_indet,
    c_IndexRange_indet,

    t_counter,
    t_ValueRank,
    t_ConcreteValueRank

ABSTRACT_CONSTANTS
    conv_NodeId_Variant,
    conv_NodeClass_Variant,

    conv_AccessLevel_CurrentWrite,
    conv_AccessLevel_CurrentRead,
    conv_AccessLevel_StatusWrite,
    conv_AccessLevel_TimestampWrite,

    conv_ExpandedNodeId_NodeId,

    t_BrowseDirection,
    t_TimestampsToReturn,
    t_AttributeId,
    t_NodeClass,
    t_Variant,
    t_DataValue,
    t_NodeId,
    t_ExpandedNodeId,
    t_Reference,
    t_Node,
    t_ReadValue,
    t_WriteValue,
    t_WriteValuePointer,
    t_BrowseValue,
    t_BrowseResult,
    t_RefIndex,
    t_byte_buffer,
    t_LocalizedText,
    t_QualifiedName,

    cast_t_session,
    cast_t_channel,
    cast_t_subscription,
    cast_t_ReadValue,
    cast_t_WriteValue,
    cast_t_BrowseValue,
    cast_t_BrowseResult,

    t_endpoint_config_idx,
    t_session,
    t_session_token,
    t_user,
    t_user_token,
    t_Nonce,
    t_SignatureData,
    t_subscription,
    t_subscriptionState,
    t_notif_msg,
    t_sub_seq_num,
    t_timeref,
    t_notificationQueue,
    t_publishReqQueue,
    t_notifRepublishQueue,
    t_notifRepublishQueueIterator,
    t_monitoredItemQueue,
    t_monitoredItemQueueIterator,
    t_monitoringMode,
    t_monitoredItemReqIndex,
    t_monitoredItemPointer,
    t_monitoredItemId,
    t_monitoredItemNotifIndex,
    t_buffer_in_state,
    t_msg_header,
    t_msg,
    t_request_context,
    t_msg_type,
    t_msg_header_type,
    t_msg_service_class,
    t_channel_config_idx,
    t_channel,
    t_client_request_handle,
    t_timer_id,
    t_IndexRange,
    t_operation_type,
    t_registerNodesReqIndex,
    t_unregisterNodesReqIndex,

    s_session_treatment_reqs,
    s_session_treatment_resps,
    s_service_on_session_reqs,
    s_service_on_session_resps,
    s_discovery_service_reqs,
    s_discovery_service_resps,
    s_service_reqs,
    s_service_resps,

    Is_Dir_Forward_Compatible

PROPERTIES

    /*AddS <: t_Node &*/
    t_Variant <: t_Variant_i &
    c_Variant_indet : t_Variant_i &
    c_Variant_indet /: t_Variant &

    t_DataValue <: t_DataValue_i &
    c_DataValue_indet : t_DataValue_i &
    c_DataValue_indet /: t_DataValue &

    t_Node <: t_Node_i &
    c_Node_indet : t_Node_i &
    c_Node_indet /: t_Node &

    c_NodeId_indet : t_NodeId_i &
    t_NodeId = t_NodeId_i - {c_NodeId_indet} &

    c_Null_Type_NodeId : t_NodeId_i &
    c_Null_Type_NodeId : t_NodeId &

    c_ByteString_Type_NodeId : t_NodeId_i &
    c_ByteString_Type_NodeId : t_NodeId &

    c_Byte_Type_NodeId : t_NodeId_i &
    c_Byte_Type_NodeId : t_NodeId &

    t_ExpandedNodeId <: t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet : t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet /: t_ExpandedNodeId &

    t_Reference <: t_Reference_i &
    c_Reference_indet : t_Reference_i &
    c_Reference_indet /: t_Reference &

    t_BrowseDirection = {e_bd_forward,
                         e_bd_inverse,
                         e_bd_both} &

    t_TimestampsToReturn = {e_ttr_source,
                            e_ttr_server,
                            e_ttr_both,
                            e_ttr_neither} &

    t_AttributeId = t_AttributeId_i - {c_AttributeId_indet} &

    t_NodeClass = t_NodeClass_i - {c_NodeClass_indet} &

    conv_NodeId_Variant : t_NodeId --> t_Variant &
    conv_NodeClass_Variant : t_NodeClass --> t_Variant &

    conv_AccessLevel_CurrentWrite : t_access_level --> BOOL &
    conv_AccessLevel_CurrentRead : t_access_level --> BOOL &
    conv_AccessLevel_StatusWrite : t_access_level --> BOOL &
    conv_AccessLevel_TimestampWrite : t_access_level --> BOOL &

    conv_ExpandedNodeId_NodeId : t_ExpandedNodeId +-> t_NodeId &

    k_n_read_resp_max : NAT &
    k_n_WriteResponse_max : NAT &
    k_n_BrowseResponse_max : NAT &
    k_n_BrowseTarget_max : NAT &
    k_n_monitoredItem_max : NAT &
    k_n_monitoredItemNotif_max : NAT &
    k_n_publishRequestPerSub_max : NAT &
    k_n_publishRequestPerSub_max > 0 &
    k_n_republishNotifPerSub_max : NAT &
    k_n_republishNotifPerSub_max > 0 &
    k_n_genericOperationPerReq_max : NAT &
    k_n_IndexRange_max : NAT &
    k_n_registerNodes_max : NAT &
    k_n_unregisterNodes_max : NAT &

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

    t_WriteValuePointer <: t_WriteValuePointer_i &
    c_WriteValuePointer_indet : t_WriteValuePointer_i &
    c_WriteValuePointer_indet /: t_WriteValuePointer &

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

    t_monitoredItemReqIndex <: NAT &
    t_monitoredItemNotifIndex <: NAT &

    t_registerNodesReqIndex <: NAT &
    t_unregisterNodesReqIndex <: NAT &

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


    t_user_token <: t_user_token_i &
    c_user_token_indet : t_user_token_i &
    c_user_token_indet /: t_user_token &

    t_subscription <: t_subscription_i &
    c_subscription_indet : t_subscription_i &
    c_subscription_indet /: t_subscription &
    cast_t_subscription : NAT +-> t_subscription_i &
    cast_t_subscription : perm(t_subscription) &

    t_subscriptionState = t_subscriptionState_i - {c_subscriptionState_indet} &

    t_counter = NAT &

    t_ValueRank = INT &
    t_ConcreteValueRank <: INT &
    t_ConcreteValueRank = NAT1 \/ {-1} &

    t_notif_msg <: t_notif_msg_i &
    c_notif_msg_indet : t_notif_msg_i &
    c_notif_msg_indet /: t_notif_msg &

    t_sub_seq_num <: t_sub_seq_num_i &
    c_sub_seq_num_indet : t_sub_seq_num_i &
    c_sub_seq_num_indet /: t_sub_seq_num &

    c_sub_seq_num_init : t_sub_seq_num_i &
    c_sub_seq_num_init : t_sub_seq_num &

    t_timeref <: t_timeref_i &
    c_timeref_indet : t_timeref_i &
    c_timeref_indet /: t_timeref &

    c_opcua_duration_indet : t_opcua_duration_i &
    c_opcua_duration_zero : t_opcua_duration_i &

    t_notificationQueue <: t_notificationQueue_i &
    c_notificationQueue_indet : t_notificationQueue_i &
    c_notificationQueue_indet /: t_notificationQueue &

    t_publishReqQueue <: t_publishReqQueue_i &
    c_publishReqQueue_indet : t_publishReqQueue_i &
    c_publishReqQueue_indet /: t_publishReqQueue &

    t_notifRepublishQueue <: t_notifRepublishQueue_i &
    c_notifRepublishQueue_indet : t_notifRepublishQueue_i &
    c_notifRepublishQueue_indet /: t_notifRepublishQueue &

    t_notifRepublishQueueIterator <: t_notifRepublishQueueIterator_i &
    c_notifRepublishQueueIterator_indet : t_notifRepublishQueueIterator_i &
    c_notifRepublishQueueIterator_indet /: t_notifRepublishQueueIterator &

    t_monitoredItemQueue <: t_monitoredItemQueue_i &
    c_monitoredItemQueue_indet : t_monitoredItemQueue_i &
    c_monitoredItemQueue_indet /: t_monitoredItemQueue &

    t_monitoredItemQueueIterator <: t_monitoredItemQueueIterator_i &
    c_monitoredItemQueueIterator_indet : t_monitoredItemQueueIterator_i &
    c_monitoredItemQueueIterator_indet /: t_monitoredItemQueueIterator &

    t_monitoringMode = t_monitoringMode_i - {c_monitoringMode_indet} &

    t_monitoredItemPointer <: t_monitoredItemPointer_i &
    c_monitoredItemPointer_indet : t_monitoredItemPointer_i &
    c_monitoredItemPointer_indet /: t_monitoredItemPointer &

    t_monitoredItemId <: t_monitoredItemId_i &
    c_monitoredItemId_indet : t_monitoredItemId_i &
    c_monitoredItemId_indet /: t_monitoredItemId &

    t_msg_header <: t_msg_header_i &
    c_msg_header_indet : t_msg_header_i &
    c_msg_header_indet /: t_msg_header &

    t_msg <: t_msg_i &
    c_msg_indet : t_msg_i &
    c_msg_indet /: t_msg &

    t_request_context <: t_request_context_i &
    c_request_context_indet : t_request_context_i &
    c_request_context_indet /: t_request_context &

    t_client_request_handle <: t_client_request_handle_i &
    c_client_request_handle_indet : t_client_request_handle_i &
    c_client_request_handle_indet /: t_client_request_handle &

    c_server_request_handle_any : t_server_request_handle_i &

    c_no_application_context : t_application_context_i &

    t_channel_config_idx <: t_channel_config_idx_i &
    c_channel_config_idx_indet : t_channel_config_idx_i &
    c_channel_config_idx_indet /: t_channel_config_idx &

    t_timer_id <: t_timer_id_i &
    c_timer_id_indet : t_timer_id_i &
    c_timer_id_indet /: t_timer_id &

    t_IndexRange <: t_IndexRange_i &
    c_IndexRange_indet : t_IndexRange_i &
    c_IndexRange_indet /: t_IndexRange &

    t_buffer_in_state = { e_buffer_in_msg_not_read,
        e_buffer_in_msg_type_read,
        e_buffer_in_msg_header_read,
        e_buffer_in_msg_read } &

    t_operation_type = {e_operation_type_read,
                        e_operation_type_write} &

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
        e_msg_monitored_items_set_monitoring_mode_resp,
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

    t_msg_type <: t_msg_type_i &
    t_msg_type = s_service_reqs \/ s_service_resps &

    t_msg_header_type <: t_msg_header_type_i &
    c_msg_header_type_indet : t_msg_header_type_i &
    c_msg_header_type_indet /: t_msg_header_type &

    t_msg_service_class <: t_msg_service_class_i &
    c_msg_service_class_indet : t_msg_service_class_i &
    c_msg_service_class_indet /: t_msg_service_class &
    t_msg_service_class = t_msg_service_class_i - {c_msg_service_class_indet} &

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

    p_nid <-- getall_conv_ExpandedNodeId_NodeId(p_expnid) =
    PRE
        p_expnid : t_ExpandedNodeId_i &
        p_expnid : t_ExpandedNodeId
    THEN
        p_nid :(p_nid : t_NodeId_i &
                p_nid = conv_ExpandedNodeId_NodeId(p_expnid))
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

    p_card_subscription <-- get_card_t_subscription =
    BEGIN
        p_card_subscription := card(t_subscription)
    END
    ;

    p_subscription <-- get_cast_t_subscription(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_subscription)
    THEN
        p_subscription := cast_t_subscription(p_ind)
    END
    ;

    bres <-- is_t_access_level_currentWrite(p_access_lvl) =
    PRE
        p_access_lvl : t_access_level
    THEN
        bres := conv_AccessLevel_CurrentWrite(p_access_lvl)
    END
    ;

    bres <-- is_t_access_level_currentRead(p_access_lvl) =
    PRE
        p_access_lvl : t_access_level
    THEN
        bres := conv_AccessLevel_CurrentRead(p_access_lvl)
    END
    ;

    bres <-- is_t_access_level_statusWrite(p_access_lvl) =
    PRE
        p_access_lvl : t_access_level
    THEN
        bres := conv_AccessLevel_StatusWrite(p_access_lvl)
    END
    ;

    bres <-- is_t_access_level_timestampWrite(p_access_lvl) =
    PRE
        p_access_lvl : t_access_level
    THEN
        bres := conv_AccessLevel_TimestampWrite(p_access_lvl)
    END

END
