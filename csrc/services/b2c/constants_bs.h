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

#include <stdint.h>
#include <stdlib.h>

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

#include "sopc_address_space.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_time.h"
#include "sopc_types.h"

/*-----------------------------
   SETS Clause: deferred sets
  -----------------------------*/
typedef SOPC_ExpandedNodeId* constants_bs__t_ExpandedNodeId_i;
typedef SOPC_LocalizedText* constants_bs__t_LocalizedText_i;
typedef SOPC_NodeId* constants_bs__t_NodeId_i;

typedef SOPC_AddressSpace_Item* constants_bs__t_Node_i;
typedef SOPC_ByteString* constants_bs__t_Nonce_i;
typedef SOPC_QualifiedName* constants_bs__t_QualifiedName_i;
typedef OpcUa_ReferenceNode* constants_bs__t_Reference_i;
typedef OpcUa_SignatureData* constants_bs__t_SignatureData_i;

typedef SOPC_Variant* constants_bs__t_Variant_i;
typedef OpcUa_WriteValue* constants_bs__t_WriteValuePointer_i;
typedef uintptr_t constants_bs__t_application_context_i;
typedef SOPC_Buffer* constants_bs__t_byte_buffer_i;

typedef uint32_t constants_bs__t_channel_config_idx_i;
typedef uint32_t constants_bs__t_channel_i;
typedef uint32_t constants_bs__t_client_request_handle_i;
typedef uint32_t constants_bs__t_client_handle_i;
#define constants_bs__t_counter_i t_entier4
typedef uint32_t constants_bs__t_endpoint_config_idx_i;
typedef uint32_t constants_bs__t_monitoredItemId_i;
typedef void* constants_bs__t_monitoredItemPointer_i;
typedef SOPC_SLinkedListIterator constants_bs__t_monitoredItemQueueIterator_i;
typedef SOPC_SLinkedList* constants_bs__t_monitoredItemQueue_i;
#define constants_bs__t_monitoredItem_i t_entier4
typedef void* constants_bs__t_msg_header_i; /* OpcUa_RequestHeader OR OpcUa_ResponseHeader */
typedef void* constants_bs__t_msg_i;        /* OpcUa_* message */
typedef OpcUa_NotificationMessage* constants_bs__t_notif_msg_i;
typedef SOPC_SLinkedListIterator constants_bs__t_notificationQueueIterator_i;
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
#define constants_bs__t_user_i t_entier4
typedef SOPC_ExtensionObject* constants_bs__t_user_token_i;

/*--------------------------
   Added by the Translator
  --------------------------*/
#define constants_bs__t_ExpandedNodeId_i_max (-1)
#define constants_bs__t_LocalizedText_i_max (-1)
#define constants_bs__t_NodeId_i_max (-1)
#define constants_bs__t_Node_i_max (-1)
#define constants_bs__t_Nonce_i_max (-1)
#define constants_bs__t_QualifiedName_i_max (-1)
#define constants_bs__t_Reference_i_max (-1)
#define constants_bs__t_SignatureData_i_max (-1)
#define constants_bs__t_Variant_i_max (-1)
#define constants_bs__t_WriteValuePointer_i_max (-1)
#define constants_bs__t_application_context_i_max (-1)
#define constants_bs__t_byte_buffer_i_max (-1)
#define constants_bs__t_channel_config_idx_i_max (2 * SOPC_MAX_SECURE_CONNECTIONS)
#define constants_bs__t_channel_i_max SOPC_MAX_SECURE_CONNECTIONS
#define constants_bs__t_client_handle_i_max (-1)
#define constants_bs__t_client_request_handle_i_max (-1)
#define constants_bs__t_counter_i_max (-1)
#define constants_bs__t_endpoint_config_idx_i_max SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
#define constants_bs__t_monitoredItemId_i_max (-1)
#define constants_bs__t_monitoredItemPointer_i_max (-1)
#define constants_bs__t_monitoredItemQueueIterator_i_max (-1)
#define constants_bs__t_monitoredItemQueue_i_max (-1)
#define constants_bs__t_msg_header_i_max (-1)
#define constants_bs__t_msg_i_max (-1)
#define constants_bs__t_notif_msg_i_max (-1)
#define constants_bs__t_notificationQueue_i_max (-1)
#define constants_bs__t_opcua_duration_i_max (-1)
#define constants_bs__t_publishReqQueue_i_max (-1)
#define constants_bs__t_request_context_i_max (-1)
#define constants_bs__t_server_request_handle_i_max (-1)
#define constants_bs__t_session_i_max SOPC_MAX_SESSIONS
#define constants_bs__t_session_token_i_max (-1)
#define constants_bs__t_sub_seq_num_i_max (-1)
#define constants_bs__t_subscription_i_max SOPC_MAX_SESSIONS // 1 sub / session
#define constants_bs__t_timeref_i_max (-1)
#define constants_bs__t_user_i_max (-1)
#define constants_bs__t_user_token_i_max (-1)

/*------------------------------------------------
   CONCRETE_CONSTANTS Clause: scalars and arrays
  ------------------------------------------------*/
#define constants_bs__c_ExpandedNodeId_indet NULL
#define constants_bs__c_LocalizedText_indet 0
#define constants_bs__c_NodeId_indet 0
#define constants_bs__c_Node_indet 0
#define constants_bs__c_Nonce_indet 0
#define constants_bs__c_QualifiedName_indet 0
#define constants_bs__c_Reference_indet 0
#define constants_bs__c_SignatureData_indet 0
#define constants_bs__c_Variant_indet 0
#define constants_bs__c_WriteValuePointer_indet 0
#define constants_bs__c_byte_buffer_indet 0
#define constants_bs__c_channel_config_idx_indet 0
#define constants_bs__c_channel_indet 0
#define constants_bs__c_client_request_handle_indet 0
#define constants_bs__c_endpoint_config_idx_indet 0
#define constants_bs__c_monitoredItemId_indet 0
#define constants_bs__c_monitoredItemPointer_indet 0
#define constants_bs__c_monitoredItemQueueIterator_indet 0
#define constants_bs__c_monitoredItemQueue_indet 0
#define constants_bs__c_msg_header_indet 0
#define constants_bs__c_msg_indet 0
#define constants_bs__c_no_application_context 0
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
#define constants_bs__k_n_BrowseResponse_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_BrowseTarget_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_WriteResponse_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_monitoredItemNotif_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_monitoredItem_max SOPC_MAX_OPERATIONS_PER_MSG
#define constants_bs__k_n_read_resp_max SOPC_MAX_OPERATIONS_PER_MSG

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void constants_bs__get_Is_SubType(const constants_bs__t_NodeId_i constants_bs__p_type1,
                                         const constants_bs__t_NodeId_i constants_bs__p_type2,
                                         t_bool* const constants_bs__p_res);
extern void constants_bs__get_card_t_channel(t_entier4* const constants_bs__p_card_channel);
extern void constants_bs__get_card_t_session(t_entier4* const constants_bs__p_card_session);
extern void constants_bs__get_card_t_subscription(t_entier4* const constants_bs__p_card_subscription);
extern void constants_bs__get_cast_t_channel(const t_entier4 constants_bs__p_ind,
                                             constants_bs__t_channel_i* const constants_bs__p_channel);
extern void constants_bs__get_cast_t_session(const t_entier4 constants_bs__p_ind,
                                             constants_bs__t_session_i* const constants_bs__p_session);
extern void constants_bs__get_cast_t_subscription(const t_entier4 constants_bs__p_ind,
                                                  constants_bs__t_subscription_i* const constants_bs__p_subscription);
extern void constants_bs__getall_conv_ExpandedNodeId_NodeId(
    const constants_bs__t_ExpandedNodeId_i constants_bs__p_expnid,
    t_bool* const constants_bs__p_isvalid,
    constants_bs__t_NodeId_i* const constants_bs__p_nid);
extern void constants_bs__is_t_channel(const constants_bs__t_channel_i constants_bs__p_channel,
                                       t_bool* const constants_bs__p_res);
extern void constants_bs__is_t_channel_config_idx(const constants_bs__t_channel_config_idx_i constants_bs__p_config_idx,
                                                  t_bool* const constants_bs__p_res);
extern void constants_bs__is_t_endpoint_config_idx(
    const constants_bs__t_endpoint_config_idx_i constants_bs__p_endpoint_config_idx,
    t_bool* const constants_bs__p_res);

#endif
