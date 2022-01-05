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

/** \file
 *
 * Hand-modified _bs.h to specify some types
 */

#ifndef CONSTANTS_BS_H_
#define CONSTANTS_BS_H_

#include <stddef.h>
#include <stdint.h>

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

#include "continuation_point_impl.h"
#include "opcua_identifiers.h"
#include "service_discovery_servers_internal.h"
#include "sopc_address_space.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_services_api_internal.h"
#include "sopc_singly_linked_list.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/* Access levels, taken from Part 3 v1.03 §5.6.2 Table 8 */
#define SOPC_AccessLevelMask_CurrentRead (uint8_t) 1     // bit0
#define SOPC_AccessLevelMask_CurrentWrite (uint8_t) 2    // bit1
#define SOPC_AccessLevelMask_StatusWrite (uint8_t) 32    // bit5
#define SOPC_AccessLevelMask_TimestampWrite (uint8_t) 64 // bit6

/*-----------------------------
   SETS Clause: deferred sets
  -----------------------------*/
typedef OpcUa_ApplicationDescription* constants_bs__t_ApplicationDescription_i;
typedef OpcUa_Argument* constants_bs__t_Argument_i;
typedef void* constants_bs__t_ArrayDimensions_i;
typedef uint32_t constants_bs__t_BrowseNodeClassMask_i;
typedef uint32_t constants_bs__t_BrowsePath_i;
typedef uint32_t constants_bs__t_BrowseResultMask_i;
typedef OpcUa_ReferenceDescription* constants_bs__t_BrowseResultReferences_i;
typedef uint8_t constants_bs__t_Byte;
typedef int32_t constants_bs__t_CallMethod_i;
typedef uint64_t constants_bs__t_ContinuationPointId_i;
typedef SOPC_ContinuationPointData constants_bs__t_ContinuationPoint_i;
typedef SOPC_DataValue* constants_bs__t_DataValue_i;
typedef SOPC_ExpandedNodeId* constants_bs__t_ExpandedNodeId_i;
typedef SOPC_String* constants_bs__t_IndexRange_i;
typedef int32_t constants_bs__t_Int32;
typedef char** constants_bs__t_LocaleIds_i;
typedef SOPC_LocalizedText* constants_bs__t_LocalizedText_i;
typedef OpcUa_MdnsDiscoveryConfiguration* constants_bs__t_MdnsDiscoveryConfig_i;
typedef SOPC_NodeId* constants_bs__t_NodeId_i;
typedef SOPC_AddressSpace_Node* constants_bs__t_Node_i;
typedef SOPC_ByteString* constants_bs__t_Nonce_i;
typedef SOPC_QualifiedName* constants_bs__t_QualifiedName_i;
typedef SOPC_StatusCode constants_bs__t_RawStatusCode;
typedef OpcUa_ReferenceNode* constants_bs__t_Reference_i;
typedef SOPC_RegisterServer2Record_Internal* constants_bs__t_RegisteredServer2Info_i;
typedef OpcUa_RegisteredServer* constants_bs__t_RegisteredServer_i;
typedef OpcUa_RelativePathElement* constants_bs__t_RelativePathElt_i;
typedef OpcUa_RelativePath* constants_bs__t_RelativePath_i;
typedef SOPC_ServerCapabilities_Internal constants_bs__t_ServerCapabilities;
typedef SOPC_String* constants_bs__t_ServerUri;
typedef SOPC_String* constants_bs__t_ServerUris;
typedef OpcUa_SignatureData* constants_bs__t_SignatureData_i;
typedef SOPC_Value_Timestamp constants_bs__t_Timestamp;
typedef SOPC_Variant* constants_bs__t_Variant_i;
typedef OpcUa_WriteValue* constants_bs__t_WriteValuePointer_i;
typedef SOPC_Byte constants_bs__t_access_level;
typedef uintptr_t constants_bs__t_application_context_i;
typedef SOPC_Internal_SessionAppContext* constants_bs__t_session_application_context_i;
typedef SOPC_Buffer* constants_bs__t_byte_buffer_i;
typedef uint32_t constants_bs__t_channel_config_idx_i;
typedef uint32_t constants_bs__t_channel_i;
typedef uint32_t constants_bs__t_client_handle_i;
typedef uint32_t constants_bs__t_client_request_handle_i;
#define constants_bs__t_counter_i t_entier4
typedef uint32_t constants_bs__t_endpoint_config_idx_i;
typedef uint32_t constants_bs__t_monitoredItemId_i;
typedef void* constants_bs__t_monitoredItemPointer_i;
typedef SOPC_SLinkedListIterator* constants_bs__t_monitoredItemQueueIterator_i;
typedef SOPC_SLinkedList* constants_bs__t_monitoredItemQueue_i;
typedef void* constants_bs__t_msg_header_i; /* OpcUa_RequestHeader OR OpcUa_ResponseHeader */
typedef void* constants_bs__t_msg_i;        /* OpcUa_* message */
typedef SOPC_SLinkedListIterator* constants_bs__t_notifRepublishQueueIterator_i;
typedef SOPC_SLinkedList* constants_bs__t_notifRepublishQueue_i;
typedef OpcUa_NotificationMessage* constants_bs__t_notif_msg_i;
typedef SOPC_SLinkedList* constants_bs__t_notificationQueue_i;
typedef double constants_bs__t_opcua_duration_i;
typedef SOPC_SLinkedList* constants_bs__t_publishReqQueue_i;
typedef uint32_t constants_bs__t_request_context_i;
typedef uint32_t constants_bs__t_server_request_handle_i;
typedef uint32_t constants_bs__t_session_i;
typedef SOPC_NodeId* constants_bs__t_session_token_i;
typedef uint32_t constants_bs__t_sub_seq_num_i;
typedef uint32_t constants_bs__t_subscription_i;
typedef uint32_t constants_bs__t_timer_id_i;
typedef SOPC_TimeReference constants_bs__t_timeref_i;
typedef SOPC_UserWithAuthorization* constants_bs__t_user_i;
typedef SOPC_ExtensionObject* constants_bs__t_user_token_i;

/*--------------------------
   Added by the Translator
  --------------------------*/
#define constants_bs__t_channel_config_idx_i_max (2 * SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
#define constants_bs__t_channel_i_max SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED
#define constants_bs__t_endpoint_config_idx_i_max SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
#define constants_bs__t_session_i_max SOPC_MAX_SESSIONS
#define constants_bs__t_subscription_i_max SOPC_MAX_SESSIONS // 1 sub / session

/*------------------------------------------------
   CONCRETE_CONSTANTS Clause: scalars and arrays
  ------------------------------------------------*/
#define constants_bs__c_ApplicationDescription_indet NULL
#define constants_bs__c_ArrayDimensions_indet 0
#define constants_bs__c_Argument_indet NULL
#define constants_bs__c_BrowseNodeClassMask_indet 0
#define constants_bs__c_BrowseResultMask_all 63
#define constants_bs__c_BrowseResultMask_indet 0
#define constants_bs__c_BrowseResultReferences_indet NULL
extern const constants_bs__t_NodeId_i constants_bs__c_ByteString_Type_NodeId;
extern const constants_bs__t_NodeId_i constants_bs__c_Byte_Type_NodeId;
#define constants_bs__c_CallMethod_indet 0
#define constants_bs__c_ContinuationPointId_indet 0
#define constants_bs__c_ContinuationPoint_indet sopc_continuationPointData_empty;
#define constants_bs__c_DataValue_indet NULL
#define constants_bs__c_ExpandedNodeId_indet NULL
#define constants_bs__c_IndexRange_indet 0
extern constants_bs__t_LocaleIds_i constants_bs__c_LocaleIds_empty;
#define constants_bs__c_LocaleIds_indet NULL
#define constants_bs__c_LocalizedText_indet 0
#define constants_bs__c_MdnsDiscoveryConfig_indet NULL
#define constants_bs__c_NodeId_indet NULL
#define constants_bs__c_Node_indet 0
#define constants_bs__c_Nonce_indet 0
extern const constants_bs__t_NodeId_i constants_bs__c_Null_Type_NodeId;
#define constants_bs__c_QualifiedName_indet 0
#define constants_bs__c_Reference_indet 0
#define constants_bs__c_RegisteredServer2Info_indet NULL
#define constants_bs__c_RegisteredServer_indet NULL
#define constants_bs__c_RelativePathElt_indet 0
#define constants_bs__c_RelativePath_indet 0

#if 0 == WITH_NANO_EXTENDED
#define constants_bs__c_Server_Nano_Extended false
#else
#define constants_bs__c_Server_Nano_Extended true
#endif

#define constants_bs__c_SignatureData_indet 0
#define constants_bs__c_Timestamp_null \
    (SOPC_Value_Timestamp) { 0, 0 }
#define constants_bs__c_Variant_indet 0
#define constants_bs__c_WriteValuePointer_indet 0
#define constants_bs__c_browsePath_indet 0
#define constants_bs__c_byte_buffer_indet 0
#define constants_bs__c_channel_config_idx_indet 0
#define constants_bs__c_channel_indet 0
#define constants_bs__c_client_request_handle_indet 0
#define constants_bs__c_endpoint_config_idx_indet 0
#define constants_bs__c_max_channels_connected SOPC_MAX_SECURE_CONNECTIONS
#define constants_bs__c_monitoredItemId_indet 0
#define constants_bs__c_monitoredItemPointer_indet 0
#define constants_bs__c_monitoredItemQueueIterator_indet 0
#define constants_bs__c_monitoredItemQueue_indet 0
#define constants_bs__c_msg_header_indet 0
#define constants_bs__c_msg_indet 0
#define constants_bs__c_no_application_context 0
#define constants_bs__c_notifRepublishQueueIterator_indet 0
#define constants_bs__c_notifRepublishQueue_indet 0
#define constants_bs__c_notif_msg_indet 0
#define constants_bs__c_notificationQueue_indet 0
#define constants_bs__c_opcua_duration_indet -1
#define constants_bs__c_opcua_duration_zero 0
#define constants_bs__c_publishReqQueue_indet 0
#define constants_bs__c_request_context_indet 0
#define constants_bs__c_server_request_handle_any 0
#define constants_bs__c_session_indet 0
#define constants_bs__c_session_token_indet 0
#define constants_bs__c_sub_seq_num_indet 0
#define constants_bs__c_sub_seq_num_init 1
#define constants_bs__c_subscription_indet 0
#define constants_bs__c_timer_id_indet 0
#define constants_bs__c_timeref_indet 0
#define constants_bs__c_user_indet 0
#define constants_bs__c_user_token_indet 0
#define constants_bs__k_n_BrowsePathResPerElt_max SOPC_MAX_TRANSLATE_BROWSE_PATH_MATCHES
#define constants_bs__k_n_BrowsePathResPerPath_max SOPC_MAX_TRANSLATE_BROWSE_PATH_MATCHES
#define constants_bs__k_n_BrowseResponse_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_BrowseTarget_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_IndexRange_max 0
#define constants_bs__k_n_WriteResponse_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_genericOperationPerReq_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_monitoredItemNotif_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_monitoredItem_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_publishRequestPerSub_max SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS
#define constants_bs__k_n_read_resp_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_registerNodes_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_republishNotifPerSub_max (2 * SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS)
#define constants_bs__k_n_unregisterNodes_max SOPC_MAX_OPERATIONS_PER_MSG

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void constants_bs__free_ExpandedNodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_in);
extern void constants_bs__free_LocaleIds(const constants_bs__t_LocaleIds_i constants_bs__p_in);
extern void constants_bs__get_CurrentTimestamp(constants_bs__t_Timestamp* const constants_bs__p_currentTs);
extern void constants_bs__get_SupportedLocales(const constants_bs__t_endpoint_config_idx_i constants_bs__p_in,
                                               constants_bs__t_LocaleIds_i* const constants_bs__p_localeIds);
extern void constants_bs__get_card_t_channel(t_entier4* const constants_bs__p_card_channel);
extern void constants_bs__get_card_t_session(t_entier4* const constants_bs__p_card_session);
extern void constants_bs__get_card_t_subscription(t_entier4* const constants_bs__p_card_subscription);
extern void constants_bs__get_cast_t_BrowsePath(const t_entier4 constants_bs__p_ind,
                                                constants_bs__t_BrowsePath_i* const constants_bs__p_browsePath);
extern void constants_bs__get_cast_t_CallMethod(const t_entier4 constants_bs__p_ind,
                                                constants_bs__t_CallMethod_i* const constants_bs__p_callMethod);
extern void constants_bs__get_cast_t_channel(const t_entier4 constants_bs__p_ind,
                                             constants_bs__t_channel_i* const constants_bs__p_channel);
extern void constants_bs__get_cast_t_session(const t_entier4 constants_bs__p_ind,
                                             constants_bs__t_session_i* const constants_bs__p_session);
extern void constants_bs__get_cast_t_subscription(const t_entier4 constants_bs__p_ind,
                                                  constants_bs__t_subscription_i* const constants_bs__p_subscription);
extern void constants_bs__get_copy_ExpandedNodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_in,
                                                  t_bool* const constants_bs__p_alloc,
                                                  constants_bs__t_ExpandedNodeId_i* const constants_bs__p_out);
extern void constants_bs__getall_conv_ExpandedNodeId_NodeId(
    const constants_bs__t_ExpandedNodeId_i constants_bs__p_expnid,
    t_bool* const constants_bs__p_local_server,
    constants_bs__t_NodeId_i* const constants_bs__p_nid);
extern void constants_bs__is_QualifiedNames_Empty(const constants_bs__t_QualifiedName_i constants_bs__name,
                                                  t_bool* const constants_bs__p_bool);
extern void constants_bs__is_QualifiedNames_Equal(const constants_bs__t_QualifiedName_i constants_bs__name1,
                                                  const constants_bs__t_QualifiedName_i constants_bs__name2,
                                                  t_bool* const constants_bs__p_bool);
extern void constants_bs__is_t_access_level_currentRead(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                        t_bool* const constants_bs__bres);
extern void constants_bs__is_t_access_level_currentWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                         t_bool* const constants_bs__bres);
extern void constants_bs__is_t_access_level_statusWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                        t_bool* const constants_bs__bres);
extern void constants_bs__is_t_access_level_timestampWrite(
    const constants_bs__t_access_level constants_bs__p_access_lvl,
    t_bool* const constants_bs__bres);
extern void constants_bs__is_t_channel(const constants_bs__t_channel_i constants_bs__p_channel,
                                       t_bool* const constants_bs__p_res);
extern void constants_bs__is_t_channel_config_idx(const constants_bs__t_channel_config_idx_i constants_bs__p_config_idx,
                                                  t_bool* const constants_bs__p_res);
extern void constants_bs__is_t_endpoint_config_idx(
    const constants_bs__t_endpoint_config_idx_i constants_bs__p_endpoint_config_idx,
    t_bool* const constants_bs__p_res);

#endif
