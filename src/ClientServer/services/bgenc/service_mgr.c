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

 File Name            : service_mgr.c

 Date                 : 04/08/2022 14:53:10

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_mgr__get_response_type(
   const constants__t_msg_type_i service_mgr__req_msg_typ,
   constants__t_msg_type_i * const service_mgr__resp_msg_typ) {
   *service_mgr__resp_msg_typ = constants__c_msg_type_indet;
   switch (service_mgr__req_msg_typ) {
   case constants__e_msg_discovery_find_servers_req:
      *service_mgr__resp_msg_typ = constants__e_msg_discovery_find_servers_resp;
      break;
   case constants__e_msg_discovery_find_servers_on_network_req:
      *service_mgr__resp_msg_typ = constants__e_msg_discovery_find_servers_on_network_resp;
      break;
   case constants__e_msg_discovery_get_endpoints_req:
      *service_mgr__resp_msg_typ = constants__e_msg_discovery_get_endpoints_resp;
      break;
   case constants__e_msg_discovery_register_server_req:
      *service_mgr__resp_msg_typ = constants__e_msg_discovery_register_server_resp;
      break;
   case constants__e_msg_discovery_register_server2_req:
      *service_mgr__resp_msg_typ = constants__e_msg_discovery_register_server2_resp;
      break;
   case constants__e_msg_session_create_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_create_resp;
      break;
   case constants__e_msg_session_activate_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_activate_resp;
      break;
   case constants__e_msg_session_close_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_close_resp;
      break;
   case constants__e_msg_session_cancel_req:
      *service_mgr__resp_msg_typ = constants__e_msg_session_cancel_resp;
      break;
   case constants__e_msg_node_add_nodes_req:
      *service_mgr__resp_msg_typ = constants__e_msg_node_add_nodes_resp;
      break;
   case constants__e_msg_node_add_references_req:
      *service_mgr__resp_msg_typ = constants__e_msg_node_add_references_resp;
      break;
   case constants__e_msg_node_delete_nodes_req:
      *service_mgr__resp_msg_typ = constants__e_msg_node_delete_nodes_resp;
      break;
   case constants__e_msg_node_delete_references_req:
      *service_mgr__resp_msg_typ = constants__e_msg_node_delete_references_resp;
      break;
   case constants__e_msg_view_browse_req:
      *service_mgr__resp_msg_typ = constants__e_msg_view_browse_resp;
      break;
   case constants__e_msg_view_browse_next_req:
      *service_mgr__resp_msg_typ = constants__e_msg_view_browse_next_resp;
      break;
   case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
      *service_mgr__resp_msg_typ = constants__e_msg_view_translate_browse_paths_to_node_ids_resp;
      break;
   case constants__e_msg_view_register_nodes_req:
      *service_mgr__resp_msg_typ = constants__e_msg_view_register_nodes_resp;
      break;
   case constants__e_msg_view_unregister_nodes_req:
      *service_mgr__resp_msg_typ = constants__e_msg_view_unregister_nodes_resp;
      break;
   case constants__e_msg_query_first_req:
      *service_mgr__resp_msg_typ = constants__e_msg_query_first_resp;
      break;
   case constants__e_msg_query_next_req:
      *service_mgr__resp_msg_typ = constants__e_msg_query_next_resp;
      break;
   case constants__e_msg_attribute_read_req:
      *service_mgr__resp_msg_typ = constants__e_msg_attribute_read_resp;
      break;
   case constants__e_msg_attribute_history_read_req:
      *service_mgr__resp_msg_typ = constants__e_msg_attribute_history_read_resp;
      break;
   case constants__e_msg_attribute_write_req:
      *service_mgr__resp_msg_typ = constants__e_msg_attribute_write_resp;
      break;
   case constants__e_msg_attribute_history_update_req:
      *service_mgr__resp_msg_typ = constants__e_msg_attribute_history_update_resp;
      break;
   case constants__e_msg_method_call_req:
      *service_mgr__resp_msg_typ = constants__e_msg_method_call_resp;
      break;
   case constants__e_msg_monitored_items_create_req:
      *service_mgr__resp_msg_typ = constants__e_msg_monitored_items_create_resp;
      break;
   case constants__e_msg_monitored_items_modify_req:
      *service_mgr__resp_msg_typ = constants__e_msg_monitored_items_modify_resp;
      break;
   case constants__e_msg_monitored_items_set_monitoring_mode_req:
      *service_mgr__resp_msg_typ = constants__e_msg_monitored_items_set_monitoring_mode_resp;
      break;
   case constants__e_msg_monitored_items_set_triggering_req:
      *service_mgr__resp_msg_typ = constants__e_msg_monitored_items_set_triggering_resp;
      break;
   case constants__e_msg_monitored_items_delete_req:
      *service_mgr__resp_msg_typ = constants__e_msg_monitored_items_delete_resp;
      break;
   case constants__e_msg_subscription_create_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_create_resp;
      break;
   case constants__e_msg_subscription_modify_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_modify_resp;
      break;
   case constants__e_msg_subscription_set_publishing_mode_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_set_publishing_mode_resp;
      break;
   case constants__e_msg_subscription_publish_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_publish_resp;
      break;
   case constants__e_msg_subscription_republish_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_republish_resp;
      break;
   case constants__e_msg_subscription_transfer_subscriptions_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_transfer_subscriptions_resp;
      break;
   case constants__e_msg_subscription_delete_subscriptions_req:
      *service_mgr__resp_msg_typ = constants__e_msg_subscription_delete_subscriptions_resp;
      break;
   default:
      break;
   }
}

void service_mgr__treat_session_local_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service) {
   {
      constants__t_user_i service_mgr__l_user;
      constants__t_LocaleIds_i service_mgr__l_supported_locales;
      
      switch (service_mgr__req_typ) {
      case constants__e_msg_attribute_read_req:
         session_mgr__get_local_user(service_mgr__endpoint_config_idx,
            &service_mgr__l_user);
         constants__get_SupportedLocales(service_mgr__endpoint_config_idx,
            &service_mgr__l_supported_locales);
         service_read__treat_read_request(service_mgr__l_user,
            service_mgr__l_supported_locales,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_attribute_write_req:
         session_mgr__get_local_user(service_mgr__endpoint_config_idx,
            &service_mgr__l_user);
         constants__get_SupportedLocales(service_mgr__endpoint_config_idx,
            &service_mgr__l_supported_locales);
         address_space_itf__treat_write_request(service_mgr__l_user,
            service_mgr__l_supported_locales,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_browse_req:
         service_set_view__treat_browse_request(constants__c_session_indet,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
         service_set_view__treat_translate_browse_paths_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      default:
         *service_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         break;
      }
   }
}

void service_mgr__treat_session_nano_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service) {
   {
      constants__t_user_i service_mgr__l_user;
      constants__t_LocaleIds_i service_mgr__l_locales;
      
      switch (service_mgr__req_typ) {
      case constants__e_msg_attribute_read_req:
         session_mgr__get_session_user_server(service_mgr__session,
            &service_mgr__l_user);
         session_mgr__get_server_session_preferred_locales(service_mgr__session,
            &service_mgr__l_locales);
         service_read__treat_read_request(service_mgr__l_user,
            service_mgr__l_locales,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_attribute_write_req:
         session_mgr__get_session_user_server(service_mgr__session,
            &service_mgr__l_user);
         constants__get_SupportedLocales(service_mgr__endpoint_config_idx,
            &service_mgr__l_locales);
         address_space_itf__treat_write_request(service_mgr__l_user,
            service_mgr__l_locales,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_browse_req:
         service_set_view__treat_browse_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_browse_next_req:
         service_set_view__treat_browse_next_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
         service_set_view__treat_translate_browse_paths_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_register_nodes_req:
         service_register_nodes__treat_register_nodes_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_view_unregister_nodes_req:
         service_unregister_nodes__treat_unregister_nodes_request(service_mgr__req_msg,
            service_mgr__StatusCode_service);
         break;
      default:
         *service_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         break;
      }
   }
}

void service_mgr__treat_session_nano_extended_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants__t_request_context_i service_mgr__req_ctx,
   const constants__t_msg_header_i service_mgr__req_header,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service,
   t_bool * const service_mgr__async_resp_msg) {
   {
      constants__t_user_i service_mgr__l_user;
      
      *service_mgr__async_resp_msg = false;
      switch (service_mgr__req_typ) {
      case constants__e_msg_attribute_read_req:
      case constants__e_msg_attribute_write_req:
      case constants__e_msg_view_browse_req:
      case constants__e_msg_view_browse_next_req:
      case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
      case constants__e_msg_view_register_nodes_req:
      case constants__e_msg_view_unregister_nodes_req:
         service_mgr__treat_session_nano_service_req(service_mgr__endpoint_config_idx,
            service_mgr__session,
            service_mgr__req_typ,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_subscription_create_req:
         subscription_mgr__treat_create_subscription_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_subscription_modify_req:
         subscription_mgr__treat_modify_subscription_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_subscription_delete_subscriptions_req:
         subscription_mgr__treat_delete_subscriptions_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_subscription_set_publishing_mode_req:
         subscription_mgr__treat_publishing_mode_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_subscription_publish_req:
         subscription_mgr__treat_subscription_publish_request(service_mgr__session,
            service_mgr__req_header,
            service_mgr__req_msg,
            service_mgr__req_handle,
            service_mgr__req_ctx,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service,
            service_mgr__async_resp_msg);
         break;
      case constants__e_msg_subscription_republish_req:
         subscription_mgr__treat_subscription_republish_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_monitored_items_create_req:
         session_mgr__get_session_user_server(service_mgr__session,
            &service_mgr__l_user);
         subscription_mgr__treat_subscription_create_monitored_items_req(service_mgr__session,
            service_mgr__l_user,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      case constants__e_msg_method_call_req:
         call_method_mgr__treat_method_call_request(service_mgr__session,
            service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__StatusCode_service);
         break;
      default:
         *service_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         break;
      }
   }
}

void service_mgr__treat_session_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants__t_request_context_i service_mgr__req_ctx,
   const constants__t_msg_header_i service_mgr__req_header,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service,
   t_bool * const service_mgr__async_resp_msg) {
   *service_mgr__async_resp_msg = false;
   if (constants__c_Server_Nano_Extended == true) {
      service_mgr__treat_session_nano_extended_service_req(service_mgr__endpoint_config_idx,
         service_mgr__session,
         service_mgr__req_typ,
         service_mgr__req_handle,
         service_mgr__req_ctx,
         service_mgr__req_header,
         service_mgr__req_msg,
         service_mgr__resp_msg,
         service_mgr__StatusCode_service,
         service_mgr__async_resp_msg);
   }
   else {
      service_mgr__treat_session_nano_service_req(service_mgr__endpoint_config_idx,
         service_mgr__session,
         service_mgr__req_typ,
         service_mgr__req_msg,
         service_mgr__resp_msg,
         service_mgr__StatusCode_service);
   }
}

void service_mgr__treat_discovery_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      
      switch (service_mgr__req_typ) {
      case constants__e_msg_discovery_get_endpoints_req:
         service_get_endpoints_bs__treat_get_endpoints_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__endpoint_config_idx,
            &service_mgr__l_ret);
         break;
      case constants__e_msg_discovery_find_servers_req:
         service_set_discovery_server__treat_find_servers_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            service_mgr__endpoint_config_idx,
            &service_mgr__l_ret);
         break;
      case constants__e_msg_discovery_find_servers_on_network_req:
         service_set_discovery_server__treat_find_servers_on_network_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            &service_mgr__l_ret);
         break;
      case constants__e_msg_discovery_register_server2_req:
         service_set_discovery_server__treat_register_server2_request(service_mgr__req_msg,
            service_mgr__resp_msg,
            &service_mgr__l_ret);
         break;
      default:
         service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_service_unsupported;
         break;
      }
      *service_mgr__StatusCode_service = service_mgr__l_ret;
   }
}

void service_mgr__local_client_discovery_service_request(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_type_i service_mgr__l_req_typ;
      constants__t_msg_type_i service_mgr__l_resp_typ;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__get_msg_out_type(service_mgr__req_msg,
         &service_mgr__l_req_typ);
      switch (service_mgr__l_req_typ) {
      case constants__e_msg_discovery_find_servers_req:
      case constants__e_msg_discovery_find_servers_on_network_req:
      case constants__e_msg_discovery_get_endpoints_req:
      case constants__e_msg_discovery_register_server_req:
      case constants__e_msg_discovery_register_server2_req:
         service_mgr__get_response_type(service_mgr__l_req_typ,
            &service_mgr__l_resp_typ);
         message_out_bs__alloc_msg_header(true,
            &service_mgr__l_msg_header);
         message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
            &service_mgr__l_valid_msg_header);
         request_handle_bs__client_fresh_req_handle(service_mgr__l_req_typ,
            service_mgr__l_resp_typ,
            true,
            service_mgr__app_context,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if ((service_mgr__l_valid_req_handle == true) &&
            (service_mgr__l_valid_msg_header == true)) {
            request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
               service_mgr__channel);
            message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
               service_mgr__l_req_handle);
            channel_mgr__get_channel_info(service_mgr__channel,
               &service_mgr__l_channel_cfg);
            message_out_bs__encode_msg(service_mgr__l_channel_cfg,
               constants__e_msg_request_type,
               service_mgr__l_req_typ,
               service_mgr__l_msg_header,
               service_mgr__req_msg,
               service_mgr__ret,
               &service_mgr__l_buffer);
            message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
               &service_mgr__l_valid_buffer);
            if (service_mgr__l_valid_buffer == false) {
               service_mgr__l_req_handle = constants__c_client_request_handle_indet;
            }
         }
         else {
            service_mgr__l_req_handle = constants__c_client_request_handle_indet;
            *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         if (service_mgr__l_valid_msg_header == true) {
            message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         }
         break;
      default:
         *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         break;
      }
      *service_mgr__buffer_out = service_mgr__l_buffer;
      *service_mgr__req_handle = service_mgr__l_req_handle;
   }
}

void service_mgr__dealloc_msg_in_header_if_cond(
   const t_bool service_mgr__p_cond,
   const constants__t_msg_header_i service_mgr__p_req_msg_header) {
   if (service_mgr__p_cond == true) {
      message_in_bs__dealloc_msg_in_header(service_mgr__p_req_msg_header);
   }
}

void service_mgr__client_service_fault_to_resp_type(
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid,
   constants__t_msg_type_i * const service_mgr__resp_typ) {
   {
      constants__t_client_request_handle_i service_mgr__l_request_handle;
      
      message_in_bs__decode_service_fault_msg_req_handle(service_mgr__msg_buffer,
         &service_mgr__l_request_handle);
      request_handle_bs__is_valid_req_handle(service_mgr__l_request_handle,
         service_mgr__valid);
      if (*service_mgr__valid == true) {
         request_handle_bs__get_req_handle_resp_typ(service_mgr__l_request_handle,
            service_mgr__resp_typ);
      }
      else {
         *service_mgr__resp_typ = constants__c_msg_type_indet;
      }
   }
}

void service_mgr__server_receive_session_treatment_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      constants__t_server_request_handle_i service_mgr__l_request_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_session_i service_mgr__l_session;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_valid_req = false;
      *service_mgr__sc = constants_statuscodes_bs__c_StatusCode_indet;
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__server_read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         message_in_bs__read_msg_req_header_session_token(service_mgr__l_req_msg_header,
            &service_mgr__l_session_token);
         message_in_bs__decode_msg(service_mgr__req_typ,
            service_mgr__msg_buffer,
            &service_mgr__l_req_msg);
         message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
            &service_mgr__l_valid_req);
         if (service_mgr__l_valid_req == true) {
            service_mgr__get_response_type(service_mgr__req_typ,
               &service_mgr__l_resp_msg_typ);
            message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
               &service_mgr__l_resp_msg_header,
               &service_mgr__l_resp_msg);
            message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
               &service_mgr__l_valid_msg);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
               &service_mgr__l_valid_resp_header);
            if ((service_mgr__l_valid_msg == true) &&
               (service_mgr__l_valid_resp_header == true)) {
               session_mgr__server_receive_session_req(service_mgr__channel,
                  service_mgr__l_session_token,
                  service_mgr__l_req_msg,
                  service_mgr__req_typ,
                  service_mgr__l_resp_msg,
                  &service_mgr__l_session,
                  &service_mgr__l_ret);
               if (service_mgr__l_ret != constants_statuscodes_bs__e_sc_ok) {
                  service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
               }
               message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                  service_mgr__l_ret);
               message_out_bs__server_write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                  service_mgr__l_request_handle);
               channel_mgr__get_channel_info(service_mgr__channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_response_type,
                  service_mgr__l_resp_msg_typ,
                  service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg,
                  service_mgr__sc,
                  &service_mgr__l_buffer_out);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer_out,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == false) {
                  session_mgr__server_close_session(service_mgr__l_session,
                     constants_statuscodes_bs__e_sc_bad_encoding_error);
               }
               message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
               message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
            }
            else {
               *service_mgr__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
            message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
      *service_mgr__valid_req = ((service_mgr__l_valid_req_header == true) &&
         (service_mgr__l_valid_req == true));
   }
}

void service_mgr__client_receive_session_treatment_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp;
      constants__t_client_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      constants__t_session_i service_mgr__l_session;
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
         &service_mgr__l_resp_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_resp_msg_header,
         &service_mgr__l_valid_resp_header);
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__client_read_msg_header_req_handle(service_mgr__l_resp_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__client_validate_response_request_handle(service_mgr__channel,
            service_mgr__l_request_handle,
            service_mgr__resp_typ,
            &service_mgr__l_validated_req_handle);
         if (service_mgr__l_validated_req_handle == true) {
            message_in_bs__decode_msg(service_mgr__resp_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_resp_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
               &service_mgr__l_valid_resp);
            if (service_mgr__l_valid_resp == true) {
               session_mgr__client_receive_session_resp(service_mgr__channel,
                  service_mgr__l_request_handle,
                  service_mgr__resp_typ,
                  service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg,
                  &service_mgr__l_session);
               request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
               message_in_bs__dealloc_msg_in(service_mgr__l_resp_msg);
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
      }
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_session_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_request_context_i service_mgr__req_context,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   t_bool * const service_mgr__async_resp,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      constants__t_server_request_handle_i service_mgr__l_request_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      t_bool service_mgr__l_is_valid_req_on_session;
      constants__t_session_i service_mgr__l_session;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      t_bool service_mgr__l_resp_msg_allocated;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret2;
      t_bool service_mgr__l_is_valid_resp;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      constants__t_channel_i service_mgr__l_session_channel;
      constants__t_endpoint_config_idx_i service_mgr__l_endpoint_config_idx;
      t_bool service_mgr__l_is_valid_ep_config_idx;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      *service_mgr__sc = constants_statuscodes_bs__c_StatusCode_indet;
      service_mgr__l_is_valid_req_on_session = false;
      service_mgr__l_valid_req = false;
      service_mgr__l_resp_msg_typ = constants__c_msg_type_indet;
      service_mgr__l_resp_msg_header = constants__c_msg_header_indet;
      service_mgr__l_resp_msg = constants__c_msg_indet;
      *service_mgr__async_resp = false;
      service_mgr__l_valid_msg = false;
      service_mgr__l_valid_resp_header = false;
      service_mgr__l_resp_msg_allocated = false;
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__server_read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         message_in_bs__read_msg_req_header_session_token(service_mgr__l_req_msg_header,
            &service_mgr__l_session_token);
         session_mgr__server_validate_session_service_req(service_mgr__channel,
            service_mgr__l_session_token,
            &service_mgr__l_is_valid_req_on_session,
            &service_mgr__l_session,
            &service_mgr__l_ret);
         channel_mgr__server_get_endpoint_config(service_mgr__channel,
            &service_mgr__l_endpoint_config_idx);
         channel_mgr__is_valid_endpoint_config_idx(service_mgr__l_endpoint_config_idx,
            &service_mgr__l_is_valid_ep_config_idx);
         if ((service_mgr__l_is_valid_req_on_session == true) &&
            (service_mgr__l_is_valid_ep_config_idx == true)) {
            message_in_bs__decode_msg(service_mgr__req_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_req_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
               &service_mgr__l_valid_req);
            if (service_mgr__l_valid_req == true) {
               service_mgr__get_response_type(service_mgr__req_typ,
                  &service_mgr__l_resp_msg_typ);
               message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
                  &service_mgr__l_resp_msg_header,
                  &service_mgr__l_resp_msg);
               message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
                  &service_mgr__l_valid_msg);
               message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
                  &service_mgr__l_valid_resp_header);
               if ((service_mgr__l_valid_msg == true) &&
                  (service_mgr__l_valid_resp_header == true)) {
                  service_mgr__l_resp_msg_allocated = true;
                  service_mgr__treat_session_service_req(service_mgr__l_endpoint_config_idx,
                     service_mgr__l_session,
                     service_mgr__req_typ,
                     service_mgr__l_request_handle,
                     service_mgr__req_context,
                     service_mgr__l_req_msg_header,
                     service_mgr__l_req_msg,
                     service_mgr__l_resp_msg,
                     &service_mgr__l_ret,
                     service_mgr__async_resp);
                  session_mgr__server_validate_session_service_resp(service_mgr__l_session,
                     &service_mgr__l_is_valid_resp,
                     &service_mgr__l_ret2,
                     &service_mgr__l_session_channel);
                  if (service_mgr__l_ret2 != constants_statuscodes_bs__e_sc_ok) {
                     service_mgr__l_ret = service_mgr__l_ret2;
                  }
                  else if (service_mgr__l_session_channel != service_mgr__channel) {
                     service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
                  }
               }
               message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
            }
            else {
               service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_decoding_error;
            }
         }
         else {
            service_mgr__l_valid_req = true;
         }
         if (service_mgr__l_resp_msg_allocated == false) {
            message_out_bs__alloc_resp_msg(constants__e_msg_service_fault_resp,
               &service_mgr__l_resp_msg_header,
               &service_mgr__l_resp_msg);
            message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
               &service_mgr__l_valid_msg);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
               &service_mgr__l_valid_resp_header);
         }
         if (((service_mgr__l_valid_msg == true) &&
            (service_mgr__l_valid_resp_header == true)) &&
            (*service_mgr__async_resp == false)) {
            if (service_mgr__l_ret != constants_statuscodes_bs__e_sc_ok) {
               service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
            }
            message_out_bs__server_write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
               service_mgr__l_request_handle);
            message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
               service_mgr__l_ret);
            channel_mgr__get_channel_info(service_mgr__channel,
               &service_mgr__l_channel_cfg);
            message_out_bs__encode_msg(service_mgr__l_channel_cfg,
               constants__e_msg_response_type,
               service_mgr__l_resp_msg_typ,
               service_mgr__l_resp_msg_header,
               service_mgr__l_resp_msg,
               service_mgr__sc,
               &service_mgr__l_buffer_out);
            message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
            message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
         }
         else if (((service_mgr__l_valid_msg == true) &&
            (service_mgr__l_valid_resp_header == true)) &&
            (*service_mgr__async_resp == true)) {
            message_out_bs__forget_resp_msg_out(service_mgr__l_resp_msg_header,
               service_mgr__l_resp_msg);
         }
         else {
            *service_mgr__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
      *service_mgr__valid_req = ((service_mgr__l_valid_req_header == true) &&
         (service_mgr__l_valid_req == true));
   }
}

void service_mgr__client_receive_session_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_client_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      constants__t_application_context_i service_mgr__l_user_app_context;
      t_bool service_mgr__l_is_applicative_response;
      t_bool service_mgr__l_is_valid_session_resp;
      constants__t_session_i service_mgr__l_session;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp_msg;
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
         &service_mgr__l_resp_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_resp_msg_header,
         &service_mgr__l_valid_resp_header);
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__client_read_msg_header_req_handle(service_mgr__l_resp_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__client_validate_response_request_handle(service_mgr__channel,
            service_mgr__l_request_handle,
            service_mgr__resp_typ,
            &service_mgr__l_validated_req_handle);
         if (service_mgr__l_validated_req_handle == true) {
            request_handle_bs__get_req_handle_app_context(service_mgr__l_request_handle,
               &service_mgr__l_is_applicative_response,
               &service_mgr__l_user_app_context);
            session_mgr__client_validate_session_service_resp(service_mgr__channel,
               service_mgr__l_request_handle,
               &service_mgr__l_is_valid_session_resp,
               &service_mgr__l_session);
            if (service_mgr__l_is_valid_session_resp == true) {
               message_in_bs__decode_msg(service_mgr__resp_typ,
                  service_mgr__msg_buffer,
                  &service_mgr__l_resp_msg);
               message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
                  &service_mgr__l_valid_resp_msg);
               if ((service_mgr__l_valid_resp_msg == true) &&
                  (service_mgr__l_is_applicative_response == true)) {
                  message_in_bs__copy_msg_resp_header_into_msg(service_mgr__l_resp_msg_header,
                     service_mgr__l_resp_msg);
                  service_response_cb_bs__cli_service_response(service_mgr__l_session,
                     service_mgr__l_resp_msg,
                     service_mgr__l_user_app_context);
                  message_in_bs__forget_resp_msg_in(service_mgr__l_resp_msg_header,
                     service_mgr__l_resp_msg);
               }
               else {
                  message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
               }
            }
            else {
               message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
         else {
            message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
         }
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_discovery_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out) {
   {
      constants__t_msg_header_i service_mgr__l_req_msg_header;
      t_bool service_mgr__l_valid_req_header;
      constants__t_server_request_handle_i service_mgr__l_request_handle;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_req;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_endpoint_config_idx_i service_mgr__l_endpoint_config_idx;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      *service_mgr__sc = constants_statuscodes_bs__c_StatusCode_indet;
      service_mgr__l_valid_req = false;
      service_mgr__l_buffer_out = constants__c_byte_buffer_indet;
      message_in_bs__decode_msg_header(true,
         service_mgr__msg_buffer,
         &service_mgr__l_req_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_req_msg_header,
         &service_mgr__l_valid_req_header);
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__server_read_msg_header_req_handle(service_mgr__l_req_msg_header,
            &service_mgr__l_request_handle);
         message_in_bs__decode_msg(service_mgr__req_typ,
            service_mgr__msg_buffer,
            &service_mgr__l_req_msg);
         message_in_bs__is_valid_msg_in(service_mgr__l_req_msg,
            &service_mgr__l_valid_req);
         if (service_mgr__l_valid_req == true) {
            service_mgr__get_response_type(service_mgr__req_typ,
               &service_mgr__l_resp_msg_typ);
            message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
               &service_mgr__l_resp_msg_header,
               &service_mgr__l_resp_msg);
            message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
               &service_mgr__l_valid_msg);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
               &service_mgr__l_valid_resp_header);
            if ((service_mgr__l_valid_msg == true) &&
               (service_mgr__l_valid_resp_header == true)) {
               channel_mgr__server_get_endpoint_config(service_mgr__channel,
                  &service_mgr__l_endpoint_config_idx);
               service_mgr__treat_discovery_service_req(service_mgr__l_endpoint_config_idx,
                  service_mgr__req_typ,
                  service_mgr__l_req_msg,
                  service_mgr__l_resp_msg,
                  &service_mgr__l_ret);
               message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
                  service_mgr__l_ret);
               if (service_mgr__l_ret != constants_statuscodes_bs__e_sc_ok) {
                  service_mgr__l_resp_msg_typ = constants__e_msg_service_fault_resp;
               }
               message_out_bs__server_write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
                  service_mgr__l_request_handle);
               channel_mgr__get_channel_info(service_mgr__channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_response_type,
                  service_mgr__l_resp_msg_typ,
                  service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg,
                  service_mgr__sc,
                  &service_mgr__l_buffer_out);
               message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
               message_out_bs__dealloc_msg_out(service_mgr__l_resp_msg);
            }
            else {
               *service_mgr__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
            message_in_bs__dealloc_msg_in(service_mgr__l_req_msg);
         }
      }
      if (service_mgr__l_valid_req_header == true) {
         message_in_bs__dealloc_msg_in_header(service_mgr__l_req_msg_header);
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
      *service_mgr__buffer_out = service_mgr__l_buffer_out;
      *service_mgr__valid_req = ((service_mgr__l_valid_req_header == true) &&
         (service_mgr__l_valid_req == true));
   }
}

void service_mgr__client_receive_discovery_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer) {
   {
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      t_bool service_mgr__l_valid_resp_header;
      constants__t_client_request_handle_i service_mgr__l_request_handle;
      t_bool service_mgr__l_validated_req_handle;
      t_bool service_mgr__l_is_applicative_response;
      constants__t_application_context_i service_mgr__l_user_app_context;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_resp_msg;
      
      message_in_bs__decode_msg_header(false,
         service_mgr__msg_buffer,
         &service_mgr__l_resp_msg_header);
      message_in_bs__is_valid_msg_in_header(service_mgr__l_resp_msg_header,
         &service_mgr__l_valid_resp_header);
      if (service_mgr__l_valid_resp_header == true) {
         message_in_bs__client_read_msg_header_req_handle(service_mgr__l_resp_msg_header,
            &service_mgr__l_request_handle);
         request_handle_bs__client_validate_response_request_handle(service_mgr__channel,
            service_mgr__l_request_handle,
            service_mgr__resp_typ,
            &service_mgr__l_validated_req_handle);
         if (service_mgr__l_validated_req_handle == true) {
            request_handle_bs__get_req_handle_app_context(service_mgr__l_request_handle,
               &service_mgr__l_is_applicative_response,
               &service_mgr__l_user_app_context);
            message_in_bs__decode_msg(service_mgr__resp_typ,
               service_mgr__msg_buffer,
               &service_mgr__l_resp_msg);
            message_in_bs__is_valid_msg_in(service_mgr__l_resp_msg,
               &service_mgr__l_valid_resp_msg);
            if ((service_mgr__l_valid_resp_msg == true) &&
               (service_mgr__l_is_applicative_response == true)) {
               message_in_bs__copy_msg_resp_header_into_msg(service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg);
               service_response_cb_bs__cli_service_response(constants__c_session_indet,
                  service_mgr__l_resp_msg,
                  service_mgr__l_user_app_context);
               message_in_bs__forget_resp_msg_in(service_mgr__l_resp_msg_header,
                  service_mgr__l_resp_msg);
            }
            else {
               message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
            }
            request_handle_bs__client_remove_req_handle(service_mgr__l_request_handle);
         }
         else {
            message_in_bs__dealloc_msg_in_header(service_mgr__l_resp_msg_header);
         }
      }
      message_in_bs__dealloc_msg_in_buffer(service_mgr__msg_buffer);
   }
}

void service_mgr__server_receive_local_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_service_class_i service_mgr__req_class,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret) {
   {
      t_bool service_mgr__l_prev_local_treatment;
      constants__t_msg_type_i service_mgr__l_resp_msg_typ;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      constants__t_msg_i service_mgr__l_resp_msg;
      t_bool service_mgr__l_valid_msg;
      t_bool service_mgr__l_valid_resp_header;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      
      address_space_itf__is_local_service_treatment(&service_mgr__l_prev_local_treatment);
      address_space_itf__set_local_service_treatment(true);
      switch (service_mgr__req_class) {
      case constants__e_msg_session_service_class:
      case constants__e_msg_discovery_service_class:
         service_mgr__get_response_type(service_mgr__req_typ,
            &service_mgr__l_resp_msg_typ);
         message_out_bs__alloc_resp_msg(service_mgr__l_resp_msg_typ,
            &service_mgr__l_resp_msg_header,
            &service_mgr__l_resp_msg);
         message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
            &service_mgr__l_valid_msg);
         message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
            &service_mgr__l_valid_resp_header);
         break;
      default:
         message_out_bs__alloc_resp_msg(constants__e_msg_service_fault_resp,
            &service_mgr__l_resp_msg_header,
            &service_mgr__l_resp_msg);
         message_out_bs__is_valid_msg_out(service_mgr__l_resp_msg,
            &service_mgr__l_valid_msg);
         message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
            &service_mgr__l_valid_resp_header);
         break;
      }
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_resp_header == true)) {
         *service_mgr__ret = constants_statuscodes_bs__e_sc_ok;
         message_in_bs__bless_msg_in(service_mgr__req_msg);
         switch (service_mgr__req_class) {
         case constants__e_msg_session_service_class:
            service_mgr__treat_session_local_service_req(service_mgr__endpoint_config_idx,
               service_mgr__req_typ,
               service_mgr__req_msg,
               service_mgr__l_resp_msg,
               &service_mgr__l_ret);
            break;
         case constants__e_msg_discovery_service_class:
            service_mgr__treat_discovery_service_req(service_mgr__endpoint_config_idx,
               service_mgr__req_typ,
               service_mgr__req_msg,
               service_mgr__l_resp_msg,
               &service_mgr__l_ret);
            break;
         default:
            service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_service_unsupported;
            break;
         }
         message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
            service_mgr__l_ret);
         message_out_bs__copy_msg_resp_header_into_msg_out(service_mgr__l_resp_msg_header,
            service_mgr__l_resp_msg);
         service_response_cb_bs__srv_service_response(service_mgr__endpoint_config_idx,
            service_mgr__l_resp_msg,
            service_mgr__app_context);
         message_out_bs__forget_resp_msg_out(service_mgr__l_resp_msg_header,
            service_mgr__l_resp_msg);
         message_in_bs__dealloc_msg_in(service_mgr__req_msg);
      }
      else {
         *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
      address_space_itf__set_local_service_treatment(service_mgr__l_prev_local_treatment);
   }
}

void service_mgr__client_service_create_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      t_bool service_mgr__l_valid_msg;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_sc;
      t_bool service_mgr__l_valid_buffer;
      t_bool service_mgr__l_bret;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__alloc_req_msg(constants__e_msg_session_create_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg_header == true) &&
         (service_mgr__l_valid_msg == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
            constants__e_msg_session_create_resp,
            false,
            constants__c_no_application_context,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
               service_mgr__channel);
            session_mgr__client_create_session_req(service_mgr__session,
               service_mgr__channel,
               service_mgr__l_req_handle,
               service_mgr__l_req_msg,
               &service_mgr__l_bret);
            if (service_mgr__l_bret == true) {
               message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               channel_mgr__get_channel_info(service_mgr__channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_request_type,
                  constants__e_msg_session_create_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  &service_mgr__l_sc,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session,
                     service_mgr__l_sc);
               }
            }
            else {
               request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
               session_mgr__client_close_session(service_mgr__session,
                  constants_statuscodes_bs__e_sc_bad_internal_error);
            }
         }
         else {
            session_mgr__client_close_session(service_mgr__session,
               constants_statuscodes_bs__e_sc_bad_out_of_memory);
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      *service_mgr__buffer_out = service_mgr__l_buffer;
      *service_mgr__req_handle = service_mgr__l_req_handle;
   }
}

void service_mgr__client_service_activate_orphaned_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_sc;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_msg_header == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_activate_req,
            constants__e_msg_session_activate_resp,
            false,
            constants__c_no_application_context,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
               service_mgr__channel);
            session_mgr__client_sc_activate_session_req(service_mgr__session,
               service_mgr__l_req_handle,
               service_mgr__channel,
               service_mgr__l_req_msg,
               &service_mgr__l_ret,
               &service_mgr__l_session_token);
            if (service_mgr__l_ret == constants_statuscodes_bs__e_sc_ok) {
               message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                  service_mgr__l_session_token);
               channel_mgr__get_channel_info(service_mgr__channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_request_type,
                  constants__e_msg_session_activate_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  &service_mgr__l_sc,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session,
                     service_mgr__l_sc);
               }
            }
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      else {
         session_mgr__client_close_session(service_mgr__session,
            constants_statuscodes_bs__e_sc_bad_out_of_memory);
      }
      *service_mgr__buffer_out = service_mgr__l_buffer;
      *service_mgr__req_handle = service_mgr__l_req_handle;
   }
}

void service_mgr__client_service_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_token_i service_mgr__p_user_token,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__alloc_req_msg(constants__e_msg_session_activate_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_msg_header == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_activate_req,
            constants__e_msg_session_activate_resp,
            false,
            constants__c_no_application_context,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            session_mgr__client_user_activate_session_req(service_mgr__session,
               service_mgr__l_req_handle,
               service_mgr__p_user_token,
               service_mgr__l_req_msg,
               &service_mgr__l_ret,
               &service_mgr__l_channel,
               &service_mgr__l_session_token);
            if (service_mgr__l_ret == constants_statuscodes_bs__e_sc_ok) {
               request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
                  service_mgr__l_channel);
               message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                  service_mgr__l_session_token);
               channel_mgr__get_channel_info(service_mgr__l_channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_request_type,
                  constants__e_msg_session_activate_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  &service_mgr__l_ret,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == false) {
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  service_mgr__l_channel = constants__c_channel_indet;
                  session_mgr__client_close_session(service_mgr__session,
                     service_mgr__l_ret);
               }
            }
            else {
               session_mgr__client_close_session(service_mgr__session,
                  service_mgr__l_ret);
            }
         }
         else {
            session_mgr__client_close_session(service_mgr__session,
               constants_statuscodes_bs__e_sc_bad_out_of_memory);
            service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      else {
         session_mgr__client_close_session(service_mgr__session,
            constants_statuscodes_bs__e_sc_bad_out_of_memory);
         service_mgr__l_ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
      *service_mgr__ret = service_mgr__l_ret;
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__req_handle = service_mgr__l_req_handle;
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_service_close_session(
   const constants__t_session_i service_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_i service_mgr__l_req_msg;
      t_bool service_mgr__l_valid_msg;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__alloc_req_msg(constants__e_msg_session_close_req,
         &service_mgr__l_msg_header,
         &service_mgr__l_req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__l_req_msg,
         &service_mgr__l_valid_msg);
      message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
         &service_mgr__l_valid_msg_header);
      if ((service_mgr__l_valid_msg == true) &&
         (service_mgr__l_valid_msg_header == true)) {
         request_handle_bs__client_fresh_req_handle(constants__e_msg_session_close_req,
            constants__e_msg_session_close_resp,
            false,
            constants__c_no_application_context,
            &service_mgr__l_req_handle);
         request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
            &service_mgr__l_valid_req_handle);
         if (service_mgr__l_valid_req_handle == true) {
            session_mgr__client_close_session_req(service_mgr__session,
               service_mgr__l_req_handle,
               service_mgr__l_req_msg,
               service_mgr__ret,
               &service_mgr__l_channel,
               &service_mgr__l_session_token);
            if (*service_mgr__ret == constants_statuscodes_bs__e_sc_ok) {
               request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
                  service_mgr__l_channel);
               message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
                  service_mgr__l_req_handle);
               message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                  service_mgr__l_session_token);
               channel_mgr__get_channel_info(service_mgr__l_channel,
                  &service_mgr__l_channel_cfg);
               message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                  constants__e_msg_request_type,
                  constants__e_msg_session_close_req,
                  service_mgr__l_msg_header,
                  service_mgr__l_req_msg,
                  service_mgr__ret,
                  &service_mgr__l_buffer);
               message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                  &service_mgr__l_valid_buffer);
               if (service_mgr__l_valid_buffer == false) {
                  service_mgr__l_channel = constants__c_channel_indet;
                  request_handle_bs__client_remove_req_handle(service_mgr__l_req_handle);
                  session_mgr__client_close_session(service_mgr__session,
                     *service_mgr__ret);
               }
            }
         }
         else {
            *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
         message_out_bs__dealloc_msg_out(service_mgr__l_req_msg);
      }
      else {
         *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__buffer_out = service_mgr__l_buffer;
      *service_mgr__req_handle = service_mgr__l_req_handle;
   }
}

void service_mgr__client_service_request(
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      constants__t_msg_header_i service_mgr__l_msg_header;
      t_bool service_mgr__l_valid_msg_header;
      constants__t_msg_type_i service_mgr__l_req_typ;
      constants__t_msg_type_i service_mgr__l_resp_typ;
      t_bool service_mgr__l_valid_msg;
      constants__t_channel_i service_mgr__l_channel;
      constants__t_client_request_handle_i service_mgr__l_req_handle;
      t_bool service_mgr__l_valid_req_handle;
      constants__t_session_token_i service_mgr__l_session_token;
      constants__t_byte_buffer_i service_mgr__l_buffer;
      t_bool service_mgr__l_valid_buffer;
      t_bool service_mgr__l_valid_channel;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      service_mgr__l_channel = constants__c_channel_indet;
      service_mgr__l_buffer = constants__c_byte_buffer_indet;
      service_mgr__l_req_handle = constants__c_client_request_handle_indet;
      message_out_bs__bless_msg_out(service_mgr__req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__req_msg,
         &service_mgr__l_valid_msg);
      if (service_mgr__l_valid_msg == true) {
         message_out_bs__get_msg_out_type(service_mgr__req_msg,
            &service_mgr__l_req_typ);
         switch (service_mgr__l_req_typ) {
         case constants__e_msg_node_add_nodes_req:
         case constants__e_msg_node_add_references_req:
         case constants__e_msg_node_delete_nodes_req:
         case constants__e_msg_node_delete_references_req:
         case constants__e_msg_view_browse_req:
         case constants__e_msg_view_browse_next_req:
         case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
         case constants__e_msg_view_register_nodes_req:
         case constants__e_msg_view_unregister_nodes_req:
         case constants__e_msg_query_first_req:
         case constants__e_msg_query_next_req:
         case constants__e_msg_attribute_read_req:
         case constants__e_msg_attribute_history_read_req:
         case constants__e_msg_attribute_write_req:
         case constants__e_msg_attribute_history_update_req:
         case constants__e_msg_method_call_req:
         case constants__e_msg_monitored_items_create_req:
         case constants__e_msg_monitored_items_modify_req:
         case constants__e_msg_monitored_items_set_monitoring_mode_req:
         case constants__e_msg_monitored_items_set_monitoring_mode_resp:
         case constants__e_msg_monitored_items_set_triggering_req:
         case constants__e_msg_monitored_items_delete_req:
         case constants__e_msg_subscription_create_req:
         case constants__e_msg_subscription_modify_req:
         case constants__e_msg_subscription_set_publishing_mode_req:
         case constants__e_msg_subscription_publish_req:
         case constants__e_msg_subscription_republish_req:
         case constants__e_msg_subscription_transfer_subscriptions_req:
         case constants__e_msg_subscription_delete_subscriptions_req:
            service_mgr__get_response_type(service_mgr__l_req_typ,
               &service_mgr__l_resp_typ);
            message_out_bs__alloc_msg_header(true,
               &service_mgr__l_msg_header);
            message_out_bs__is_valid_msg_out_header(service_mgr__l_msg_header,
               &service_mgr__l_valid_msg_header);
            request_handle_bs__client_fresh_req_handle(service_mgr__l_req_typ,
               service_mgr__l_resp_typ,
               true,
               service_mgr__app_context,
               &service_mgr__l_req_handle);
            request_handle_bs__is_valid_req_handle(service_mgr__l_req_handle,
               &service_mgr__l_valid_req_handle);
            if ((service_mgr__l_valid_req_handle == true) &&
               (service_mgr__l_valid_msg_header == true)) {
               session_mgr__client_validate_session_service_req(service_mgr__session,
                  service_mgr__l_req_handle,
                  service_mgr__ret,
                  &service_mgr__l_channel,
                  &service_mgr__l_session_token);
               if (*service_mgr__ret == constants_statuscodes_bs__e_sc_ok) {
                  request_handle_bs__set_req_handle_channel(service_mgr__l_req_handle,
                     service_mgr__l_channel);
                  message_out_bs__client_write_msg_out_header_req_handle(service_mgr__l_msg_header,
                     service_mgr__l_req_handle);
                  message_out_bs__write_msg_out_header_session_token(service_mgr__l_msg_header,
                     service_mgr__l_session_token);
                  channel_mgr__get_channel_info(service_mgr__l_channel,
                     &service_mgr__l_channel_cfg);
                  message_out_bs__encode_msg(service_mgr__l_channel_cfg,
                     constants__e_msg_request_type,
                     constants__e_msg_session_close_req,
                     service_mgr__l_msg_header,
                     service_mgr__req_msg,
                     service_mgr__ret,
                     &service_mgr__l_buffer);
                  message_out_bs__is_valid_buffer_out(service_mgr__l_buffer,
                     &service_mgr__l_valid_buffer);
                  if (service_mgr__l_valid_buffer == false) {
                     service_mgr__l_channel = constants__c_channel_indet;
                  }
               }
            }
            else {
               *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
            if (service_mgr__l_valid_msg_header == true) {
               message_out_bs__dealloc_msg_header_out(service_mgr__l_msg_header);
            }
            break;
         case constants__e_msg_discovery_find_servers_req:
         case constants__e_msg_discovery_find_servers_on_network_req:
         case constants__e_msg_discovery_get_endpoints_req:
         case constants__e_msg_discovery_register_server_req:
         case constants__e_msg_discovery_register_server2_req:
            session_mgr__getall_valid_session_channel(service_mgr__session,
               &service_mgr__l_valid_channel,
               &service_mgr__l_channel);
            if (service_mgr__l_valid_channel == true) {
               service_mgr__local_client_discovery_service_request(service_mgr__l_channel,
                  service_mgr__req_msg,
                  service_mgr__app_context,
                  service_mgr__ret,
                  &service_mgr__l_buffer,
                  &service_mgr__l_req_handle);
               if (*service_mgr__ret != constants_statuscodes_bs__e_sc_ok) {
                  service_mgr__l_channel = constants__c_channel_indet;
               }
            }
            else {
               *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
            }
            break;
         default:
            *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
            break;
         }
      }
      else {
         *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
      }
      message_out_bs__dealloc_msg_out(service_mgr__req_msg);
      *service_mgr__channel = service_mgr__l_channel;
      *service_mgr__req_handle = service_mgr__l_req_handle;
      *service_mgr__buffer_out = service_mgr__l_buffer;
   }
}

void service_mgr__client_discovery_service_request(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle) {
   {
      t_bool service_mgr__l_valid_msg;
      
      message_out_bs__bless_msg_out(service_mgr__req_msg);
      message_out_bs__is_valid_msg_out(service_mgr__req_msg,
         &service_mgr__l_valid_msg);
      if (service_mgr__l_valid_msg == true) {
         service_mgr__local_client_discovery_service_request(service_mgr__channel,
            service_mgr__req_msg,
            service_mgr__app_context,
            service_mgr__ret,
            service_mgr__buffer_out,
            service_mgr__req_handle);
      }
      else {
         *service_mgr__ret = constants_statuscodes_bs__e_sc_bad_invalid_argument;
         *service_mgr__buffer_out = constants__c_byte_buffer_indet;
         *service_mgr__req_handle = constants__c_client_request_handle_indet;
      }
      message_out_bs__dealloc_msg_out(service_mgr__req_msg);
   }
}

void service_mgr__client_snd_msg_failure(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_client_request_handle_i service_mgr__request_handle,
   const constants_statuscodes_bs__t_StatusCode_i service_mgr__error_status) {
   {
      t_bool service_mgr__l_valid_req_handle;
      constants__t_channel_i service_mgr__l_req_handle_channel;
      constants__t_msg_type_i service_mgr__l_req_typ;
      t_bool service_mgr__l_is_applicative;
      constants__t_application_context_i service_mgr__l_app_context;
      constants__t_msg_type_i service_mgr__l_exp_resp_msg_typ;
      t_bool service_mgr__l_bres;
      constants__t_session_i service_mgr__l_session;
      
      request_handle_bs__is_valid_req_handle(service_mgr__request_handle,
         &service_mgr__l_valid_req_handle);
      request_handle_bs__get_req_handle_channel(service_mgr__request_handle,
         &service_mgr__l_req_handle_channel);
      if ((service_mgr__l_valid_req_handle == true) &&
         (service_mgr__l_req_handle_channel == service_mgr__channel)) {
         request_handle_bs__get_req_handle_resp_typ(service_mgr__request_handle,
            &service_mgr__l_exp_resp_msg_typ);
         switch (service_mgr__l_exp_resp_msg_typ) {
         case constants__e_msg_session_create_resp:
         case constants__e_msg_session_activate_resp:
         case constants__e_msg_session_close_resp:
         case constants__e_msg_session_cancel_resp:
            session_mgr__client_validate_session_service_resp(service_mgr__channel,
               service_mgr__request_handle,
               &service_mgr__l_bres,
               &service_mgr__l_session);
            if (service_mgr__l_bres == true) {
               session_mgr__client_close_session(service_mgr__l_session,
                  constants_statuscodes_bs__e_sc_bad_request_interrupted);
            }
            break;
         default:
            request_handle_bs__get_req_handle_app_context(service_mgr__request_handle,
               &service_mgr__l_is_applicative,
               &service_mgr__l_app_context);
            request_handle_bs__get_req_handle_req_typ(service_mgr__request_handle,
               &service_mgr__l_req_typ);
            request_handle_bs__client_remove_req_handle(service_mgr__request_handle);
            if (service_mgr__l_is_applicative == true) {
               service_response_cb_bs__cli_snd_failure(service_mgr__l_req_typ,
                  service_mgr__l_app_context,
                  service_mgr__error_status);
            }
            break;
         }
      }
   }
}

void service_mgr__server_send_publish_response(
   const constants__t_session_i service_mgr__session,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants_statuscodes_bs__t_StatusCode_i service_mgr__statusCode,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_msg_i service_mgr__publish_resp_msg,
   t_bool * const service_mgr__bres,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_channel_i * const service_mgr__channel) {
   {
      t_bool service_mgr__l_is_valid_resp;
      t_bool service_mgr__l_is_valid_header;
      constants_statuscodes_bs__t_StatusCode_i service_mgr__l_ret;
      constants__t_msg_header_i service_mgr__l_resp_msg_header;
      constants__t_byte_buffer_i service_mgr__l_buffer_out;
      t_bool service_mgr__l_valid_buffer;
      constants__t_channel_config_idx_i service_mgr__l_channel_cfg;
      
      *service_mgr__sc = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
      *service_mgr__bres = false;
      *service_mgr__buffer_out = constants__c_byte_buffer_indet;
      *service_mgr__channel = constants__c_channel_indet;
      message_out_bs__bless_msg_out(service_mgr__publish_resp_msg);
      session_mgr__server_validate_session_service_resp(service_mgr__session,
         &service_mgr__l_is_valid_resp,
         &service_mgr__l_ret,
         service_mgr__channel);
      if (service_mgr__l_is_valid_resp == true) {
         message_out_bs__alloc_msg_header(false,
            &service_mgr__l_resp_msg_header);
         message_out_bs__is_valid_msg_out_header(service_mgr__l_resp_msg_header,
            &service_mgr__l_is_valid_header);
         if (service_mgr__l_is_valid_header == true) {
            message_out_bs__server_write_msg_out_header_req_handle(service_mgr__l_resp_msg_header,
               service_mgr__req_handle);
            message_out_bs__write_msg_resp_header_service_status(service_mgr__l_resp_msg_header,
               service_mgr__statusCode);
            channel_mgr__get_channel_info(*service_mgr__channel,
               &service_mgr__l_channel_cfg);
            message_out_bs__encode_msg(service_mgr__l_channel_cfg,
               constants__e_msg_response_type,
               service_mgr__resp_typ,
               service_mgr__l_resp_msg_header,
               service_mgr__publish_resp_msg,
               service_mgr__sc,
               &service_mgr__l_buffer_out);
            message_out_bs__dealloc_msg_header_out(service_mgr__l_resp_msg_header);
            message_out_bs__is_valid_buffer_out(service_mgr__l_buffer_out,
               &service_mgr__l_valid_buffer);
            if (service_mgr__l_valid_buffer == true) {
               *service_mgr__buffer_out = service_mgr__l_buffer_out;
               *service_mgr__bres = true;
            }
         }
         else {
            *service_mgr__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      message_out_bs__dealloc_msg_out(service_mgr__publish_resp_msg);
   }
}

void service_mgr__internal_server_inactive_session_prio_event(
   const constants__t_session_i service_mgr__p_session,
   const constants__t_sessionState service_mgr__p_newSessionState,
   t_bool * const service_mgr__bres) {
   if (service_mgr__p_session != constants__c_session_indet) {
      *service_mgr__bres = true;
      subscription_mgr__server_subscription_session_inactive(service_mgr__p_session,
         service_mgr__p_newSessionState);
      if (service_mgr__p_newSessionState == constants__e_session_closed) {
         service_set_view__service_set_view_set_session_closed(service_mgr__p_session);
      }
   }
   else {
      *service_mgr__bres = false;
   }
}

void service_mgr__service_mgr_UNINITIALISATION(void) {
   subscription_mgr__subscription_mgr_UNINITIALISATION();
   service_set_view__service_set_view_UNINITIALISATION();
   service_set_discovery_server__service_set_discovery_server_UNINITIALISATION();
   service_mgr_bs__service_mgr_bs_UNINITIALISATION();
}

