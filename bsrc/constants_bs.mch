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
    constants_bs

SETS
    t_Node_i;
    t_NodeId_i;
    t_Variant_i;
    t_DataValue_i;
    t_byte_buffer_i;
    t_Timestamp;
    t_Int32;
    t_Byte;
    t_RawStatusCode;
    t_access_level;
    t_access_level_ex;

    t_WriteValuePointer_i;
    t_BrowseResultReferences_i;
    t_BrowseNodeClassMask_i;
    t_BrowseResultMask_i;

    t_channel_i;
    t_session_i;
    t_BrowsePath_i;
    t_RelativePath_i;
    t_RelativePathElt_i;
    t_CallMethod_i;
    t_CallMethodPointer_i;
    t_Argument_i;
    t_Identity_i;
    t_Criteria_i;
    t_ArgumentsPointer_i;
    t_NodeAttributes_i;
    t_session_token_i;
    t_user_i;
    t_user_token_i;
    t_Nonce_i;
    t_SignatureData_i;
    t_msg_header_i;
    t_msg_i;
    t_request_context_i;
    t_client_request_handle_i;
    t_server_request_handle_i;
    t_channel_config_idx_i;
    t_endpoint_config_idx_i;
    t_reverse_endpoint_config_idx_i;
    t_ExpandedNodeId_i;
    t_Reference_i;
    t_LocalizedText_i;
    t_QualifiedName_i;
    t_Event_i;
    t_session_application_context_i;
    t_application_context_i;
    t_subscription_i;
    t_notif_msg_i;
    t_sub_seq_num_i;
    t_timeref_i;
    t_counter_i;
    t_opcua_duration_i;
    t_notificationQueue_i;
    t_sessionRoles_i;
    t_RolePermissionTypes_i;
    t_PermissionType_i;
    t_publishReqQueue_i;
    t_notifRepublishQueue_i;
    t_notifRepublishQueueIterator_i;
    t_monitoredItemQueue_i;
    t_monitoredItemQueueIterator_i;
    t_monitoringFilter_i;
    t_monitoringFilterCtx_i;
    t_filterResult_i;
    t_monitoredItemPointer_i;
    t_monitoredItemId_i;
    t_eventFieldList_i;
    t_notificationData;
    t_client_handle_i;
    t_timer_id_i;
    t_IndexRange_i;
    t_ContinuationPoint_i;
    t_ContinuationPointId_i;
    t_ArrayDimensions_i;
    t_LocaleIds_i;
    t_RegisteredServer_i;
    t_ApplicationDescription_i;
    t_MdnsDiscoveryConfig_i;
    t_RegisteredServer2Info_i;
    t_DataTypeDefinition_i;
    t_ServerUri;
    t_ServerUris;
    t_ServerCapabilities;
    t_NamespaceUri;
    t_NamespaceIdx

CONCRETE_CONSTANTS
    c_Server_Nano_Extended,

    k_n_read_resp_max,
    k_n_WriteResponse_max,
    k_n_BrowseResponse_max,
    k_n_BrowseTarget_max,
    k_n_BrowsePathResPerElt_max,
    k_n_BrowsePathResPerPath_max,
    k_n_monitoredItem_max,
    k_n_monitoredItemNotif_max,
    k_n_notifEventQueueSize_min,
    k_n_notifEventQueueSize_default,
    k_n_notifQueueSize_max,
    k_n_publishRequestPerSub_max,
    k_n_republishNotifPerSub_max,
    k_n_genericOperationPerReq_max,
    k_n_registerNodes_max,
    k_n_unregisterNodes_max,
    k_n_nodesToAdd_max,
    k_n_nodesToDelete_max,
    k_n_UserAuthAttempts_max,
    k_n_NamespaceIndex_max,

    c_Null_Type_NodeId,
    c_ByteString_Type_NodeId,
    c_Byte_Type_NodeId,
    c_HierarchicalReferences_Type_NodeId,
    c_Server_NodeId,
    c_BaseEventType_NodeId,
    c_Server_ServerCapabilities_RoleSet_NodeId,
    c_RoleType_NodeId,
    c_HasComponentType_NodeId,
    c_HasPropertyType_NodeId,
    c_Server_NamespaceArray_NodeId,
    c_NamespaceMetadataType_NodeId,
    c_Server_Namespaces_NodeId,
    c_RolePermissionType_NodeId,

    c_NamespaceUri_QualifiedName,
    c_DefaultRolePermissions_QualifiedName,
    c_Identities_QualifiedName,

    c_Variant_indet,
    c_DataValue_indet,
    c_Node_indet,
    c_NodeId_indet,
    c_byte_buffer_indet,

    c_channel_indet,
    c_max_channels_connected,
    c_max_sessions_per_channel,
    c_channel_lock_create_session_delay,
    c_session_activation_min_delay,
    c_user_indet,
    c_user_token_indet,
    c_session_indet,
    c_session_token_indet,
    c_Nonce_indet,
    c_SignatureData_indet,
    c_msg_header_indet,
    c_msg_indet,
    c_request_context_indet,
    c_channel_config_idx_indet,
    c_endpoint_config_idx_indet,
    c_reverse_endpoint_config_idx_indet,
    c_client_request_handle_indet,
    c_server_request_handle_any,
    c_no_application_context,
    c_ExpandedNodeId_indet,
    c_Reference_indet,
    c_LocalizedText_indet,
    c_QualifiedName_indet,
    c_Event_indet,
    c_Timestamp_null,
    c_subscription_indet,
    c_notif_msg_indet,
    c_sub_seq_num_indet,
    c_sub_seq_num_init,
    c_timeref_indet,
    c_opcua_duration_indet,
    c_opcua_duration_zero,
    c_notificationQueue_indet,
    c_sessionRoles_indet,
    c_sessionRoles_empty,
    c_RolePermissionTypes_indet,
    c_PermissionType_none,
    c_publishReqQueue_indet,
    c_notifRepublishQueue_indet,
    c_notifRepublishQueueIterator_indet,
    c_monitoredItemQueue_indet,
    c_monitoredItemQueueIterator_indet,
    c_monitoringFilter_indet,
    c_monitoringFilterCtx_indet,
    c_filterResult_indet,
    c_monitoredItemPointer_indet,
    c_monitoredItemId_indet,
    c_eventFieldList_indet,
    c_WriteValuePointer_indet,
    c_BrowseResultReferences_indet,
    c_BrowseNodeClassMask_indet,
    c_BrowseResultMask_indet,
    c_BrowseResultMask_all,
    c_timer_id_indet,
    c_IndexRange_indet,
    c_ContinuationPoint_indet,
    c_ContinuationPointId_indet,
    c_ArrayDimensions_indet,
    c_LocaleIds_indet,
    c_LocaleIds_empty,
    c_browsePath_indet,
    c_RelativePath_indet,
    c_RelativePathElt_indet,
    c_CallMethod_indet,
    c_CallMethodPointer_indet,
    c_Argument_indet,
    c_Identity_indet,
    c_Criteria_indet,
    c_ArgumentsPointer_indet,
    c_NodeAttributes_indet,
    c_RegisteredServer_indet,
    c_ApplicationDescription_indet,
    c_MdnsDiscoveryConfig_indet,
    c_RegisteredServer2Info_indet,
    c_DataTypeDefinition_indet,
    c_ServerUri_empty,
    c_NamespaceUri_empty

ABSTRACT_CONSTANTS

    t_Variant,
    t_DataValue,
    t_Node,
    t_NodeId,
    t_byte_buffer,

    t_channel,
    cast_t_channel,
    t_session,
    cast_t_session,
    t_BrowsePath,
    cast_t_BrowsePath,
    t_RelativePath,
    t_RelativePathElt,
    t_CallMethod,
    cast_t_CallMethod,
    t_CallMethodPointer,
    t_Argument,
    t_ArgumentsPointer,
    t_Identity,
    t_Criteria,
    t_NodeAttributes,
    t_session_token,
    t_user,
    t_user_token,
    t_Nonce,
    t_SignatureData,
    t_msg_header,
    t_msg,
    t_request_context,
    t_channel_config_idx,
    cast_t_channel_config_idx,
    cast_t_NamespaceIdx,

    t_endpoint_config_idx,
    t_reverse_endpoint_config_idx,
    t_client_request_handle,
    t_ExpandedNodeId,
    t_Reference,
    t_LocalizedText,
    t_QualifiedName,
    t_Event,
    t_subscription,
    cast_t_subscription,
    t_notif_msg,
    t_sub_seq_num,
    t_timeref,
    t_notificationQueue,
    t_sessionRoles,
    t_RolePermissionTypes,
    t_notifRepublishQueue,
    t_notifRepublishQueueIterator,
    t_publishReqQueue,
    t_monitoredItemQueue,
    t_monitoredItemQueueIterator,
    t_monitoringFilter,
    t_monitoringFilterCtx,
    t_filterResult,
    t_monitoredItemPointer,
    t_monitoredItemId,
    t_eventFieldList,
    t_WriteValuePointer,
    t_BrowseResultReferences,
    t_timer_id,
    t_IndexRange,
    t_ContinuationPoint,
    t_ContinuationPointId,
    t_ArrayDimensions,
    t_LocaleIds,
    t_RegisteredServer,
    t_ApplicationDescription,
    t_MdnsDiscoveryConfig,
    t_RegisteredServer2Info,
    t_DataTypeDefinition,

    conv_AccessLevel_CurrentRead,
    conv_AccessLevel_CurrentWrite,
    conv_AccessLevel_StatusWrite,
    conv_AccessLevel_TimestampWrite,

    conv_ExpandedNodeId_NodeId,
    conv_NodeId_ExpandedNodeId,
    conv_NodeId_NamespaceIdx

PROPERTIES
    c_Server_Nano_Extended : BOOL &

    k_n_read_resp_max : NAT &
    k_n_WriteResponse_max : NAT &
    k_n_BrowseResponse_max : NAT &
    k_n_BrowseTarget_max : NAT &
    k_n_BrowsePathResPerElt_max : NAT &
    k_n_BrowsePathResPerPath_max : NAT &
    k_n_monitoredItem_max : NAT &
    k_n_monitoredItemNotif_max : NAT &
    k_n_notifEventQueueSize_min : NAT &
    k_n_notifEventQueueSize_min > 0 &
    k_n_notifEventQueueSize_default : NAT &
    k_n_notifEventQueueSize_default > 0 &
    k_n_notifQueueSize_max : NAT &
    k_n_notifQueueSize_max > 0 &
    k_n_publishRequestPerSub_max : NAT &
    k_n_publishRequestPerSub_max > 0 &
    k_n_republishNotifPerSub_max : NAT &
    k_n_republishNotifPerSub_max > 0 &
    k_n_genericOperationPerReq_max : NAT &
    k_n_registerNodes_max : NAT &
    k_n_unregisterNodes_max : NAT &
    k_n_nodesToAdd_max : NAT &
    k_n_nodesToDelete_max : NAT &
    k_n_UserAuthAttempts_max : NAT &
    k_n_NamespaceIndex_max : NAT &

    t_Variant <: t_Variant_i &
    c_Variant_indet : t_Variant_i &
    c_Variant_indet /: t_Variant &
    t_Variant /= {} &

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

    c_HierarchicalReferences_Type_NodeId : t_NodeId_i &
    c_HierarchicalReferences_Type_NodeId : t_NodeId &

    c_Null_Type_NodeId       /= c_ByteString_Type_NodeId &
    c_ByteString_Type_NodeId /= c_Byte_Type_NodeId &
    c_Byte_Type_NodeId       /= c_Null_Type_NodeId &

    c_HierarchicalReferences_Type_NodeId /= c_Null_Type_NodeId &
    c_HierarchicalReferences_Type_NodeId /= c_ByteString_Type_NodeId &
    c_HierarchicalReferences_Type_NodeId /= c_Byte_Type_NodeId &

    c_Server_NodeId : t_NodeId_i &
    c_Server_NodeId : t_NodeId &

    c_Server_NodeId /= c_Null_Type_NodeId &
    c_Server_NodeId /= c_ByteString_Type_NodeId &
    c_Server_NodeId /= c_Byte_Type_NodeId &
    c_Server_NodeId /= c_HierarchicalReferences_Type_NodeId &

    c_BaseEventType_NodeId : t_NodeId_i &
    c_BaseEventType_NodeId : t_NodeId &

    c_BaseEventType_NodeId /= c_Null_Type_NodeId &
    c_BaseEventType_NodeId /= c_ByteString_Type_NodeId &
    c_BaseEventType_NodeId /= c_Byte_Type_NodeId &
    c_BaseEventType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_BaseEventType_NodeId /= c_Server_NodeId &

    c_Server_ServerCapabilities_RoleSet_NodeId : t_NodeId_i &
    c_Server_ServerCapabilities_RoleSet_NodeId : t_NodeId &

    c_Server_ServerCapabilities_RoleSet_NodeId /= c_Null_Type_NodeId &
    c_Server_ServerCapabilities_RoleSet_NodeId /= c_ByteString_Type_NodeId &
    c_Server_ServerCapabilities_RoleSet_NodeId /= c_Byte_Type_NodeId &
    c_Server_ServerCapabilities_RoleSet_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_Server_ServerCapabilities_RoleSet_NodeId /= c_Server_NodeId &
    c_Server_ServerCapabilities_RoleSet_NodeId /= c_BaseEventType_NodeId &

    c_RoleType_NodeId : t_NodeId_i &
    c_RoleType_NodeId : t_NodeId &

    c_RoleType_NodeId /= c_Null_Type_NodeId &
    c_RoleType_NodeId /= c_ByteString_Type_NodeId &
    c_RoleType_NodeId /= c_Byte_Type_NodeId &
    c_RoleType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_RoleType_NodeId /= c_Server_NodeId &
    c_RoleType_NodeId /= c_BaseEventType_NodeId &
    c_RoleType_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &

    c_HasComponentType_NodeId : t_NodeId_i &
    c_HasComponentType_NodeId : t_NodeId &

    c_HasComponentType_NodeId /= c_Null_Type_NodeId &
    c_HasComponentType_NodeId /= c_ByteString_Type_NodeId &
    c_HasComponentType_NodeId /= c_Byte_Type_NodeId &
    c_HasComponentType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_HasComponentType_NodeId /= c_Server_NodeId &
    c_HasComponentType_NodeId /= c_BaseEventType_NodeId &
    c_HasComponentType_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_HasComponentType_NodeId /= c_RoleType_NodeId &

    c_HasPropertyType_NodeId : t_NodeId_i &
    c_HasPropertyType_NodeId : t_NodeId &

    c_HasPropertyType_NodeId /= c_Null_Type_NodeId &
    c_HasPropertyType_NodeId /= c_ByteString_Type_NodeId &
    c_HasPropertyType_NodeId /= c_Byte_Type_NodeId &
    c_HasPropertyType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_HasPropertyType_NodeId /= c_Server_NodeId &
    c_HasPropertyType_NodeId /= c_BaseEventType_NodeId &
    c_HasPropertyType_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_HasPropertyType_NodeId /= c_RoleType_NodeId &
    c_HasPropertyType_NodeId /= c_HasComponentType_NodeId &

    c_Server_NamespaceArray_NodeId : t_NodeId_i &
    c_Server_NamespaceArray_NodeId : t_NodeId &

    c_Server_NamespaceArray_NodeId /= c_Null_Type_NodeId &
    c_Server_NamespaceArray_NodeId /= c_ByteString_Type_NodeId &
    c_Server_NamespaceArray_NodeId /= c_Byte_Type_NodeId &
    c_Server_NamespaceArray_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_Server_NamespaceArray_NodeId /= c_Server_NodeId &
    c_Server_NamespaceArray_NodeId /= c_BaseEventType_NodeId &
    c_Server_NamespaceArray_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_Server_NamespaceArray_NodeId /= c_RoleType_NodeId &
    c_Server_NamespaceArray_NodeId /= c_HasComponentType_NodeId &
    c_Server_NamespaceArray_NodeId /= c_HasPropertyType_NodeId &

    c_NamespaceMetadataType_NodeId : t_NodeId_i &
    c_NamespaceMetadataType_NodeId : t_NodeId &

    c_NamespaceMetadataType_NodeId /= c_Null_Type_NodeId &
    c_NamespaceMetadataType_NodeId /= c_ByteString_Type_NodeId &
    c_NamespaceMetadataType_NodeId /= c_Byte_Type_NodeId &
    c_NamespaceMetadataType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_NamespaceMetadataType_NodeId /= c_Server_NodeId &
    c_NamespaceMetadataType_NodeId /= c_BaseEventType_NodeId &
    c_NamespaceMetadataType_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_NamespaceMetadataType_NodeId /= c_RoleType_NodeId &
    c_NamespaceMetadataType_NodeId /= c_HasComponentType_NodeId &
    c_NamespaceMetadataType_NodeId /= c_HasPropertyType_NodeId &
    c_NamespaceMetadataType_NodeId /= c_Server_NamespaceArray_NodeId &

    c_Server_Namespaces_NodeId : t_NodeId_i &
    c_Server_Namespaces_NodeId : t_NodeId &

    c_Server_Namespaces_NodeId /= c_Null_Type_NodeId &
    c_Server_Namespaces_NodeId /= c_ByteString_Type_NodeId &
    c_Server_Namespaces_NodeId /= c_Byte_Type_NodeId &
    c_Server_Namespaces_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_Server_Namespaces_NodeId /= c_Server_NodeId &
    c_Server_Namespaces_NodeId /= c_BaseEventType_NodeId &
    c_Server_Namespaces_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_Server_Namespaces_NodeId /= c_RoleType_NodeId &
    c_Server_Namespaces_NodeId /= c_HasComponentType_NodeId &
    c_Server_Namespaces_NodeId /= c_HasPropertyType_NodeId &
    c_Server_Namespaces_NodeId /= c_Server_NamespaceArray_NodeId &
    c_Server_Namespaces_NodeId /= c_NamespaceMetadataType_NodeId &

    c_RolePermissionType_NodeId : t_NodeId_i &
    c_RolePermissionType_NodeId : t_NodeId &

    c_RolePermissionType_NodeId /= c_Null_Type_NodeId &
    c_RolePermissionType_NodeId /= c_ByteString_Type_NodeId &
    c_RolePermissionType_NodeId /= c_Byte_Type_NodeId &
    c_RolePermissionType_NodeId /= c_HierarchicalReferences_Type_NodeId &
    c_RolePermissionType_NodeId /= c_Server_NodeId &
    c_RolePermissionType_NodeId /= c_BaseEventType_NodeId &
    c_RolePermissionType_NodeId /= c_Server_ServerCapabilities_RoleSet_NodeId &
    c_RolePermissionType_NodeId /= c_RoleType_NodeId &
    c_RolePermissionType_NodeId /= c_HasComponentType_NodeId &
    c_RolePermissionType_NodeId /= c_HasPropertyType_NodeId &
    c_RolePermissionType_NodeId /= c_Server_NamespaceArray_NodeId &
    c_RolePermissionType_NodeId /= c_NamespaceMetadataType_NodeId &
    c_RolePermissionType_NodeId /= c_Server_Namespaces_NodeId &

    t_byte_buffer <: t_byte_buffer_i &
    c_byte_buffer_indet : t_byte_buffer_i &
    c_byte_buffer_indet /: t_byte_buffer &

    t_channel <: t_channel_i &
    c_channel_indet : t_channel_i &
    c_channel_indet /: t_channel &
    cast_t_channel : NAT +-> t_channel_i &
    cast_t_channel : perm(t_channel) &

    c_max_channels_connected : NAT &
    c_max_sessions_per_channel : NAT &
    c_channel_lock_create_session_delay : NAT &
    c_session_activation_min_delay : NAT &

    t_session <: t_session_i &
    t_session /= {} &
    c_session_indet : t_session_i &
    c_session_indet /: t_session &
    cast_t_session : NAT +-> t_session_i &
    cast_t_session : perm(t_session) &

    t_session_token <: t_session_token_i &
    c_session_token_indet : t_session_token_i &
    c_session_token_indet /: t_session_token &

    t_user <: t_user_i &
    c_user_indet : t_user_i &
    c_user_indet /: t_user &

    t_user_token <: t_user_token_i &
    c_user_token_indet : t_user_token_i &
    c_user_token_indet /: t_user_token &

    t_Nonce <: t_Nonce_i &
    c_Nonce_indet : t_Nonce_i &
    c_Nonce_indet /: t_Nonce &

    t_SignatureData <: t_SignatureData_i &
    c_SignatureData_indet : t_SignatureData_i &
    c_SignatureData_indet /: t_SignatureData &

    t_subscription <: t_subscription_i &
    c_subscription_indet : t_subscription_i &
    c_subscription_indet /: t_subscription &
    cast_t_subscription : NAT +-> t_subscription_i &
    cast_t_subscription : perm(t_subscription) &

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

    t_sessionRoles <: t_sessionRoles_i &
    c_sessionRoles_indet : t_sessionRoles_i &
    c_sessionRoles_indet /: t_sessionRoles &
    c_sessionRoles_empty : t_sessionRoles_i &
    c_sessionRoles_empty /: t_sessionRoles &

    t_RolePermissionTypes <: t_RolePermissionTypes_i &
    c_RolePermissionTypes_indet : t_RolePermissionTypes_i &

    c_PermissionType_none : t_PermissionType_i &

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

    t_monitoringFilter <: t_monitoringFilter_i &
    c_monitoringFilter_indet : t_monitoringFilter_i &
    c_monitoringFilter_indet /: t_monitoringFilter &

    t_monitoringFilterCtx <: t_monitoringFilterCtx_i &
    c_monitoringFilterCtx_indet : t_monitoringFilterCtx_i &
    c_monitoringFilterCtx_indet /: t_monitoringFilterCtx &

    t_filterResult <: t_filterResult_i &
    c_filterResult_indet : t_filterResult_i &
    c_filterResult_indet /: t_filterResult &

    t_monitoredItemPointer <: t_monitoredItemPointer_i &
    c_monitoredItemPointer_indet : t_monitoredItemPointer_i &
    c_monitoredItemPointer_indet /: t_monitoredItemPointer &

    t_monitoredItemId <: t_monitoredItemId_i &
    c_monitoredItemId_indet : t_monitoredItemId_i &
    c_monitoredItemId_indet /: t_monitoredItemId &

    t_eventFieldList <: t_eventFieldList_i &
    c_eventFieldList_indet : t_eventFieldList_i &
    c_eventFieldList_indet /: t_eventFieldList &

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
    cast_t_channel_config_idx : NAT +-> t_channel_config_idx_i &
    cast_t_channel_config_idx : perm(t_channel_config_idx) &

    cast_t_NamespaceIdx : NAT +-> t_NamespaceIdx &
    1..k_n_NamespaceIndex_max = dom(cast_t_NamespaceIdx) &
    cast_t_NamespaceIdx : perm(t_NamespaceIdx) &

    t_endpoint_config_idx <: t_endpoint_config_idx_i &
    c_endpoint_config_idx_indet : t_endpoint_config_idx_i &
    c_endpoint_config_idx_indet /: t_endpoint_config_idx &

    t_reverse_endpoint_config_idx <: t_reverse_endpoint_config_idx_i &
    c_reverse_endpoint_config_idx_indet : t_reverse_endpoint_config_idx_i &
    c_reverse_endpoint_config_idx_indet /: t_reverse_endpoint_config_idx &

    t_ExpandedNodeId <: t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet : t_ExpandedNodeId_i &
    c_ExpandedNodeId_indet /: t_ExpandedNodeId &

    t_Reference <: t_Reference_i &
    c_Reference_indet : t_Reference_i &
    c_Reference_indet /: t_Reference &

    t_LocalizedText <: t_LocalizedText_i &
    c_LocalizedText_indet : t_LocalizedText_i &
    c_LocalizedText_indet /: t_LocalizedText &

    t_QualifiedName <: t_QualifiedName_i &
    c_QualifiedName_indet : t_QualifiedName_i &
    c_QualifiedName_indet /: t_QualifiedName &

    c_NamespaceUri_QualifiedName : t_QualifiedName_i &
    c_NamespaceUri_QualifiedName : t_QualifiedName &

    c_DefaultRolePermissions_QualifiedName : t_QualifiedName_i &
    c_DefaultRolePermissions_QualifiedName : t_QualifiedName &

    c_Identities_QualifiedName : t_QualifiedName_i &
    c_Identities_QualifiedName : t_QualifiedName &

    t_Event <: t_Event_i &
    c_Event_indet : t_Event_i &
    c_Event_indet /: t_Event &

    c_Timestamp_null : t_Timestamp &

    t_WriteValuePointer <: t_WriteValuePointer_i &
    c_WriteValuePointer_indet : t_WriteValuePointer_i &
    c_WriteValuePointer_indet /: t_WriteValuePointer &

    t_BrowseResultReferences <: t_BrowseResultReferences_i &
    c_BrowseResultReferences_indet : t_BrowseResultReferences_i &
    c_BrowseResultReferences_indet /: t_BrowseResultReferences &

    c_BrowseNodeClassMask_indet : t_BrowseNodeClassMask_i &

    c_BrowseResultMask_indet : t_BrowseResultMask_i &
    c_BrowseResultMask_all : t_BrowseResultMask_i &

    t_timer_id <: t_timer_id_i &
    c_timer_id_indet : t_timer_id_i &
    c_timer_id_indet /: t_timer_id &

    t_IndexRange <: t_IndexRange_i &
    c_IndexRange_indet : t_IndexRange_i &
    c_IndexRange_indet /: t_IndexRange &

    t_ContinuationPoint <: t_ContinuationPoint_i &
    c_ContinuationPoint_indet : t_ContinuationPoint_i &
    c_ContinuationPoint_indet /: t_ContinuationPoint &

    c_ContinuationPointId_indet : t_ContinuationPointId_i &
    t_ContinuationPointId = t_ContinuationPointId_i - {c_ContinuationPointId_indet} &

    c_ArrayDimensions_indet : t_ArrayDimensions_i &
    t_ArrayDimensions = t_ArrayDimensions_i - {c_ArrayDimensions_indet} &

    c_LocaleIds_indet : t_LocaleIds_i &
    t_LocaleIds = t_LocaleIds_i - {c_LocaleIds_indet} &
    c_LocaleIds_empty : t_LocaleIds_i &
    c_LocaleIds_empty : t_LocaleIds &

    c_RegisteredServer_indet : t_RegisteredServer_i &
    t_RegisteredServer = t_RegisteredServer_i - {c_RegisteredServer_indet} &

    c_ApplicationDescription_indet : t_ApplicationDescription_i &
    t_ApplicationDescription = t_ApplicationDescription_i - {c_ApplicationDescription_indet} &

    c_MdnsDiscoveryConfig_indet : t_MdnsDiscoveryConfig_i &
    t_MdnsDiscoveryConfig = t_MdnsDiscoveryConfig_i - {c_MdnsDiscoveryConfig_indet} &

    c_RegisteredServer2Info_indet : t_RegisteredServer2Info_i &
    t_RegisteredServer2Info = t_RegisteredServer2Info_i - {c_RegisteredServer2Info_indet} &

    c_DataTypeDefinition_indet : t_DataTypeDefinition_i &
    t_DataTypeDefinition = t_DataTypeDefinition_i - {c_DataTypeDefinition_indet} &

    c_ServerUri_empty : t_ServerUri &

    c_NamespaceUri_empty : t_NamespaceUri &

    conv_AccessLevel_CurrentWrite : t_access_level --> BOOL &
    conv_AccessLevel_CurrentRead : t_access_level --> BOOL &
    conv_AccessLevel_StatusWrite : t_access_level --> BOOL &
    conv_AccessLevel_TimestampWrite : t_access_level --> BOOL &

    conv_ExpandedNodeId_NodeId : t_ExpandedNodeId +-> t_NodeId &
    conv_NodeId_ExpandedNodeId : t_NodeId >-> t_ExpandedNodeId &
    conv_NodeId_NamespaceIdx : t_NodeId --> t_NamespaceIdx &

    t_BrowsePath <: t_BrowsePath_i &
    c_browsePath_indet : t_BrowsePath_i &
    c_browsePath_indet /: t_BrowsePath &
    cast_t_BrowsePath : NAT +-> t_BrowsePath_i &
    cast_t_BrowsePath : perm(t_BrowsePath) &

    t_RelativePath <: t_RelativePath_i &
    c_RelativePath_indet : t_RelativePath_i &
    c_RelativePath_indet /: t_RelativePath &

    t_RelativePathElt <: t_RelativePathElt_i &
    c_RelativePathElt_indet : t_RelativePathElt_i &
    c_RelativePathElt_indet /: t_RelativePathElt &

    t_CallMethod <: t_CallMethod_i &
    c_CallMethod_indet : t_CallMethod_i &
    c_CallMethod_indet /: t_CallMethod &
    cast_t_CallMethod : NAT +-> t_CallMethod_i &
    cast_t_CallMethod : perm(t_CallMethod) &

    t_CallMethodPointer <: t_CallMethodPointer_i &
    c_CallMethodPointer_indet : t_CallMethodPointer_i &
    c_CallMethodPointer_indet /: t_CallMethodPointer &

    t_Argument <: t_Argument_i &
    c_Argument_indet : t_Argument_i &
    c_Argument_indet /: t_Argument &

    t_ArgumentsPointer <: t_ArgumentsPointer_i &
    c_ArgumentsPointer_indet : t_ArgumentsPointer_i &
    c_ArgumentsPointer_indet /: t_ArgumentsPointer &

    t_Identity <: t_Identity_i &
    c_Identity_indet : t_Identity_i &
    c_Identity_indet /: t_Identity &

    t_Criteria <: t_Criteria_i &
    c_Criteria_indet : t_Criteria_i &
    c_Criteria_indet /: t_Criteria &

    t_NodeAttributes <: t_NodeAttributes_i &
    c_NodeAttributes_indet : t_NodeAttributes_i &
    c_NodeAttributes_indet /: t_NodeAttributes

OPERATIONS

    p_res <-- is_t_channel_config_idx(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i
    THEN
        p_res := bool(p_config_idx : t_channel_config_idx)
    END
    ;

    p_card_channel_config <-- get_card_t_channel_config_idx =
    BEGIN
        p_card_channel_config := card(t_channel_config_idx)
    END
    ;

    p_config <-- get_cast_t_channel_config_idx(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_channel_config_idx)
    THEN
        p_config := cast_t_channel_config_idx(p_ind)
    END
    ;

    p_nsIndex <-- get_cast_t_NamespaceIdx(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_NamespaceIdx)
    THEN
        p_nsIndex := cast_t_NamespaceIdx(p_ind)
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

    /* This "conversion" is a borrow, you shall not clean the given nid */
    /* The borrow is always valid. */
    p_local_server, p_nid <-- getall_conv_ExpandedNodeId_NodeId(p_expnid) =
    PRE
        p_expnid : t_ExpandedNodeId_i &
        p_expnid /= c_ExpandedNodeId_indet
    THEN
        p_local_server, p_nid :(p_local_server : BOOL &
                                p_local_server = bool(p_expnid : dom(conv_ExpandedNodeId_NodeId)) &
                                p_nid : t_NodeId_i &
                                (p_local_server = TRUE =>
                                    p_nid = conv_ExpandedNodeId_NodeId(p_expnid)) &
                                (p_local_server = FALSE =>
                                    p_nid = c_NodeId_indet)
                               )
    END
    ;

    p_idx <-- get_NodeId_NamespaceIndex(p_nodeId) =
    PRE
        p_nodeId : t_NodeId_i &
        p_nodeId : t_NodeId
    THEN
        p_idx := conv_NodeId_NamespaceIdx(p_nodeId)
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
    END;

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
    ;

    bres <-- is_EventNotifier_SubscribeToEvents(p_eventNotifierByte) =
    PRE
        p_eventNotifierByte : t_Byte
    THEN
        bres :: BOOL
    END
    ;

    p_browsePath <-- get_cast_t_BrowsePath(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_BrowsePath)
    THEN
        p_browsePath := cast_t_BrowsePath(p_ind)
    END
    ;

    p_callMethod <-- get_cast_t_CallMethod(p_ind) =
    PRE
        p_ind : NAT &
        p_ind : dom(cast_t_CallMethod)
    THEN
        p_callMethod := cast_t_CallMethod(p_ind)
    END
    ;

    p_bool <-- is_QualifiedNames_Equal(name1, name2) =
    PRE
        name1: t_QualifiedName_i &
        name1: t_QualifiedName &
        name2: t_QualifiedName_i &
        name2: t_QualifiedName
    THEN
        p_bool := bool(name1 = name2)
    END
    ;

    p_bool <-- is_QualifiedNames_Empty(name) =
    PRE
        name: t_QualifiedName_i
    THEN
        p_bool := bool(name = c_QualifiedName_indet)
    END
    ;

    p_alloc, p_out <-- get_copy_ExpandedNodeId(p_in) =
    PRE
        p_in : t_ExpandedNodeId_i &
        p_in : t_ExpandedNodeId
    THEN
        p_alloc,
        p_out
        :(p_alloc : BOOL &
          p_out : t_ExpandedNodeId_i &
          (p_alloc = FALSE => p_out = c_ExpandedNodeId_indet) &
          (p_alloc = TRUE => p_out = p_in))
    END
    ;

    free_ExpandedNodeId(p_in) =
    PRE
        p_in : t_ExpandedNodeId_i &
        p_in : t_ExpandedNodeId
    THEN
        skip
    END
    ;

    free_roles(p_in) =
    PRE
        p_in : t_sessionRoles_i &
        p_in : t_sessionRoles
    THEN
        skip
    END
    ;

    free_LocaleIds(p_in) =
    PRE
        p_in : t_LocaleIds_i &
        p_in : t_LocaleIds
    THEN
        skip
    END
    ;

    p_localeIds <-- get_SupportedLocales(p_in) =
    PRE
        p_in : t_endpoint_config_idx_i &
        p_in : t_endpoint_config_idx
    THEN
        p_localeIds :(p_localeIds : t_LocaleIds_i & p_localeIds : t_LocaleIds)
    END
    ;

    p_currentTs <-- get_CurrentTimestamp =
    BEGIN
        p_currentTs :(p_currentTs : t_Timestamp & p_currentTs /= c_Timestamp_null)
    END
    ;

    bres <-- is_Timestamps_Null (p_ts) =
    PRE
        p_ts : t_Timestamp
    THEN
        bres := bool(p_ts = c_Timestamp_null)
    END
    ;

    /* Returns true if clients are allowed to use NodeManagement service set, false otherwise */
    bres <-- is_ClientNodeManagementActive =
    BEGIN
        bres :: BOOL
    END

END
