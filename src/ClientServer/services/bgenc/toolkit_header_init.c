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

 File Name            : toolkit_header_init.c

 Date                 : 05/08/2022 08:41:25

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "toolkit_header_init.h"

/*---------------------
   List of Components
  ---------------------*/
#include "address_space.h"
#include "address_space_bs.h"
#include "address_space_itf.h"
#include "address_space_local.h"
#include "address_space_typing.h"
#include "address_space_typing_bs.h"
#include "app_cb_call_context_bs.h"
#include "argument_pointer_bs.h"
#include "browse_treatment.h"
#include "browse_treatment_1.h"
#include "browse_treatment_context.h"
#include "browse_treatment_context_bs.h"
#include "browse_treatment_continuation_points.h"
#include "browse_treatment_continuation_points_bs.h"
#include "browse_treatment_continuation_points_session_it.h"
#include "browse_treatment_result_bs.h"
#include "browse_treatment_result_it.h"
#include "browse_treatment_target_it.h"
#include "call_method_bs.h"
#include "call_method_it.h"
#include "call_method_mgr.h"
#include "call_method_result_it.h"
#include "channel_mgr.h"
#include "channel_mgr_1.h"
#include "channel_mgr_bs.h"
#include "channel_mgr_it.h"
#include "constants.h"
#include "constants_bs.h"
#include "constants_statuscodes_bs.h"
#include "data_value_pointer_bs.h"
#include "gen_subscription_event_bs.h"
#include "io_dispatch_mgr.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "monitored_item_notification_queue_bs.h"
#include "monitored_item_pointer_bs.h"
#include "monitored_item_queue_bs.h"
#include "monitored_item_queue_it_bs.h"
#include "msg_browse_bs.h"
#include "msg_browse_next_bs.h"
#include "msg_call_method_bs.h"
#include "msg_find_servers_bs.h"
#include "msg_find_servers_on_network_bs.h"
#include "msg_read_request.h"
#include "msg_read_request_bs.h"
#include "msg_read_response_bs.h"
#include "msg_register_nodes.h"
#include "msg_register_nodes_bs.h"
#include "msg_register_server2.h"
#include "msg_register_server2_bs.h"
#include "msg_session_bs.h"
#include "msg_subscription_create_bs.h"
#include "msg_subscription_create_monitored_item.h"
#include "msg_subscription_create_monitored_item_bs.h"
#include "msg_subscription_delete_bs.h"
#include "msg_subscription_publish_ack_bs.h"
#include "msg_subscription_publish_bs.h"
#include "msg_subscription_set_publishing_mode_bs.h"
#include "msg_translate_browse_path_bs.h"
#include "msg_unregister_nodes.h"
#include "msg_unregister_nodes_bs.h"
#include "node_id_pointer_bs.h"
#include "notification_republish_queue_bs.h"
#include "notification_republish_queue_it_bs.h"
#include "publish_request_queue_bs.h"
#include "register_nodes_it.h"
#include "request_handle_bs.h"
#include "response_write_bs.h"
#include "service_browse_it.h"
#include "service_get_endpoints_bs.h"
#include "service_mgr.h"
#include "service_mgr_bs.h"
#include "service_read.h"
#include "service_read_1.h"
#include "service_read_it.h"
#include "service_register_nodes.h"
#include "service_register_server2.h"
#include "service_register_server2_set_bs.h"
#include "service_response_cb_bs.h"
#include "service_set_discovery_server.h"
#include "service_set_discovery_server_data_bs.h"
#include "service_set_view.h"
#include "service_unregister_nodes.h"
#include "service_write.h"
#include "service_write_1.h"
#include "service_write_1_it.h"
#include "service_write_decode_bs.h"
#include "session_channel_it.h"
#include "session_core.h"
#include "session_core_1.h"
#include "session_core_1_it.h"
#include "session_core_2.h"
#include "session_core_bs.h"
#include "session_core_it.h"
#include "session_mgr.h"
#include "session_mgr_it.h"
#include "session_request_handle_bs.h"
#include "subscription_absent_node.h"
#include "subscription_absent_node_bs.h"
#include "subscription_core.h"
#include "subscription_core_1.h"
#include "subscription_core_bs.h"
#include "subscription_core_it.h"
#include "subscription_create_monitored_item_it.h"
#include "subscription_mgr.h"
#include "time_reference_bs.h"
#include "toolkit_header.h"
#include "translate_browse_path.h"
#include "translate_browse_path_1.h"
#include "translate_browse_path_element_it.h"
#include "translate_browse_path_it.h"
#include "translate_browse_path_result_it.h"
#include "translate_browse_path_source_it.h"
#include "unregister_nodes_it.h"
#include "user_authentication.h"
#include "user_authentication_bs.h"
#include "user_authorization_bs.h"
#include "write_value_pointer_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void INITIALISATION(void) {
   constants_bs__INITIALISATION();
   constants__INITIALISATION();
   constants_statuscodes_bs__INITIALISATION();
   message_in_bs__INITIALISATION();
   request_handle_bs__INITIALISATION();
   message_out_bs__INITIALISATION();
   channel_mgr_1__INITIALISATION();
   channel_mgr_it__INITIALISATION();
   channel_mgr_bs__INITIALISATION();
   time_reference_bs__INITIALISATION();
   channel_mgr__INITIALISATION();
   service_mgr_bs__INITIALISATION();
   session_core_1_it__INITIALISATION();
   session_core_2__INITIALISATION();
   session_core_bs__INITIALISATION();
   session_core_1__INITIALISATION();
   session_core_it__INITIALISATION();
   session_channel_it__INITIALISATION();
   msg_session_bs__INITIALISATION();
   user_authentication_bs__INITIALISATION();
   user_authentication__INITIALISATION();
   session_core__INITIALISATION();
   session_mgr_it__INITIALISATION();
   session_request_handle_bs__INITIALISATION();
   app_cb_call_context_bs__INITIALISATION();
   session_mgr__INITIALISATION();
   msg_read_request_bs__INITIALISATION();
   msg_read_request__INITIALISATION();
   msg_read_response_bs__INITIALISATION();
   service_read_it__INITIALISATION();
   user_authorization_bs__INITIALISATION();
   data_value_pointer_bs__INITIALISATION();
   address_space_bs__INITIALISATION();
   address_space_typing_bs__INITIALISATION();
   address_space_typing__INITIALISATION();
   address_space_local__INITIALISATION();
   gen_subscription_event_bs__INITIALISATION();
   service_write_decode_bs__INITIALISATION();
   service_response_cb_bs__INITIALISATION();
   write_value_pointer_bs__INITIALISATION();
   address_space__INITIALISATION();
   service_write_1_it__INITIALISATION();
   response_write_bs__INITIALISATION();
   service_write_1__INITIALISATION();
   service_write__INITIALISATION();
   address_space_itf__INITIALISATION();
   service_read_1__INITIALISATION();
   service_read__INITIALISATION();
   service_get_endpoints_bs__INITIALISATION();
   service_browse_it__INITIALISATION();
   msg_browse_bs__INITIALISATION();
   msg_browse_next_bs__INITIALISATION();
   translate_browse_path_1__INITIALISATION();
   msg_translate_browse_path_bs__INITIALISATION();
   translate_browse_path_element_it__INITIALISATION();
   translate_browse_path_source_it__INITIALISATION();
   translate_browse_path_result_it__INITIALISATION();
   browse_treatment_context_bs__INITIALISATION();
   node_id_pointer_bs__INITIALISATION();
   browse_treatment_context__INITIALISATION();
   browse_treatment_continuation_points_bs__INITIALISATION();
   browse_treatment_continuation_points_session_it__INITIALISATION();
   browse_treatment_continuation_points__INITIALISATION();
   browse_treatment_result_bs__INITIALISATION();
   browse_treatment_1__INITIALISATION();
   browse_treatment_target_it__INITIALISATION();
   browse_treatment_result_it__INITIALISATION();
   browse_treatment__INITIALISATION();
   translate_browse_path__INITIALISATION();
   translate_browse_path_it__INITIALISATION();
   service_set_view__INITIALISATION();
   subscription_core_1__INITIALISATION();
   monitored_item_queue_bs__INITIALISATION();
   subscription_core_bs__INITIALISATION();
   subscription_core_it__INITIALISATION();
   monitored_item_notification_queue_bs__INITIALISATION();
   monitored_item_pointer_bs__INITIALISATION();
   publish_request_queue_bs__INITIALISATION();
   notification_republish_queue_bs__INITIALISATION();
   msg_subscription_publish_bs__INITIALISATION();
   notification_republish_queue_it_bs__INITIALISATION();
   monitored_item_queue_it_bs__INITIALISATION();
   subscription_core__INITIALISATION();
   msg_subscription_create_bs__INITIALISATION();
   msg_subscription_delete_bs__INITIALISATION();
   msg_subscription_publish_ack_bs__INITIALISATION();
   msg_subscription_set_publishing_mode_bs__INITIALISATION();
   msg_subscription_create_monitored_item_bs__INITIALISATION();
   msg_subscription_create_monitored_item__INITIALISATION();
   subscription_create_monitored_item_it__INITIALISATION();
   subscription_absent_node_bs__INITIALISATION();
   subscription_absent_node__INITIALISATION();
   subscription_mgr__INITIALISATION();
   msg_register_server2_bs__INITIALISATION();
   msg_register_server2__INITIALISATION();
   service_register_server2_set_bs__INITIALISATION();
   service_set_discovery_server_data_bs__INITIALISATION();
   service_register_server2__INITIALISATION();
   msg_find_servers_on_network_bs__INITIALISATION();
   msg_find_servers_bs__INITIALISATION();
   service_set_discovery_server__INITIALISATION();
   msg_register_nodes_bs__INITIALISATION();
   msg_register_nodes__INITIALISATION();
   register_nodes_it__INITIALISATION();
   service_register_nodes__INITIALISATION();
   msg_unregister_nodes_bs__INITIALISATION();
   msg_unregister_nodes__INITIALISATION();
   unregister_nodes_it__INITIALISATION();
   service_unregister_nodes__INITIALISATION();
   call_method_bs__INITIALISATION();
   msg_call_method_bs__INITIALISATION();
   call_method_it__INITIALISATION();
   call_method_result_it__INITIALISATION();
   argument_pointer_bs__INITIALISATION();
   call_method_mgr__INITIALISATION();
   service_mgr__INITIALISATION();
   io_dispatch_mgr__INITIALISATION();
   toolkit_header__INITIALISATION();
}

